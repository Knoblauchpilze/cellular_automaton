
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
    m_size(),
    m_liveArea(),

    m_root(nullptr)
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
  CellsQuadTree::fetchCells(std::vector<State>& cells,
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

    // Reset with dead cells as to not display some randomness.
    std::fill(cells.begin(), cells.end(), State::Dead);

    // Fetch the cells from the internal data.
    m_root->fetchCells(cells, evenized);

    // Return the area that is actually represented by the
    // returns `cells` array.
    return evenized;
  }

  std::vector<ColonyTileShPtr>
  CellsQuadTree::generateSchedule() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // We need to traverse the children of this node if any and create the corresponding
    // rendering tiles.
    std::vector<ColonyTileShPtr> tiles;
    m_root->registerTiles(tiles);

    return tiles;
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

    // Account for some security buffer space around the real colony.
    // This will allow for cells to be generated on the whole area and
    // still be able to grow even in the first step.
    m_size = utils::Sizei(evenized.w() * 2, evenized.h() * 2);
    m_liveArea = utils::Boxi::fromSize(evenized, true);

    // Create the root node: we want to create a node which is at least
    // the size of `m_nodesSize`, even if it is larger than the desired
    // dimensions.
    int w = static_cast<int>(std::ceil(1.0f * m_size.w() / m_nodesSize.w())) * m_nodesSize.w();
    int h = static_cast<int>(std::ceil(1.0f * m_size.h() / m_nodesSize.h())) * m_nodesSize.h();

    utils::Boxi area(0, 0, w, h);

    m_root = std::make_shared<CellsQuadTreeNode>(area, m_ruleset, m_nodesSize);
  }

}
