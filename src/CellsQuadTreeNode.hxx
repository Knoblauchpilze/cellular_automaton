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
                                    const borders::Name& direction) noexcept
  {
    // Distinguish based on the desired direction.
    switch (direction) {
      case borders::Name::NorthWest:
        return utils::Boxi(world.x() - world.w() / 4, world.y() + world.h() / 4, world.w() / 2, world.h() / 2);
      case borders::Name::NorthEast:
        return utils::Boxi(world.x() + world.w() / 4, world.y() + world.h() / 4, world.w() / 2, world.h() / 2);
      case borders::Name::SouthWest:
        return utils::Boxi(world.x() - world.w() / 4, world.y() - world.h() / 4, world.w() / 2, world.h() / 2);
      case borders::Name::SouthEast:
        return utils::Boxi(world.x() + world.w() / 4, world.y() - world.h() / 4, world.w() / 2, world.h() / 2);
      case borders::Name::None:
      default:
        // Return the input area as a fallback behavior.
        break;
    }

    return world;
  }

  inline
  CellsQuadTreeNodeShPtr
  CellsQuadTreeNode::createChild(const borders::Name& direction,
                                 CellsQuadTreeNode* parent) noexcept
  {
    // Consistency check.
    if (parent == nullptr) {
      return nullptr;
    }

    // Create the child.
    CellsQuadTreeNodeShPtr child = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        getBoxForChild(parent->getArea(), direction),
        parent->m_ruleset,
        parent,
        direction,
        parent->m_minSize
      )
    );

    // Register it as a child of the parent.
    parent->m_children[direction] = child;

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

    // Update the orientation for this node.
    assignOrientationFromDirection();
  }

  inline
  void
  CellsQuadTreeNode::assignOrientationFromDirection() {
    // The root node has a full orientation.
    if (isRoot()) {
      m_orientation.set(borders::Direction::East);
      m_orientation.set(borders::Direction::West);
      m_orientation.set(borders::Direction::North);
      m_orientation.set(borders::Direction::South);

      return;
    }

    // Start with the orientation of the parent.
    m_orientation = m_parent->m_orientation;

    // Unset the bit based on the direction of the child relatively
    // to its parent.
    switch (m_direction) {
      case borders::Name::NorthEast:
        m_orientation.unset(borders::Direction::South);
        m_orientation.unset(borders::Direction::West);
        break;
      case borders::Name::NorthWest:
        m_orientation.unset(borders::Direction::South);
        m_orientation.unset(borders::Direction::East);
        break;
      case borders::Name::SouthEast:
        m_orientation.unset(borders::Direction::North);
        m_orientation.unset(borders::Direction::West);
        break;
      case borders::Name::SouthWest:
        m_orientation.unset(borders::Direction::North);
        m_orientation.unset(borders::Direction::East);
        break;
      case borders::Name::None:
      default:
        break;
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
    // direction from the root. This is kept by the `m_orientation` attribute
    // so we can just check it: if it is not empty it means that the path
    // corresponding to the associated direction is continuous from the root
    // and thus that it is a border.
    return isRoot() || !m_orientation.empty();
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
    if (isLeaf() && (hasLiveCells() || includeEmpty) && isBoundary()) {
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

  inline
  bool
  CellsQuadTreeNode::attach(CellsQuadTreeNodeShPtr child,
                            const borders::Name& direction) noexcept
  {
    // Check consistency.
    if (child == nullptr) {
      log(std::string("Trying to attach invalid null child to ") + getName());
      return false;
    }

    // Check whether the parent already has a child with the specified direction.
    ChildrenMap::const_iterator ex = m_children.find(direction);
    if (ex != m_children.cend()) {
      log(
        std::string("Could not attach ") + child->getName() + " to " + getName() +
        ", child " + ex->second->getName() + " is already occuying the spot",
        utils::Level::Error
      );

      return false;
    }

    // Attach the child to this node.
    bool check = child->attachTo(this, direction);
    if (!check) {
      log(
        std::string("Could not attach ") + child->getName() + " to " + getName(),
        utils::Level::Error
      );

      return false;
    }

    // Register this child in the internal list.
    m_children[direction] = child;

    return true;
  }

  inline
  bool
  CellsQuadTreeNode::attachTo(CellsQuadTreeNode* parent,
                              const borders::Name& direction) noexcept
  {
    // Check consistency.
    if (parent == nullptr) {
      log(std::string("Trying to attach ") + getName() + " to invalid null parent", utils::Level::Error);
      return false;
    }

    // Detach from current parent.
    if (m_parent != nullptr) {
      log(
        std::string("Reparenting ") + getName() + " from " + m_parent->getName() + " to " + parent->getName(),
        utils::Level::Verbose
      );

      std::size_t check = m_parent->m_children.erase(m_direction);

      if (check == 0u) {
        log(
          std::string("Could not find child ") + getName() + " in parent " + m_parent->getName(),
          utils::Level::Error
        );
      }

      // Decrement the alive count with the count brought by this node.
      m_parent->m_aliveCount -= m_aliveCount;
    }

    // Attach to the new parent and reset information.
    m_parent = parent;
    m_parent->m_aliveCount += m_aliveCount;
    m_direction = direction;
    assignOrientationFromDirection();

    return true;
  }

  inline
  void
  CellsQuadTreeNode::createSiblings(CellsQuadTreeNode* child) {
    // Speed up.
    if (isLeaf()) {
      return;
    }
    if (!m_area.contains(child->m_area)) {
      return;
    }

    // Check whether we could find the input node as a child of this
    // element. If this is the case, we need to create the siblings
    // of it. Otherwise we need to transmit the data to the child
    // containing it.
    CellsQuadTreeNode* n = nullptr;
    ChildrenMap::const_iterator it = m_children.cbegin();

    while (it != m_children.cend() && it->second.get() != child) {
      // Save the child containing the box of the input element.
      if (it->second->m_area.contains(child->m_area)) {
        n = it->second.get();
      }

      ++it;
    }

    // Check whether we could find the input node in the children
    // here.
    if (it == m_children.cend()) {
      // Transmit to the best child if any.
      if (n == nullptr) {
        log(
          std::string("Could not create sibling for ") + child->m_area.toString() +
          ", no child spans this area in " + m_area.toString(),
          utils::Level::Error
        );

        return;
      }

      n->createSiblings(child);

      return;
    }

    // The child is a direct child of this node. We can allocate all the
    // missing nodes. The allocation in itself will be handled directly
    // by checking the size of the node: if `child` is a leaf node (which
    // should be the case) the allocation will occur (see the `initialize
    // method for further details).
    bool hasNW = m_children.find(borders::Name::NorthWest) != m_children.cend();
    bool hasNE = m_children.find(borders::Name::NorthEast) != m_children.cend();
    bool hasSW = m_children.find(borders::Name::SouthWest) != m_children.cend();
    bool hasSE = m_children.find(borders::Name::SouthEast) != m_children.cend();

    if (child->m_direction != borders::Name::NorthWest && !hasNW) {
      createChild(borders::Name::NorthWest, this);
    }
    if (child->m_direction != borders::Name::NorthEast && !hasNE) {
      createChild(borders::Name::NorthEast, this);
    }
    if (child->m_direction != borders::Name::SouthWest && !hasSW) {
      createChild(borders::Name::SouthWest, this);
    }
    if (child->m_direction != borders::Name::SouthEast && !hasSE) {
      createChild(borders::Name::SouthEast, this);
    }
  }

}

#endif    /* CELLS_QUAD_TREE_NODE_HXX */
