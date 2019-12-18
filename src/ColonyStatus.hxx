#ifndef    COLONY_STATUS_HXX
# define   COLONY_STATUS_HXX

# include "ColonyStatus.hh"

namespace cellulator {

  inline
  sdl::graphic::Button&
  ColonyStatus::getStartSimilationButton() {
    return *getChildAs<sdl::graphic::Button>(getStartSimulationButtonName());
  }

  inline
  sdl::graphic::Button&
  ColonyStatus::getStopSimilationButton() {
    return *getChildAs<sdl::graphic::Button>(getStopSimulationButtonName());
  }

  inline
  sdl::graphic::Button&
  ColonyStatus::getNextStepButton() {
    return *getChildAs<sdl::graphic::Button>(getNextStepButtonName());
  }

  inline
  sdl::graphic::Button&
  ColonyStatus::getGenerateColonyButton() {
    return *getChildAs<sdl::graphic::Button>(getRandomGenerationButtonName());
  }

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
  ColonyStatus::getNextStepButtonName() noexcept {
    return "colony_status_next";
  }

  inline
  const char*
  ColonyStatus::getButtonFontName() noexcept {
    return "data/fonts/Goodtime.ttf";
  }

  inline
  utils::Sizef
  ColonyStatus::getSimulationButtonMaxSize() noexcept {
    return utils::Sizef(100.0f, getStatusMaxHeight() - getGlobalMargins());
  }

}

#endif    /* COLONY_STATUS_HXX */
