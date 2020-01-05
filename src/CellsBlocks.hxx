#ifndef    CELLS_BLOCKS_HXX
# define   CELLS_BLOCKS_HXX

# include "CellsBlocks.hh"

namespace cellulator {

  inline
  utils::Boxi
  CellsBlocks::getLiveArea() noexcept {
    // Protect from concurrent access.
    Guard guard(m_propsLocker);

    return m_liveArea;
  }

  inline
  std::pair<State, int>
  CellsBlocks::getCellStatus(const utils::Vector2i& coord) {
    // Protect from concurrent access.
    Guard guard(m_propsLocker);

    std::pair<State, int> out = std::make_pair(State::Dead, -1);

    // Eliminate trivial cases where the input coordinate are not in the
    // live area of the colony.
    if (!m_liveArea.contains(coord)) {
      return out;
    }

    // Traverse the list of blocks and find the one spanning the input
    // coordinate. If none can be found (or at least none active) the
    // default value will be returned.
    unsigned id = 0u;
    bool found = false;

    while (id < m_blocks.size() && !found) {
      // Discard inactive blocks.
      if (m_blocks[id].active && m_blocks[id].area.contains(coord)) {
        int dataID = indexFromCoord(m_blocks[id], coord);

        out.first = m_states[dataID];
        out.second = m_ages[dataID];

        found = true;
      }

      ++id;
    }

    return out;
  }

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

  inline
  int
  CellsBlocks::indexFromCoord(const BlockDesc& block,
                              const utils::Vector2i& coord) const
  {
    // Convert the input coordinate in the local block's cooordinate frame.
    utils::Vector2i local(coord.x() - block.area.x(), coord.y() - block.area.y());

    // Compute the one-dimensional index from this.
    int xMin = block.area.getLeftBound();
    int yMin = block.area.getBottomBound();

    return (local.y() - yMin) * block.area.w() + (local.x() - xMin);
  }

  inline
  void
  CellsBlocks::updateCellsAge() noexcept {
    // Traverse the current states.
    for (unsigned id = 0u ; id < m_states.size() ; ++id) {
      if (m_states[id] == State::Alive) {
        ++m_ages[id];
      }
    }
  }

}

#endif    /* CELLS_BLOCKS_HXX */
