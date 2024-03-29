#ifndef    CELLS_BLOCKS_HXX
# define   CELLS_BLOCKS_HXX

# include "CellsBlocks.hh"

namespace cellulator {

  inline
  unsigned
  CellsBlocks::step() {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    return stepPrivate();
  }

  inline
  utils::Boxf
  CellsBlocks::getLiveArea() noexcept {
    // Protect from concurrent access.
    const std::lock_guard guard(m_propsLocker);

    return m_liveArea;
  }

  inline
  void
  CellsBlocks::setRuleset(CellEvolverShPtr ruleset) {
    // Protect from concurrnet access.
    const std::lock_guard guard(m_propsLocker);

    m_ruleset = ruleset;
  }

  inline
  float
  CellsBlocks::getDeadCellProbability() noexcept {
    return 0.7f;
  }

  inline
  float
  CellsBlocks::getThresholdForBlockSearch() noexcept {
    return 0.01f;
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
                              const utils::Vector2i& coord,
                              bool global) const
  {
    // Convert the input coordinate in the local block's cooordinate frame
    // if needed.
    utils::Vector2i local = coord;
    if (global) {
      local.x() = coord.x() - block.area.x() + block.area.w() / 2;
      local.y() = coord.y() - block.area.y() + block.area.h() / 2;
    }

    // Compute the one-dimensional index from this.
    return block.start + local.y() * block.area.w() + local.x();
  }

  inline
  utils::Vector2i
  CellsBlocks::coordFromIndex(const BlockDesc& block,
                              unsigned coord,
                              bool global) const
  {
    // Compute local `x` and `y` coordinates for the input index.
    int x = (coord - block.start) % block.area.w();
    int y = (coord - block.start) / block.area.w();

    utils::Vector2i pos(x, y);

    // Convert to global if needed.
    if (global) {
      pos.x() += block.area.getLeftBound();
      pos.y() += block.area.getBottomBound();
    }

    // Translate into vector semantic.
    return pos;
  }

  inline
  void
  CellsBlocks::updateCellsAge() noexcept {
    // Traverse the current states.
    for (unsigned id = 0u ; id < m_states.size() ; ++id) {
      if (m_states[id] == State::Alive) {
        ++m_ages[id];
      }
      else {
        m_ages[id] = 0;
      }
    }
  }

  inline
  void
  CellsBlocks::updateLiveArea() noexcept {
    int xMin = std::numeric_limits<int>::max();
    int yMin = std::numeric_limits<int>::max();
    int xMax = std::numeric_limits<int>::min();
    int yMax = std::numeric_limits<int>::min();

    // Traverse all blocks and update minimum and maximum values
    // for the live area.
    unsigned cnt = 0u;

    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      // Ignore inactive and dead blocks.
      if (!m_blocks[id].active || m_blocks[id].alive == 0u) {
        continue;
      }

      ++cnt;

      const BlockDesc& b = m_blocks[id];
      for (unsigned idC = b.start ; idC < b.end ; ++idC) {
        if (m_states[idC] == State::Alive) {
          utils::Vector2i c = coordFromIndex(b, idC, true);

          xMin = std::min(xMin, c.x());
          yMin = std::min(yMin, c.y());
          xMax = std::max(xMax, c.x());
          yMax = std::max(yMax, c.y());
        }
      }
    }

    // Account for cases when there's no active block left and
    // thus the live area can only be meaningless.
    if (cnt == 0u) {
      verbose("No active block registered in the colony, keeping old live area of " + m_liveArea.toString());
      return;
    }

    // Add one to the max bounds as we're using a bottom left
    // based box semantic: typically the cell at `(-1, 5)`
    // covers the real world pixels unit `(-1, 6)`.

    m_liveArea = utils::Boxf(
      1.0f * (xMin + xMax + 1) / 2.0f,
      1.0f * (yMin + yMax + 1) / 2.0f,
      1.0f * (xMax + 1 - xMin),
      1.0f * (yMax + 1 - yMin)
    );

    verbose("Live area is now " + m_liveArea.toString());
  }

}

#endif    /* CELLS_BLOCKS_HXX */
