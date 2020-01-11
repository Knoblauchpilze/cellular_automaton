#ifndef    COLONY_TILE_HXX
# define   COLONY_TILE_HXX

# include "ColonyTile.hh"

namespace cellulator {

  inline
  ColonyTile::ColonyTile(unsigned blockID,
                         CellsBlocks* cells):
    utils::AsynchronousJob(std::string("tile_") + std::to_string(blockID)),

    m_blockID(blockID),
    m_data(cells)
  {
    setService("colony");

    if (m_data == nullptr) {
      error(
        std::string("Could not create evolution tile for block ") + std::to_string(m_blockID),
        std::string("Invaild null cells' data")
      );
    }
  }

  inline
  ColonyTile::ColonyTile():
    utils::AsynchronousJob(std::string("tile_closure")),

    m_blockID(0u),
    m_data(nullptr)
  {
    setService("colony");
  }

  inline
  void
  ColonyTile::compute() {
    // Use the dedicated handler on the data if needed.
    if (m_data != nullptr) {
      m_data->evolve(m_blockID);
    }
  }

  inline
  bool
  ColonyTile::closure() {
    return m_data == nullptr;
  }
}

#endif    /* COLONY_TILE_HXX */
