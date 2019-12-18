
# include "Colony.hh"

namespace cellulator {

  Colony::Colony(const utils::Sizei& dims,
                 const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_dims(),
    m_cells()
  {
    // Check consistency.
    if (!dims.valid()) {
      error(
        std::string("Could not create colony"),
        std::string("Invalid dimensions ") + dims.toString()
      );
    }

    reset(dims);
  }

  void
  Colony::start() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Start the colony.
  }

  void
  Colony::step() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Simulate a single step of the colony.
  }

  void
  Colony::stop() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Stop the colony.
  }

  void
  Colony::generate() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // TODO: Be sure that the simulation has stopped.

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

}

