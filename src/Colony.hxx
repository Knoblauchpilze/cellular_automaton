#ifndef    COLONY_HXX
# define   COLONY_HXX

# include "Colony.hh"

namespace cellulator {

  inline
  utils::Boxi
  Colony::getArea() noexcept {
    // TODO: Restore that.
    return utils::Boxi();
  }

  inline
  utils::Boxi
  Colony::fetchCells(std::vector<State>& cells,
                     const utils::Boxf& area)
  {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

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

}

#endif    /* COLONY_HXX */
