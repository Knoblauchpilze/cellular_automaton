
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  CellsQuadTreeNode::CellsQuadTreeNode(const utils::Boxi& area,
                                       const rules::Type& ruleset,
                                       const utils::Sizei& minSize):
    utils::CoreObject(std::string("quadtree_node_") + area.toString()),

    m_area(),
    m_ruleset(ruleset),
    m_minSize(minSize),
    m_depth(0u),

    m_cells(),
    m_adjacency(),
    m_nextAdjacency(),

    m_aliveCount(0u),
    m_dyingCount(0u),

    m_parent(nullptr),
    m_direction(borders::Name::None),
    m_orientation(),
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

    initialize(area, State::Dead);

    // Split this root node until it reaches the desired node size.
    if (m_area.w() >= m_minSize.w() || m_area.h() >= m_minSize.h()) {
      split();
    }
  }

  CellsQuadTreeNode::CellsQuadTreeNode(const utils::Boxi& area,
                                       const rules::Type& ruleset,
                                       CellsQuadTreeNode* parent,
                                       const borders::Name& direction,
                                       const utils::Sizei& minSize):
    utils::CoreObject(std::string("quadtree_node_") + area.toString()),

    m_area(),
    m_ruleset(ruleset),
    m_minSize(minSize),
    m_depth(parent == nullptr ? 0u : parent->m_depth + 1u),

    m_cells(),
    m_adjacency(),
    m_nextAdjacency(),

    m_aliveCount(0u),
    m_dyingCount(0u),

    m_parent(parent),
    m_direction(direction),
    m_orientation(),
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

    initialize(area, State::Dead);
  }

  void
  CellsQuadTreeNode::fetchCells(std::vector<State>& cells,
                                const utils::Boxi& area)
  {
    // If no cells are available, we can return early.
    if (!isDying()) {
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
  CellsQuadTreeNode::randomize(const utils::Boxi& area) {
    // Randomize each cell. In case children are available, we need
    // to call the randomization method on them.
    if (!isLeaf()) {
      m_aliveCount = 0u;
      m_dyingCount = 0u;

      for (ChildrenMap::const_iterator it = m_children.cbegin() ;
           it != m_children.cend() ;
           ++it)
      {
        if (area.intersectsBottomLeft(it->second->m_area)) {
          it->second->randomize(area);
          m_aliveCount += it->second->getAliveCellsCount();
          m_dyingCount += it->second->getDyingCellsCount();
        }
      }

      return;
    }

    // Update both the internal alive count and the adjacency values.
    // Note that we will have to indicate to the `updateAdjacencyFor`
    // method that we want to update the current adjacency.
    // We don't want to generate elements on the boundaries of the
    int xOffset = m_area.w() / 2;
    int yOffset = m_area.h() / 2;

    int uB = m_cells.size();
    for (int id = 0 ; id < uB ; ++id) {
      int x = id % m_area.w();
      int y = id / m_area.w();

      utils::Vector2i coord(
        m_area.x() + x - xOffset,
        m_area.y() + y - yOffset
      );

      // Prevent generation of random values on the boundary if needed.
      if (!area.contains(coord)) {
        continue;
      }

      State s = m_cells[id].randomize();

      bool alive = false;
      switch (s) {
        case State::Newborn:
        case State::Alive:
          alive = true;
          ++m_aliveCount;
          break;
        case State::Dying:
          ++m_dyingCount;
          break;
        default:
          // Do not consider the cell alive by default.
          break;
      }

      updateAdjacencyFor(coord, alive, true);
    }
  }

  void
  CellsQuadTreeNode::registerTiles(std::vector<ColonyTileShPtr>& tiles) {
    // In the case this node is a leaf, we only need to register a single
    // tile corresponding to the internal data of this node. Otherwise we
    // will transmit the request to the children if any.
    // We should also create a boundary job for this node in the case the
    // node is not a leaf node: this will allow the processing of cells
    // that are on the boundaries of children and thus require information
    // from sibling nodes to be processed.
    // Note that we try to minimize the number of tiles to create by not
    // registering anything if the node is `dead` (meaning that no live
    // cell can be found in it).
    if (isDead()) {
      return;
    }

    if (isLeaf()) {
      log("Registering job for area " + m_area.toString() + " (already " + std::to_string(tiles.size()) + " registered)", utils::Level::Verbose);

      tiles.push_back(
        std::make_shared<ColonyTile>(
          m_area,
          this,
          ColonyTile::Type::Interior
        )
      );

      return;
    }
    else {
      tiles.push_back(
        std::make_shared<ColonyTile>(
          m_area,
          this,
          ColonyTile::Type::Border
        )
      );
    }

    // Register the jobs needed to update the children of this node.
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


    // Do not waste processing time if there's no live cells: the only evolution
    // that could happen comes from the boundaries and it is not handled in this
    // method.
    if (isDead()) {
      return;
    }

    // First we need to compute the internal elements of the quadtree node. This
    // corresponds to all the cells except the boundaries, where we would need to
    // access the data from adjacent nodes. In the meantime we need to update the
    // adjacency count.
    int xOffset = m_area.w() / 2;
    int yOffset = m_area.h() / 2;

    for (int y = 1 ; y < m_area.h() - 1 ; ++y) {
      int offset = y * m_area.w();

      for (int x = 1 ; x < m_area.w() - 1 ; ++x) {
        State s = m_cells[offset + x].update(m_adjacency[offset + x]);
        bool alive = (s == State::Alive || s == State::Newborn);

        utils::Vector2i coord(
          m_area.x() + x - xOffset,
          m_area.y() + y - yOffset
        );

        updateAdjacencyFor(coord, alive);
      }
    }
  }

  void
  CellsQuadTreeNode::evolveBoundaries() {
    // This method aims at providing an evolution for the cells that are
    // on the boundaries of data nodes (i.e. leaves). We have two kind of
    // boundaries:
    //  - the internal boundaries which are boundaries not on the exterior
    //    of the colony and which usually link two sibling nodes.
    //  - the exterior of the colony.
    //
    // For any node, the internal boundaries are always defined within the
    // children of the node: basically if we go deep enough we will finally
    // reach the cells data for the boundaries. The process is thus to get
    // the relevant data to pass to each children node so that they can
    // update their boundaries if needed: this is to account for the fact
    // that the cells data might be defined several layers below this node
    // (in case the boundaries link two different parts of the hierarchy).
    //
    // One important part of the process is to provide information to the
    // nodes about the state of their neighbors. If the neighbors are not
    // allocated yet we will provide a dummy data to be used, and we will
    // also have to create the node if needed (in case the cells exits the
    // node for example). This will effectively allow to expand the colony.
    //
    // Finally the root node should also handle the exterior boundaries of
    // the colony: given our process it should always be useless because at
    // each step we expand the colony if the boundary nodes contain alive
    // cells: this allow that we will always have some extra buffer space
    // between the first live cell and the boundaries of the colony.

    if (isRoot()) {
      // Handle exterior boundaries: top and bottom rows then left and right
      // rows.
      unsigned countB = 0u, countU = 0u, countL = 0u, countR = 0u;

      utils::Vector2i coord1(m_area.getLeftBound(), m_area.getBottomBound());
      utils::Vector2i coord2(m_area.getLeftBound(), m_area.getTopBound() - 1);

      for (int x = 0 ; x < m_area.w() ; ++x) {
        if (evolveBoundaryElement(coord1)) {
          ++countB;
        }
        if (evolveBoundaryElement(coord2)) {
          ++countU;
        }

        ++coord1.x();
        ++coord2.x();
      }

      // We don't need to compute the top and bottom cell of the rows as it
      // has already been handled by the top and bottom rows.
      coord1.x() = m_area.getLeftBound();
      coord1.y() = m_area.getBottomBound() + 1;
      coord2.x() = m_area.getRightBound() - 1;
      coord2.y() = m_area.getBottomBound() + 1;

      int yMax = m_area.h() - 1;
      for (int y = 1 ; y < yMax ; ++y) {
        if (evolveBoundaryElement(coord1)) {
          ++countL;
        }
        if (evolveBoundaryElement(coord2)) {
          ++countR;
        }

        ++coord1.y();
        ++coord2.y();
      }

      // Given the generation and scheduling process we should never have any alive
      // cells in the boundaries of the root node.
      if (countR > 0 || countU > 0 || countL > 0 || countB > 0) {
        log(
          std::string("Exterior boundaries contained [r: ") + std::to_string(countR) + " u: " + std::to_string(countU) +
          " l: " + std::to_string(countL) + " b: " + std::to_string(countB) + "]",
          utils::Level::Warning
        );
      }
    }

    // Handle interior boundaries if the node has at least one living cell.
    if (isDead()) {
      return;
    }

    // Handle interior boundaries: this can be split up into the vertical
    // component and the horizontal component. Note that we don't include
    // the first element of each part because it will be handled by the
    // parent node (as it requires knowlegds of the boundaries of the grand
    // parent of this node so out of reach).
    // Not that in case the dimensions of the area do not allow for any
    // interior we don't need to go further.

    utils::Vector2i coord1(m_area.getLeftBound() + 1, m_area.y() - 1);
    utils::Vector2i coord2(m_area.getLeftBound() + 1, m_area.y());

    if (m_area.w() >= 8 && m_area.h() >= 4) {
      int xMax = m_area.w() - 1;
      for (int x = 1 ; x < xMax ; ++x) {
        evolveBoundaryElement(coord1);
        evolveBoundaryElement(coord2);

        ++coord1.x();
        ++coord2.x();
      }
    }

    if (m_area.w() >= 4 && m_area.h() >= 8) {
      coord1.x() = m_area.x() - 1;
      coord1.y() = m_area.getBottomBound() + 1;
      coord2.x() = m_area.x();
      coord2.y() = m_area.getBottomBound() + 1;

      int yMax = m_area.h() - 1;
      for (int y = 1 ; y < yMax ; ++y) {
        // Account for cells which are both in the horizontal and vertical parts.
        if (coord1.y() != m_area.y() && coord1.y() != m_area.y() - 1) {
          evolveBoundaryElement(coord1);
          evolveBoundaryElement(coord2);
        }

        ++coord1.y();
        ++coord2.y();
      }
    }
  }

  CellsQuadTreeNodeShPtr
  CellsQuadTreeNode::expand(CellsQuadTreeNodeShPtr root,
                            utils::Boxi& liveArea)
  {
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

    // Update maximum boundaries.
    // TODO: This does not work: we only get boundaries but some non-boundaries node
    // might describe some more extreme live cells. We should instead traverse the
    // tree and detect the extreme cells. This could maybe be saved as a property of
    // each quadtree node so that we can get this quickly. It could be introduced when
    // handling `improve dead areas` statement.
    utils::Vector2i rX = utils::Vector2i::maxmin();
    utils::Vector2i rY = utils::Vector2i::maxmin();

    for (unsigned id = 0u ; id < nodes.size() ; ++id) {
      rX.x() = std::min(rX.x(), nodes[id]->getArea().getLeftBound());
      rX.y() = std::max(rX.y(), nodes[id]->getArea().getRightBound());

      rY.x() = std::min(rY.x(), nodes[id]->getArea().getBottomBound());
      rY.y() = std::max(rY.y(), nodes[id]->getArea().getTopBound());
    }

    liveArea = utils::Boxi(
      (rX.x() + rX.y()) / 2,
      (rY.x() + rY.y()) / 2,
      rX.y() - rX.x(),
      rY.y() - rY.x()
    );

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
    utils::Boxi world(root->m_area.x(), root->m_area.y(), root->m_area.w() * 2, root->m_area.h() * 2);

    CellsQuadTreeNodeShPtr newRoot = std::shared_ptr<CellsQuadTreeNode>(
      new CellsQuadTreeNode(
        world, root->m_ruleset, nullptr, borders::Name::None, m_minSize
      )
    );

    log("Allocating root with area " + world.toString(), utils::Level::Info);
    unsigned saveAlive = root->m_aliveCount;

    // Create the children of the new root: they will either be complex node if
    // the root itself is a complex node (i.e. with a depth bigger than `1`) or
    // leaf node if the root itself is a leaf.
    // Hence the use of the `isLeaf` method to determine whether the children
    // nodes should be allocated.
    // Note also that the node are added to the root but their alive cells count
    // is kept for when the data has actually been allocated.
    CellsQuadTreeNodeShPtr nw = createChild(borders::Name::NorthWest, newRoot.get());
    log("Creating north west with " + nw->m_area.toString());

    CellsQuadTreeNodeShPtr ne = createChild(borders::Name::NorthEast, newRoot.get());
    log("Creating north east with " + ne->m_area.toString());

    CellsQuadTreeNodeShPtr sw = createChild(borders::Name::SouthWest, newRoot.get());
    log("Creating south west with " + sw->m_area.toString());

    CellsQuadTreeNodeShPtr se = createChild(borders::Name::SouthEast, newRoot.get());
    log("Creating south east with " + se->m_area.toString());

    // Now determine what we need to do to populate the children nodes of the
    // `newRoot`. In case the root has a depth greater than `1` we want to copy
    // the children to their respective parents.
    if (!root->isLeaf()) {
      // Assign children of this root node to their new parent if any.
      ChildrenMap::const_iterator it = root->m_children.find(borders::Name::NorthWest);
      if (it != root->m_children.cend()) {
        nw->attach(it->second, borders::Name::SouthEast);
      }

      it = root->m_children.find(borders::Name::NorthEast);
      if (it != root->m_children.cend()) {
        ne->attach(it->second, borders::Name::SouthWest);
      }

      it = root->m_children.find(borders::Name::SouthWest);
      if (it != root->m_children.cend()) {
        sw->attach(it->second, borders::Name::NorthEast);
      }

      it = root->m_children.find(borders::Name::SouthEast);
      if (it != root->m_children.cend()) {
        se->attach(it->second, borders::Name::NorthWest);
      }
    }
    else {
      // The root is a leaf node: we need to split up the data and populate each
      // bit in the corresponding children.
      CellsQuadTreeNode* n = nullptr;
      utils::Boxi oldWorld = root->m_area;
      int uB = world.area();

      for (int y = 0 ; y < oldWorld.h() ; ++y) {
        int offset = y * oldWorld.w();

        for (int x = 0 ; x < oldWorld.w() ; ++x) {
          // Fetch values.
          Cell c = root->m_cells[offset + x];
          unsigned a = root->m_adjacency[offset + x];
          unsigned na = root->m_nextAdjacency[offset + x];

          // Dispatch each element in the correct child. The internal vector starts
          // with the bottom left most cell and continue from west to east and from
          // south to north.
          int coord = -1;

          if (y < oldWorld.h() / 2) {
            if (x < oldWorld.w() / 2) {
              // South west child.
              n = sw.get();
              coord = (y + oldWorld.h() / 2) * oldWorld.w() + (x + oldWorld.w() / 2);
            }
            else {
              // South east child.
              n = se.get();
              coord = (y + oldWorld.h() / 2) * oldWorld.w() + (x - oldWorld.w() / 2);
            }
          }
          else {
            if (x < oldWorld.w() / 2) {
              // North west child.
              n = nw.get();
              coord = (y - oldWorld.h() / 2) * oldWorld.w() + (x + oldWorld.w() / 2);
            }
            else {
              // North east child.
              n = ne.get();
              coord = (y - oldWorld.h() / 2) * oldWorld.w() + (x - oldWorld.w() / 2);
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

    // Loop through leaves and create their siblings.
    for (unsigned id = 0u ; id < nodes.size() ; ++id) {
      newRoot->createSiblings(nodes[id]);
    }

    // Finally assign the alive cells count for the new root.
    newRoot->m_aliveCount += nw->getAliveCellsCount();
    newRoot->m_aliveCount += ne->getAliveCellsCount();
    newRoot->m_aliveCount += sw->getAliveCellsCount();
    newRoot->m_aliveCount += se->getAliveCellsCount();

    newRoot->m_dyingCount += nw->getDyingCellsCount();
    newRoot->m_dyingCount += ne->getDyingCellsCount();
    newRoot->m_dyingCount += sw->getDyingCellsCount();
    newRoot->m_dyingCount += se->getDyingCellsCount();

    log("Old root had " + std::to_string(saveAlive) + " alive cell(s), new one has " + std::to_string(newRoot->m_aliveCount));
    log(
      "nw: " + std::to_string(nw->getAliveCellsCount()) +
      ", ne: " + std::to_string(ne->getAliveCellsCount()) +
      ", sw: " + std::to_string(sw->getAliveCellsCount()) +
      ", se: " + std::to_string(se->getAliveCellsCount())
    );

    return newRoot;
  }

  void
  CellsQuadTreeNode::split() {
    // We consider that if the input size is larger than the current
    // size of the node we don't need to do anything.
    if (m_minSize.contains(m_area.toSize())) {
      return;
    }

    // Also if some children are already available, this is a problem.
    if (!isLeaf()) {
      error(
        std::string("Could not split quadtree node to reach ") + m_minSize.toString(),
        std::string("Node is already splitted")
      );
    }

    // Check whether the input size is a perfect divisor of the internal
    // size. Otherwise we won't be able to split the node into sub-nodes
    // while still keeping equal size: we could try to determine a size
    // that divides best the input size but what about prime numbers ?
    // So better not try anything.
    if (m_area.w() % m_minSize.w() != 0 || m_area.h() % m_minSize.h() != 0) {
      error(
        std::string("Could not split quadtree node to reach ") + m_minSize.toString(),
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
        std::string("Could not split quadtree node to reach ") + m_minSize.toString(),
        std::string("Internal size ") + m_area.toSize().toString() + " cannot be divided evenly"
      );
    }

    // Create children.
    createChild(borders::Name::NorthWest, this);
    createChild(borders::Name::NorthEast, this);
    createChild(borders::Name::SouthWest, this);
    createChild(borders::Name::SouthEast, this);

    // Split children nodes in case we need more than one split operation.
    for (ChildrenMap::const_iterator it = m_children.cbegin() ;
         it != m_children.cend() ;
         ++it)
    {
      it->second->split();
    }

    // Clear the internal data.
    m_aliveCount = 0u;
  }

  void
  CellsQuadTreeNode::updateAdjacencyFor(const utils::Vector2i& coord,
                                        bool alive,
                                        bool makeCurrent,
                                        CellsQuadTreeNode* ignore)
  {
    // To update the adjacency we need to update the corresponding entries in
    // the `m_nextAdjacency` array. This can either mean updating the internal
    // arrays (in the case this node is a leaf) or calling the adequate method
    // on the relevant child.
    // We could also need to create the node if needed (i.e. if the relevant
    // child does not exist).

    // No need to update anything if the status is `Dead`.
    if (!alive) {
      return;
    }

    // Check whether the node is a leaf first: in this case it is quite easy
    // because we can direcly handle the modification of the internal data.
    // Note that in case we hit a boundary we will not attempt to update the
    // siblings: instead we will assume that the process will somehow call the
    // method on the sibling elsewhere.
    if (isLeaf()) {
      // Convert the coord to local array coordinates.
      int lXMin = (coord.x() - 1) - m_area.getLeftBound();
      int lYMin = (coord.y() - 1) - m_area.getBottomBound();
      int lXMax = lXMin + 2;
      int lYMax = lYMin + 2;

      int xTgt = lXMin + 1;
      int yTgt = lYMin + 1;

      for (int y = lYMin ; y <= lYMax ; ++y) {
        // Check whether the coordinate lies inside the boundaries.
        if (y < 0 || y > m_area.h() - 1) {
          continue;
        }

        int offset = y * m_area.w();

        for(int x = lXMin ; x <= lXMax ; ++x) {
          // Check consistency of the coordinates.
          if (x < 0 || x > m_area.w() - 1) {
            continue;
          }

          // Do not update the count for this cell.
          if (y == yTgt && x == xTgt) {
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

      // Transmit to the parent if any: indeed we need to propagate
      // modifications of the adjacency to siblings in case we are
      // updating the adjacency of a boundary cell.
      if (!isRoot() && m_parent != ignore) {
        m_parent->updateAdjacencyFor(coord, alive, makeCurrent, this);
      }

      return;
    }

    // This node is not a leaf: we need to call the method on the
    // relevant children if they contain the coordinate. Note that
    // this include creating the node if needed.
    utils::Boxi aoe(coord.x(), coord.y(), 2, 2);

    utils::Boxi nw = getBoxForChild(m_area, borders::Name::NorthWest);
    ChildrenMap::const_iterator nwIt = m_children.find(borders::Name::NorthWest);

    utils::Boxi ne = getBoxForChild(m_area, borders::Name::NorthEast);
    ChildrenMap::const_iterator neIt = m_children.find(borders::Name::NorthEast);

    utils::Boxi sw = getBoxForChild(m_area, borders::Name::SouthWest);
    ChildrenMap::const_iterator swIt = m_children.find(borders::Name::SouthWest);

    utils::Boxi se = getBoxForChild(m_area, borders::Name::SouthEast);
    ChildrenMap::const_iterator seIt = m_children.find(borders::Name::SouthEast);

    // Check whether the area of effect of the adjacency will impact the node.
    // If this is the case we either need to update the node directly if it
    // already exists or create it (as we are sure that we will have an alive
    // neighbor for one of the cell of the node).
    if (nw.intersectsBottomLeft(aoe)) {
      if (nwIt != m_children.cend()) {
        if (nwIt->second.get() != ignore) {
          nwIt->second->updateAdjacencyFor(coord, alive, makeCurrent, this);
        }
      }
      else {
        // Create the node.
        CellsQuadTreeNodeShPtr c = createChild(borders::Name::NorthWest, this);
        log("Creating 2 north west with " + c->m_area.toString() + " from " + coord.toString());

        c->updateAdjacencyFor(coord, alive, makeCurrent, this);
      }
    }

    if (ne.intersectsBottomLeft(aoe)) {
      if (neIt != m_children.cend()) {
        if (neIt->second.get() != ignore) {
          neIt->second->updateAdjacencyFor(coord, alive, makeCurrent, this);
        }
      }
      else {
        // Create the node.
        CellsQuadTreeNodeShPtr c = createChild(borders::Name::NorthEast, this);
        log("Creating 2 north east with " + c->m_area.toString() + " from " + coord.toString());

        c->updateAdjacencyFor(coord, alive, makeCurrent, this);
      }
    }

    if (sw.intersectsBottomLeft(aoe)) {
      if (swIt != m_children.cend()) {
        if (swIt->second.get() != ignore) {
          swIt->second->updateAdjacencyFor(coord, alive, makeCurrent, this);
        }
      }
      else {
        // Create the node.
        CellsQuadTreeNodeShPtr c = createChild(borders::Name::SouthWest, this);
        log("Creating 2 south west with " + c->m_area.toString() + " from " + coord.toString());

        c->updateAdjacencyFor(coord, alive, makeCurrent, this);
      }
    }

    if (se.intersectsBottomLeft(aoe)) {
      if (seIt != m_children.cend()) {
        if (seIt->second.get() != ignore) {
          seIt->second->updateAdjacencyFor(coord, alive, makeCurrent, this);
        }
      }
      else {
        // Create the node.
        CellsQuadTreeNodeShPtr c = createChild(borders::Name::SouthEast, this);
        log("Creating 2 south east with " + c->m_area.toString() + " from " + coord.toString());

        c->updateAdjacencyFor(coord, alive, makeCurrent, this);
      }
    }

    // Transmit to the parent as well: we need to propagate as far as needed which
    // can be quite far if the colony is large and the number of elements in the
    // tree is large: very close cells might be in very different parts of the tree
    // hierarchy.
    if (!isRoot() && m_parent != ignore) {
      m_parent->updateAdjacencyFor(coord, alive, makeCurrent, this);
    }
  }

  Cell*
  CellsQuadTreeNode::at(const utils::Vector2i& coord,
                        unsigned& alive,
                        bool& inside,
                        bool& created) noexcept
  {
    // Assume the cell is not inside the node.
    inside = false;
    created = false;
    alive = 0u;

    // Check whether the coordinates are inside the area defined for the node.
    if (!m_area.contains(coord)) {
      return nullptr;
    }

    // The cell is inside the node: either this node is a leaf in which case
    // we can retrieve the corresponding cell. Otherwise we need to call the
    // method on the correct child.
    inside = true;

    // Convert the coordinate to reach the cell in the internal array.
    int x = coord.x() - m_area.getLeftBound();
    int y = coord.y() - m_area.getBottomBound();

    if (isLeaf()) {
      created = true;

      int offset = y * m_area.w();
      alive = m_adjacency[offset + x];

      return &m_cells[offset + x];
    }

    // The cell is in one of the children: determine which one and either
    // call this method on it or indicate the the cell has not yet been
    // created.
    borders::Name orientation = borders::Name::None;
    if (y < m_area.h() / 2) {
      if (x < m_area.w() / 2) {
        orientation = borders::Name::SouthWest;
      }
      else {
        orientation = borders::Name::SouthEast;
      }
    }
    else {
      if (x < m_area.w() / 2) {
        orientation = borders::Name::NorthWest;
      }
      else {
        orientation = borders::Name::NorthEast;
      }
    }

    // Try to fetch the corresponding child.
    ChildrenMap::const_iterator it = m_children.find(orientation);

    if (it == m_children.cend()) {
      // If the child does not exist, notify the caller.
      return nullptr;
    }

    return it->second->at(coord, alive, inside, created);
  }

  bool
  CellsQuadTreeNode::evolveBoundaryElement(const utils::Vector2i& coord) {
    // Retrieve information about the cell.
    unsigned neighbors;
    bool inside, created;

    Cell* c = at(coord, neighbors, inside, created);

    bool alive = false;

    // Check consistency for bottom row.
    if (!inside) {
      log(
        std::string("Could not compute boundary at ") + coord.toString() + " (somehow outside of boundaries)",
        utils::Level::Error
      );
    }
    else if (c == nullptr && created) {
      log(
        std::string("Could not fetch element at ") + coord.toString() + " (cell is null)",
        utils::Level::Error
      );
    }
    else if (!created) {
      // The cell is not created yet: check whether a cell would be created given
      // the number of neighbors: if this is the case we will need to create the
      // corresponding child, otherwise everything is fine.
      Cell c(State::Dead, m_ruleset);
      State s = c.update(neighbors);

      // Only react if the produced state would not be a `Dead` cell.
      if (s != State::Dead) {
        if (s == State::Dying) {
          log(
            std::string("Created dying cell from dead cell at ") + coord.toString(),
            utils::Level::Warning
          );
        }

        log(
          std::string("Should have created element to hold cell at ") + coord.toString(),
          utils::Level::Error
        );

        alive = (s == State::Alive || s == State::Newborn);
      }
    }
    else {
      // The cell already exists, evolve it.
      State s = c->update(neighbors);
      alive = (s == State::Alive || s == State::Newborn);

      updateAdjacencyFor(coord, alive);
    }

    return alive;
  }

}
