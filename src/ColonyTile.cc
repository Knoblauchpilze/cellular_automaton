
# include "ColonyTile.hh"

namespace cellulator {

  ColonyTile::ColonyTile(const utils::Boxi& area,
                         CellsQuadTreeNode* cells):
    utils::CoreObject(std::string("tile_") + area.toString()),
    utils::AsynchronousJob(),

    m_area(area),
    m_cells(cells)
  {
    setService("colony");

    if (m_cells == nullptr) {
      error(
        std::string("Could not create colony tile for ") + area.toString(),
        std::string("Invalid null colony")
      );
    }
  }

  void
  ColonyTile::compute() {
    log("Should compute " + m_area.toString(), utils::Level::Warning);
    // TODO: Implementation.
  }

}
