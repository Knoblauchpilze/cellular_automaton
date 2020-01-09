
# include "Colony.hh"

namespace cellulator {

  Colony::Colony(const utils::Sizei& dims,
                 const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_generation(0u),
    m_liveCells(0u),

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

    build(dims);
  }

  unsigned
  Colony::step(unsigned* alive) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // We need to swap the internal arrays to move on to the next state.
    m_liveCells = m_cells->step();

    // One more generation has been computed.
    ++m_generation;

    // Fill in the number of alive cells if needed.
    if (alive != nullptr) {
      *alive = m_liveCells;
    }

    return m_generation;
  }

  unsigned
  Colony::generate() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    m_liveCells = m_cells->randomize();

    // The colony is back to square one.
    m_generation = 0u;

    return m_liveCells;
  }

  std::vector<ColonyTileShPtr>
  Colony::generateSchedule() {
    // Generate the schedule using the internal cells' data.
    std::vector<ColonyTileShPtr> tiles;
    m_cells->generateSchedule(tiles);

    // In case the generated schedule is empty, it means that
    // we don't have any evolution for this generation.
    // we still need to move forward of one generation though
    // as it still counts as a time step.
    if (tiles.empty()) {
      Guard guard(m_propsLocker);

      ++m_generation;
    }

    return tiles;
  }

  void
  Colony::build(const utils::Sizei& dims) {
    // Create the cells' data.
    m_cells = std::make_shared<CellsBlocks>(getCellBlockDims());

    // Allocate initial blocks.
    m_cells->allocateTo(dims);
  }

}

