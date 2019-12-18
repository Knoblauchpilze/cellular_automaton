
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
    m_colonyRendered(true),

    m_colony(nullptr)
  {
    setService(std::string("colony_renderer"));

    build();
  }

  void
  ColonyRenderer::start(const std::string& dummy) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Should start the simulation of the colony.
    log("Should start the similation from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::stop(const std::string& dummy) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Should stop the simulation of the colony.
    log("Should stop the similation from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::nextStep(const std::string& dummy) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Should simulate the next step of the simulation.
    log("Should simulate the next step of the similation from " + dummy, utils::Level::Warning);
  }

  void
  ColonyRenderer::generate(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Create the colony if needed.
    if (m_colony == nullptr) {
      m_colony = std::make_shared<Colony>(
        utils::Sizei(500, 300),
        std::string("Drop it like it's Hoth")
      );
    }

    // Generate it at random.
    m_colony->generate();

    // Indicate that the colony changed so that we can repaint it.
    setColonyChanged();
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
    // TODO: Handle creation of content.
  }

}
