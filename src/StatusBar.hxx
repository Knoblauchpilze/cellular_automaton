#ifndef    STATUS_BAR_HXX
# define   STATUS_BAR_HXX

# include "StatusBar.hh"

namespace cellulator {

  inline
  void
  StatusBar::onGenerationComputed(unsigned generation) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    sdl::graphic::LabelWidget* gen = getGenerationLabel();
    if (gen == nullptr) {
      log(
        std::string("Could not find label to update generation to ") + std::to_string(generation),
        utils::Level::Error
      );

      return;
    }

    gen->setText(std::string("Generation: ") + std::to_string(generation));
  }

  inline
  float
  StatusBar::getStatusMaxHeight() noexcept {
    return 30.0f;
  }

  inline
  const char*
  StatusBar::getInfoLabelFont() noexcept {
    return "data/fonts/Goodtime.ttf";
  }

  inline
  float
  StatusBar::getGlobalMargins() noexcept {
    return 2.0f;
  }

  inline
  float
  StatusBar::getComponentMargins() noexcept {
    return 7.0f;
  }

  inline
  const char*
  StatusBar::getMouseCoordsLabelName() noexcept {
    return "mouse_coords_label";
  }

  inline
  const char*
  StatusBar::getGenerationLabelName() noexcept {
    return "generation_label";
  }

  inline
  const char*
  StatusBar::getAliveCellsLabelName() noexcept {
    return "alive_cells_label";
  }

  inline
  sdl::graphic::LabelWidget*
  StatusBar::getMouseCoordsLabel() {
    return getChildAs<sdl::graphic::LabelWidget>(getMouseCoordsLabelName());
  }

  inline
  sdl::graphic::LabelWidget*
  StatusBar::getGenerationLabel() {
    return getChildAs<sdl::graphic::LabelWidget>(getGenerationLabelName());
  }

  sdl::graphic::LabelWidget*
  StatusBar::getAliveCellsLabel() {
    return getChildAs<sdl::graphic::LabelWidget>(getAliveCellsLabelName());
  }

}

#endif    /* STATUS_BAR_HXX */
