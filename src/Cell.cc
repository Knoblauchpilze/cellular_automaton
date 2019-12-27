
# include "Cell.hh"

namespace cellulator {

  Cell::Cell(const State& state,
             const rules::Type& ruleset):
    m_ruleset(ruleset),

    m_state(state),
    m_next(m_state)
  {}

  State
  Cell::evolveGameOfLife(const State& current,
                         unsigned living) noexcept
  {
    // Update the state based on the number of living neighbors
    // and the current state of the cell.
    switch (current) {
      case State::Newborn:
        // Similar state as `Alive`.
      case State::Alive:
        return (living >= 2 && living <= 3 ? State::Alive : State::Dying);
      case State::Dying:
        // Similar state as `Dead`.
      case State::Dead:
        return (living == 3 ? State::Newborn : State::Dead);
      default:
        break;
    }

    // Unknown state, return the current state.
    return current;
  }

}
