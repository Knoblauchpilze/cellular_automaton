#ifndef    CELL_HXX
# define   CELL_HXX

# include "Cell.hh"
# include <type_traits>
# include <cstdlib>
# include <core_utils/CoreException.hh>

namespace cellulator {

  inline
  State
  Cell::state() const noexcept {
    return m_state;
  }

  inline
  void
  Cell::randomize() {
    // Pick a value among the possible states. We will use the basic
    // `rand` method: even though it is probably not perfect it is
    // more than enough for this use case.
    using UType = std::underlying_type<State>::type;
    UType max = static_cast<UType>(State::Count);

    UType v = std::rand() % max;

    m_state = static_cast<State>(v);
  }

  inline
  void
  Cell::step() {
    // Apply the next state to the current.
    m_next = m_state;
  }

  inline
  State
  Cell::update(unsigned livingNeigboors) {
    // Based on the ruleset, call the dedicated handler.
    switch (m_ruleset) {
      case rules::Type::GameOfLife:
        m_next = evolveGameOfLife(m_state, livingNeigboors);
        break;
      default:
        // Unknown ruleset.
        break;
    }

    throw utils::CoreException(
      std::string("Could not update cell"),
      std::string("cell"),
      std::string("colony"),
      std::string("Unknown ruleset ") + std::to_string(static_cast<int>(m_ruleset))
    );
  }

}

#endif    /* CELL_HXX */
