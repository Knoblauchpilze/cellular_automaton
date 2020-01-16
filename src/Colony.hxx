#ifndef    COLONY_HXX
# define   COLONY_HXX

# include "Colony.hh"

namespace cellulator {

  inline
  utils::Boxf
  Colony::getArea() noexcept {
    return m_cells->getLiveArea();
  }

  inline
  unsigned
  Colony::getGeneration() noexcept {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);
    
    return m_generation;
  }

  inline
  unsigned
  Colony::getLiveCellsCount() noexcept {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    return m_liveCells;
  }

  inline
  utils::Boxi
  Colony::fetchCells(std::vector<std::pair<State, unsigned>>& cells,
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

    // Fetch the cells from the internal data.
    m_cells->fetchCells(cells, evenized);

    // Return the area that is actually represented by the
    // returns `cells` array.
    return evenized;
  }

  inline
  std::pair<State, int>
  Colony::getCellState(const utils::Vector2i& coord) {
    return m_cells->getCellStatus(coord);
  }

  inline
  void
  Colony::setRuleset(CellEvolverShPtr ruleset) {
    // Call the dedicated handler.
    m_cells->setRuleset(ruleset);
  }

  inline
  unsigned
  Colony::paint(const CellBrush& brush,
                const utils::Vector2i& coord)
  {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    m_liveCells = m_cells->paint(brush, coord);

    return m_liveCells;
  }

  inline
  utils::Sizei
  Colony::getCellBlockDims() noexcept {
    return utils::Sizei(256, 256);
  }

  inline
  utils::Boxi
  Colony::fromFPCoordinates(const utils::Boxf& in) const noexcept {
    // First compute extremum for the input box.
    float minX = std::floor(in.getLeftBound());
    float maxX = std::ceil(in.getRightBound());

    float minY = std::floor(in.getBottomBound());
    float maxY = std::ceil(in.getTopBound());

    int w = static_cast<int>(maxX - minX);
    int h = static_cast<int>(maxY - minY);

    // Determine whether the dimensions are even: if this is the
    // case we are guaranteed to have an integer coordinates center.
    // otherwise we need to add one to the dimension and offset the
    // center so that it still covers the area provided in input.
    int rW = w % 2;
    int rH = h % 2;

    int cX = (maxX + minX) / 2;
    if (rW != 0) {
      // Move the center to the left and add one to the width.
      // Given the rules of the integer division, the center is
      // already offseted by one.
      ++w;
    }

    int cY = (maxY + minY) / 2;
    if (rH != 0) {
      // Similar to what happens for `x`.
      ++h;
    }

    return utils::Boxi(cX, cY, w, h);
  }

}

#endif    /* COLONY_HXX */
