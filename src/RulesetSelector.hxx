#ifndef    RULESET_SELECTOR_HXX
# define   RULESET_SELECTOR_HXX

# include "RulesetSelector.hxx"

namespace cellulator {

  inline
  const char*
  RulesetSelector::getGeneralTextFont() noexcept {
    return "data/fonts/times.ttf";
  }

  inline
  float
  RulesetSelector::getGlobalMargins() noexcept {
    return 5.0f;
  }

  inline
  float
  RulesetSelector::getCheckboxMaxHeight() noexcept {
    return 100.0f;
  }

  inline
  utils::Sizef
  RulesetSelector::getIconMaxSize() noexcept {
    return utils::Sizef(50.0f, 50.0f);
  }

  inline
  std::string
  RulesetSelector::generateNameForLiveNeighbors(unsigned number) noexcept {
    return "live_cb_" + std::to_string(number);
  }

  inline
  std::string
  RulesetSelector::generateNameForDeadNeighbors(unsigned number) noexcept {
    return "dead_cb_" + std::to_string(number);
  }

  inline
  sdl::graphic::Checkbox*
  RulesetSelector::getCheckboxFromName(const std::string& name) {
    return getChildAs<sdl::graphic::Checkbox>(name);
  }

}

#endif    /* RULESET_SELECTOR_HXX */
