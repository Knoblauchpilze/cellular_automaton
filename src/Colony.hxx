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
