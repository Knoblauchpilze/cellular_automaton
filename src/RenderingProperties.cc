
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
    sdl::graphic::GridLayoutShPtr layout = std::make_shared<sdl::graphic::GridLayout>(
      "rendering_layout",
      this,
      2u,
      15u,
      getGlobalMargins()
    );

    layout->setRowsMinimumHeight(getComponentMargins());

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
    for (unsigned id = 0u ; id < 6u ; ++id) {
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

      sdl::core::SdlWidget* palette = new sdl::core::SdlWidget(
        generateNameForPalette(id),
        utils::Sizef(),
        this,
        sdl::core::engine::Color::NamedColor::White
      );
      if (palette == nullptr) {
        error(
          std::string("Could not create rendering options panel"),
          std::string("Could not create palette for age ") + std::to_string(id)
        );
      }

      utils::Sizef m = getPaletteMaxSize();
      m.w() = std::numeric_limits<float>::max();
      label->setMaxSize(m);
      label->setFocusPolicy(sdl::core::FocusPolicy());

      palette->setMaxSize(getPaletteMaxSize());
      palette->setFocusPolicy(sdl::core::FocusPolicy());

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
    layout->addItem(apply, 0u, 14u, 2u, 1u);
  }

  void
  RenderingProperties::onApplyButtonClicked(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

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
    unsigned maxAge = convertToUnsigned(v, getDefaultMaxAge(), success);

    if (!success) {
      log(
        std::string("Could not convert text \"") + v + "\" to valid max age, using " + std::to_string(maxAge) + " instead",
        utils::Level::Warning
      );
    }

    // Create a default color palette.
    ColorPaletteShPtr palette = std::make_shared<ColorPalette>(maxAge);

    // Build the gradient.
    sdl::core::engine::GradientShPtr gradient = nullptr;

    if (gradient != nullptr) {
      palette->setGradient(gradient);
    }
    else {
      log(
        std::string("Could not build color gradient to use for palette, using default one"),
        utils::Level::Warning
      );
    }

    // Notify listeners through the signal.
    onPaletteChanged.safeEmit(
      std::string("onPaletteChanged(") + std::to_string(maxAge) + ")",
      palette
    );
  }

}
