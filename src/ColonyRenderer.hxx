#ifndef    COLONY_RENDERER_HXX
# define   COLONY_RENDERER_HXX

# include "ColonyRenderer.hh"

namespace cellulator {

  inline
  ColonyRenderer::~ColonyRenderer() {
    // Protect from concurrent accesses
    Guard guard (m_propsLocker);

    clearColony();
  }

  inline
  void
  ColonyRenderer::start(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Request to start the simulation.
    m_colony->start();
  }

  inline
  void
  ColonyRenderer::stop(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Request to stop the simulation.
    m_colony->stop();
  }

  inline
  void
  ColonyRenderer::nextStep(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Request a next step.
    m_colony->step();
  }

  inline
  bool
  ColonyRenderer::mouseMoveEvent(const sdl::core::engine::MouseEvent& e) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Convert the mouse position to cell coordinates.
    utils::Vector2f fpC = convertGlobalToRealWorld(e.getMousePosition());

    // Convert this into integer coordinates.
    utils::Vector2i cell(
      static_cast<int>(std::floor(fpC.x())),
      static_cast<int>(std::floor(fpC.y()))
    );

    // Notify external listeners.
    onCoordChanged.safeEmit(
      std::string("onCoordChanged(") + cell.toString() + ")",
      cell
    );

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
  ColonyRenderer::convertGlobalToRealWorld(const utils::Vector2f& global) {
    // In order to compute the local cells coordinates from the input
    // global data, we need to:
    // - convert it to local coordinate frame.
    // - determine the size of a single cell.
    // - compute the floating point coordinates of the cell.

    // Convert the input position to local coordinate frame.
    utils::Vector2f local = mapFromGlobal(global);

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

    log("Changed area from " + area.toString() + " to " + newArea.toString() + " (center: " + center.toString() + ", f: " + std::to_string(factor) + ")");

    // Assign it to the internal area.
    m_settings.area = newArea;
  }

}

#endif    /* COLONY_RENDERER_HXX */
