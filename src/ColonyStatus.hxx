#ifndef    COLONY_STATUS_HXX
# define   COLONY_STATUS_HXX

# include "ColonyStatus.hh"

namespace cellulator {

  inline
  sdl::graphic::Button&
  ColonyStatus::getFitToContentButton() {
    return *getChildAs<sdl::graphic::Button>(getFitToContentButtonName());
  }

  inline
  sdl::graphic::Button&
  ColonyStatus::getGenerateColonyButton() {
    return *getChildAs<sdl::graphic::Button>(getRandomGenerationButtonName());
  }

  inline
  void
  ColonyStatus::onSimulationHalted() {
    // Untoggle the start simulation button.
    getStartSimulationButton().toggle(false);
  }

  inline
  float
  ColonyStatus::getStatusMaxHeight() noexcept {
    return 200.0f;
  }

  inline
  float
  ColonyStatus::getButtonBorderSize() noexcept {
    return 5.0f;
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
  ColonyStatus::getFitToContentButtonName() noexcept {
    return "colony_fit_to_content";
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

  inline
  sdl::graphic::Button&
  ColonyStatus::getStartSimulationButton() {
    return *getChildAs<sdl::graphic::Button>(getStartSimulationButtonName());
  }

  inline
  sdl::graphic::Button&
  ColonyStatus::getStopSimulationButton() {
    return *getChildAs<sdl::graphic::Button>(getStopSimulationButtonName());
  }

  inline
  sdl::graphic::Button&
  ColonyStatus::getNextStepButton() {
    return *getChildAs<sdl::graphic::Button>(getNextStepButtonName());
  }

}

#endif    /* COLONY_STATUS_HXX */
