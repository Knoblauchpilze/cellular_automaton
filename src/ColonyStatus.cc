
# include "ColonyStatus.hh"
# include <sdl_graphic/LinearLayout.hh>
# include <sdl_graphic/Button.hh>

namespace cellulator {

  ColonyStatus::ColonyStatus(const utils::Sizef& hint,
                             sdl::core::SdlWidget* parent):
    sdl::core::SdlWidget(std::string("colony_status"),
                         hint,
                         parent),

    m_propsLocker()
  {
    build();
  }

  void
  ColonyStatus::build() {
    // Set a focus policy which do not allow to hover or click on this widget.
    setFocusPolicy(sdl::core::FocusPolicy());

    // The colony status is composed of a display allowing to display some
    // options to animate the colony and possibly generate some random data
    // to create a new one.
    sdl::graphic::LinearLayoutShPtr layout = std::make_shared<sdl::graphic::LinearLayout>(
      "colony_status_layout",
      this,
      sdl::graphic::LinearLayout::Direction::Horizontal,
      getGlobalMargins(),
      getComponentMargins()
    );

    // And assign the layout to this widget.
    setLayout(layout);

    sdl::graphic::Button* generate = new sdl::graphic::Button(
      getRandomGenerationButtonName(),
      "Generate",
      "data/img/generate.bmp",
      getButtonFontName(),
      sdl::graphic::button::Type::Regular,
      15u,
      this,
      getButtonBorderSize(),
      utils::Sizef(),
      sdl::core::engine::Color::fromRGB(0.7031f, 0.7031f, 0.7031f)
    );
    if (generate == nullptr) {
      error(
        std::string("Could not create colony status"),
        std::string("Random generation button not allocated")
      );
    }

    sdl::graphic::Button* fitToContent = new sdl::graphic::Button(
      getFitToContentButtonName(),
      std::string(),
      std::string("data/img/fit.bmp"),
      getButtonFontName(),
      sdl::graphic::button::Type::Regular,
      15u,
      this,
      getButtonBorderSize(),
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::White
    );
    if (fitToContent == nullptr) {
      error(
        std::string("Could not create colony status"),
        std::string("Fit to content button not allocated")
      );
    }

    sdl::graphic::Button* start = new sdl::graphic::Button(
      getStartSimulationButtonName(),
      std::string(),
      std::string("data/img/start.bmp"),
      getButtonFontName(),
      sdl::graphic::button::Type::Toggle,
      15u,
      this,
      getButtonBorderSize(),
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::White
    );
    if (start == nullptr) {
      error(
        std::string("Could not create colony status"),
        std::string("Start simulation button not allocated")
      );
    }

    sdl::graphic::Button* next = new sdl::graphic::Button(
      getNextStepButtonName(),
      std::string(),
      std::string("data/img/next.bmp"),
      getButtonFontName(),
      sdl::graphic::button::Type::Regular,
      15u,
      this,
      getButtonBorderSize(),
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::White
    );
    if (next == nullptr) {
      error(
        std::string("Could not create colony status"),
        std::string("Next step button not allocated")
      );
    }

    sdl::graphic::Button* stop = new sdl::graphic::Button(
      getStopSimulationButtonName(),
      std::string(),
      std::string("data/img/stop.bmp"),
      getButtonFontName(),
      sdl::graphic::button::Type::Regular,
      15u,
      this,
      getButtonBorderSize(),
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::White
    );
    if (stop == nullptr) {
      error(
        std::string("Could not create colony status"),
        std::string("Stop simulation button not allocated")
      );
    }

    // Assign maximum size to component(s) if needed.
    fitToContent->setMaxSize(getSimulationButtonMaxSize());
    start->setMaxSize(getSimulationButtonMaxSize());
    stop->setMaxSize(getSimulationButtonMaxSize());
    next->setMaxSize(getSimulationButtonMaxSize());

    // Add each element to the layout.
    layout->addItem(generate);
    layout->addItem(fitToContent);
    layout->addItem(start);
    layout->addItem(next);
    layout->addItem(stop);
  }

}
