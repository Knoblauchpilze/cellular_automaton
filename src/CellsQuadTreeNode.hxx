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
    if (!m_children.empty()) {
      for (unsigned id = 0u ; id < m_children.size() ; ++id) {
        m_children[id]->randomize();
      }

      return;
    }

    std::for_each(
      m_cells.begin(),
      m_cells.end(),
      [](Cell& c) {
        c.randomize();
      }
    );
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

}

#endif    /* CELLS_QUAD_TREE_NODE_HXX */
