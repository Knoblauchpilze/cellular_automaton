
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

  sdl::core::engine::BrushShPtr
  Colony::createBrush() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Traverse the internal array of cells and build an array of
    // colors to use to represent the colony.
    sdl::core::engine::Color def = sdl::core::engine::Color::NamedColor::Black;
    std::vector<sdl::core::engine::Color> colors(m_dims.area(), def);

    for (int y = 0 ; y < m_dims.h() ; ++y) {
      // Compute the coordinate of this pixel in the output canvas. Note that
      // we perform an inversion of the internal data array along the `y` axis:
      // indeed as we will use it to generate a surface we need to account for
      // the axis inversion that will be applied there.
      int offset = (m_dims.h() - 1 - y) * m_dims.w();

      for (int x = 0 ; x < m_dims.w() ; ++x) {
        // Determine the color for this cell.
        sdl::core::engine::Color c = def;
        switch (m_cells[offset + x].state()) {
          case State::Newborn:
            c = sdl::core::engine::Color::NamedColor::Green;
            break;
          case State::Alive:
            c = sdl::core::engine::Color::NamedColor::Blue;
            break;
          case State::Dying:
            c = sdl::core::engine::Color::NamedColor::Red;
            break;
          case State::Dead:
          default:
            // Keep the default color.
            break;
        }

        colors[offset + x] = c;
      }
    }

    // Create a brush from the array of colors.
    sdl::core::engine::BrushShPtr brush = std::make_shared<sdl::core::engine::Brush>(
      std::string("brush_for_") + getName(),
      false
    );

    brush->createFromRaw(m_dims, colors);

    return brush;
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

