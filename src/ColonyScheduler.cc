
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

    onGenerationComputed(),
    onSimulationToggled()
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
    const std::lock_guard guard(m_propsLocker);

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
    const std::lock_guard guard(m_propsLocker);

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
    const std::lock_guard guard(m_propsLocker);

    // Check whether the simulation is already stopped.
    if (m_simulationState == SimulationState::Stopped) {
      return;
    }

    // Request the simulation to stop.
    m_simulationState = SimulationState::Stopped;
  }

  void
  ColonyScheduler::toggle() {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // Toggle the simulation state.
    bool changed = true;

    switch (m_simulationState) {
      case SimulationState::Running:
        m_simulationState = SimulationState::Stopped;
        break;
      case SimulationState::Stopped:
        m_simulationState = SimulationState::Running;
        break;
      case SimulationState::SingleStep:
        // In case the simulation is set to `SingleStep` we can't
        // really invert anything. Same for the default case where
        // the state is not recognized. We thus won't do a thing.
      default:
        changed = false;
        break;
    }

    // Scheduler a rendering if needed.
    if (changed) {
      // We only want to schedule a new rendering if we need to start
      // the simulation.
      if (m_simulationState == SimulationState::Running) {
        scheduleRendering();
      }

      // Notify external listeners.
      onSimulationToggled.safeEmit(
        std::string("onSimulationToggled(") + std::to_string(m_simulationState == SimulationState::Running) + ")",
        m_simulationState == SimulationState::Running
      );
    }
  }

  void
  ColonyScheduler::generate() {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // Check whether the simulation is stopped.
    if (m_simulationState != SimulationState::Stopped) {
      warn("Could not generate new colony while current one is running");
      return;
    }

    // Use the dedicated handler to generate the colony.
    unsigned alive = m_colony->generate();

    // Reset the generation count.
    onGenerationComputed.safeEmit(
      std::string("onGenerationComputed(0, ") + std::to_string(alive) + ")",
      0u,
      alive
    );
  }

  void
  ColonyScheduler::onRulesetChanged(CellEvolverShPtr ruleset) {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // Check whether the simulation is stopped.
    if (m_simulationState != SimulationState::Stopped) {
      warn("Could not change ruleset, simulation is running");
      return;
    }

    m_colony->setRuleset(ruleset);
  }

  void
  ColonyScheduler::build() {
    // Connect the results provider signal of the thread pool to the local slot.
    m_scheduler->onJobsCompleted.connect_member<ColonyScheduler>(
      this,
      &ColonyScheduler::handleTilesComputed
    );

    // Disable logging for the scheduler.
    m_scheduler->setAllowLog(false);
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
      // The scheduling yields no tiles: this usually means that the colony
      // is composed only of `Dead` cells and still life patterns.
      // We still need to move on to the next generation and notify listeners.
      unsigned gen = m_colony->getGeneration();
      unsigned alive = m_colony->getLiveCellsCount();

      onGenerationComputed.safeEmit(
        std::string("onGenerationComputed(") + std::to_string(gen) + ", " + std::to_string(alive) + ")",
        gen,
        alive
      );

      // Reset the internal simulation state.
      m_simulationState = SimulationState::Stopped;

      warn("Scheduled a rendering but no jobs where created, discarding request");
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
    const std::lock_guard guard(m_propsLocker);

    // Append the number of tiles to the internal count.
    m_taskProgress += tiles.size();

    // In case all the tiles have been computed for this generation, notify external
    // listeners. Otherwise wait for the generation to complete.
    if (m_taskProgress == m_taskTotal) {
      // First thing to determine is to check whether a closure exists in the input
      // tiles: if this is the case it means that we don't want to step the colony
      // one step forward.
      bool closure = false;

      unsigned id = 0u;
      while (id < tiles.size() && !closure) {
        ColonyTileShPtr t = std::dynamic_pointer_cast<ColonyTile>(tiles[id]);

        // Check consistency.
        if (t == nullptr) {
          warn("Received completion for unknown job type \"" + tiles[id]->getName() + "\"");

          continue;
        }

        closure = t->closure();
        ++id;
      }

      // In case a closure is detected, stop the processing and notify that the
      // simulation is halted.
      if (closure) {
        m_simulationState = SimulationState::Stopped;

        onSimulationToggled.safeEmit(
          std::string("onSimulationToggled(false)"),
          false
        );

        return;
      }

      // Step the colony one generation ahead in time.
      unsigned alive = 0u;
      unsigned gen = m_colony->step(&alive);

      onGenerationComputed.safeEmit(
        std::string("onGenerationComputed(") + std::to_string(gen) + ", " + std::to_string(alive) + ")",
        gen,
        alive
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

