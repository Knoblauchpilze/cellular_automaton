
# include "ColonyScheduler.hh"
# include "ColonyTile.hh"

namespace cellulator {

  ColonyScheduler::ColonyScheduler(ColonyShPtr colony):
    utils::CoreObject(std::string("scheduler_for_") + colony->getName()),

    m_propsLocker(),

    m_scheduler(std::make_shared<utils::ThreadPool>(getWorkerThreadCount())),
    m_simulationState(SimulationState::Stopped),
    m_taskProgress(0u),
    m_taskTotal(1u),

    m_colony(colony),

    onGenerationComputed()
  {
    setService("scheduler");

    // Check consistency.
    if (m_colony == nullptr) {
      error(
        std::string("Could not create colony scheduler"),
        std::string("Invalid null colony")
      );
    }

    build();
  }

  void
  ColonyScheduler::start() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Check whether the simulation is already started.
    if (m_simulationState == SimulationState::Running) {
      return;
    }

    // Assign the new state.
    m_simulationState = SimulationState::Running;

    scheduleRendering();
  }

  void
  ColonyScheduler::step() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Check whether the simulation is already computing a next step.
    if (m_simulationState != SimulationState::Stopped) {
      return;
    }

    // Assign the new state.
    m_simulationState = SimulationState::SingleStep;

    scheduleRendering();
  }

  void
  ColonyScheduler::stop() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Check whether the simulation is already stopped.
    if (m_simulationState == SimulationState::Stopped) {
      return;
    }

    // Request the simulation to stop.
    m_simulationState = SimulationState::Stopped;

    scheduleRendering();
  }

  void
  ColonyScheduler::generate() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Check whether the simulation is stopped.
    if (m_simulationState != SimulationState::Stopped) {
      log(
        std::string("Could not generate new colony while current one is running"),
        utils::Level::Warning
      );

      return;
    }

    // Use the dedicated handler to generate the colony.
    unsigned gen = m_colony->generate();

    // Reset the generation count.
    onGenerationComputed.safeEmit(
      std::string("onGenerationComputed(") + std::to_string(gen) + ")",
      gen
    );
  }

  void
  ColonyScheduler::build() {
    // Connect the results provider signal of the thread pool to the local slot.
    m_scheduler->onJobsCompleted.connect_member<ColonyScheduler>(
      this,
      &ColonyScheduler::handleTilesComputed
    );

    // Disable logging for the scheduler.
    m_scheduler->allowLog(false);
  }

  void
  ColonyScheduler::scheduleRendering() {
    // Cancel existing rendering operations.
    m_scheduler->cancelJobs();

    // Generate the launch schedule.
    std::vector<ColonyTileShPtr> tiles = m_colony->generateSchedule();

    // Convert to required pointer type.
    std::vector<utils::AsynchronousJobShPtr> tilesAsJobs(tiles.begin(), tiles.end());

    // Return early if nothing needs to be scheduled.
    if (tilesAsJobs.empty()) {
      return;
    }

    m_scheduler->enqueueJobs(tilesAsJobs, false);

    // Notify listeners that the progression is now `0`.
    m_taskProgress = 0u;
    m_taskTotal = tilesAsJobs.size();

    // Start the computing.
    m_scheduler->notifyJobs();
  }

  void
  ColonyScheduler::handleTilesComputed(const std::vector<utils::AsynchronousJobShPtr>& tiles) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Append the number of tiles to the internal count.
    m_taskProgress += tiles.size();

    // In case all the tiles have been computed for this generation, notify external
    // listeners. Otherwise wait for the generation to complete.
    if (m_taskProgress == m_taskTotal) {
      // Step the colony one generation ahead in time.
      unsigned gen = m_colony->step();

      onGenerationComputed.safeEmit(
        std::string("onGenerationComputed(") + std::to_string(gen) + ")",
        gen
      );

      // Check whether we should schedule a new generation based on the status of
      // the simulation. We will also update the simulation state accordingly.
      switch (m_simulationState) {
        case SimulationState::Running:
          scheduleRendering();
          break;
        case SimulationState::SingleStep:
          // Reset the simulation to a waiting state.
          m_simulationState = SimulationState::Stopped;
          break;
        case SimulationState::Stopped:
          // Nothing to do, the simulation is already stopped we will not start a
          // new rendering nor modify the state.
        default:
          // Same behavior as if the simulation was stopped.
          break;
      }
    }
  }

}

