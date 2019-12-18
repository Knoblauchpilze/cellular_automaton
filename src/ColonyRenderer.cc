
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
