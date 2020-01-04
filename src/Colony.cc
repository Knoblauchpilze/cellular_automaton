
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
  Colony::step() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Evolve all the cells.

    // One more generation has been computed.
    ++m_generation;

    return m_generation;
  }

  unsigned
  Colony::generate() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Should randomize the colony.

    // The colony is back to square one.
    m_generation = 0u;

    return m_generation;
  }

  std::vector<ColonyTileShPtr>
  Colony::generateSchedule() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Should create the schedule.
    return std::vector<ColonyTileShPtr>();
  }

  void
  Colony::build(const utils::Sizei& dims,
                const rules::Type& ruleset)
  {
    // Create the cells' data.
    m_cells.resize(dims.area(), Cell(State::Dead, ruleset));
  }

}

