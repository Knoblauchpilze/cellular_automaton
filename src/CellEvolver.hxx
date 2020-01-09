#ifndef    CELL_EVOLVER_HXX
# define   CELL_EVOLVER_HXX

# include "CellEvolver.hh"

namespace cellulator {

  inline
  CellEvolver::CellEvolver(const std::vector<unsigned>& live,
                           const std::vector<unsigned>& survive):
    utils::CoreObject(std::string("evolver")),

    m_born(),
    m_survive()
  {
    setService("cells");

    registerVector(live, true);
    registerVector(survive, false);
  }

  inline
  void
  CellEvolver::clear() noexcept {
    m_born.clear();
    m_survive.clear();
  }

  inline
  bool
  CellEvolver::addBornOption(unsigned neighbor) noexcept {
    std::pair<std::unordered_set<unsigned>::iterator, bool> check = m_born.insert(neighbor);
    return check.second;
  }

  inline
  bool
  CellEvolver::addSurvivingOption(unsigned neighbor) noexcept {
    std::pair<std::unordered_set<unsigned>::iterator, bool> check = m_survive.insert(neighbor);
    return check.second;
  }

  inline
  bool
  CellEvolver::isBorn(unsigned neighbor) {
    return m_born.count(neighbor) > 0;
  }

  inline
  bool
  CellEvolver::survives(unsigned neighbor) {
    return m_survive.count(neighbor) > 0;
  }

  inline
  void
  CellEvolver::registerVector(const std::vector<unsigned>& vec,
                              bool live) noexcept
  {
    for (unsigned id = 0u ; id < vec.size() ; ++id) {
      if (live) {
        addBornOption(vec[id]);
      }
      else {
        addSurvivingOption(vec[id]);
      }
    }
  }

}

#endif    /* CELL_EVOLVER_HXX */
