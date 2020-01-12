#ifndef    CELL_BRUSH_HXX
# define   CELL_BRUSH_HXX

# include "CellBrush.hh"

namespace cellulator {

  inline
  CellBrushShPtr
  CellBrush::fromFile(const std::string& file) {
    // Assume no inversion of the `y` axis.
    return std::make_shared<CellBrush>(
      file, false
    );
  }

  inline
  bool
  CellBrush::valid() const noexcept {
    return isMonotonic() || (m_size.valid() && m_size.area() == static_cast<int>(m_data.size()));
  }

  inline
  utils::Sizei
  CellBrush::getSize() const noexcept {
    return m_size;
  }

  inline
  State
  CellBrush::getStateAt(const utils::Vector2i& coord) const noexcept {
    // Use the dedicated handler.
    return getStateAt(coord.x(), coord.y());
  }

  inline
  State
  CellBrush::getStateAt(int x,
                        int y) const noexcept
  {

    if (!valid()) {
      return State::Dead;
    }

    // In case the coordinate is not inside the area covered by this
    // brush return `Dead` as well.
    if (x < 0 || y < 0 || x >= m_size.w() || y >= m_size.h()) {
      return State::Dead;
    }

    // If the brush is monotonic, return the associated state.
    if (isMonotonic()) {
      return m_monotonicState;
    }

    // Convert the coordinate to a one-dimensional offset. We can be
    // certain that this will yield a valid index as we checked that
    // the coordinate was inside the area covered by this brush.
    int off = y * m_size.w() + x;

    return m_data[off];
  }

  inline
  bool
  CellBrush::isMonotonic() const noexcept {
    return m_monotonic;
  }

}

#endif    /* CELL_BRUSH_HXX */
