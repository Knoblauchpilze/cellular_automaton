#ifndef    COLONY_HXX
# define   COLONY_HXX

# include "Colony.hh"

namespace cellulator {

  inline
  utils::Boxi
  Colony::getArea() noexcept {
    return m_cells->getLiveArea();
  }

  inline
  utils::Boxi
  Colony::fetchCells(std::vector<State>& cells,
                     const utils::Boxf& area)
  {
    // TODO: Restore that.
    utils::Boxi iArea(
      static_cast<int>(std::round(area.x())),
      static_cast<int>(std::round(area.y())),
      static_cast<int>(std::round(area.w())),
      static_cast<int>(std::round(area.h()))
    );

    cells.resize(iArea.area());
    std::fill(cells.begin(), cells.end(), State::Dead);

    return iArea;
  }

  inline
  std::pair<State, int>
  Colony::getCellState(const utils::Vector2i& coord) {
    return m_cells->getCellStatus(coord);
  }

  inline
  utils::Sizei
  Colony::getCellBlockDims() noexcept {
    return utils::Sizei(8, 8);
  }

}

#endif    /* COLONY_HXX */
