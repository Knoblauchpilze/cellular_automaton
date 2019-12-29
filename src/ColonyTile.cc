
# include "ColonyTile.hh"

namespace cellulator {

  ColonyTile::ColonyTile(const utils::Boxi& area,
                         CellsQuadTreeNode* cells):
    utils::CoreObject(std::string("tile_") + area.toString()),
    utils::AsynchronousJob(),

    m_area(area),
    m_data(cells)
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
    m_data->evolve();
  }

}
