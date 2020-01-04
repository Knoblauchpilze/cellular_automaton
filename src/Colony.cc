
# include "Colony.hh"

namespace cellulator {

  Colony::Colony(const utils::Sizei& dims,
                 const rules::Type& ruleset,
                 const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_generation(0u),
    m_cells()
  {
    setService("cells");

    // Check consistency.
    if (!dims.valid()) {
      error(
        std::string("Could not create colony"),
        std::string("Invalid dimensions ") + dims.toString()
      );
    }

    build(dims, ruleset);
  }

  unsigned
  Colony::step(unsigned* alive) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // We need to swap the internal arrays to move on to the next state.
    unsigned ac = m_cells->step();

    // One more generation has been computed.
    ++m_generation;

    // Fill in the number of alive cells if needed.
    if (alive != nullptr) {
      *alive = ac;
    }

    return m_generation;
  }

  unsigned
  Colony::generate() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    unsigned count = m_cells->randomize();

    // The colony is back to square one.
    m_generation = 0u;

    return count;
  }

  std::vector<ColonyTileShPtr>
  Colony::generateSchedule() {
    std::vector<ColonyTileShPtr> tiles;
    m_cells->generateSchedule(tiles);

    return tiles;
  }

  void
  Colony::build(const utils::Sizei& dims,
                const rules::Type& ruleset)
  {
    // Create the cells' data.
    m_cells = std::make_shared<CellsBlocks>(ruleset, getCellBlockDims());

    // Allocate initial blocks.
    m_cells->allocateTo(dims, State::Dead);
  }

}

