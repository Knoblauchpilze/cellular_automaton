
# include "Colony.hh"

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
    // TODO: Create schedule.

    // Convert to required pointer type.
    std::vector<utils::AsynchronousJobShPtr> tilesAsJobs;

    // Return early if nothing needs to be scheduled. We still want to trigger a repaint
    // though so we need to mark the tiles as dirty.
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
    }
  }

}

