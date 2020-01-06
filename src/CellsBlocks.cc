
# include "CellsBlocks.hh"
# include "ColonyTile.hh"

namespace {

  inline
  cellulator::State
  evolveCell(const cellulator::State& s,
             unsigned live)
  {
    if (s == cellulator::State::Dead && live == 3u) {
      return cellulator::State::Alive;
    }

    if (s == cellulator::State::Alive && live >= 2 && live <= 3) {
      return cellulator::State::Alive;
    }

    return cellulator::State::Dead;
  }

  inline
  unsigned
  hashCoordinate(const utils::Vector2i& v) {
    unsigned A = (v.x() >= 0 ? 2u * v.x() : -2 * v.x() - 1);
    unsigned B = (v.y() >= 0 ? 2u * v.y() : -2 * v.y() - 1);

    unsigned C = (A >= B ? A * A + A + B : A + B * B) / 2;

    return ((v.x() < 0 && v.y() < 0) || (v.x() >= 0 && v.y() >= 0)) ? C : -C - 1;
  }

}

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
    m_adjacencyLocker(),
    m_ages(),

    m_blocks(),
    m_freeBlocks(),
    m_blocksIndex(),

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

    // Update live area to reflect the newly generated cells.
    updateLiveArea();

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

      // Destroy the block if needed.
      if (m_blocks[id].alive == 0u) {
        destroyBlock(m_blocks[id].id);
      }
    }

    // Update live area to reflect the new states of cells.
    updateLiveArea();

    // Now we should expand and create new blocks to account for cells that
    // might overflow the current state of the colony.
    // In order to do that we need to scan the internal list of blocks and
    // check for each one whether it is a boundary and if it risks to need
    // some other elements in the next generation.
    // As we will perform some heavy recycling and allocation while we are
    // travsersing the list it might be a problem. However we know that:
    //  - we can only create nodes at the end of the queue or on a place
    //    where we already have a block allocated but marked inactive.
    //  - all created blocks will be empty at first.
    // These considerations make it possible to only iterate once through
    // the internal list of blocks and call it a day.
    // We will possibly iterate on blocks which have just been created but
    // this should be fast as they are clearly not active.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      allocateBoundary(id);
    }

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
            m_blocks[id].id,
            this
          )
        );
      }
    }
  }

  void
  CellsBlocks::evolve(unsigned blockID) {
    // Retrieve the block's description.
    BlockDesc& b = m_blocks[blockID];

    b.nAlive = 0u;

    // Evolve each cell.
    for (unsigned id = b.start ; id < b.end ; ++id) {
      State s = evolveCell(m_states[id], m_adjacency[id]);

      m_nextStates[id] = s;

      log("Cell " + coordFromIndex(b, id - b.start, true).toString() + " has " + std::to_string(m_adjacency[id]) + " neighbor(s) and is now " + std::to_string(static_cast<int>(s)));

      if (s == State::Alive) {
        ++b.nAlive;
        updateAdjacency(b, coordFromIndex(b, id - b.start, false), false);
      }
    }
  }

  std::pair<State, int>
  CellsBlocks::getCellStatus(const utils::Vector2i& coord) {
    // Protect from concurrent access.
    Guard guard(m_propsLocker);

    std::pair<State, int> out = std::make_pair(State::Dead, -1);

    // Eliminate trivial cases where the input coordinate are not in the
    // live area of the colony.
    if (!m_liveArea.contains(utils::Vector2f(1.0f * coord.x(), 1.0f * coord.y()))) {
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
        int dataID = indexFromCoord(m_blocks[id], coord, true);

        out.first = m_states[dataID];
        out.second = m_ages[dataID];

        found = true;
      }

      ++id;
    }

    return out;
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
    
    // TODO: Handle neighboring of nodes.

    // Assign the internal areas.
    m_totalArea = area;
    m_liveArea = utils::Boxf(1.0f * m_totalArea.x(), 1.0f * m_totalArea.y(), 0.0f, 0.0f);
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
      -1,

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
      m_ages.resize(block.end, 0);

      m_nextStates.resize(block.end, State::Dead);
      m_nextAdjacency.resize(block.end, 0u);
    }
    else {
      std::fill(m_states.begin() + block.start, m_states.begin() + block.end, State::Dead);
      std::fill(m_ages.begin() + block.start, m_ages.begin() + block.end, 0);
      std::fill(m_adjacency.begin() + block.start, m_adjacency.begin() + block.end, 0u);
    }

    // Register the block and return it.
    if (newB) {
      m_blocks.push_back(block);
    }
    else {
      m_blocks[block.id] = block;
    }

    // Also register the block in the table allowing to fetch by area.
    // Note that it is enough to use the center of the area of the block
    // to compute a key because we should only ever have a single block
    // representing the area. As we're quite rigid in the conditions that
    // lead to create a block we can control that and thus make sure that
    // we only ever create one block spanning any point of the colony.
    // This fact guarantees that the only way we have for two blocks to
    // overlap is for them to be the same.
    unsigned key = hashCoordinate(area.getCenter());
    AreaToBlockIndex::const_iterator it = m_blocksIndex.find(key);

    if (it != m_blocksIndex.cend()) {
      log(
        std::string("Overriding key ") + std::to_string(key) + " (associated to " + m_blocks[it->second].area.toString() +
        " with " + area.toString(),
        utils::Level::Error
      );
    }

    m_blocksIndex[key] = block.id;

    // Return the created block.
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

    log("Destroying block " + std::to_string(blockID) + " (internal: " + std::to_string(m_blocks[blockID].id) + ") spanning " + m_blocks[blockID].area.toString());

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

      // Finally unregister its key from the internal table.
      unsigned key = hashCoordinate(m_blocks[blockID].area.getCenter());
      AreaToBlockIndex::const_iterator it = m_blocksIndex.find(key);

      if (it == m_blocksIndex.cend()) {
        log(
          std::string("Could not remove block ") + m_blocks[blockID].area.toString() + " from association table",
          utils::Level::Error
        );
      }
      else {
        m_blocksIndex.erase(it);
      }
    }

    return save;
  }

  void
  CellsBlocks::updateAdjacency(const BlockDesc& block,
                               const utils::Vector2i& coord,
                               bool makeCurrent)
  {
    // Convert global coord to local coordinate frame.
    utils::Vector2i lCoord(coord.x() - block.area.x(), coord.y() - block.area.y());

    // Check whether the input coordinate corresponds to a cell
    // on the boundary of the node.
    if (coord.x() > 1 && coord.x() < block.area.w() - 2 &&
        coord.y() > 1 && coord.y() < block.area.h() - 2)
    {
      // Interior cell, we can just update the adjacency values
      // without worrying about multithreading.
      int xMin = coord.x() - 1;
      int yMin = coord.y() - 1;
      int xMax = coord.x() + 1;
      int yMax = coord.y() + 1;

      utils::Vector2i cell;

      for (int y = yMin ; y <= yMax ; ++y) {
        for (int x = xMin ; x <= xMax ; ++x) {
          // Ignore the cell for which adjacency should be updated.
          if (x == coord.x() && y == coord.y()) {
            continue;
          }

          cell.x() = x;
          cell.y() = y;

          if (makeCurrent) {
            ++m_adjacency[indexFromCoord(block, cell, false)];
          }
          else {
            ++m_nextAdjacency[indexFromCoord(block, cell, false)];
          }
        }
      }

      // We're done.
      return;
    }

    // Acquire the lock on the mutex protecting adjacency arrays.
    Guard guard(m_adjacencyLocker);

    // We will follow a standard process, the only thing is that we
    // need to find the correct block based on whether the adjacency
    // to update is in the input `block` or in one of the neighors.
    // To prepare the work we will fetch the blocks beforehand.
    const BlockDesc* e = (block.east >= 0 ? &m_blocks[block.east] : nullptr);
    const BlockDesc* w = (block.west >= 0 ? &m_blocks[block.west] : nullptr);
    const BlockDesc* s = (block.south >= 0 ? &m_blocks[block.south] : nullptr);
    const BlockDesc* n = (block.north >= 0 ? &m_blocks[block.north] : nullptr);

    const BlockDesc* ne = (block.ne >= 0 ? &m_blocks[block.ne] : nullptr);
    const BlockDesc* nw = (block.nw >= 0 ? &m_blocks[block.nw] : nullptr);
    const BlockDesc* sw = (block.sw >= 0 ? &m_blocks[block.sw] : nullptr);
    const BlockDesc* se = (block.se >= 0 ? &m_blocks[block.se] : nullptr);

    int xMin = coord.x() - 1;
    int yMin = coord.y() - 1;
    int xMax = coord.x() + 1;
    int yMax = coord.y() + 1;

    utils::Vector2i cell;

    int uBW = block.area.w();
    int uBH = block.area.h();

    for (int y = yMin ; y <= yMax ; ++y) {
      for (int x = xMin ; x <= xMax ; ++x) {
        // Do not update the cell itself.
        if (x == coord.x() && y == coord.y()) {
          continue;
        }

        cell.x() = (x + uBW) % uBW;
        cell.y() = (y + uBH) % uBH;

        // Determine which block should be used.
        const BlockDesc* toUse = nullptr;
        bool okX = (x >= 0 && x < uBW);
        bool okY = (y >= 0 && y < uBH);

        if (okX) {
          if (okY) {
            toUse = &block;
          }
          else if (y < 0) {
            toUse = s;
          }
          else {
            // Only `y > uBH` remaining.
            toUse = n;
          }
        }
        else if (x < 0) {
          if (okY) {
            toUse = w;
          }
          else if (y < 0) {
            toUse = sw;
          }
          else {
            // Only `y > uBH` remaining.
            toUse = nw;
          }
        }
        else {
          // Only `x > uBW` remaining.
          if (okY) {
            toUse = e;
          }
          else if (y < 0) {
            toUse = se;
          }
          else {
            // Only `y > uBH` remaining.
            toUse = ne;
          }
        }

        if (toUse == nullptr) {
          log(
            std::string("Could not update adjacency for " + cell.toString() + " (on behalf of " + coord.toString() + ") from block " + block.area.toString()),
            utils::Level::Error
          );

          continue;
        }

        if (makeCurrent) {
          ++m_adjacency[indexFromCoord(*toUse, cell, false)];
        }
        else {
          ++m_nextAdjacency[indexFromCoord(*toUse, cell, false)];
        }
      }
    }
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
        if (makeCurrent) {
          ++desc.alive;
        }
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

    for (unsigned id = desc.start ; id < desc.end ; ++id) {
      coord = coordFromIndex(desc, id - desc.start, false);

      bool alive = false;
      if (makeCurrent) {
        alive = (m_states[id] == State::Alive);
      }
      else {
        alive = (m_nextStates[id] == State::Alive);
      }

      if (alive) {
        updateAdjacency(desc, coord, makeCurrent);
      }
    }
  }

  bool
  CellsBlocks::allocateBoundary(unsigned blockID) noexcept {
    BlockDesc b = m_blocks[blockID];

    // Discard inactive blocks.
    if (!b.active) {
      return false;
    }

    // Check whether this block is a border.
    if (b.east >= 0 && b.west >= 0 && b.south >= 0 && b.north >= 0 &&
        b.ne >= 0 && b.nw >= 0 && b.se >= 0 && b.sw >= 0)
    {
      // The block is not a boundary.
      return false;
    }

    // Allocate any missing children if needed: that is if the node
    // has at least one live cell.
    if (b.alive == 0u) {
      return false;
    }

    // For each node to create we need to compute its associated area
    // which can be done using the current area of the node with some
    // offset.
    // What we know is that any area will have dimensions matching the
    // `m_nodesDims` attribute.
    utils::Boxi area(0, 0, m_nodesDims);

    if (b.ne < 0) {
      area.x() = b.area.x() + m_nodesDims.w();
      area.y() = b.area.y() + m_nodesDims.w();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::SouthWest);
    }

    if (b.north < 0) {
      area.x() = b.area.x();
      area.y() = b.area.y() + m_nodesDims.h();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::South);
    }

    if (b.nw < 0) {
      area.x() = b.area.x() - m_nodesDims.w();
      area.y() = b.area.y() + m_nodesDims.w();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::SouthEast);
    }

    if (b.west < 0) {
      area.x() = b.area.x() - m_nodesDims.w();
      area.y() = b.area.y();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::East);
    }

    if (b.sw < 0) {
      area.x() = b.area.x() - m_nodesDims.w();
      area.y() = b.area.y() - m_nodesDims.w();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::NorthEast);
    }

    if (b.south < 0) {
      area.x() = b.area.x();
      area.y() = b.area.y() - m_nodesDims.h();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::North);
    }

    if (b.se < 0) {
      area.x() = b.area.x() + m_nodesDims.w();
      area.y() = b.area.y() - m_nodesDims.w();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::NorthWest);
    }

    if (b.east < 0) {
      area.x() = b.area.x() + m_nodesDims.w();
      area.y() = b.area.y();

      BlockDesc o = registerNewBlock(area);
      attachTo(b, o, Orientation::West);
    }

    // At least one node has been created.
    return true;
  }

  bool
  CellsBlocks::find(const utils::Boxi& area,
                    BlockDesc& desc)
  {
    // We need to use the `m_blocksIndex` table to find the block corresponding
    // to the input area if it exists.
    unsigned key = hashCoordinate(area.getCenter());
    AreaToBlockIndex::const_iterator it = m_blocksIndex.find(key);

    if (it == m_blocksIndex.cend()) {
      // The input area is not registered yet, we need to indicate that the block
      // does not exist.
      return false;
    }

    // The block exists, let's perform some consistency checks and return its
    // description if we can.
    if (it->second >= m_blocks.size()) {
      log(
        std::string("Found block ") + area.toString() + " at " + std::to_string(it->second) + " but only " +
        std::to_string(m_blocks.size()) + " block(s) available",
        utils::Level::Error
      );

      // Remove this faulty entry.
      m_blocksIndex.erase(it);

      // We did not find the block after all.
      return false;
    }

    desc = m_blocks[it->second];

    return true;
  }

  void
  CellsBlocks::attachTo(BlockDesc& from,
                        BlockDesc& to,
                        const Orientation& orientation)
  {
    // Convenience lambda to attach a block to another.
    auto attach = [](BlockDesc& lhs, BlockDesc& rhs, const Orientation& o){
      switch (o) {
        case Orientation::NorthEast:
          lhs.ne = rhs.id;
          break;
        case Orientation::North:
          lhs.north = rhs.id;
          break;
        case Orientation::NorthWest:
          lhs.nw = rhs.id;
          break;
        case Orientation::West:
          lhs.west = rhs.id;
          break;
        case Orientation::SouthWest:
          lhs.sw = rhs.id;
          break;
        case Orientation::South:
          lhs.south = rhs.id;
          break;
        case Orientation::SouthEast:
          lhs.se = rhs.id;
          break;
        case Orientation::East:
          lhs.east = rhs.id;
          break;
        default:
          return false;
      }

      return true;
    };

    // First, attach the `from` to `to` with the desired orientation.
    bool check = attach(to, from, orientation);
    if (!check) {
      error(
        std::string("Could not attach block " + std::to_string(to.id) + " to " + std::to_string(from.id)),
        std::string("Invalid return code")
      );
    }

    // Use the `find` method to attach all the other blocks to the
    // `from` element. We could save some computations by reusing
    // the blocks already attached to `to` if any but actually it
    // is more complex to put in place than just looking into the
    // `m_blocksIndex` table which is already quite fast anyways.
    // For each node we should both link it to the `from` node but
    // also link the other one to `from` node with opposite dir.
    utils::Boxi area(0, 0, m_nodesDims);
    BlockDesc o;

    // North east.
    area.x() = from.area.x() + m_nodesDims.w();
    area.y() = from.area.y() + m_nodesDims.w();

    bool f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to north east of " + o.area.toString());
      from.ne = o.id;
      o.sw = from.id;
    }

    // North.
    area.x() = from.area.x();
    area.y() = from.area.y() + m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to north of " + o.area.toString());
      from.north = o.id;
      o.south = from.id;
    }

    // North west.
    area.x() = from.area.x() - m_nodesDims.w();
    area.y() = from.area.y() + m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to north west of " + o.area.toString());
      from.nw = o.id;
      o.se = from.id;
    }

    // West.
    area.x() = from.area.x() - m_nodesDims.w();
    area.y() = from.area.y();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to west of " + o.area.toString());
      from.west = o.id;
      o.east = from.id;
    }

    // South west.
    area.x() = from.area.x() - m_nodesDims.w();
    area.y() = from.area.y() - m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to south west of " + o.area.toString());
      from.sw = o.id;
      o.ne = from.id;
    }

    // south.
    area.x() = from.area.x();
    area.y() = from.area.y() - m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to south of " + o.area.toString());
      from.south = o.id;
      o.north = from.id;
    }

    // South east.
    area.x() = from.area.x() + m_nodesDims.w();
    area.y() = from.area.y() - m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to south east of " + o.area.toString());
      from.se = o.id;
      o.nw = from.id;
    }

    // East.
    area.x() = from.area.x() + m_nodesDims.w();
    area.y() = from.area.y();

    f = find(area, o);
    if (f) {
      log("Linking " + o.area.toString() + " to east of " + o.area.toString());
      from.east = o.id;
      o.west = from.id;
    }
  }

}
