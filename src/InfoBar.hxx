#ifndef    INFO_BAR_HXX
# define   INFO_BAR_HXX

# include "InfoBar.hh"

namespace cellulator {

  inline
  void
  InfoBar::onGenerationComputed(unsigned generation) {
    sdl::graphic::LabelWidget* gen = getGenerationLabel();
    if (gen == nullptr) {
      warn("Could not find label to update generation to " + std::to_string(generation));
      return;
    }

    gen->setText(std::string("Generation: ") + std::to_string(generation));
  }

  inline
  void
  InfoBar::onSelectedCellChanged(utils::Vector2i coords,
                                 int age)
  {
    std::string ageStr;
    if (age < 0) {
      ageStr += "(dead)";
    }
    else {
      ageStr += "(age: " + std::to_string(age) + ")";
    }

    sdl::graphic::LabelWidget* mc = getMouseCoordsLabel();
    if (mc == nullptr) {
      warn("Could not find label to update coordinates to " + coords.toString());
      return;
    }
    else {
      mc->setText(
        std::string("x: ") + std::to_string(coords.x()) +
        " y: " + std::to_string(coords.y()) + " " +
        ageStr
      );
    }
  }

  void
  InfoBar::onAliveCellsChanged(unsigned count) {
    sdl::graphic::LabelWidget* ac = getAliveCellsLabel();
    if (ac == nullptr) {
      warn("Could not find label to update alive cells count to " + std::to_string(count));
      return;
    }

    ac->setText(std::string("Alive: ") + std::to_string(count));
  }

  inline
  sdl::graphic::Button&
  InfoBar::getDisplayGridButton() {
    return *getChildAs<sdl::graphic::Button>(getDisplayGridButtonName());
  }

  inline
  float
  InfoBar::getStatusMaxHeight() noexcept {
    return 30.0f;
  }

  inline
  const char*
  InfoBar::getInfoLabelFont() noexcept {
    return "data/fonts/Goodtime.ttf";
  }

  inline
  float
  InfoBar::getGlobalMargins() noexcept {
    return 2.0f;
  }

  inline
  float
  InfoBar::getComponentMargins() noexcept {
    return 7.0f;
  }

  inline
  const char*
  InfoBar::getMouseCoordsLabelName() noexcept {
    return "mouse_coords_label";
  }

  inline
  const char*
  InfoBar::getGenerationLabelName() noexcept {
    return "generation_label";
  }

  inline
  const char*
  InfoBar::getAliveCellsLabelName() noexcept {
    return "alive_cells_label";
  }

  inline
  const char*
  InfoBar::getDisplayGridButtonName() noexcept {
    return "grid_display_button";
  }

  inline
  sdl::graphic::LabelWidget*
  InfoBar::getMouseCoordsLabel() {
    return getChildAs<sdl::graphic::LabelWidget>(getMouseCoordsLabelName());
  }

  inline
  sdl::graphic::LabelWidget*
  InfoBar::getGenerationLabel() {
    return getChildAs<sdl::graphic::LabelWidget>(getGenerationLabelName());
  }

  inline
  sdl::graphic::LabelWidget*
  InfoBar::getAliveCellsLabel() {
    return getChildAs<sdl::graphic::LabelWidget>(getAliveCellsLabelName());
  }

  inline
  void
  InfoBar::onGridToggled(std::string /*dummy*/,
                         bool toggled) {
    // Fire this class' signal.
    onGridDisplayChanged.safeEmit(
      std::string("onGridDisplayChanged(") + std::to_string(toggled) + ")",
      toggled
    );
  }

}

#endif    /* INFO_BAR_HXX */
