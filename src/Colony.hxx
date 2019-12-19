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
  void
  Colony::reset(const utils::Sizei& dims) {
    // Reset the cells.
    m_cells.resize(dims.area());

    // Fill in with `Dead` cells.
    std::fill(m_cells.begin(), m_cells.end(), State::Dead);

    // Assign the dimensions of the colony.
    m_dims = dims;
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
