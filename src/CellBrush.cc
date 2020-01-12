
# include "CellBrush.hh"

namespace cellulator {

  CellBrush::CellBrush(const std::string& name):
    utils::CoreObject(name),

    m_size(1, 1),

    m_monotonic(true),
    m_monotonicState(State::Alive),

    m_data()
  {
    setService("brush");
  }

  CellBrush::CellBrush(const std::string& file,
                       bool invertY):
    CellBrush(file)
  {
    // Load the data from the file.
    loadFromFile(file, invertY);
  }

  CellBrush::CellBrush(const utils::Sizei& size,
                       const State& state):
    CellBrush(size.toString() + "_" + std::to_string(static_cast<int>(state)))
  {
    // Check whether the input size is valid.
    if (!size.valid()) {
      error(
        std::string("Could not create cell brush"),
        std::string("Invalid input size ") + size.toString()
      );
    }

    // Reset monotonic brush' properties.
    m_size = size;
    m_monotonicState = state;
  }

  State
  CellBrush::getStateAt(int x,
                        int y) const noexcept
  {

    if (!valid()) {
      return State::Dead;
    }

    // In case the coordinate is not inside the area covered by this
    // brush return `Dead` as well.
    if (x < 0 || y < 0 || x >= m_size.w() || y >= m_size.h()) {
      return State::Dead;
    }

    // If the brush is monotonic, return the associated state.
    if (isMonotonic()) {
      return m_monotonicState;
    }

    // Convert the coordinate to a one-dimensional offset. We can be
    // certain that this will yield a valid index as we checked that
    // the coordinate was inside the area covered by this brush.
    int off = y * m_size.w() + x;

    return m_data[off];
  }

  void
  CellBrush::loadFromFile(const std::string& file,
                          bool /*invertY*/)
  {
    // TODO: Implementation.
    log("Should load brush from data file \"" + file + "\"", utils::Level::Warning);
  }

}

