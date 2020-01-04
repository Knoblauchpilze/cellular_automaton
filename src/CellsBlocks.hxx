#ifndef    CELLS_BLOCKS_HXX
# define   CELLS_BLOCKS_HXX

# include "CellsBlocks.hh"

namespace cellulator {

  inline
  float
  CellsBlocks::getDeadCellProbability() noexcept {
    return 0.7f;
  }

  inline
  void
  CellsBlocks::clear() {
    m_states.clear();
    m_adjacency.clear();

    m_nextStates.clear();
    m_nextAdjacency.clear();

    m_blocks.clear();
  }

  inline
  unsigned
  CellsBlocks::dataIDFromBlock(unsigned blockID) const noexcept {
    return blockID * sizeOfBlock();
  }

  inline
  unsigned
  CellsBlocks::sizeOfBlock() const noexcept {
    return m_nodesDims.area();
  }


}

#endif    /* CELLS_BLOCKS_HXX */
