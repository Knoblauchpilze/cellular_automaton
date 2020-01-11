
# include "CellBrush.hh"

namespace cellulator {

  CellBrush::CellBrush(const std::string& file):
    utils::CoreObject(file)
  {
    setService("brush");
  }

  CellBrush::CellBrush(const utils::Sizei& size,
                       const State& state):
    utils::CoreObject(size.toString() + "_" + std::to_string(static_cast<int>(state)))
  {
    setService("brush");
  }

}

