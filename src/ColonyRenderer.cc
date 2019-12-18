
# include "ColonyRenderer.hh"
# include <sdl_engine/PaintEvent.hh>

namespace cellulator {

  ColonyRenderer::ColonyRenderer(const utils::Sizef& hint,
                                 sdl::core::SdlWidget* parent):
    sdl::graphic::ScrollableWidget(std::string("colony_renderer"),
                                   parent,
                                   hint),

    m_propsLocker(),

    m_tex(),
    m_colonyRendered(true)
  {
    setService(std::string("colony_renderer"));

    build();
  }

  void
  ColonyRenderer::start(const std::string& dummy) {
    // TODO: Should start the simulation of the colony.
    log("Should start the similation from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::stop(const std::string& dummy) {
    // TODO: Should stop the simulation of the colony.
    log("Should stop the similation from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::nextStep(const std::string& dummy) {
    // TODO: Should simulate the next step of the simulation.
    log("Should simulate the next step of the similation from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::generate(const std::string& dummy) {
    // TODO: Should try to generate a new colony.
    log("Should generate a new colony from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::drawContentPrivate(const utils::Uuid& /*uuid*/,
                                     const utils::Boxf& /*area*/)
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

    // TODO: Handle drawing.
  }

  void
  ColonyRenderer::build() {
    // TODO: Handle creation of content.
  }

}
