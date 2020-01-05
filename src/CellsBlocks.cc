
# include "CellsBlocks.hh"
# include "ColonyTile.hh"

namespace cellulator {

  CellsBlocks::CellsBlocks(const rules::Type& ruleset,
                           const utils::Sizei& nodeDims):
    utils::CoreObject(std::string("cells_blocks")),

    m_propsLocker(),

    m_ruleset(ruleset),
    m_nodesDims(nodeDims),

    m_states(),
    m_adjacency(),
    m_nextStates(),
    m_nextAdjacency(),
    m_ages(),

    m_blocks(),

    m_totalArea(),
    m_liveArea()
  {
    setService("blocks");

    // Check consistency.
    if (!m_nodesDims.valid()) {
      error(
        std::string("Could not allocate cells blocks"),
        std::string("Invalid nodes dimensions ") + m_nodesDims.toString()
      );
    }
  }

  utils::Boxi
  CellsBlocks::allocateTo(const utils::Sizei& dims) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Round the dimensions so that they fit perfectly the expected nodes size.
    // At least get a whole multiple of said dimensions.
    utils::Sizei cDims(
      dims.w() + (m_nodesDims.w() - dims.w() % m_nodesDims.w()) % m_nodesDims.w(),
      dims.h() + (m_nodesDims.h() - dims.h() % m_nodesDims.h()) % m_nodesDims.h()
    );

    utils::Boxi global(0, 0, cDims);

    // Allocate using the rectified dimensions.
    allocate(global);

    return global;
  }

  unsigned
  CellsBlocks::randomize() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // We want to randomize only currently active blocks.
    unsigned count = 0u;

    // Traverse all the existing nodes and randomize each one.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      if (m_blocks[id].active) {
        makeRandom(m_blocks[id], getDeadCellProbability(), true);

        count += m_blocks[id].alive;
      }
    }

    return count;
  }

  unsigned
  CellsBlocks::step() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // We first need to evolve all the cells to their next state. This is
    // achieved by swapping the internal vectors, which is cheap and fast.
    m_states.swap(m_nextStates);
    m_adjacency.swap(m_nextAdjacency);

    // Update cells' age.
    updateCellsAge();

    // We also need to reset the next adjacency count to `0` everywhere.
    std::fill(m_nextAdjacency.begin(), m_nextAdjacency.end(), 0u);

    // Now we need to update the alive count for each block.
    unsigned alive = 0u;

    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      // Only handle active blocks.
      if (!m_blocks[id].active) {
        continue;
      }

      m_blocks[id].alive = m_blocks[id].nAlive;
      alive += m_blocks[id].alive;
    }

    // Now we should expand and create new blocks to account for cells that
    // might overflow the current state of the colony.
    // TODO: Should handle expansion.

    return alive;
  }

  void
  CellsBlocks::generateSchedule(std::vector<ColonyTileShPtr>& tiles) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // We want to generate colony tiles for all active blocks.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      // Only handle this block if it is active.
      if (m_blocks[id].active) {
        tiles.push_back(
          std::make_shared<ColonyTile>(
            m_blocks[id].area,
            this,
            ColonyTile::Type::Interior
          )
        );
      }
    }
  }

  void
  CellsBlocks::fetchCells(std::vector<State>& cells,
                          const utils::Boxi& area)
  {
    // Reset with dead cells as to not display some randomness.
    std::fill(cells.begin(), cells.end(), State::Dead);

    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Traverse the blocks and fill in any element requested from
    // the input area.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      const BlockDesc& b = m_blocks[id];

      // Check whether this block intersect the input area.
      // We don't actually explicitly check for intersection
      // but rather we count on the fact that a block which
      // does not intersect will yield an empty area to fetch
      // cells into.
      int gXMin = area.getLeftBound();
      int gYMin = area.getBottomBound();
      int gXMax = area.getRightBound();
      int gYMax = area.getTopBound();

      int lXMin = b.area.getLeftBound();
      int lYMin = b.area.getBottomBound();
      int lXMax = b.area.getRightBound();
      int lYMax = b.area.getTopBound();

      int xMin = std::max(gXMin, lXMin);
      int yMin = std::max(gYMin, lYMin);
      int xMax = std::min(gXMax, lXMax);
      int yMax = std::min(gYMax, lYMax);

      int uB = static_cast<int>(b.end - b.start);

      for (int y = yMin ; y < yMax ; ++y) {
        // Convert logical coordinates to valid cells coordinates.
        int offset = (y - gYMin) * area.w();
        int rOffset = (y - lYMin) * b.area.w();

        for (int x = xMin ; x < xMax ; ++x) {
          // Convert the `x` coordinate similarly to the `y` coordinate.
          int xOff = x - gXMin;
          int rXOff = x - lXMin;

          // Check whether the cell exists in the internal data. If this
          // is the case we assign it, otherwise we don't modify the value.
          int coord = rOffset + rXOff;
          if (rOffset >= 0 && rOffset < uB &&
              rXOff >= 0 && rXOff < b.area.w())
          {
            cells[offset + xOff] = m_states[b.start + coord];
          }
        }
      }
    }
  }

  void
  CellsBlocks::allocate(const utils::Boxi& area) {
    // Compute the number of blocks to allocate.
    unsigned bcW = static_cast<unsigned>(std::ceil(1.0f * area.w() / m_nodesDims.w()));
    unsigned bcH = static_cast<unsigned>(std::ceil(1.0f * area.h() / m_nodesDims.h()));

    if (area.w() % m_nodesDims.w() != 0 || area.h() % m_nodesDims.h()) {
      log(
        std::string("Trying to allocate colony with dimensions ") + area.toString() +
        " not fitting internal node dimensions of " + m_nodesDims.toString(),
        utils::Level::Error
      );
    }

    // Prevent empty areas.
    if (!area.valid()) {
      error(
        std::string("Could not allocate cells data"),
        std::string("Invalid area ") + area.toString()
      );
    }

    // Register each block.
    int minX = area.getLeftBound() + m_nodesDims.w() / 2;
    int minY = area.getBottomBound() + m_nodesDims.h() / 2;

    utils::Boxi lArea(0, 0, m_nodesDims);

    for (unsigned y = 0u ; y < bcH ; ++y) {
      lArea.y() = minY + y * m_nodesDims.h();

      for (unsigned x = 0u ; x < bcW ; ++x) {
        lArea.x() = minX + x * m_nodesDims.w();

        registerNewBlock(lArea);
      }
    }

    // Assign the internal areas.
    m_totalArea = area;
    m_liveArea = utils::Boxi(m_totalArea.getCenter(), 0, 0);
  }

  bool
  CellsBlocks::find(const utils::Boxi& /*area*/,
                    BlockDesc& /*desc*/)
  {
    // TODO: Implementation.
    return false;
  }

  CellsBlocks::BlockDesc
  CellsBlocks::registerNewBlock(const utils::Boxi& area) {
    // Check whether some already instantiated blocks exist.
    unsigned id = m_blocks.size();

    bool newB = true;
    if (!m_freeBlocks.empty()) {
      id = m_freeBlocks.back();
      m_freeBlocks.pop_back();
      newB = false;
    }

    unsigned s = dataIDFromBlock(id);

    BlockDesc block{
      id,

      area,
      s,
      s + sizeOfBlock(),

      true,
      0u,
      0u,
      sizeOfBlock(),

      -1,
      -1,
      -1,
      -1
    };

    log("Created block " + std::to_string(id) + " for " + area.toString() + " (range: " + std::to_string(block.start) + " - " + std::to_string(block.end) + ")");

    // Allocate cells data if needed and reset the existing data.
    if (newB) {
      m_states.resize(block.end, State::Dead);
      m_adjacency.resize(block.end, 0u);
      m_ages.resize(block.end);

      m_nextStates.resize(block.end, State::Dead);
      m_nextAdjacency.resize(block.end, 0u);
    }
    else {
      std::fill(m_states.begin() + block.start, m_states.begin() + block.end, State::Dead);
      std::fill(m_ages.begin() + block.start, m_ages.end() + block.end, 0);
      std::fill(m_adjacency.begin() + block.start, m_adjacency.begin() + block.end, 0u);
    }

    // Register the block and return it.
    if (newB) {
      m_blocks.push_back(block);
    }
    else {
      m_blocks[block.id] = block;
    }

    return block;
  }

  bool
  CellsBlocks::destroyBlock(unsigned blockID) {
    // Check whether the speciifed block exists.
    if (blockID >= m_blocks.size()) {
      log(
        std::string("Could not destroy block ") + std::to_string(blockID) + ", only " + std::to_string(m_blocks.size()) + " registered",
        utils::Level::Error
      );

      return false;
    }

    bool save = m_blocks[blockID].active;
    if (save) {
      // Deactivate the block and register it in the list of
      // available ones.
      m_blocks[blockID].active = false;
      m_freeBlocks.push_back(blockID);

      // Deactivate some properties to be on the safe side.
      m_blocks[blockID].alive = 0u;
      m_blocks[blockID].nAlive = 0u;
      m_blocks[blockID].changed = 0u;
    }

    return save;
  }

  void
  CellsBlocks::updateAdjacency(const BlockDesc& /*block*/,
                               const utils::Vector2i& /*coord*/,
                               bool /*makeCurrent*/)
  {
    // TODO: Implementation.
  }

  void
  CellsBlocks::makeRandom(BlockDesc& desc,
                          float deadProb,
                          bool makeCurrent)
  {
    // Traverse the cells for this block and randomize each one.
    float prob = 0.0f;

    desc.alive = 0u;
    desc.changed = desc.end - desc.start;

    for (unsigned id = desc.start ; id < desc.end ; ++id) {
      prob = 1.0f * std::rand() / RAND_MAX;

      State s = State::Dead;
      if (prob >= deadProb) {
        s = State::Alive;
        ++desc.alive;
      }

      if (makeCurrent) {
        m_states[id] = s;
      }
      else {
        m_nextStates[id] = s;
      }
    }

    // Update adjacency for the cells of this block. We will start
    // by internal cells and then handle the borders.
    utils::Vector2i coord;
    unsigned w = desc.area.w();

    for (unsigned id = desc.start ; id < desc.end ; ++id) {
      coord.x() = id % w;
      coord.y() = id / w;

      updateAdjacency(desc, coord, makeCurrent);
    }
  }

}
