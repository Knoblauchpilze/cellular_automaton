
# include "CellsBlocks.hh"

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

    // Clamp dimensions to be at least a full block.
    utils::Sizei cDims(
      std::max(dims.w(), m_nodesDims.w()),
      std::max(dims.h(), m_nodesDims.h())
    );

    // Compute the number of blocks to allocate.
    unsigned bcW = static_cast<unsigned>(std::ceil(1.0f * cDims.w() / m_nodesDims.w()));
    unsigned bcH = static_cast<unsigned>(std::ceil(1.0f * cDims.h() / m_nodesDims.h()));

    // Derive the total size of the colony.
    utils::Boxi global(0, 0, bcW * m_nodesDims.w(), bcH * m_nodesDims.h());

    // Register each block.
    int minX = global.getLeftBound() + m_nodesDims.w() / 2;
    int minY = global.getBottomBound() + m_nodesDims.h() / 2;

    utils::Boxi area(0, 0, m_nodesDims);

    for (unsigned y = 0u ; y < bcH ; ++y) {
      area.y() = minY + y * m_nodesDims.h();

      for (unsigned x = 0u ; x < bcW ; ++x) {
        area.x() = minX + x * m_nodesDims.w();

        registerNewBlock(area, state);
      }
    }

    return global;
  }

  unsigned
  CellsBlocks::randomize() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    unsigned count = 0u;

    // Traverse all the existing nodes and randomize each one.
    for (unsigned id = 0u ; id < m_blocks.size() ; ++id) {
      makeRandom(m_blocks[id], getDeadCellProbability(), true);

      count += m_blocks[id].alive;
    }

    return count;
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
    m_blocks.push_back(block);

    return block;
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