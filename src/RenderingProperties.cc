
# include "RenderingProperties.hh"
# include <sdl_graphic/GridLayout.hh>
# include <sdl_graphic/LabelWidget.hh>
# include <sdl_graphic/TextBox.hh>
# include <sdl_graphic/PictureWidget.hh>
# include <sdl_graphic/Button.hh>

namespace cellulator {

  RenderingProperties::RenderingProperties(const utils::Sizef& hint,
                                           sdl::core::SdlWidget* parent):
    sdl::core::SdlWidget(std::string("rendering_props"),
                         hint,
                         parent,
                         getDefaultColor()),

    m_propsLocker(),

    onPaletteChanged()
  {
    build();
  }

  void
  RenderingProperties::build() {
    // Create the palette.
    generatePalette();

    // Create the grid layout to register each individual color picker.
    // The layout is composed of a single line allowing to register the
    // max age represented by the palette, then a number of steps that
    // is defined by `getPaletteSteps`: as we want some nice spacing in
    // between each steps we double the number of rows. We also want a
    // space after the last option.
    // And then the `Apply` button allowing to notify the palette opts.
    sdl::graphic::GridLayoutShPtr layout = std::make_shared<sdl::graphic::GridLayout>(
      "rendering_layout",
      this,
      2u,
      1u + (2u * getPaletteSteps() + 1u) + 1u,
      getGlobalMargins()
    );

    layout->setRowsMinimumHeight(getComponentMargins());

    layout->setAllowLog(false);

    // Assign the layout to this widget.
    setLayout(layout);

    // Create the max age textbox.
    sdl::graphic::LabelWidget* desc = new sdl::graphic::LabelWidget(
      std::string("desc_label"),
      std::string("Max age:"),
      getGeneralTextFont(),
      15u,
      sdl::graphic::LabelWidget::HorizontalAlignment::Right,
      sdl::graphic::LabelWidget::VerticalAlignment::Center,
      this,
      getDefaultColor()
    );
    if (desc == nullptr) {
      error(
        std::string("Could not create rendering options panel"),
        std::string("Could not create description label")
      );
    }

    sdl::graphic::TextBox* box = new sdl::graphic::TextBox(
      getMaxAgeTextboxName(),
      getGeneralTextFont(),
      std::string("10"),
      15u,
      this
    );
    if (box == nullptr) {
      error(
        std::string("Could not create rendering options panel"),
        std::string("Could not create max age textbox")
      );
    }

    utils::Sizef max(std::numeric_limits<float>::max(), getMaxAgeSelectionHeight());

    desc->setMaxSize(max);
    box->setMaxSize(max);

    layout->addItem(desc, 0u, 0u, 1u, 1u);
    layout->addItem(box , 1u, 0u, 1u, 1u);

    // Register each palette.
    for (unsigned id = 0u ; id < getPaletteSteps() ; ++id) {
      sdl::graphic::LabelWidget* label = new sdl::graphic::LabelWidget(
        std::string("label_for_") + std::to_string(id),
        std::string("Step ") + std::to_string(id + 1u),
        getGeneralTextFont(),
        15u,
        sdl::graphic::LabelWidget::HorizontalAlignment::Right,
        sdl::graphic::LabelWidget::VerticalAlignment::Center,
        this,
        getDefaultColor()
      );
      if (label == nullptr) {
        error(
          std::string("Could not create rendering options panel"),
          std::string("Could not create palette for age ") + std::to_string(id)
        );
      }

      sdl::graphic::SelectorWidget* palette = createPaletteFromIndex(id);

      utils::Sizef m = getPaletteMaxSize();
      m.w() = std::numeric_limits<float>::max();
      label->setMaxSize(m);
      label->setFocusPolicy(sdl::core::FocusPolicy());

      layout->addItem(label,   0u, 2u + 2u * id, 1u, 1u);
      layout->addItem(palette, 1u, 2u + 2u * id, 1u, 1u);
    }

    // Create the `Apply` button.
    sdl::graphic::Button* apply = new sdl::graphic::Button(
      std::string("apply_button"),
      std::string("Apply"),
      std::string(),
      getGeneralTextFont(),
      sdl::graphic::button::Type::Regular,
      15u,
      this,
      5.0f,
      utils::Sizef(),
      sdl::core::engine::Color::NamedColor::Teal
    );
    if (apply == nullptr) {
      error(
        std::string("Could not create rendering options panel"),
        std::string("Could not create apply button")
      );
    }

    apply->setMaxSize(
      utils::Sizef(
        std::numeric_limits<float>::max(),
        getApplyButtonHeight()
      )
    );

    // Connect its `clicked` signal to the local slot.
    apply->onClick.connect_member<RenderingProperties>(
      this,
      &RenderingProperties::onApplyButtonClicked
    );

    // Add it to the layout.
    layout->addItem(apply, 0u, 1u + (2u * getPaletteSteps() + 1u), 2u, 1u);
  }

  void
  RenderingProperties::generatePalette() noexcept {
    // We want to generate all the named color in the palette. In order
    // to provide some sort of consistency we will also set the first
    // ones to be equal to the default gradient usedin the `ColorPalette`
    // item.
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Indigo);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Purple);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Blue);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Green);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Yellow);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Orange);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Red);

    // Register the rest of the colors.
    m_colors.push_back(sdl::core::engine::Color::NamedColor::White);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Black);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Cyan);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Magenta);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Silver);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Gray);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Maroon);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Olive);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Pink);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Teal);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::Navy);
    m_colors.push_back(sdl::core::engine::Color::NamedColor::CorneFlowerBlue);
  }

  void
  RenderingProperties::onApplyButtonClicked(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    const std::lock_guard guard(m_propsLocker);

    // Retrieve the maximum age of a cell.
    sdl::graphic::TextBox* tb = getMaxAgeTextbox();

    std::string v;
    withSafetyNet(
      [tb, &v]() {
        v = tb->getValue();
      },
      std::string("maxAge::getValue")
    );

    bool success = false;
    unsigned maxAge = utils::convert(v, getDefaultMaxAge(), success);

    if (!success) {
      warn("Could not convert text \"" + v + "\" to valid max age, using " + std::to_string(maxAge) + " instead");
    }

    // Create a default color palette.
    ColorPaletteShPtr palette = std::make_shared<ColorPalette>(maxAge);

    // Build the gradient.
    sdl::core::engine::GradientShPtr gradient = std::make_shared<sdl::core::engine::Gradient>(
      std::string("palette_for_age"),
      sdl::core::engine::gradient::Mode::Linear
    );

    for (unsigned id = 0u ; id < getPaletteSteps() ; ++id) {
      // Retrieve the palette.
      std::string n = generateNameForPalette(id);
      sdl::graphic::SelectorWidget* sw = getPaletteFromName(n);

      // Populate the color based on the current active on in the palette.
      unsigned cID = static_cast<unsigned>(sw->getActiveItem());

      if (cID >= m_colors.size()) {
        warn(
          "Could not retrieve invalid color " + std::to_string(cID) + ", only " +
          std::to_string(m_colors.size()) + " available"
        );

        continue;
      }

      gradient->setColorAt(1.0f * id / getPaletteSteps(), m_colors[cID]);
    }

    if (gradient != nullptr) {
      palette->setGradient(gradient);
    }
    else {
      warn("Could not build color gradient to use for palette, using default one");
    }

    // Notify listeners through the signal.
    onPaletteChanged.safeEmit(
      std::string("onPaletteChanged(") + std::to_string(maxAge) + ")",
      palette
    );
  }

}
