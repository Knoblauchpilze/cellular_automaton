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

      for (unsigned id = 0u ; id < m_children.size() ; ++id) {
        m_children[id]->randomize();
        m_aliveCount += m_children[id]->getAliveCellsCount();
      }

      return;
    }

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
  CellsQuadTreeNode::initialize(const utils::Boxi& area,
                                const State& state)
  {
    m_area = area;

    // Create the internal array of cells.
    m_cells.resize(m_area.area(), Cell(state, m_ruleset));
  }

  inline
  bool
  CellsQuadTreeNode::isLeaf() const noexcept {
    return m_children.empty();
  }

}

#endif    /* CELLS_QUAD_TREE_NODE_HXX */
