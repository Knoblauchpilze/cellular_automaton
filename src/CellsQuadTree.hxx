#ifndef    CELLS_QUAD_TREE_HXX
# define   CELLS_QUAD_TREE_HXX

# include "CellsQuadTree.hh"

namespace cellulator {

  inline
  utils::Sizei
  CellsQuadTree::getSize() noexcept {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    return m_size;
  }

  inline
  utils::Boxi
  CellsQuadTree::getLiveArea() noexcept {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    return m_liveArea;
  }

  inline
  void
  CellsQuadTree::randomize() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    m_root->randomize();
  }

  inline
  void
  CellsQuadTree::step() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    m_root->step();
  }

  inline
  void
  CellsQuadTree::expand() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Expand the root using the dedicated handler
    m_root = m_root->expand(m_root, m_liveArea);

    // Update the size of the colony.
    m_size = m_root->getArea().toSize();
  }

  inline
  utils::Boxi
  CellsQuadTree::fromFPCoordinates(const utils::Boxf& in) const noexcept {
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

#endif    /* CELLS_QUAD_TREE_HXX */
