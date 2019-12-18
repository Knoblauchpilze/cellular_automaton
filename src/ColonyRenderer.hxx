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
  }

  inline
  void
  ColonyRenderer::loadColony() {
    // Clear any existing texture representing the colony.
    clearColony();

    // TODO: Handle creation.
  }

}

#endif    /* COLONY_RENDERER_HXX */
