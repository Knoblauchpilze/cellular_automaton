
# include "ColonyRenderer.hh"
# include <sdl_engine/PaintEvent.hh>
# include <sdl_engine/Color.hh>

namespace cellulator {

  ColonyRenderer::ColonyRenderer(ColonyShPtr colony,
                                 const utils::Sizef& hint,
                                 sdl::core::SdlWidget* parent):
    sdl::graphic::ScrollableWidget(std::string("colony_renderer"),
                                   parent,
                                   hint),

    m_propsLocker(),

    m_tex(),
    m_settings(),
    m_colonyRendered(true),

    m_scheduler(std::make_shared<ColonyScheduler>(colony)),
    m_colony(colony),
    m_generationComputedSignalID(utils::Signal<unsigned>::NO_ID),

    m_lastKnownMousePos(),

    m_display(Display{
      sdl::core::engine::Color::NamedColor::Black,
      std::make_shared<ColorPalette>(),

      false,
      sdl::core::engine::Color::NamedColor::White,
      utils::Vector2i(1, 1),

      false,
      sdl::core::engine::Color::NamedColor::Yellow,
      sdl::core::engine::Color::NamedColor::Gray,
      0.5f,
      0.25f,
      std::make_shared<CellBrush>(utils::Sizei(1, 1), State::Alive)
    }),

    onGenerationComputed(),
    onCoordChanged()
  {
    setService(std::string("colony_renderer"));

    // Check consistency.
    if (m_colony == nullptr) {
      error(
        std::string("Could not create renderer"),
        std::string("Invalid null colony")
      );
    }
    if (m_scheduler == nullptr) {
      error(
        std::string("Could not create renderer"),
        std::string("Invalid null colony scheduler")
      );
    }

    build();
  }

  bool
  ColonyRenderer::handleContentScrolling(const utils::Vector2f& /*posToFix*/,
                                         const utils::Vector2f& /*whereTo*/,
                                         const utils::Vector2f& motion,
                                         bool /*notify*/)
  {
    // We want to apply a motion of `motion` in local coordinate frame to the
    // underlying support area. In order to do that we need to convert the
    // motion into a real world coordinate frame.
    const std::lock_guard guard(m_propsLocker);

    // Note: we need to invert the motion's direction for some reasons.
    utils::Sizef cellsDims = getCellsDims();
    utils::Vector2f realWorldMotion(
      -motion.x() / cellsDims.w(),
      -motion.y() / cellsDims.h()
    );

    // Compute the new rendering area by offseting the old one with the motion.
    utils::Boxf newArea(
      m_settings.area.x() + realWorldMotion.x(),
      m_settings.area.y() + realWorldMotion.y(),
      m_settings.area.toSize()
    );

    verbose("Moving from " + m_settings.area.toString() + " to " + newArea.toString() + " (motion: " + motion.toString() + ", real: " + realWorldMotion.toString() + ")");

    // Update the rendering area. We don't need to update the grid resolution as
    // we're not yet handling scrolling and zooming at the same time. So the old
    // resolution should be just fine with the new area which is just translated
    // from the old one.
    m_settings.area = newArea;

    // Update the position and age of the cell pointed at by the mouse.
    if (isMouseInside()) {
      notifyCoordinatePointedTo(m_lastKnownMousePos, true);
    }

    // Request a repaint operation.
    setColonyChanged();

    // Notify the caller that we changed the area.
    return true;
  }

  bool
  ColonyRenderer::keyPressEvent(const sdl::core::engine::KeyEvent& e) {
    // Check for simulation's state toggling key.
    if (e.getRawKey() == getSimulationStateToggleKey()) {
      // Toggle the scheduler's state.
      m_scheduler->toggle();

      return sdl::graphic::ScrollableWidget::keyPressEvent(e);
    }

    // Check for arrow keys.
    bool move = false;
    utils::Vector2f motion;
    float delta = getArrowKeyMotion();

    if (e.getRawKey() == sdl::core::engine::RawKey::Left) {
      move = true;
      motion.x() += delta;
    }
    if (e.getRawKey() == sdl::core::engine::RawKey::Right) {
      move = true;
      motion.x() -= delta;
    }
    if (e.getRawKey() == sdl::core::engine::RawKey::Down) {
      move = true;
      motion.y() += delta;
    }
    if (e.getRawKey() == sdl::core::engine::RawKey::Up) {
      move = true;
      motion.y() -= delta;
    }

    // Schedule a scrolling if some motion has been detected.
    if (move) {
      {
        const std::lock_guard guard(m_propsLocker);

        // Convert the motion (which is right now expressed in terms of pixel(s))
        // in a cell coordinates motion. This can be done by computing the size
        // of a single cell.
        utils::Sizef cellsDims = getCellsDims();

        motion.x() /= cellsDims.w();
        motion.y() /= cellsDims.h();
      }

      if (handleContentScrolling(m_settings.area.getCenter(), m_lastKnownMousePos, motion, false)) {
        requestRepaint();
      }
    }

    // Use the base handler to provide a return value.
    return sdl::graphic::ScrollableWidget::keyPressEvent(e);
  }

  bool
  ColonyRenderer::keyReleaseEvent(const sdl::core::engine::KeyEvent& e) {
    // Check for toggle brush overlay.
    if (e.getRawKey() == getToggleBrushOverlayKey()) {
      const std::lock_guard guard(m_propsLocker);

      m_display.bDisplay = !m_display.bDisplay;

      // Request a repaint: not that we can save some time if the display
      // does not include any active brush: in this case it is obvious that
      // the overlay will not change a thing.
      if (m_display.brush != nullptr) {
        setColonyChanged();
      }
    }

    return sdl::graphic::ScrollableWidget::keyReleaseEvent(e);
  }

  bool
  ColonyRenderer::mouseButtonReleaseEvent(const sdl::core::engine::MouseEvent& e) {
    // Detect whether the event concerns the button to use to perform
    // the paint of the active brush.
    if (e.getButton() == getBrushPaintButton()) {
      const std::lock_guard guard(m_propsLocker);

      paintBrush();

      // Schedule a repaint.
      setColonyChanged();
    }

    return sdl::graphic::ScrollableWidget::mouseButtonReleaseEvent(e);
  }

  bool
  ColonyRenderer::mouseWheelEvent(const sdl::core::engine::MouseEvent& e) {
    // We want to trigger zooming operations only when the mouse is inside
    // this widget.
    bool toReturn = sdl::graphic::ScrollableWidget::mouseWheelEvent(e);

    if (!isMouseInside()) {
      return toReturn;
    }

    // Protect from concurrent accesses to perform the zoom operation and
    // also request a repaint.
    utils::Vector2i motion = e.getScroll();

    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // Perform the zoom in operation.
    float factor = motion.y() > 0 ? getDefaultZoomInFactor() : getDefaultZoomOutFactor();

    utils::Vector2f conv = convertPosToRealWorld(e.getMousePosition(), true);

    zoom(conv, factor);

    // Set the colony as dirty.
    setColonyChanged();

    return toReturn;
  }

  void
  ColonyRenderer::drawContentPrivate(const utils::Uuid& uuid,
                                     const utils::Boxf& area)
  {
    // Acquire the lock on the attributes of this widget.
    const std::lock_guard guard(m_propsLocker);

    // Load the colony: this should happen only if some cells have been
    // rendered since the last draw operation. This status is kept by
    // the `m_colonyRendered` boolean. Generally this indicates that a
    // compute operation is running and that some more tiles have been
    // received.
    if (colonyChanged()) {
      // Load the colony.
      loadColony();

      // The colony has been updated.
      m_colonyRendered = false;
    }

    // Check whether there's something to display.
    if (!m_tex.valid()) {
      return;
    }

    // Convert the area to local so that we blit only the right area of
    // the texture representing the fractal.
    utils::Boxf thisArea = LayoutItem::getRenderingArea().toOrigin();
    utils::Sizef sizeEnv = getEngine().queryTexture(uuid);
    utils::Sizef texSize = getEngine().queryTexture(m_tex);

    utils::Boxf srcArea = thisArea.intersect(area);
    utils::Boxf dstArea = thisArea.intersect(area);

    utils::Boxf srcEngine = convertToEngineFormat(srcArea, texSize);
    utils::Boxf dstEngine = convertToEngineFormat(dstArea, sizeEnv);

    getEngine().drawTexture(m_tex, &srcEngine, &uuid, &dstEngine);
  }

  void
  ColonyRenderer::build() {
    // Connect to the handler signal.
    m_generationComputedSignalID = m_scheduler->onGenerationComputed.connect_member<ColonyRenderer>(
      this,
      &ColonyRenderer::handleGenerationComputed
    );

    // Assign the rendering window: by default we consider that the whole area is visible.
    m_settings.area = getDefaultRenderingArea();

    updateGridResolution();
  }

  void
  ColonyRenderer::loadColony() {
    // Clear any existing texture representing the colony.
    clearColony();

    // Create the brush representing the tile using the palette
    // provided by the user.
    // To do so we need to fetch the cells from the colony that
    // represent the visible area. Then we need to create a brush
    // from the cells with the specified dimensions and finally
    // a texture from the brush.

    // Fetch the cells that are visible. We need to convert the
    // input area.
    std::vector<std::pair<State, unsigned>> cells;
    utils::Boxi out = m_colony->fetchCells(cells, m_settings.area);

    // Create the colors needed for the brush.
    sdl::core::engine::BrushShPtr brush = createBrushFromCells(cells, out);

    // Check consistency.
    if (brush == nullptr) {
      error(
        std::string("Could not create texture to represent colony"),
        std::string("Failed to create brush data")
      );
    }

    // Use the brush to create a texture.
    m_tex = getEngine().createTextureFromBrush(brush);

    if (!m_tex.valid()) {
      error(
        std::string("Could not create texture to represent colony"),
        std::string("Failed to transform brush into texture")
      );
    }
  }

  void
  ColonyRenderer::handleGenerationComputed(unsigned generation,
                                           unsigned liveCells)
  {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // The colony need to be rendered again.
    setColonyChanged();

    // Notify listeners.
    onGenerationComputed.safeEmit(
      std::string("onGenerationComputed(") + std::to_string(generation) + ")",
      generation
    );

    onAliveCellsChanged.safeEmit(
      std::string("onAliveCellsChanged(") + std::to_string(liveCells) + ")",
      liveCells
    );
  }

  sdl::core::engine::BrushShPtr
  ColonyRenderer::createBrushFromCells(const std::vector<std::pair<State, unsigned>>& cells,
                                       const utils::Boxi& area)
  {
    // Determine the size of a single cell.
    utils::Sizef env = LayoutItem::getRenderingArea().toSize();
    utils::Sizef cellsDims = getCellsDims();
    utils::Sizei iEnv(
      static_cast<int>(std::floor(env.w())),
      static_cast<int>(std::floor(env.h()))
    );

    // Create a canvas of the expected size.
    std::vector<sdl::core::engine::Color> colors(
      iEnv.area(),
      sdl::core::engine::Color::NamedColor::Black
    );

    // Fill in each cell with the corresponding color.
    for (int y = 0 ; y < iEnv.h() ; ++y) {
      for (int x = 0 ; x < iEnv.w() ; ++x) {
        // Compute the float cell coordinate for this pixel.
        float rX = x / cellsDims.w();
        float rY = y / cellsDims.h();

        // Invert the `y` axis because the surface that will be created from the
        // raw pixels will be assumed to represent a top down image.
        int off = (iEnv.h() - 1 - y) * iEnv.w() + x;

        // Compute the cell coordinate from the floating point coords.
        int cX = static_cast<int>(std::floor(m_settings.area.getLeftBound() + rX));
        int cY = static_cast<int>(std::floor(m_settings.area.getBottomBound() + rY));

        // Transform this using the provided area.
        utils::Vector2i c(cX - area.getLeftBound(), cY - area.getBottomBound());

        // Retrieve the state of this cell.
        const std::pair<State, unsigned>& ce = cells[c.y() * area.w() + c.x()];

        // Assign the corresponding color to the pixel.
        sdl::core::engine::Color co = sdl::core::engine::Color::NamedColor::Pink;
        switch (ce.first) {
          case State::Alive:
            co = m_display.cells->colorize(ce.second);
            break;
          case State::Dead:
          default:
            co = m_display.bgColor;
            break;
        }

        // Register the color.
        colors[off] = co;
      }
    }

    // Displayt grid if needed.
    if (m_display.gDisplay) {
      // We need to overlay the grid based on the current desired resolution. This
      // include traversing the area and draw each line.

      // Compute integer coordinate of the left and right bound of the rendering
      // area: this will help determining the lines that we should display.
      int sX = static_cast<int>(std::floor(m_settings.area.getLeftBound()));
      int eX = static_cast<int>(std::ceil(m_settings.area.getRightBound()));

      int sY = static_cast<int>(std::floor(m_settings.area.getBottomBound()));
      int eY = static_cast<int>(std::ceil(m_settings.area.getTopBound()));

      // Account for the resolution: we only want to display grid lines for some
      // integer coordinates but not all.
      utils::Vector2i res = m_display.gRes;
      utils::Boxf a = m_settings.area;

      int xMin = sX - sX % res.x();
      int yMin = sY - sY % res.y();

      int xMax = eX + (res.x() - eX % res.x()) % res.x();
      int yMax = eY + (res.y() - eY % res.y()) % res.y();

      // Iterate over the lines to display.
      for (int x = xMin ; x <= xMax ; x += res.x()) {
        // Discard elements which are not visible in the rendering area.
        if (x < sX || x > eX) {
          continue;
        }

        // Compute the closest pixel coordinate for this line.
        float fpix = 1.0f * (x - a.getLeftBound()) / a.w() * iEnv.w();
        int pix = static_cast<int>(std::round(fpix));

        // Check whether this pixel is within the admissible range.
        if (pix < 0 || pix >= iEnv.w()) {
          continue;
        }

        // Fill this line (along the `y` axis) with grid color. Just like
        // for cells rendering we need to flip the `y` axis because the
        // underlying API expects a top-down image.
        for (int y = 0 ; y < iEnv.h() ; ++y) {
          colors[(iEnv.h() - 1 - y) * iEnv.w() + pix] = m_display.gColor;
        }
      }

      for (int y = yMin ; y <= yMax ; y += res.y()) {
        // Discard elements which are not visible in the rendering area.
        if (y < sY || y > eY) {
          continue;
        }

        // Compute the closest pixel coordinate for this line.
        float fpix = 1.0f * (y - a.getBottomBound()) / a.h() * iEnv.h();
        int pix = static_cast<int>(std::round(fpix));

        // Check whether this pixel is within the admissible range.
        if (pix < 0 || pix >= iEnv.h()) {
          continue;
        }

        // Fill this line (along the `x` axis) with grid color. Just like
        // for cells rendering we need to flip the `y` axis because the
        // underlying API expects a top-down image.
        for (int x = 0 ; x < iEnv.w() ; ++x) {
          colors[(iEnv.h() - 1 - pix) * iEnv.w() + x] = m_display.gColor;
        }
      }
    }

    // Display brush overlay if needed.
    if (m_display.bDisplay && m_display.brush != nullptr && m_display.brush->valid()) {
      CellBrush& b = *m_display.brush;
      utils::Sizei size = b.getSize();

      // Compute the coordinate of the mouse in cell's reference frame.
      utils::Vector2f mCoords = convertPosToRealWorld(m_lastKnownMousePos, true);

      // Round it to obtain integer coordinates.
      utils::Vector2i mICoords(
        static_cast<int>(std::floor(mCoords.x())),
        static_cast<int>(std::floor(mCoords.y()))
      );

      // Compute the extremum reached by the brush. In case the size of the brush
      // is not even in any dimensions we need to offset the top right corner so
      // that it accounts for this otherwise there is a risk that we won't display
      // all the brush' content.
      utils::Vector2i bBL(mICoords.x() - size.w() / 2, mICoords.y() - size.h() / 2);
      utils::Vector2i bTR(
        mICoords.x() + (size.w() + size.w() % 2) / 2,
        mICoords.y() + (size.h() + size.h() % 2) / 2
      );

      // Compute the pixels related to the area covered by the brush.
      utils::Vector2i areaLocalBL(
        static_cast<int>(std::floor((bBL.x() - m_settings.area.getLeftBound()) * cellsDims.w())),
        static_cast<int>(std::floor((bBL.y() - m_settings.area.getBottomBound()) * cellsDims.h()))
      );
      utils::Vector2i areaLocalTR(
        static_cast<int>(std::floor((bTR.x() - m_settings.area.getLeftBound()) * cellsDims.w())),
        static_cast<int>(std::floor((bTR.y() - m_settings.area.getBottomBound()) * cellsDims.h()))
      );

      // Traverse all the pixels covered by the brush.
      int pixW = areaLocalTR.x() - areaLocalBL.x();
      int pixH = areaLocalTR.y() - areaLocalBL.y();

      for (int y = 0 ; y < pixH ; ++y) {
        int gY = y + areaLocalBL.y() + 1;

        // Discard elements outside of the canvas.
        if (gY < 0 || gY >= iEnv.h()) {
          continue;
        }

        for (int x = 0 ; x < pixW ; ++x) {
          int gX = x + areaLocalBL.x() + 1;

          // Discard elements which do not fit in the general canvas.
          if (gX < 0 || gX >= iEnv.w()) {
            continue;
          }

          // Compute the cell related to this pixel coordinate.
          int cX = static_cast<int>(std::floor(x / cellsDims.w()));
          int cY = static_cast<int>(std::floor(y / cellsDims.h()));

          // Invert the `y` axis because the surface that will be created from the
          // raw pixels will be assumed to represent a top down image.
          int off = (iEnv.h() - 1 - gY) * iEnv.w() + gX;

          // Fetch the state of the cell in the brush and assign the overlay color
          // if the cell is alive.
          sdl::core::engine::Color c = m_display.bAColor;
          float bl = m_display.bABlend;

          if (b.getStateAt(cX, cY) == State::Dead) {
            c = m_display.bDColor;
            bl = m_display.bDBlend;
          }

          colors[off] = colors[off].blend(c, bl);
        }
      }
    }

    // Create the brush and return it.
    sdl::core::engine::BrushShPtr brush = std::make_shared<sdl::core::engine::Brush>(
      std::string("brush_for_") + getName(),
      false
    );

    brush->createFromRaw(iEnv, colors);

    return brush;
  }

  void
  ColonyRenderer::paintBrush() {
    // Check whether we have an active brush: if this is not the case there's
    // nothing to paint so we can return early.
    if (m_display.brush == nullptr || !m_display.brush->valid()) {
      return;
    }

    // Convert the mouse position into a cell's coordinate frame.
    utils::Vector2f fCell = convertPosToRealWorld(m_lastKnownMousePos, true);

    // Convert this into integer coordinates.
    utils::Vector2i cell(
      static_cast<int>(std::floor(fCell.x())),
      static_cast<int>(std::floor(fCell.y()))
    );

    // Paint the brush at this coordinates.
    unsigned liveCells = m_scheduler->paint(*m_display.brush, cell);

    // Notify of the new live cells count.
    onAliveCellsChanged.safeEmit(
      std::string("onAliveCellsChanged(") + std::to_string(liveCells) + ")",
      liveCells
    );
  }

}
