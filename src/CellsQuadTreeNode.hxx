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
  utils::Boxi
  CellsQuadTreeNode::getBoxForChild(const utils::Boxi& world,
                                    const Child& orientation) noexcept
  {
    // Distinguish based on the desired orientation.
    switch (orientation) {
      case Child::NorthWest:
        return utils::Boxi(world.x() - world.w() / 4, world.y() + world.h() / 4, world.w() / 2, world.h() / 2);
      case Child::NorthEast:
        return utils::Boxi(world.x() + world.w() / 4, world.y() + world.h() / 4, world.w() / 2, world.h() / 2);
      case Child::SouthWest:
        return utils::Boxi(world.x() - world.w() / 4, world.y() - world.h() / 4, world.w() / 2, world.h() / 2);
      case Child::SouthEast:
        return utils::Boxi(world.x() + world.w() / 4, world.y() - world.h() / 4, world.w() / 2, world.h() / 2);
      case Child::None:
      default:
        // Return the input area as a fallback behavior.
        break;
    }

    return world;
  }

  inline
  CellsQuadTreeNodeShPtr
  CellsQuadTreeNode::createChild(const Child& orientation,
                                 CellsQuadTreeNode* parent) noexcept
  {
    // Consistency check.
    if (parent == nullptr) {
      return nullptr;
    }

    // Create the child.
    CellsQuadTreeNodeShPtr child = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        getBoxForChild(parent->getArea(), orientation),
        parent->m_ruleset,
        parent,
        orientation,
        parent->m_minSize
      )
    );

    // Register it as a child of the parent.
    parent->m_children[orientation] = child;

    // Return the built-in child.
    return child;
  }

  inline
  void
  CellsQuadTreeNode::initialize(const utils::Boxi& area,
                                const State& state)
  {
    m_area = area;

    // Allocate the data if the area has the required dimensions
    // (i.e. if we can't subdivide this node any further).
    if (m_area.w() <= m_minSize.w() && m_area.h() <= m_minSize.h()) {
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
  CellsQuadTreeNode::isBoundary() const noexcept {
    // A node is a boundary if there exists a contiguous path of the same
    // direction from the root. Basically we are not allowed to move what
    // is called `backward` in the tree hierarchy.
    // For example any child whose parents all are `NorthWest` nodes is
    // a boundary element, but should any parent be either `NorthEast` or
    // `SouthEast` and it can't be a boundary.
    // Basically we want to check for continuous path to parents.
    return isRoot() || m_parent->validFor(m_orientation);
  }

  inline
  bool
  CellsQuadTreeNode::validFor(const Child& orientation) const noexcept {
    // Any path exists from the root directly.
    // The root is a boundary, along with all the level `1` nodes.
    if (isRoot()) {
      return true;
    }

    // Prevent any utilization of the `Child::None` value.
    if (orientation == Child::None) {
      return false;
    }

    // If the parent does not have a valid path for its own orientation
    // we can't find a path for this one.
    if (!isBoundary()) {
      return false;
    }

    // Check whether the parent is valid for the input orientation: indeed
    // the chain must remain valid if we cut any part of it.
    if (!m_parent->validFor(orientation)) {
      return false;
    }

    switch (m_orientation) {
      case Child::NorthWest:
        return orientation != Child::SouthEast;
      case Child::NorthEast:
        return orientation != Child::SouthWest;
      case Child::SouthWest:
        return orientation != Child::NorthEast;
      case Child::SouthEast:
        return orientation != Child::NorthWest;
      case Child::None:
      default:
        log(
          "Cannot determine path on element " + getName() + ", unknown role " +
          std::to_string(static_cast<int>(m_orientation))
        );
        break;

    }

    return false;
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
      if (it->second->isBoundary() && (it->second->hasLiveCells() || includeEmpty)) {
        it->second->collectBoundaries(nodes, includeEmpty);
      }
    }
  }

}

#endif    /* CELLS_QUAD_TREE_NODE_HXX */
