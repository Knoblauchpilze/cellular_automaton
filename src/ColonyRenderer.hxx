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

}

#endif    /* COLONY_RENDERER_HXX */
