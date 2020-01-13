#ifndef    CELL_BRUSH_HXX
# define   CELL_BRUSH_HXX

# include "CellBrush.hh"

namespace cellulator {

  inline
  CellBrushShPtr
  CellBrush::fromFile(const std::string& file) {
    // Assume no inversion of the `y` axis.
    return std::make_shared<CellBrush>(
      file, true
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
  constexpr char
  CellBrush::getDeadCellCharacter() noexcept {
    return '0';
  }

  inline
  constexpr char
  CellBrush::getLiveCellCharacter() noexcept {
    return '2';
  }

  inline
  bool
  CellBrush::isMonotonic() const noexcept {
    return m_monotonic;
  }

}

#endif    /* CELL_BRUSH_HXX */
