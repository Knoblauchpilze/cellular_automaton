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
    return m_cells->getSize();
  }

  inline
  utils::Boxi
  Colony::fetchCells(std::vector<Cell>& cells,
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
    return utils::Sizei(256, 256);
  }

}

#endif    /* COLONY_HXX */
