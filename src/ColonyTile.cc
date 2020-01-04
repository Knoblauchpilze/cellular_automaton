
# include "ColonyTile.hh"

namespace cellulator {

  ColonyTile::ColonyTile(const utils::Boxi& area,
                         CellsBlocksShPtr cells,
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
        std::string("Could not create evolution tile at ") + area.toString(),
        std::string("Invaild null cells' data")
      );
    }
  }

  void
  ColonyTile::compute() {
    // Use the dedicated handler on the data.
    // TODO: Restore this.
  }

}
