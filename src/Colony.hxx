#ifndef    COLONY_HXX
# define   COLONY_HXX

# include "Colony.hh"

namespace cellulator {

  inline
  Colony::~Colony() {
    // Stops the colony.
    stop();
  }

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
    return m_cells->fetchCells(cells, area);
  }

  inline
  unsigned
  Colony::getWorkerThreadCount() noexcept {
    return 3u;
  }

  inline
  utils::Sizei
  Colony::getQuadTreeNodeSize() noexcept {
    // return utils::Sizei(256, 256);
    return utils::Sizei(8, 8);
  }

}

#endif    /* COLONY_HXX */
