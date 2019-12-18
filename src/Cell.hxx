#ifndef    CELL_HXX
# define   CELL_HXX

# include "Cell.hh"
# include <type_traits>
# include <cstdlib>

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

}

#endif    /* CELL_HXX */
