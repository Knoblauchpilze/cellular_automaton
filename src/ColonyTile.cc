
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
    // access the data from adjacent nodes. In the meantime we need to update the
    // adjacency count.
    log("Updating area " + m_area.toString());

    for (int y = 1 ; y < m_area.h() - 1 ; ++y) {
      int offset = y * m_area.w();

      for (int x = 1 ; x < m_area.w() - 1 ; ++x) {
        State s = m_data->m_cells[offset + x].update(m_data->m_adjacency[offset + x]);
        bool alive = (s == State::Alive || s == State::Newborn);

        m_data->updateAdjacencyFor(utils::Vector2i(x, y), alive);
      }
    }
  }

}
