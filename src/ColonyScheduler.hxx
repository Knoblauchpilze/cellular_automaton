#ifndef    COLONY_SCHEDULER_HXX
# define   COLONY_SCHEDULER_HXX

# include "ColonyScheduler.hh"

namespace cellulator {

  inline
  ColonyScheduler::~ColonyScheduler() {
    // Stops the colony.
    stop();
  }

  inline
  unsigned
  ColonyScheduler::getWorkerThreadCount() noexcept {
    return 3u;
  }

}

#endif    /* COLONY_SCHEDULER_HXX */
