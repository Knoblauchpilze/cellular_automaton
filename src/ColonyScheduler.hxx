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
  utils::Boxi
  ColonyScheduler::getArea() noexcept {
    return m_colony->getArea();
  }

  inline
  utils::Boxi
  ColonyScheduler::fetchCells(std::vector<State>& cells,
                              const utils::Boxf& area)
  {
    return m_colony->fetchCells(cells, area);
  }

  inline
  unsigned
  ColonyScheduler::getWorkerThreadCount() noexcept {
    return 3u;
  }

}

#endif    /* COLONY_SCHEDULER_HXX */
