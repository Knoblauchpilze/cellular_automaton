
# include "CellsQuadTree.hh"

namespace cellulator {

  CellsQuadTree::CellsQuadTree(const utils::Sizei& dims,
                               const utils::Sizei& nodeDims,
                               const rules::Type& ruleset,
                               const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_ruleset(ruleset),
    m_nodesSize(nodeDims),
    m_size()
  {
    setService("cells_quadtree");

    // Check consistency.
    if (!m_nodesSize.valid()) {
      error(
        std::string("Could not create cells quadtree"),
        std::string("Invalid dimensions ") + m_nodesSize.toString()
      );
    }

    reset(dims);
  }

  utils::Boxi
  CellsQuadTree::fetchCells(std::vector<Cell>& cells,
                            const utils::Boxf& area)
  {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Clamp the area to get only relevant cells.
    utils::Boxi evenized = fromFPCoordinates(area);

    // Resize the output vector if needed.
    if (cells.size() != static_cast<unsigned>(evenized.area())) {
      cells.resize(evenized.area());
    }

    // Populate the needed cells.
    int xMin = evenized.getLeftBound();
    int yMin = evenized.getBottomBound();
    int xMax = evenized.getRightBound();
    int yMax = evenized.getTopBound();

    for (int y = yMin ; y < yMax ; ++y) {
      // Convert logical coordinates to valid cells coordinates.
      int offset = (y - yMin) * evenized.w();
      // int rOffset = (y + m_size.h() / 2) * m_size.w();

      for (int x = xMin ; x < xMax ; ++x) {
        // Convert the `x` coordinate similarly to the `y` coordinate.
        int xOff = x - xMin;
        // int rXOff = x + m_size.w() / 2;

        // Check whether the cell exists in the internal data.
        // int coord = rOffset + rXOff;
        Cell c(State::Dead);
        // if (rOffset >= 0 && rOffset < static_cast<int>(m_cells.size()) &&
        //     rXOff >= 0 && rXOff < m_size.w())
        // {
        //   c = m_cells[coord];
        // }

        cells[offset + xOff] = c;
      }
    }

    return evenized;
  }

  void
  CellsQuadTree::reset(const utils::Sizei& dims) {
    // Make the dims even if this is not already the case and assign
    // the dimensions. Making things even will ease the process when
    // fetching some cells.
    utils::Sizei evenized(dims);
    evenized.w() += evenized.w() % 2;
    evenized.h() += evenized.h() % 2;

    if (evenized != dims) {
      log(
        std::string("Changed dimensions for colony from ") + dims.toString() + " to " + evenized.toString(),
        utils::Level::Warning
      );
    }

    m_size = evenized;

    // Reset the cells.
    // TODO: Implementation.
    log("Should resize quad tree with specified dims " + dims.toString());
    // m_cells->reset(evenized);

    // Fill in with `Dead` cells.
    // std::fill(m_cells.begin(), m_cells.end(), Cell(State::Dead, m_ruleset));
  }

}
