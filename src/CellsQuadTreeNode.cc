
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  CellsQuadTreeNode::CellsQuadTreeNode(const utils::Boxi& area,
                                       const rules::Type& ruleset):
    utils::CoreObject(std::string("quadtree_node_") + area.toString()),

    m_area(),
    m_ruleset(ruleset),

    m_cells(),
    m_adjacency(),
    m_nextAdjacency(),

    m_aliveCount(0u),

    m_parent(nullptr),
    m_orientation(Child::None),
    m_children()
  {
    setService("node");

    // Check consistency.
    if (!area.valid()) {
      error(
        std::string("Could not create cells quadtree node"),
        std::string("Invalid dimensions ") + area.toString()
      );
    }

    initialize(area, State::Dead, true);
  }

  CellsQuadTreeNode::CellsQuadTreeNode(const utils::Boxi& area,
                                       const rules::Type& ruleset,
                                       CellsQuadTreeNode* parent,
                                       const Child& orientation,
                                       bool allocate):
    utils::CoreObject(std::string("quadtree_node_") + area.toString()),

    m_area(),
    m_ruleset(ruleset),

    m_cells(),
    m_adjacency(),
    m_nextAdjacency(),

    m_aliveCount(0u),

    m_parent(parent),
    m_orientation(parent == nullptr ? Child::None : orientation),
    m_children()
  {
    setService("node");

    // Check consistency.
    if (!area.valid()) {
      error(
        std::string("Could not create cells quadtree node"),
        std::string("Invalid dimensions ") + area.toString()
      );
    }

    initialize(area, State::Dead, allocate);
  }

  void
  CellsQuadTreeNode::fetchCells(std::vector<State>& cells,
                                const utils::Boxi& area)
  {
    // If no cells are available, we can return early.
    if (m_aliveCount == 0) {
      return;
    }

    // Traverse children if there are any.
    if (!isLeaf()) {
      for (ChildrenMap::const_iterator it = m_children.cbegin() ;
           it != m_children.cend() ;
           ++it)
      {
        it->second->fetchCells(cells, area);
      }

      return;
    }

    // Populate the needed cells.
    int gXMin = area.getLeftBound();
    int gYMin = area.getBottomBound();
    int gXMax = area.getRightBound();
    int gYMax = area.getTopBound();

    int lXMin = m_area.getLeftBound();
    int lYMin = m_area.getBottomBound();
    int lXMax = m_area.getRightBound();
    int lYMax = m_area.getTopBound();

    int xMin = std::max(gXMin, lXMin);
    int yMin = std::max(gYMin, lYMin);
    int xMax = std::min(gXMax, lXMax);
    int yMax = std::min(gYMax, lYMax);

    int uB = static_cast<int>(m_cells.size());

    for (int y = yMin ; y < yMax ; ++y) {
      // Convert logical coordinates to valid cells coordinates.
      int offset = (y - gYMin) * area.w();
      int rOffset = (y - lYMin) * m_area.w();

      for (int x = xMin ; x < xMax ; ++x) {
        // Convert the `x` coordinate similarly to the `y` coordinate.
        int xOff = x - gXMin;
        int rXOff = x - lXMin;

        // Check whether the cell exists in the internal data. If this
        // is the case we assign it, otherwise we don't modify the value.
        int coord = rOffset + rXOff;
        if (rOffset >= 0 && rOffset < uB &&
            rXOff >= 0 && rXOff < m_area.w())
        {
          cells[offset + xOff] = m_cells[coord].state();
        }
      }
    }
  }

  void
  CellsQuadTreeNode::splitUntil(const utils::Sizei& size) {
    // We consider that if the input size is larger than the current
    // size of the node we don't need to do anything.
    if (size.contains(m_area.toSize())) {
      return;
    }

    // Also if some children are already available, this is a problem.
    if (!isLeaf()) {
      error(
        std::string("Could not split quadtree node to reach ") + size.toString(),
        std::string("Node is already splitted")
      );
    }

    // Check whether the input size is a perfect divisor of the internal
    // size. Otherwise we won't be able to split the node into sub-nodes
    // while still keeping equal size: we could try to determine a size
    // that divides best the input size but what about prime numbers ?
    // So better not try anything.
    if (m_area.w() % size.w() != 0 || m_area.h() % size.h() != 0) {
      error(
        std::string("Could not split quadtree node to reach ") + size.toString(),
        std::string("Internal size ") + m_area.toSize().toString() +
        " is not a multiple of it"
      );
    }

    // The dimensions of the child nodes are the dimensions of the parent
    // divided by 2. Indeed we want to create 4 children nodes. If the
    // dimensions are not even, we will declare a failure as we can't do
    // much with integer arithmetic.
    if (m_area.w() % 2 != 0 || m_area.h() % 2 != 0) {
      error(
        std::string("Could not split quadtree node to reach ") + size.toString(),
        std::string("Internal size ") + m_area.toSize().toString() + " cannot be divided evenly"
      );
    }

    // Create children.
    int cW = m_area.w() / 2;
    int cH = m_area.h() / 2;

    // Top left.
    int x = m_area.x() - cW / 2;
    int y = m_area.y() + cH / 2;

    m_children[Child::NorthWest] = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        utils::Boxi(x, y, cW, cH),
        m_ruleset,
        this,
        Child::NorthWest,
        true
      )
    );

    // Top right.
    x = m_area.x() + cW / 2;
    y = m_area.y() + cH / 2;

    m_children[Child::NorthEast] = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        utils::Boxi(x, y, cW, cH),
        m_ruleset,
        this,
        Child::NorthEast,
        true
      )
    );

    // Bottom left.
    x = m_area.x() - cW / 2;
    y = m_area.y() - cH / 2;

    m_children[Child::SouthWest] = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        utils::Boxi(x, y, cW, cH),
        m_ruleset,
        this,
        Child::SouthWest,
        true
      )
    );

    // Bottom right.
    x = m_area.x() + cW / 2;
    y = m_area.y() - cH / 2;

    m_children[Child::SouthEast] = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        utils::Boxi(x, y, cW, cH),
        m_ruleset,
        this,
        Child::SouthEast,
        true
      )
    );

    // Split children nodes in case we need more than one split operation.
    for (ChildrenMap::const_iterator it = m_children.cbegin() ;
         it != m_children.cend() ;
         ++it)
    {
      it->second->splitUntil(size);
    }

    // Clear the internal data.
    m_cells.clear();
    m_aliveCount = 0u;
  }

  void
  CellsQuadTreeNode::registerTiles(std::vector<ColonyTileShPtr>& tiles) {
    // In the case this node is a leaf, we only need to register a single
    // tile corresponding to the internal data of this node. Otherwise we
    // will transmit the request to the children if any.
    if (isLeaf()) {
      log("Registering job for area " + m_area.toString() + " (already " + std::to_string(tiles.size()) + " registered)", utils::Level::Verbose);

      tiles.push_back(
        std::make_shared<ColonyTile>(
          m_area,
          this
        )
      );

      return;
    }

    for (ChildrenMap::const_iterator it = m_children.cbegin() ;
         it != m_children.cend() ;
         ++it)
    {
      it->second->registerTiles(tiles);
    }
  }

  void
  CellsQuadTreeNode::evolve() {
    // Check whether this node is a leaf.
    if (!isLeaf()) {
      log(
        std::string("Cannot evolve node spanning ") + m_area.toString() + ", node is not a leaf",
        utils::Level::Error
      );

      return;
    }

    // First we need to compute the internal elements of the quadtree node. This
    // corresponds to all the cells except the boundaries, where we would need to
    // access the data from adjacent nodes. In the meantime we need to update the
    // adjacency count.
    for (int y = 1 ; y < m_area.h() - 1 ; ++y) {
      int offset = y * m_area.w();

      for (int x = 1 ; x < m_area.w() - 1 ; ++x) {
        State s = m_cells[offset + x].update(m_adjacency[offset + x]);
        bool alive = (s == State::Alive || s == State::Newborn);

        updateAdjacencyFor(utils::Vector2i(x, y), alive);
      }
    }
  }

  CellsQuadTreeNodeShPtr
  CellsQuadTreeNode::expand(CellsQuadTreeNodeShPtr root) {
    // Check consistency of the input node.
    if (root == nullptr) {
      return CellsQuadTreeNodeShPtr();
    }

    // Retrieve boundaries in order to detect whether an expansion is needed:
    // if no boundaries have some alive cells, there's no need to expand.
    std::vector<CellsQuadTreeNode*> nodes;
    root->collectBoundaries(nodes, false);

    // If no boundary node contain at least a living cell there's nothing to
    // expand for.
    if (nodes.empty()) {
      return root;
    }

    // Expanding a node is done in two different ways according to whether the
    // input `root` node is a single node or a composite one.
    // in case of a single node we need to split it in 4 and create a wrapper
    // node above the four splitted children. This will effectively guarantee
    // that some extra space is created around the possible problems on the
    // boundaries.
    // In case of a complex node, we want to do a similar behavior where the
    // 4 top most children of the current root will be allocated in 4 different
    // children of a larger wrapper node.
    // Note that we don't really care about allocating any of the children at
    // this step, it will be handled on the fly when evolving the colony. It
    // is indeed better to keep the allocation part to when we know exactly
    // what needs to be created (because a cell will go out of the world for
    // example).
    //
    // That being said part of the process can be factorized, i.e. the creation
    // of the new root node and its children.
    utils::Boxi world = root->m_area;
    utils::Boxi area(world.x(), world.y(), world.w() * 2, world.h() * 2);

    CellsQuadTreeNodeShPtr newRoot = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        area, root->m_ruleset, nullptr, Child::None, false
      )
    );

    log("Allocating root with area " + area.toString());

    // Create the children of the new root: they will either be complex node if
    // the root itself is a complex node (i.e. with a depth bigger than `1`) or
    // leaf node if the root itself is a leaf.
    // Hence the use of the `isLeaf` method to determine whether the children
    // nodes should be allocated.
    // Note also that the node are added to the root but their alive cells count
    // is kept for when the data has actually been allocated.
    area = utils::Boxi(-world.w() / 2, world.h() / 2, world.w(), world.h());
    CellsQuadTreeNodeShPtr nw = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        area,
        root->m_ruleset,
        newRoot.get(),
        Child::NorthWest,
        root->isLeaf()
      )
    );
    log("Creating north west with " + area.toString());

    area = utils::Boxi(world.w() / 2, world.h() / 2, world.w(), world.h());
    CellsQuadTreeNodeShPtr ne = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        area,
        root->m_ruleset,
        newRoot.get(),
        Child::NorthEast,
        root->isLeaf()
      )
    );
    log("Creating north east with " + area.toString());

    area = utils::Boxi(-world.w() / 2, -world.h() / 2, world.w(), world.h());
    CellsQuadTreeNodeShPtr sw = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        area,
        root->m_ruleset,
        newRoot.get(),
        Child::SouthWest,
        root->isLeaf()
      )
    );
    log("Creating south west with " + area.toString());

    area = utils::Boxi(world.w() / 2, -world.h() / 2, world.w(), world.h());
    CellsQuadTreeNodeShPtr se = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        area,
        root->m_ruleset,
        newRoot.get(),
        Child::SouthEast,
        root->isLeaf()
      )
    );
    log("Creating south east with " + area.toString());

    newRoot->m_children[Child::NorthWest] = nw;
    newRoot->m_children[Child::NorthEast] = ne;
    newRoot->m_children[Child::SouthWest] = sw;
    newRoot->m_children[Child::SouthEast] = se;

    // Now determine what we need to do to populate the children nodes of the
    // `newRoot`. In case the root has a depth greater than `1` we want to copy
    // the children to their respective parents.
    if (!root->isLeaf()) {
      // Assign children of this root node to their new parent if any.
      ChildrenMap::const_iterator it = root->m_children.find(Child::NorthWest);
      if (it != root->m_children.cend()) {
        nw->m_children[Child::SouthEast] = it->second;
        nw->m_aliveCount += it->second->m_aliveCount;
      }

      it = root->m_children.find(Child::NorthEast);
      if (it != root->m_children.cend()) {
        ne->m_children[Child::SouthWest] = it->second;
        ne->m_aliveCount += it->second->m_aliveCount;
      }

      it = root->m_children.find(Child::SouthWest);
      if (it != root->m_children.cend()) {
        sw->m_children[Child::NorthEast] = it->second;
        sw->m_aliveCount += it->second->m_aliveCount;
      }

      it = root->m_children.find(Child::SouthEast);
      if (it != root->m_children.cend()) {
        se->m_children[Child::NorthWest] = it->second;
        se->m_aliveCount += it->second->m_aliveCount;
      }
    }
    else {
      // The root is a leaf node: we need to split up the data and populate each
      // bit in the corresponding children.
      CellsQuadTreeNode* n = nullptr;
      int uB = world.area();

      for (int y = 0 ; y < world.h() ; ++y) {
        int offset = y * world.w();

        for (int x = 0 ; x < world.w() ; ++x) {
          // Fetch values.
          Cell c = root->m_cells[offset + x];
          unsigned a = root->m_adjacency[offset + x];
          unsigned na = root->m_nextAdjacency[offset + x];

          // Dispatch each element in the correct child. The internal vector starts
          // with the bottom left most cell and continue from west to east and from
          // south to north.
          int coord = -1;

          if (y < world.h() / 2) {
            if (x < world.w() / 2) {
              // South west child.
              n = sw.get();
              coord = (y + world.h() / 2) * world.w() + (x + world.w() / 2);
            }
            else {
              // South east child.
              n = se.get();
              coord = (y + world.h() / 2) * world.w() + (x - world.w() / 2);
            }
          }
          else {
            if (x < world.w() / 2) {
              // North west child.
              n = nw.get();
              coord = (y - world.h() / 2) * world.w() + (x + world.w() / 2);
            }
            else {
              // North east child.
              n = ne.get();
              coord = (y - world.h() / 2) * world.w() + (x - world.w() / 2);
            }
          }

          // Assign values.
          if (coord >= 0 && coord < uB && n != nullptr) {
            n->m_cells[coord] = c;
            n->m_adjacency[coord] = a;
            n->m_nextAdjacency[coord] = na;

            if (c.state() == State::Newborn || c.state() == State::Alive) {
              ++n->m_aliveCount;
            }
          }
          else {
            // At least print an error message.
            log(
              "Cannot assign cell at [" + std::to_string(x) + "x" + std::to_string(y) + "] to any quadtree node (converted to " + std::to_string(coord) + ")",
              utils::Level::Error
            );
          }
        }
      }

    }

    // Finally assign the alive cells count for the new root.
    newRoot->m_aliveCount += nw->getAliveCellsCount();
    newRoot->m_aliveCount += ne->getAliveCellsCount();
    newRoot->m_aliveCount += sw->getAliveCellsCount();
    newRoot->m_aliveCount += se->getAliveCellsCount();

    log("Old root had " + std::to_string(root->m_aliveCount) + " alive cell(s), new one has " + std::to_string(newRoot->m_aliveCount));
    log(
      "nw: " + std::to_string(nw->getAliveCellsCount()) +
      ", ne: " + std::to_string(ne->getAliveCellsCount()) +
      ", sw: " + std::to_string(sw->getAliveCellsCount()) +
      ", se: " + std::to_string(se->getAliveCellsCount())
    );

    return newRoot;
  }

  void
  CellsQuadTreeNode::updateAdjacencyFor(const utils::Vector2i& coord,
                                        bool alive,
                                        bool makeCurrent)
  {
    // We need to update the internal adjacency values with the alive data.
    for (int y = coord.y() - 1 ; y <= coord.y() + 1 ; ++y) {
      // Check whether the coordinate lies inside the boundaries.
      if (y < 0 || y > m_area.h() - 1) {
        continue;
      }

      int offset = y * m_area.w();

      for(int x = coord.x() - 1 ; x <= coord.x() + 1 ; ++x) {
        // Check consistency of the coordinates.
        if (x < 0 || x > m_area.w() - 1) {
          continue;
        }

        // Do not update the count for this cell.
        if (y == coord.y() && x == coord.x()) {
          continue;
        }

        if (alive) {
          if (makeCurrent) {
            ++m_adjacency[offset + x];
          }
          else {
            ++m_nextAdjacency[offset + x];
          }
        }
      }
    }
  }

}
