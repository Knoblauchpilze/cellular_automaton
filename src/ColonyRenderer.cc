
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
    m_generationComputedSignalID(utils::Signal<unsigned>::NoID),

    m_lastKnownMousePos(),

    m_palette(std::make_shared<ColorPalette>()),

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
                                         const utils::Vector2f& whereTo,
                                         const utils::Vector2f& motion,
                                         bool /*notify*/)
  {
    // We want to apply a motion of `motion` in local coordinate frame to the
    // underlying support area. In order to do that we need to convert the
    // motion into a real world coordinate frame.
    Guard guard(m_propsLocker);


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

    log("Moving from " + m_settings.area.toString() + " to " + newArea.toString() + " (motion: " + motion.toString() + ", real: " + realWorldMotion.toString() + ")", utils::Level::Verbose);

    // Update the rendering area.
    m_settings.area = newArea;

    // Update the position and age of the cell pointed at by the mouse.
    if (isMouseInside()) {
      notifyCoordinatePointedTo(whereTo, true);
    }

    // Request a repaint operation.
    setColonyChanged();

    // Notify the caller that we changed the area.
    return true;
  }

  bool
  ColonyRenderer::keyPressEvent(const sdl::core::engine::KeyEvent& e) {
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
        Guard guard(m_propsLocker);

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
    Guard guard(m_propsLocker);

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
    Guard guard(m_propsLocker);

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

    // check consistency.
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
    Guard guard(m_propsLocker);

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
            co = m_palette->colorize(ce.second);
            break;
          case State::Dead:
          default:
            co = sdl::core::engine::Color::NamedColor::Black;
            break;
        }

        // Invert the `y` axis because the surface that will be created from the raw pixels
        // will be assumed to represent a top down image.
        int off = (iEnv.h() - 1 - y) * iEnv.w() + x;
        colors[off] = co;
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

}
