#ifndef    COLONY_HXX
# define   COLONY_HXX

# include "Colony.hh"
# include <algorithm>

namespace cellulator {

  inline
  Colony::~Colony() {
    // Stops the colony.
    stop();
  }

  inline
  utils::Sizei
  Colony::getSize() noexcept {
    Guard guard(m_propsLocker);

    return m_dims;
  }

  inline
  unsigned
  Colony::getWorkerThreadCount() noexcept {
    return 3u;
  }

  inline
  utils::Boxi
  Colony::fromFPCoordinates(const utils::Boxf& in) noexcept {
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

  inline
  void
  Colony::reset(const utils::Sizei& dims) {
    // Make the dims even if this is not already the case and assign
    // the dimensions. Making things even will ease the process when
    // fetching some cells.
    m_dims.w() = dims.w() + dims.w() % 2;
    m_dims.h() = dims.h() + dims.h() % 2;

    if (m_dims != dims) {
      log(
        std::string("Changed dimensions for colony from ") + dims.toString() + " to " + m_dims.toString(),
        utils::Level::Warning
      );
    }

    // Reset the cells.
    m_cells.resize(m_dims.area());

    // Fill in with `Dead` cells.
    std::fill(m_cells.begin(), m_cells.end(), State::Dead);
  }

  inline
  void
  Colony::randomize() {
    // Randomize each cell.
    std::for_each(
      m_cells.begin(),
      m_cells.end(),
      [](Cell& c) {
        c.randomize();
      }
    );
  }

}

#endif    /* COLONY_HXX */
