#ifndef    STATUS_BAR_HXX
# define   STATUS_BAR_HXX

# include "StatusBar.hh"

namespace cellulator {

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
