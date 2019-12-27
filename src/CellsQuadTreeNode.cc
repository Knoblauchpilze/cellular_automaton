
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  CellsQuadTreeNode::CellsQuadTreeNode(const utils::Boxi& area,
                                       const rules::Type& ruleset):
    utils::CoreObject(std::string("quadtree_node_") + area.toString()),

    m_area(),
    m_ruleset(ruleset),

    m_cells()
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
    // Populate the needed cells.
    int xMin = area.getLeftBound();
    int yMin = area.getBottomBound();
    int xMax = area.getRightBound();
    int yMax = area.getTopBound();

    for (int y = yMin ; y < yMax ; ++y) {
      // Convert logical coordinates to valid cells coordinates.
      int offset = (y - yMin) * area.w();
      int rOffset = (y + m_area.h() / 2) * m_area.w();

      for (int x = xMin ; x < xMax ; ++x) {
        // Convert the `x` coordinate similarly to the `y` coordinate.
        int xOff = x - xMin;
        int rXOff = x + m_area.w() / 2;

        // Check whether the cell exists in the internal data.
        int coord = rOffset + rXOff;
        Cell c(State::Dead);
        if (rOffset >= 0 && rOffset < static_cast<int>(m_cells.size()) &&
            rXOff >= 0 && rXOff < m_area.w())
        {
          c = m_cells[coord];
        }

        cells[offset + xOff] = c.state();
      }
    }
  }

}
