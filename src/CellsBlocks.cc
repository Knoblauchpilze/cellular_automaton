
# include "CellsBlocks.hh"

namespace cellulator {

  CellsBlocks::CellsBlocks(const rules::Type& ruleset,
                           const utils::Sizei& nodeDims):
    utils::CoreObject(std::string("cells_blocks")),

    m_ruleset(ruleset),
    m_nodesDims(nodeDims),

    m_states(),
    m_nextStates()
  {
    setService("blocks");

    // Check consistency.
    if (!m_nodesDims.valid()) {
      error(
        std::string("Could not allocate cells blocks"),
        std::string("Invalid nodes dimensions ") + m_nodesDims.toString()
      );
    }
  }

  utils::Boxi
  CellsBlocks::allocateTo(const utils::Sizei& /*dims*/,
                          const State& /*state*/)
  {
    // TODO: Handle this.
    return utils::Boxi();
  }

  unsigned
  CellsBlocks::randomize() {
    // TODO: Handle this.
    return 0u;
  }


}
