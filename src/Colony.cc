
# include "Colony.hh"
# include "ColonyTile.hh"

namespace cellulator {

  Colony::Colony(const utils::Sizei& dims,
                 const rules::Type& ruleset,
                 const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_cells(nullptr),
    m_generation(0u),

    m_scheduler(std::make_shared<utils::ThreadPool>(getWorkerThreadCount())),
    m_simulationState(SimulationState::Stopped),
    m_taskProgress(0u),
    m_taskTotal(1u),

    onGenerationComputed()
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

  void
  Colony::start() {
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
  Colony::step() {
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
  Colony::stop() {
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
  Colony::generate() {
    // Protect from concurrent accesses.
    {
      Guard guard(m_propsLocker);

      // Check whether the simulation is stopped.
      if (m_simulationState != SimulationState::Stopped) {
        log(
          std::string("Could not generate new colony while current one is running"),
          utils::Level::Warning
        );

        return;
      }

      // Reset the generation count.
      m_generation = 0u;

      onGenerationComputed.safeEmit(
        std::string("onGenerationComputed(") + std::to_string(m_generation) + ")",
        m_generation
      );
    }

    // Use the dedicated handler to generate the colony.
    m_cells->randomize();
  }

  void
  Colony::build(const utils::Sizei& dims,
                const rules::Type& ruleset)
  {
    // Connect the results provider signal of the thread pool to the local slot.
    m_scheduler->onJobsCompleted.connect_member<Colony>(
      this,
      &Colony::handleTilesComputed
    );

    // Disable logging for the scheduler.
    m_scheduler->allowLog(false);

    // Create the cells' data.
    m_cells = std::make_shared<CellsQuadTree>(
      dims,
      getQuadTreeNodeSize(),
      ruleset,
      std::string("cells_quad_tree")
    );
  }

  void
  Colony::scheduleRendering() {
    // Cancel existing rendering operations.
    m_scheduler->cancelJobs();

    // Generate the launch schedule.
    std::vector<ColonyTileShPtr> tiles = m_cells->generateSchedule();

    // Convert to required pointer type.
    std::vector<utils::AsynchronousJobShPtr> tilesAsJobs(tiles.begin(), tiles.end());

    // Return early if nothing needs to be scheduled. We still want to notify listeners
    // that a new generation has been computed.
    if (tilesAsJobs.empty()) {
      onGenerationComputed.safeEmit(
        std::string("onGenerationComputed(") + std::to_string(m_generation) + ")",
        m_generation
      );

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
  Colony::handleTilesComputed(const std::vector<utils::AsynchronousJobShPtr>& tiles) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Append the number of tiles to the internal count.
    m_taskProgress += tiles.size();

    // In case all the tiles have been computed for this generation, notify external
    // listeners. Otherwise wait for the generation to complete.
    if (m_taskProgress == m_taskTotal) {
      ++m_generation;

      onGenerationComputed.safeEmit(
        std::string("onGenerationComputed(") + std::to_string(m_generation) + ")",
        m_generation
      );

      // Step up all the cells to their next state.
      m_cells->step();

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

