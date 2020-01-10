#ifndef    RENDERING_PROPERTIES_HXX
# define   RENDERING_PROPERTIES_HXX

# include "RenderingProperties.hh"
# include <sdl_graphic/Validator_utils.hxx>

namespace cellulator {

  inline
  const char*
  RenderingProperties::getGeneralTextFont() noexcept {
    return "data/fonts/times.ttf";
  }

  inline
  const char*
  RenderingProperties::getMaxAgeTextboxName() noexcept {
    return "max_age_textbox";
  }

  inline
  unsigned
  RenderingProperties::convertToUnsigned(const std::string& text,
                                         unsigned def,
                                         bool& converted) noexcept
  {
    // Use the dedicated handler to handle the conversion.
    bool valid = false;

    int conv = sdl::graphic::convertToInt(text, &valid);

    if (!valid || conv < 0) {
      converted = false;
      return def;
    }

    converted = true;
    return static_cast<unsigned>(conv);
  }

  inline
  sdl::core::engine::Color
  RenderingProperties::getDefaultColor() noexcept {
    return sdl::core::engine::Color::fromRGB(0.1255f, 0.4196f, 0.7961f);
  }

  inline
  float
  RenderingProperties::getGlobalMargins() noexcept {
    return 5.0f;
  }

  inline
  float
  RenderingProperties::getComponentMargins() noexcept {
    return 5.0f;
  }

  inline
  float
  RenderingProperties::getMaxAgeSelectionHeight() noexcept {
    return 60.0f;
  }

  inline
  float
  RenderingProperties::getApplyButtonHeight() noexcept {
    return 50.0f;
  }

  inline
  utils::Sizef
  RenderingProperties::getPaletteMaxSize() noexcept {
    return utils::Sizef(80.0f, 50.0f);
  }

  inline
  unsigned
  RenderingProperties::getDefaultMaxAge() noexcept {
    return 10u;
  }

  inline
  std::string
  RenderingProperties::generateNameForPalette(unsigned index) noexcept {
    return std::string("palette_selector_") + std::to_string(index);
  }

  inline
  sdl::core::SdlWidget*
  RenderingProperties::getPaletteFromName(const std::string& name) {
    return getChildAs<sdl::core::SdlWidget>(name);
  }

  inline
  sdl::graphic::TextBox*
  RenderingProperties::getMaxAgeTextbox() {
    return getChildAs<sdl::graphic::TextBox>(getMaxAgeTextboxName());
  }

}

#endif    /* RENDERING_PROPERTIES_HXX */
