#ifndef    CELLS_QUAD_TREE_NODE_HXX
# define   CELLS_QUAD_TREE_NODE_HXX

# include "CellsQuadTreeNode.hh"
# include <algorithm>

namespace cellulator {

  inline
  utils::Boxi
  CellsQuadTreeNode::getArea() const noexcept {
    return m_area;
  }

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

    // Update both the internal alive count and the adjacency values.
    // Note that we will have to indicate to the `updateAdjacencyFor`
    // method that we want to update the current adjacency.
    for (unsigned id = 0u ; id < m_cells.size() ; ++id) {
      State s = m_cells[id].randomize();

      bool alive = false;
      switch (s) {
        case State::Newborn:
        case State::Alive:
          ++m_aliveCount;
          alive = true;
          break;
        default:
          // Do not consider the cell alive by default.
          break;
      }

      updateAdjacencyFor(utils::Vector2i(id % m_area.w(), id / m_area.w()), alive, true);
    }
  }

  inline
  unsigned
  CellsQuadTreeNode::getAliveCellsCount() const noexcept {
    return m_aliveCount;
  }

  inline
  void
  CellsQuadTreeNode::step() {
    // If this node is not a leaf; call the adequate method for all children.
    m_aliveCount = 0u;

    if (!isLeaf()) {

      for (ChildrenMap::const_iterator it = m_children.cbegin() ;
           it != m_children.cend() ;
           ++it)
      {
        it->second->step();
        m_aliveCount += it->second->m_aliveCount;
      }

      return;
    }

    // If this node is a leaf, update all the internal cells.
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
                                const State& state,
                                bool allocate)
  {
    m_area = area;

    if (allocate) {
      // Create the internal array of cells.
      m_cells.resize(m_area.area(), Cell(state, m_ruleset));

      // Update the adjacency elements.
      m_adjacency.resize(m_area.area(), 0);
      m_nextAdjacency.resize(m_area.area(), 0);
    }
  }

  inline
  bool
  CellsQuadTreeNode::isRoot() const noexcept {
    return m_parent == nullptr;
  }

  inline
  bool
  CellsQuadTreeNode::isLeaf() const noexcept {
    return m_children.empty();
  }

  inline
  bool
  CellsQuadTreeNode::hasLiveCells() const noexcept {
    return getAliveCellsCount() > 0u;
  }

  inline
  bool
  CellsQuadTreeNode::isDead() const noexcept {
    return !hasLiveCells();
  }

  inline
  void
  CellsQuadTreeNode::collectBoundaries(std::vector<CellsQuadTreeNode*>& nodes,
                                       bool includeEmpty) noexcept
  {
    // If this quadtree node is a leaf, add it to the input list and return early.
    // We also care about the `includeEmpty` input boolean.
    if (isLeaf() && (hasLiveCells() || includeEmpty)) {
      nodes.push_back(this);

      return;
    }

    // Based on the role of this child in the parent, only collect boundaries on some
    // particular children. In the case of a root node (i.e. with no parent) all the
    // children can contain boundaries.
    for (ChildrenMap::const_iterator it = m_children.cbegin() ;
         it != m_children.cend() ;
         ++it)
    {
      bool collect = false;

      switch (it->first) {
        case Child::NorthEast:
          collect = (isRoot() || m_orientation != Child::SouthWest);
          break;
        case Child::NorthWest:
          collect = (isRoot() || m_orientation != Child::SouthEast);
          break;
        case Child::SouthEast:
          collect = (isRoot() || m_orientation != Child::NorthWest);
          break;
        case Child::SouthWest:
          collect = (isRoot() || m_orientation != Child::NorthEast);
          break;
        case Child::None:
        default:
          // Do nothing, unknown child role.
          log(
            "Cannot collect boundary on child " + it->second->getName() + ", unknown role " +
            std::to_string(static_cast<int>(it->first))
          );
          break;
      }

      if (collect && (it->second->hasLiveCells() || includeEmpty)) {
        it->second->collectBoundaries(nodes, includeEmpty);
      }
    }
  }

}

#endif    /* CELLS_QUAD_TREE_NODE_HXX */
