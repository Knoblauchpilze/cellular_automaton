
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

  void
  CellBrush::loadFromFile(const std::string& file,
                          bool /*invertY*/)
  {
    // TODO: Implementation.
    log("Should load brush from data file \"" + file + "\"", utils::Level::Warning);
  }

}

