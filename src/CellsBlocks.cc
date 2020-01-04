
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

    m_blocks()
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
  CellsBlocks::allocateTo(const utils::Sizei& dims,
                          const State& state)
  {
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
    allocate(global, state);

    return global;
  }

  unsigned
  CellsBlocks::randomize() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Clear any existing data and save the current size so that we
    // can reallocate the colony with a similar size.
    // TODO: Fetch old area from the quadtree.
    utils::Boxi old;

    clear();

    // Allocate to reach the old size.
    allocate(old);

    unsigned count = 0u;

    // Traverse all the existing nodes and randomize each one.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      makeRandom(m_blocks[id], getDeadCellProbability(), true);

      count += m_blocks[id].alive;
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
  CellsBlocks::allocate(const utils::Boxi& area,
                        const State& state)
  {
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

    // Register each block.
    int minX = area.getLeftBound() + m_nodesDims.w() / 2;
    int minY = area.getBottomBound() + m_nodesDims.h() / 2;

    utils::Boxi lArea(0, 0, m_nodesDims);

    for (unsigned y = 0u ; y < bcH ; ++y) {
      lArea.y() = minY + y * m_nodesDims.h();

      for (unsigned x = 0u ; x < bcW ; ++x) {
        lArea.x() = minX + x * m_nodesDims.w();

        registerNewBlock(lArea, state);
      }
    }
  }

  bool
  CellsBlocks::find(const utils::Boxi& /*area*/,
                    BlockDesc& /*desc*/)
  {
    // TODO: Implementation.
    return false;
  }

  CellsBlocks::BlockDesc
  CellsBlocks::registerNewBlock(const utils::Boxi& area,
                                const State& state)
  {
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
      sizeOfBlock()
    };

    log("Created block " + std::to_string(id) + " for " + area.toString() + " (range: " + std::to_string(block.start) + " - " + std::to_string(block.end) + ")");

    // Allocate cells data if needed and reset the existing data.
    if (newB) {
      m_states.resize(block.end, state);
      m_adjacency.resize(block.end, 0u);

      m_nextStates.resize(block.end, state);
      m_nextAdjacency.resize(block.end, 0u);
    }
    else {
      std::fill(
        m_states.begin() + block.start,
        m_states.begin() + block.end,
        state
      );
    }

    // Update adjacency count.
    // TODO: Implementation.

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
    }

    return save;
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

    // TODO: Update adjacency.
  }

}
