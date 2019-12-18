#ifndef    COLONY_HXX
# define   COLONY_HXX

# include "Colony.hh"

namespace cellulator {

  inline
  Colony::~Colony() {
    // Stops the colony.
    stop();
  }

}

#endif    /* COLONY_HXX */
