#ifndef    CELLS_QUAD_TREE_NODE_HXX
# define   CELLS_QUAD_TREE_NODE_HXX

# include "CellsQuadTreeNode.hh"
# include <algorithm>

namespace cellulator {

  inline
  void
  CellsQuadTreeNode::randomize() {
    // Randomize each cell. In case children are available, we need
    // to call the randomization method on them.
    if (!isLeaf()) {
      m_aliveCount = 0u;

      for (ChildrenMap::const_iterator it = m_children.cbegin() ;
           it != m_children.cend() ;
           ++it)
      {
        it->second->randomize();
        m_aliveCount += it->second->getAliveCellsCount();
      }

      return;
    }

    // TODO: We should update the adjacency count.
    for (unsigned id = 0u ; id < m_cells.size() ; ++id) {
      State s = m_cells[id].randomize();

      switch (s) {
        case State::Newborn:
        case State::Alive:
          ++m_aliveCount;
          break;
        default:
          // Do not consider the cell alive by default.
          break;
      }
    }
  }

  inline
  unsigned
  CellsQuadTreeNode::getAliveCellsCount() noexcept {
    return m_aliveCount;
  }

  inline
  void
  CellsQuadTreeNode::step() {
    // If this node is not a leaf; call the adequate method for all children.
    if (!isLeaf()) {
      for (ChildrenMap::const_iterator it = m_children.cbegin() ;
           it != m_children.cend() ;
           ++it)
      {
        it->second->step();
      }

      return;
    }

    // If this node is a leaf, update all the internal cells.
    m_aliveCount = 0u;

    for (unsigned id = 0u ; id < m_cells.size() ; ++id) {
      State s = m_cells[id].step();

      if (s == State::Alive || s == State::Newborn) {
        ++m_aliveCount;
      }
    }

    // Swap the internal values for the adjacency.
    m_adjacency.swap(m_nextAdjacency);

    // Reset the next adjacency to allow for easy computations
    // in the next evolution.
    std::fill(m_nextAdjacency.begin(), m_nextAdjacency.end(), 0u);
  }

  inline
  void
  CellsQuadTreeNode::initialize(const utils::Boxi& area,
                                const State& state)
  {
    m_area = area;

    // Create the internal array of cells.
    m_cells.resize(m_area.area(), Cell(state, m_ruleset));

    // Update the adjacency elements.
    m_adjacency.resize(m_area.area(), 0);
    m_nextAdjacency.resize(m_area.area(), 0);
  }

  inline
  bool
  CellsQuadTreeNode::isLeaf() const noexcept {
    return m_children.empty();
  }

}

#endif    /* CELLS_QUAD_TREE_NODE_HXX */
