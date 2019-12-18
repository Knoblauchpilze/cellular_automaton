#ifndef    COLONY_STATUS_HXX
# define   COLONY_STATUS_HXX

# include "ColonyStatus.hh"

namespace cellulator {

  inline
  float
  ColonyStatus::getStatusMaxHeight() noexcept {
    return 200.0f;
  }

  inline
  float
  ColonyStatus::getGlobalMargins() noexcept {
    return 0.0f;
  }

  inline
  float
  ColonyStatus::getComponentMargins() noexcept {
    return 15.0f;
  }

  inline
  const char*
  ColonyStatus::getStartSimulationButtonName() noexcept {
    return "colony_status_start";
  }

  inline
  const char*
  ColonyStatus::getStopSimulationButtonName() noexcept {
    return "colony_status_stop";
  }

  inline
  const char*
  ColonyStatus::getRandomGenerationButtonName() noexcept {
    return "colony_status_random";
  }

  inline
  const char*
  ColonyStatus::getButtonFontName() noexcept {
    return "data/fonts/Goodtime.ttf";
  }

}

#endif    /* COLONY_STATUS_HXX */
