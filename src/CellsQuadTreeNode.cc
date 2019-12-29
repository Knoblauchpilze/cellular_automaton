
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

    m_children[Child::NorthWest] = std::make_shared<CellsQuadTreeNode>(
      utils::Boxi(x, y, cW, cH),
      m_ruleset
    );

    // Top right.
    x = m_area.x() + cW / 2;
    y = m_area.y() + cH / 2;

    m_children[Child::NorthEast] = std::make_shared<CellsQuadTreeNode>(
      utils::Boxi(x, y, cW, cH),
      m_ruleset
    );

    // Bottom left.
    x = m_area.x() - cW / 2;
    y = m_area.y() - cH / 2;

    m_children[Child::SouthEast] = std::make_shared<CellsQuadTreeNode>(
      utils::Boxi(x, y, cW, cH),
      m_ruleset
    );

    // Bottom right.
    x = m_area.x() + cW / 2;
    y = m_area.y() - cH / 2;

    m_children[Child::SouthWest] = std::make_shared<CellsQuadTreeNode>(
      utils::Boxi(x, y, cW, cH),
      m_ruleset
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
    for (int y = 0 ; y < m_area.h() ; ++y) {
      int offset = y * m_area.w();

      for (int x = 0 ; x < m_area.w() ; ++x) {
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

    // TODO: Implementation.

    return root;
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
