
# include "Colony.hh"

namespace cellulator {

  Colony::Colony(const utils::Sizei& dims,
                 const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_dims(),
    m_cells(),
    m_generation(0u),

    m_scheduler(std::make_shared<utils::ThreadPool>(getWorkerThreadCount())),
    m_simulationState(SimulationState::Stopped),
    m_taskProgress(0u),
    m_taskTotal(1u),

    onGenerationComputed()
  {
    // Check consistency.
    if (!dims.valid()) {
      error(
        std::string("Could not create colony"),
        std::string("Invalid dimensions ") + dims.toString()
      );
    }

    build();

    reset(dims);
  }

  utils::Boxi
  Colony::fetchCells(std::vector<Cell>& cells,
                     const utils::Boxf& area)
  {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Clamp the area to get only relevant cells.
    utils::Boxi evenized = fromFPCoordinates(area);

    utils::Vector2i rC(
      std::max(-m_dims.w() / 2, std::min(m_dims.w() / 2, evenized.x())),
      std::max(-m_dims.h() / 2, std::min(m_dims.h() / 2, evenized.y()))
    );
    utils::Sizei rD(
      2 * std::min(evenized.w() / 2, std::min(rC.x() + m_dims.w() / 2, m_dims.w() / 2 - rC.x())),
      2 * std::min(evenized.h() / 2, std::min(rC.y() + m_dims.h() / 2, m_dims.h() / 2 - rC.y()))
    );

    utils::Boxi real(rC, rD);

    // Resize the output vector if needed.
    if (cells.size() != static_cast<unsigned>(real.area())) {
      cells.resize(real.area());
    }

    // Populate the needed cells.
    int xMin = real.getLeftBound();
    int yMin = real.getBottomBound();
    int xMax = real.getRightBound();
    int yMax = real.getTopBound();

    for (int y = yMin ; y < yMax ; ++y) {
      // Convert logical coordinates to valid cells coordinates.
      int offset = (y  + real.h() / 2) * real.w();
      int rOffset = (y + m_dims.h() / 2) * m_dims.w();

      for (int x = xMin ; x < xMax ; ++x) {
        // Convert the `x` coordinate similarly to the `y` coordinate.
        int xOff = x + real.w() / 2;
        int rXOff = x + m_dims.w() / 2;

        cells[offset + xOff] = m_cells[rOffset + rXOff];
      }
    }

    return real;
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
    randomize();
  }

  void
  Colony::build() {
    // Connect the results provider signal of the thread pool to the local slot.
    m_scheduler->onJobsCompleted.connect_member<Colony>(
      this,
      &Colony::handleTilesComputed
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

