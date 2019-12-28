
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
    // First we need to compute the internal elements of the quadtree node. This
    // corresponds to all the cells except the boundaries, where we would need to
    // access the data from adjacent nodes.
    for (int y = 0 ; y < m_area.h() ; ++y) {
      // int offset = y * m_area.w();

      // for (int x = 0 ; x < m_area.w() ; ++x) {
      //   m_data->m_cells[offset + x].update()
      // }
    }

    log("Should compute " + m_area.toString(), utils::Level::Warning);
    // TODO: Implementation.
  }

}
