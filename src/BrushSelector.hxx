#ifndef    BRUSH_SELECTOR_HXX
# define   BRUSH_SELECTOR_HXX

# include "BrushSelector.hh"

namespace cellulator {

  inline
  const char*
  BrushSelector::getGeneralTextFont() noexcept {
    return "data/fonts/times.ttf";
  }

  inline
  float
  BrushSelector::getGlobalMargins() noexcept {
    return 5.0f;
  }

  inline
  float
  BrushSelector::getComponentMargins() noexcept {
    return 2.0f;
  }

  inline
  float
  BrushSelector::getBrushSelectMaxHeight() noexcept {
    return 50.0f;
  }

  inline
  sdl::core::engine::Color
  BrushSelector::getDefaultColor() noexcept {
    return sdl::core::engine::Color::fromRGB(0.1255f, 0.4196f, 0.7961f);
  }

  inline
  const char*
  BrushSelector::getBrushSizeSliderName() noexcept {
    return "brush_size_slider";
  }

  inline
  sdl::graphic::Button*
  BrushSelector::getBrushButtonFromName(const std::string& name) {
    return getChildAs<sdl::graphic::Button>(name);
  }

  inline
  sdl::graphic::Slider&
  BrushSelector::getBrushSizeSlize() {
    return *getChildAs<sdl::graphic::Slider>(getBrushSizeSliderName());
  }

  inline
  sdl::graphic::Button*
  BrushSelector::createButtonFromBrushName(const std::string& name,
                                           const std::string& icon)
  {
    // Create the button.
    sdl::graphic::Button* b = new sdl::graphic::Button(
      std::string("button_for_") + name,
      name,
      icon,
      getGeneralTextFont(),
      sdl::graphic::button::Type::Toggle,
      15u,
      this,
      3.0f,
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::CorneFlowerBlue
    );

    if (b == nullptr) {
      error(
        std::string("Could not create brush selection panel"),
        std::string("Could not create brush button")
      );
    }

    b->setMaxSize(
      utils::Sizef(
        std::numeric_limits<float>::max(),
        getBrushSelectMaxHeight()
      )
    );

    // Register this button in the internal table.
    m_brushes[b->getName()] = name;

    // Connect the button to the local slot.
    b->onButtonToggled.connect_member<BrushSelector>(
      this,
      &BrushSelector::onBrushSelected
    );

    return b;
  }

}

#endif    /* BRUSH_SELECTOR_HXX */
