
# include "ColonyTile.hh"

namespace cellulator {

  ColonyTile::ColonyTile(const utils::Boxi& area,
                         CellsQuadTreeNode* cells,
                         const Type& type):
    utils::AsynchronousJob(std::string("tile_") + area.toString(),
                           type == Type::Interior ? utils::Priority::Normal : utils::Priority::Low),

    m_area(area),
    m_data(cells),
    m_type(type)
  {
    setService("colony");

    if (m_data == nullptr) {
      error(
        std::string("Could not create colony tile for ") + area.toString(),
        std::string("Invalid null colony")
      );
    }
  }

  void
  ColonyTile::compute() {
    // Use the dedicated handler on the data.
    switch (m_type) {
      case Type::Border:
        m_data->evolveBoundaries();
        break;
      case Type::Interior:
        m_data->evolve();
        break;
      default:
        log(
          std::string("Cannot determine the processing to apply for type ") + std::to_string(static_cast<int>(m_type)),
          utils::Level::Error
        );
        break;
    }
  }

}
