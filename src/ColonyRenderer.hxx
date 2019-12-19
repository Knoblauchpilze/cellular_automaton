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

    // Request to start the simulation if possible.
    if (m_colony != nullptr) {
      m_colony->start();
    }
  }

  inline
  void
  ColonyRenderer::stop(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Request to stop the simulation if possible.
    if (m_colony != nullptr) {
      m_colony->stop();
    }
  }

  inline
  void
  ColonyRenderer::nextStep(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Request a next step if possible.
    if (m_colony != nullptr) {
      m_colony->step();
    }
  }

  inline
  void
  ColonyRenderer::updatePrivate(const utils::Boxf& window) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Use the base handler.
    sdl::core::SdlWidget::updatePrivate(window);

    // TODO: Handle update.
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

  void
  ColonyRenderer::setColonyChanged() noexcept {
    m_colonyRendered = true;

    requestRepaint();
  }

  inline
  void
  ColonyRenderer::loadColony() {
    // Clear any existing texture representing the colony.
    clearColony();

    // Check whether the colony is created: if this is not the case
    // there's nothing to create and nothing to display.
    if (m_colony == nullptr) {
      return;
    }

    // Create the brush representing the tile using the palette
    // provided by the user.
    sdl::core::engine::BrushShPtr brush = m_colony->createBrush();

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

}

#endif    /* COLONY_RENDERER_HXX */
