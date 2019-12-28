
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  CellsQuadTreeNode::CellsQuadTreeNode(const utils::Boxi& area,
                                       const rules::Type& ruleset):
    utils::CoreObject(std::string("quadtree_node_") + area.toString()),

    m_area(),
    m_ruleset(ruleset),

    m_cells(),
    m_adjacency(),

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
      for (unsigned id = 0u ; id < m_children.size() ; ++id) {
        m_children[id]->fetchCells(cells, area);
      }

      return;
    }

    // Populate the needed cells.
    int xMin = area.getLeftBound();
    int yMin = area.getBottomBound();
    int xMax = area.getRightBound();
    int yMax = area.getTopBound();

    for (int y = yMin ; y < yMax ; ++y) {
      // Convert logical coordinates to valid cells coordinates.
      int offset = (y - yMin) * area.w();
      int rOffset = (y - m_area.getBottomBound()) * m_area.w();

      for (int x = xMin ; x < xMax ; ++x) {
        // Convert the `x` coordinate similarly to the `y` coordinate.
        int xOff = x - xMin;
        int rXOff = x - m_area.getLeftBound();

        // Check whether the cell exists in the internal data. If this
        // is the case we assign it, otherwise we don't modify the value.
        int coord = rOffset + rXOff;
        if (rOffset >= 0 && rOffset < static_cast<int>(m_cells.size()) &&
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

    m_children.push_back(
      std::make_shared<CellsQuadTreeNode>(
        utils::Boxi(x, y, cW, cH),
        m_ruleset
      )
    );

    // Top right.
    x = m_area.x() + cW / 2;
    y = m_area.y() + cH / 2;

    m_children.push_back(
      std::make_shared<CellsQuadTreeNode>(
        utils::Boxi(x, y, cW, cH),
        m_ruleset
      )
    );

    // Bottom left.
    x = m_area.x() - cW / 2;
    y = m_area.y() - cH / 2;

    m_children.push_back(
      std::make_shared<CellsQuadTreeNode>(
        utils::Boxi(x, y, cW, cH),
        m_ruleset
      )
    );

    // Bottom right.
    x = m_area.x() + cW / 2;
    y = m_area.y() - cH / 2;

    m_children.push_back(
      std::make_shared<CellsQuadTreeNode>(
        utils::Boxi(x, y, cW, cH),
        m_ruleset
      )
    );

    // Split children nodes in case we need more than one split operation.
    for (unsigned id = 0u ; id < m_children.size() ; ++id) {
      m_children[id]->splitUntil(size);
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
      log("Registering job for area " + m_area.toString() + " (already " + std::to_string(tiles.size()) + " registered)");

      tiles.push_back(
        std::make_shared<ColonyTile>(
          m_area,
          this
        )
      );

      return;
    }

    for (unsigned id = 0u ; id < m_children.size() ; ++id) {
      m_children[id]->registerTiles(tiles);
    }
  }

  void
  CellsQuadTreeNode::step() {
    // If this node is not a leaf; call the adequate method for all children.
    if (!isLeaf()) {
      for (unsigned id = 0u ; id < m_children.size() ; ++id) {
        m_children[id]->step();
      }

      return;
    }

    // If this node is a leaf, update all the internal cells.
    for (unsigned id = 0u ; id < m_cells.size() ; ++id) {
      m_cells[id].step();
    }
  }

  void
  CellsQuadTreeNode::updateAdjacencyFor(const utils::Vector2i& /*coord*/,
                                        bool /*alive*/)
  {
    // TODO: Implementation.
  }

}
