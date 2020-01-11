#ifndef    COLONY_RENDERER_HXX
# define   COLONY_RENDERER_HXX

# include "ColonyRenderer.hh"

namespace cellulator {

  inline
  ColonyRenderer::~ColonyRenderer() {
    m_scheduler.reset();

    // Protect from concurrent accesses
    Guard guard(m_propsLocker);

    clearColony();
  }

  inline
  void
  ColonyRenderer::fitToContent(const std::string& /*dummy*/) {
    // Protect from concurrent accesses
    Guard guard(m_propsLocker);

    // We want the entire colony to fit inside the screen. We also want to
    // keep the best aspect ratio given the colony's size.
    utils::Sizef env = LayoutItem::getRenderingArea().toSize();
    utils::Boxf cArea = m_colony->getArea();

    // Determine the dimension of a cell in pixel's coordinate frame for
    // each axis.
    float cW = env.w() / cArea.w();
    float cH = env.h() / cArea.h();

    // We will use the smallest of the two values to scale the colony: this
    // will guarantee that the smallest dimensions get the best out of the
    // available space and the second one still gets not too stretched.
    float ratio = std::min(cW, cH);
    utils::Boxf area(cArea.x(), cArea.y(), env.w() / ratio, env.h() / ratio);

    log(
      "Changing rendering area from " + m_settings.area.toString() + " to " + area.toString() + " (colony is " +
      cArea.toSize().toString() + ")",
      utils::Level::Verbose
    );

    // Assign the new rendering area.
    m_settings.area = area;

    // Update the grid resolution.
    updateGridResolution();

    // Request a repaint.
    setColonyChanged();
  }

  inline
  void
  ColonyRenderer::start() {
    m_scheduler->start();
  }

  inline
  void
  ColonyRenderer::stop() {
    m_scheduler->stop();
  }

  inline
  void
  ColonyRenderer::nextStep() {
    m_scheduler->step();
  }

  inline
  void
  ColonyRenderer::generate(const std::string& /*dummy*/) {
    // Generate random cells in the colony.
    m_scheduler->generate();

    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Indicate that the colony changed so that we can repaint it.
    setColonyChanged();
  }

  inline
  ColonySchedulerShPtr
  ColonyRenderer::getScheduler() noexcept {
    return m_scheduler;
  }

  inline
  void
  ColonyRenderer::onPaletteChanded(ColorPaletteShPtr palette) {
    // Discard invalid palette.
    if (palette == nullptr) {
      log(
        std::string("Discarding palette operation with invalid null palette"),
        utils::Level::Error
      );

      return;
    }

    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Assign the palette.
    m_display.cells = palette;

    // Request a repaint in order to display the colony with its new color.
    setColonyChanged();
  }

  inline
  void
  ColonyRenderer::onGridDisplayToggled(bool toggled) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Update the grid display status and request a repaint if needed.
    bool changed = toggled != m_display.grid;
    m_display.grid = toggled;

    if (changed) {
      setColonyChanged();
    }
  }

  inline
  void
  ColonyRenderer::onBrushChanged(CellBrushShPtr brush) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Assign the new brush.
    m_display.brush = brush;

    if (m_display.brush == nullptr) {
      log("No more active brush");
    }
    else {
      log("New active brush is \"" + brush->getName() + "\"");
    }

    // Request a repaint if the brush is displayed.
    if (m_display.visible) {
      setColonyChanged();
    }
  }

  inline
  bool
  ColonyRenderer::mouseMoveEvent(const sdl::core::engine::MouseEvent& e) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Notify listeners through the dedicated handler.
    notifyCoordinatePointedTo(e.getMousePosition(), true);

    // Use the base handler to provide a return value.
    return sdl::graphic::ScrollableWidget::mouseMoveEvent(e);
  }

  inline
  float
  ColonyRenderer::getDefaultZoomInFactor() noexcept {
    return 2.0f;
  }

  inline
  float
  ColonyRenderer::getDefaultZoomOutFactor() noexcept {
    // Inverse of the zoom in factor.
    return 1.0f / getDefaultZoomInFactor();
  }

  inline
  float
  ColonyRenderer::getArrowKeyMotion() noexcept {
    return 30.0f;
  }

  inline
  int
  ColonyRenderer::getGridRoundup() noexcept {
    return 5;
  }

  inline
  int
  ColonyRenderer::getMinGridLines() noexcept {
    return 5u;
  }

  int
  ColonyRenderer::getMaxGridLines() noexcept {
    return 15;
  }

  inline
  utils::Boxf
  ColonyRenderer::getDefaultRenderingArea() noexcept {
    return utils::Boxf(0.0f, 0.0f, 16.0f, 8.0f);
  }

  inline
  void
  ColonyRenderer::clearColony() {
    if (m_tex.valid()) {
      getEngine().destroyTexture(m_tex);
      m_tex.invalidate();
    }
  }

  inline
  bool
  ColonyRenderer::colonyChanged() const noexcept {
    return m_colonyRendered;
  }

  inline
  void
  ColonyRenderer::setColonyChanged() noexcept {
    m_colonyRendered = true;

    requestRepaint();
  }

  inline
  utils::Sizef
  ColonyRenderer::getCellsDims() {
    // Retrieve the rendering area and the canvas size and compute the ratio
    // to obtain the size of a single cell.
    utils::Sizef env = LayoutItem::getRenderingArea().toSize();

    return utils::Sizef(
      env.w() / m_settings.area.w(),
      env.h() / m_settings.area.h()
    );
  }

  inline
  utils::Vector2f
  ColonyRenderer::convertPosToRealWorld(const utils::Vector2f& pos,
                                        bool global)
  {
    // In order to compute the local cells coordinates from the input
    // global data, we need to:
    // - convert it to local coordinate frame.
    // - determine the size of a single cell.
    // - compute the floating point coordinates of the cell.

    // Convert the input position to local coordinate frame.
    utils::Vector2f local = (global ? mapFromGlobal(pos) : pos);

    // Compute the size of a single cell.
    utils::Sizef env = LayoutItem::getRenderingArea().toSize();
    utils::Sizef cellsDims = getCellsDims();

    // Convert the local mouse position into floatnig point cell coordinates.
    float dToLeft = local.x() + env.w() / 2.0f;
    float dToBottom = local.y() + env.h() / 2.0f;

    float rX = dToLeft / cellsDims.w();
    float rY = dToBottom / cellsDims.h();

    // Offset to be in the reference defined by the current rendering area.
    return utils::Vector2f(
      m_settings.area.getLeftBound() + rX,
      m_settings.area.getBottomBound() + rY
    );
  }

  inline
  void
  ColonyRenderer::zoom(const utils::Vector2f& center,
                       float factor)
  {
    // We want to keep the center at the same position compared to the current
    // zoom level while reducing (or increasing) the dimensions of the window
    // by `factor`.
    const utils::Boxf& area = m_settings.area;

    // Compute the offset of the desired `center` compared to the current area.
    float toLeft = center.x() - area.getLeftBound();
    float toRight = area.getRightBound() - center.x();
    float toBottom = center.y() - area.getBottomBound();
    float toUp = area.getTopBound() - center.y();

    // Each dimension should be changed by a factor as provided in input.
    toLeft /= factor;
    toRight /= factor;
    toBottom /= factor;
    toUp /= factor;

    // Compute the new extreme points.
    float minX = center.x() - toLeft;
    float maxX = center.x() + toRight;
    float minY = center.y() - toBottom;
    float maxY = center.y() + toUp;

    // Compute the new area.
    utils::Boxf newArea = utils::Boxf(
      (minX + maxX) / 2.0f,
      (minY + maxY) / 2.0f,
      maxX - minX,
      maxY - minY
    );

    log(
      "Changed area from " + area.toString() + " to " + newArea.toString() +
      " (center: " + center.toString() + ", f: " + std::to_string(factor) + ")",
      utils::Level::Verbose
    );

    // Assign it to the internal area.
    m_settings.area = newArea;

    // Update grid resolution.
    updateGridResolution();
  }

  inline
  bool
  ColonyRenderer::updateGridResolution() {
    // We want to keep a number of grid lines between the values returned by the
    // `getMinGridLines` and `getMaxGridLines` method. For that we will adjust
    // the step of the grid lines and still guarantee some sort of consistency
    // by using the `getGridRoundup` method to still keep the grid indicating a
    // common value: it's better to always keep the grid displaying multiple of
    // say `5` than having the grid's step ranging from `3` and then `7`, etc.
    int avg = (getMinGridLines() + getMaxGridLines()) / 2;
    int gru = getGridRoundup();

    // Compute the current number of grid lines displayed in the grid.
    utils::Vector2f cur(
      m_settings.area.w() / m_display.resolution.x(),
      m_settings.area.h() / m_display.resolution.y()
    );

    // Adjust the resolution of the grid along each axis if needed.
    utils::Vector2i res = m_display.resolution;
    bool changed = false;

    if (cur.x() < getMinGridLines() || cur.x() > getMaxGridLines()) {
      // Choose a new resolution allowing to display the average number of lines
      // requested from the minimum and max.
      float expected = m_settings.area.w() / avg;

      // Round it up to get a consistent step. Consider that we can't have less
      // than a step of `1`.
      res.x() = std::max(static_cast<int>(std::floor(expected)), 1);
      if (res.x() != 1) {
        int overshoot = (gru - res.x() % gru) % gru;
        res.x() += overshoot;
      }

      // The resolution has been changed.
      changed = true;
    }

    if (cur.y() < getMinGridLines() || cur.y() > getMaxGridLines()) {
      // Choose a new resolution allowing to display the average number of lines
      // requested from the minimum and max.
      float expected = m_settings.area.h() / avg;

      // Round it up to get a consistent step. Consider that we can't have less
      // than a step of `1`.
      res.y() = std::max(static_cast<int>(std::floor(expected)), 1);
      if (res.y() != 1) {
        int overshoot = (gru - res.y() % gru) % gru;
        res.y() += overshoot;
      }

      // The resolution has been changed.
      changed = true;
    }

    utils::Vector2f ne(
      m_settings.area.w() / res.x(),
      m_settings.area.h() / res.y()
    );

    log(
      "Grid resolution was " + m_display.resolution.toString() + " leading to " +
      cur.toString() + " line(s) in viewport, correcting to " +
      res.toString() + " leading to " + ne.toString(),
      utils::Level::Verbose
    );

    // Assign the grid resolution if needed.
    if (changed) {
      m_display.resolution = res;
    }

    return changed;
  }

  inline
  void
  ColonyRenderer::notifyCoordinatePointedTo(const utils::Vector2f& pos,
                                            bool global)
  {
    // Note that we only want to notify coordinates in case the mouse is inside the
    // widget otherwise it makes no sense.
    if (!isMouseInside()) {
      return;
    }

    // Update the last known position of the mouse.
    m_lastKnownMousePos = (global ? pos : mapToGlobal(pos));

    // Convert the mouse position to cell coordinates.
    utils::Vector2f fpC = convertPosToRealWorld(pos, global);

    // Convert this into integer coordinates.
    utils::Vector2i cell(
      static_cast<int>(std::floor(fpC.x())),
      static_cast<int>(std::floor(fpC.y()))
    );

    std::pair<State, int> s = m_colony->getCellState(cell);

    // Notify external listeners.
    onCoordChanged.safeEmit(
      std::string("onCoordChanged(") + cell.toString() + ")",
      cell,
      s.first == State::Dead ? -1 : s.second
    );
  }

}

#endif    /* COLONY_RENDERER_HXX */
