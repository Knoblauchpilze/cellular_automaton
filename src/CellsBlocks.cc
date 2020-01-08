
# include "CellsBlocks.hh"
# include <numeric>
# include <unordered_set>
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

    // We want to generate data on all currently registered
    // nodes. In order to allow for the simulation to happen
    // smoothly though we need to also allocate the boundary
    // nodes of the current nodes in order to allow cells to
    // evolve.
    // As the `allocateBoundary` create some nodes we need
    // to register somehow the list of existing blocks in
    // order to process only these.
    std::unordered_set<int> blocks;
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      // If the block is active right now we can save it
      // and randomize it.
      if (m_blocks[id].active) {
        blocks.insert(m_blocks[id].id);
      }
    }

    // Allocate boundaries on nodes so that we can correctly
    // perform the evolution of the cells that will be created
    // during the randomization process.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      if (blocks.count(m_blocks[id].id) > 0) {
        allocateBoundary(m_blocks[id].id, true);
      }
    }

    // Randomize the list of nodes that already existed in the
    // colony (but not the newly created boundaries).
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      if (blocks.count(m_blocks[id].id) > 0) {
        makeRandom(m_blocks[id], getDeadCellProbability());

        count += m_blocks[id].alive;
      }
    }

    // Update live area to reflect the newly generated cells.
    stepPrivate();

    return count;
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

    // Handle cases where there were no changes in this block: we will just copy and
    // paste the value to the next generation.
    if (b.changed == 0u) {
      b.nAlive = b.alive;

      // Copy cells' states.
      for (unsigned id = b.start ; id < b.end ; ++id) {
        m_nextStates[id] = m_states[id];

        if (m_nextStates[id] == State::Alive) {
          updateAdjacency(b, coordFromIndex(b, id, false));
        }
      }

      return;
    }

    b.nAlive = 0u;

    // Evolve each cell.
    for (unsigned id = b.start ; id < b.end ; ++id) {
      State s = evolveCell(m_states[id], m_adjacency[id]);

      m_nextStates[id] = s;

      if (s == State::Alive) {
        ++b.nAlive;
        updateAdjacency(b, coordFromIndex(b, id, false));
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
      // Discard inactive blocks and only react when the block contains
      // the coordinate. Note however that as we're managing bottom left
      // based coordinates, the `m_blocks[id].area.getRightBound()` is
      // actually not considered to be in the area. So we can't use the
      // base `contains` method and instead focus on our method.
      // Similar reasoning apply to the top bound.
      if (m_blocks[id].active) {
        if (m_blocks[id].area.contains(coord) &&
            coord.x() < m_blocks[id].area.getRightBound() &&
            coord.y() < m_blocks[id].area.getTopBound())
        {
          int dataID = indexFromCoord(m_blocks[id], coord, true);

          out.first = m_states[dataID];
          out.second = m_ages[dataID];

          found = true;
        }
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

    log(
      "Created block " + std::to_string(id) + " for " + area.toString() +
      " (range: " + std::to_string(block.start) + " - " + std::to_string(block.end) + ")",
      utils::Level::Verbose
    );

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
      std::fill(m_nextAdjacency.begin() + block.start, m_nextAdjacency.begin() + block.end, 0u);
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

    // Attach this node to its neighbors.
    attach(block.id);

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

    log(
      "Destroying block " + std::to_string(blockID) +
      " (internal: " + std::to_string(m_blocks[blockID].id) + ") spanning " +
      m_blocks[blockID].area.toString(),
      utils::Level::Verbose
    );

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

      // Detach the block from neighbors.
      detach(blockID);
    }

    return save;
  }

  void
  CellsBlocks::updateAdjacency(const BlockDesc& block,
                               const utils::Vector2i& coord)
  {
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

          ++m_nextAdjacency[indexFromCoord(block, cell, false)];
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
            std::string("Could not update adjacency for " + cell.toString() + " (on behalf of " + coord.toString() + ", local: " + std::to_string(x) + "x" + std::to_string(y) + ") from block " + block.area.toString()),
            utils::Level::Error
          );

          continue;
        }

        ++m_nextAdjacency[indexFromCoord(*toUse, cell, false)];
      }
    }
  }

  void
  CellsBlocks::makeRandom(BlockDesc& desc,
                          float deadProb)
  {
    // Traverse the cells for this block and randomize each one.
    float prob = 0.0f;

    desc.nAlive = 0u;
    desc.changed = 0u;

    for (unsigned id = desc.start ; id < desc.end ; ++id) {
      prob = 1.0f * std::rand() / RAND_MAX;

      State s = State::Dead;
      if (prob >= deadProb) {
        s = State::Alive;
        ++desc.nAlive;
      }
      if (m_states[id] != s) {
        ++desc.changed;
      }

      m_ages[id] = 0u;
      m_nextStates[id] = s;
    }

    // Update adjacency for the cells of this block. We will start
    // by internal cells and then handle the borders.
    utils::Vector2i coord;

    for (unsigned id = desc.start ; id < desc.end ; ++id) {
      coord = coordFromIndex(desc, id, false);

      bool alive = (m_nextStates[id] == State::Alive);

      if (alive) {
        updateAdjacency(desc, coord);
      }
    }
  }

  bool
  CellsBlocks::allocateBoundary(unsigned blockID,
                                bool force) noexcept
  {
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
    if (b.alive == 0u && !force) {
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

      registerNewBlock(area);
    }

    if (b.north < 0) {
      area.x() = b.area.x();
      area.y() = b.area.y() + m_nodesDims.h();

      registerNewBlock(area);
    }

    if (b.nw < 0) {
      area.x() = b.area.x() - m_nodesDims.w();
      area.y() = b.area.y() + m_nodesDims.w();

      registerNewBlock(area);
    }

    if (b.west < 0) {
      area.x() = b.area.x() - m_nodesDims.w();
      area.y() = b.area.y();

      registerNewBlock(area);
    }

    if (b.sw < 0) {
      area.x() = b.area.x() - m_nodesDims.w();
      area.y() = b.area.y() - m_nodesDims.w();

      registerNewBlock(area);
    }

    if (b.south < 0) {
      area.x() = b.area.x();
      area.y() = b.area.y() - m_nodesDims.h();

      registerNewBlock(area);
    }

    if (b.se < 0) {
      area.x() = b.area.x() + m_nodesDims.w();
      area.y() = b.area.y() - m_nodesDims.w();

      registerNewBlock(area);
    }

    if (b.east < 0) {
      area.x() = b.area.x() + m_nodesDims.w();
      area.y() = b.area.y();

      registerNewBlock(area);
    }

    // At least one node has been created.
    return true;
  }

  bool
  CellsBlocks::find(const utils::Boxi& area,
                    int& desc)
  {
    // We need to use the `m_blocksIndex` table to find the block corresponding
    // to the input area if it exists.
    unsigned key = hashCoordinate(area.getCenter());
    AreaToBlockIndex::const_iterator it = m_blocksIndex.find(key);

    // Assume the block will not be found.
    desc = -1;

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

    desc = it->second;

    return true;
  }

  void
  CellsBlocks::attach(int from) {
    // Use the `find` method to attach all the other blocks to the
    // `from` element. We could save some computations by reusing
    // the blocks already attached to `to` if any but actually it
    // is more complex to put in place than just looking into the
    // `m_blocksIndex` table which is already quite fast anyways.
    // For each node we should both link it to the `from` node but
    // also link the other one to `from` node with opposite dir.
    BlockDesc& b = m_blocks[from];

    utils::Boxi area(0, 0, m_nodesDims);
    int o;

    // North east.
    area.x() = b.area.x() + m_nodesDims.w();
    area.y() = b.area.y() + m_nodesDims.w();

    bool f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to north east of " + b.area.toString(), utils::Level::Verbose);
      b.ne = m_blocks[o].id;
      m_blocks[o].sw = b.id;
    }

    // North.
    area.x() = b.area.x();
    area.y() = b.area.y() + m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to north of " + b.area.toString(), utils::Level::Verbose);
      b.north = m_blocks[o].id;
      m_blocks[o].south = b.id;
    }

    // North west.
    area.x() = b.area.x() - m_nodesDims.w();
    area.y() = b.area.y() + m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to north west of " + b.area.toString(), utils::Level::Verbose);
      b.nw = m_blocks[o].id;
      m_blocks[o].se = b.id;
    }

    // West.
    area.x() = b.area.x() - m_nodesDims.w();
    area.y() = b.area.y();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to west of " + b.area.toString(), utils::Level::Verbose);
      b.west = m_blocks[o].id;
      m_blocks[o].east = b.id;
    }

    // South west.
    area.x() = b.area.x() - m_nodesDims.w();
    area.y() = b.area.y() - m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to south west of " + b.area.toString(), utils::Level::Verbose);
      b.sw = m_blocks[o].id;
      m_blocks[o].ne = b.id;
    }

    // south.
    area.x() = b.area.x();
    area.y() = b.area.y() - m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to south of " + b.area.toString(), utils::Level::Verbose);
      b.south = m_blocks[o].id;
      m_blocks[o].north = b.id;
    }

    // South east.
    area.x() = b.area.x() + m_nodesDims.w();
    area.y() = b.area.y() - m_nodesDims.w();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to south east of " + b.area.toString(), utils::Level::Verbose);
      b.se = m_blocks[o].id;
      m_blocks[o].nw = b.id;
    }

    // East.
    area.x() = b.area.x() + m_nodesDims.w();
    area.y() = b.area.y();

    f = find(area, o);
    if (f) {
      log("Linking " + m_blocks[o].area.toString() + " to east of " + b.area.toString(), utils::Level::Verbose);
      b.east = m_blocks[o].id;
      m_blocks[o].west = b.id;
    }
  }

  void
  CellsBlocks::detach(int from) {
    // We assume that the `from` block is in a consistent state that is
    // all the blocks that point to it are also pointed at by it.
    // We just have to detach all the neighbors.
    BlockDesc& b = m_blocks[from];

    // North east.
    if (b.ne >= 0) {
      log("Unlinking " + m_blocks[b.ne].area.toString() + " at north east from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.ne].sw = -1;
      b.ne = -1;
    }

    // North.
    if (b.north >= 0) {
      log("Unlinking " + m_blocks[b.ne].area.toString() + " at north from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.north].south = -1;
      b.north = -1;
    }

    // North west.
    if (b.nw >= 0) {
      log("Unlinking " + m_blocks[b.nw].area.toString() + " at north west from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.nw].se = -1;
      b.nw = -1;
    }

    // West.
    if (b.west >= 0) {
      log("Unlinking " + m_blocks[b.west].area.toString() + " at west from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.west].east = -1;
      b.west = -1;
    }

    // South west.
    if (b.sw >= 0) {
      log("Unlinking " + m_blocks[b.sw].area.toString() + " at south west from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.sw].ne = -1;
      b.sw = -1;
    }

    // South.
    if (b.south >= 0) {
      log("Unlinking " + m_blocks[b.south].area.toString() + " at south from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.south].north = -1;
      b.south = -1;
    }

    // South east.
    if (b.se >= 0) {
      log("Unlinking " + m_blocks[b.se].area.toString() + " at south east from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.se].nw = -1;
      b.se = -1;
    }

    // East.
    if (b.east >= 0) {
      log("Unlinking " + m_blocks[b.east].area.toString() + " at east from " + b.area.toString(), utils::Level::Verbose);
      m_blocks[b.east].west = -1;
      b.east = -1;
    }
  }

  unsigned
  CellsBlocks::stepPrivate() {
    // We first need to evolve all the cells to their next state. This is
    // achieved by swapping the internal vectors, which is cheap and fast.
    m_states.swap(m_nextStates);

    // Update change count: this is computed by checking whether at least
    // one adjacency value has been updated.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      // Only handle active blocks.
      if (!m_blocks[id].active) {
        continue;
      }

      // Update change count: this is computed by checking whether at least
      // an adjacency value has been updated.
      m_blocks[id].changed = 0u;

      for (unsigned cell = m_blocks[id].start ; cell < m_blocks[id].end ; ++cell) {
        if (m_adjacency[cell] != m_nextAdjacency[cell]) {
          ++m_blocks[id].changed;
        }
      }
    }

    // Swap the adjacencies now that we're done checking for differences.
    m_adjacency.swap(m_nextAdjacency);
    std::fill(m_nextAdjacency.begin(), m_nextAdjacency.end(), 0u);

    // Update cells' age.
    updateCellsAge();

    // Now we need to update the alive count for each block.
    unsigned alive = 0u;

    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      // Only handle active blocks.
      if (!m_blocks[id].active) {
        continue;
      }

      m_blocks[id].alive = m_blocks[id].nAlive;
      alive += m_blocks[id].alive;

      unsigned neighbors = std::accumulate(
        m_adjacency.begin() + m_blocks[id].start,
        m_adjacency.begin() + m_blocks[id].end,
        0u
      );

      // Destroy the block if needed, that is if it does not contain any
      // cells and no neighbors are registered.
      if (m_blocks[id].alive == 0u && neighbors == 0u) {
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
      allocateBoundary(m_blocks[id].id, false);
    }

    return alive;
  }

}
