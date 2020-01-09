
# include "InfoBar.hh"
# include <sdl_graphic/LinearLayout.hh>

namespace cellulator {

  InfoBar::InfoBar(const utils::Sizef& hint,
                   sdl::core::SdlWidget* parent):
    sdl::core::SdlWidget(std::string("info_bar"),
                         hint,
                         parent),

    m_propsLocker()
  {
    build();
  }

  void
  InfoBar::build() {
    // Set a focus policy which do not allow to hover or click on this widget.
    setFocusPolicy(sdl::core::FocusPolicy());

    // This info bar is composed of a display allowing to display the current
    // coordinates of the mouse cursor in real world frame along with a label
    // displaying the total rendering area.
    sdl::graphic::LinearLayoutShPtr layout = std::make_shared<sdl::graphic::LinearLayout>(
      "info_bar_layout",
      this,
      sdl::graphic::LinearLayout::Direction::Horizontal,
      getGlobalMargins(),
      getComponentMargins()
    );

    // And assign the layout to this widget.
    setLayout(layout);

    sdl::graphic::LabelWidget* mouseCoords = new sdl::graphic::LabelWidget(
      getMouseCoordsLabelName(),
      "x: 0 y: 0",
      getInfoLabelFont(),
      15u,
      sdl::graphic::LabelWidget::HorizontalAlignment::Left,
      sdl::graphic::LabelWidget::VerticalAlignment::Center,
      this,
      sdl::core::engine::Color::NamedColor::Gray
    );
    if (mouseCoords == nullptr) {
      error(
        std::string("Could not create info bar"),
        std::string("Mouse coordinates label not allocated")
      );
    }

    sdl::graphic::LabelWidget* generation = new sdl::graphic::LabelWidget(
      getGenerationLabelName(),
      "Generation: 0",
      getInfoLabelFont(),
      15u,
      sdl::graphic::LabelWidget::HorizontalAlignment::Center,
      sdl::graphic::LabelWidget::VerticalAlignment::Center,
      this,
      sdl::core::engine::Color::fromRGB(1.0f, 0.75, 0.25f)
    );
    if (generation == nullptr) {
      error(
        std::string("Could not create info bar"),
        std::string("Generation label not allocated")
      );
    }

    sdl::graphic::LabelWidget* aliveCells = new sdl::graphic::LabelWidget(
      getAliveCellsLabelName(),
      "Alive: 0",
      getInfoLabelFont(),
      15u,
      sdl::graphic::LabelWidget::HorizontalAlignment::Center,
      sdl::graphic::LabelWidget::VerticalAlignment::Center,
      this,
      sdl::core::engine::Color::NamedColor::Gray
    );
    if (aliveCells == nullptr) {
      error(
        std::string("Could not create info bar"),
        std::string("Alive cells label not allocated")
      );
    }

    sdl::graphic::Button* gridDisplay = new sdl::graphic::Button(
      getDisplayGridButtonName(),
      std::string("Grid"),
      std::string(),
      getInfoLabelFont(),
      sdl::graphic::button::Type::Toggle,
      15u,
      this,
      2.0f,
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::Gray
    );
    if (gridDisplay == nullptr) {
      error(
        std::string("Could nto create info bar"),
        std::string("Display grid button not allocated")
      );
    }

    // Configure each element.
    mouseCoords->setFocusPolicy(sdl::core::FocusPolicy());
    generation->setFocusPolicy(sdl::core::FocusPolicy());
    aliveCells->setFocusPolicy(sdl::core::FocusPolicy());

    gridDisplay->setMaxSize(utils::Sizef(60.0f, 60.0f));

    // Add each element to the layout.
    layout->addItem(mouseCoords);
    layout->addItem(generation);
    layout->addItem(aliveCells);
    layout->addItem(gridDisplay);
  }

}
