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
  ColonyScheduler::paint(const CellBrush& brush,
                         const utils::Vector2i& coord)
  {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // In case the simulation is not stopped, we can't paint the brush.
    if (m_simulationState != SimulationState::Stopped) {
      warn("Could not paint brush " + brush.getName() + " at " + coord.toString() + ", simulation is running");

      return m_colony->getLiveCellsCount();
    }

    // Call the dedicated method on the scheduler.
    return m_colony->paint(brush, coord);
  }

  inline
  unsigned
  ColonyScheduler::getWorkerThreadCount() noexcept {
    return 3u;
  }

}

#endif    /* COLONY_SCHEDULER_HXX */
