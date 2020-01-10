#ifndef    COLOR_PALETTE_HXX
# define   COLOR_PALETTE_HXX

# include "ColorPalette.hh"

namespace cellulator {

  inline
  ColorPalette::ColorPalette(unsigned maxAge):
    utils::CoreObject(std::string("palette")),

    m_maxAge(maxAge),
    m_gradient(getDefaultPalette())
  {
    setService("color");
  }

  inline
  void
  ColorPalette::setGradient(sdl::core::engine::GradientShPtr palette) {
    m_gradient = palette;
  }

  inline
  sdl::core::engine::Color
  ColorPalette::colorize(unsigned age) {
    if (m_gradient == nullptr) {
      error(
        std::string("Could not colorize cell ") + std::to_string(age) + " old",
        std::string("No palette provided")
      );
    }

    // Modulate the age with the maximum age.
    float p = 1.0f * std::min(age, m_maxAge) / m_maxAge;

    // Use the gradient to retrieve the corresponding color.
    return m_gradient->getColorAt(p);
  }

  inline
  sdl::core::engine::GradientShPtr
  ColorPalette::getDefaultPalette() noexcept {
    sdl::core::engine::GradientShPtr palette;

    palette = std::make_shared<sdl::core::engine::Gradient>(
      "default_palette",
      sdl::core::engine::gradient::Mode::Linear
    );

    // Define a palette using a regular temperature-like gradient.
    palette->setColorAt(0.0000f, sdl::core::engine::Color::NamedColor::Indigo);
    palette->setColorAt(0.1666f, sdl::core::engine::Color::NamedColor::Purple);
    palette->setColorAt(0.3333f, sdl::core::engine::Color::NamedColor::Blue);
    palette->setColorAt(0.5000f, sdl::core::engine::Color::NamedColor::Green);
    palette->setColorAt(0.6666f, sdl::core::engine::Color::NamedColor::Yellow);
    palette->setColorAt(0.8333f, sdl::core::engine::Color::NamedColor::Orange);
    palette->setColorAt(1.0000f, sdl::core::engine::Color::NamedColor::Red);

    return palette;
  }

}

#endif    /* COLOR_PALETTE_HXX */
