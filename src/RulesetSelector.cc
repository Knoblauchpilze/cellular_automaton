
# include "RulesetSelector.hh"
# include <sdl_graphic/GridLayout.hh>
# include <sdl_graphic/LabelWidget.hh>
# include <sdl_graphic/PictureWidget.hh>
# include <sdl_graphic/Button.hh>

namespace cellulator {

  RulesetSelector::RulesetSelector(const utils::Sizef& hint,
                                   sdl::core::SdlWidget* parent):
    sdl::core::SdlWidget(std::string("ruleset_selector"),
                         hint,
                         parent,
                         sdl::core::engine::Color::NamedColor::CorneFlowerBlue),

    m_propsLocker(),

    onRulesetChanged()
  {
    build();
  }

  void
  RulesetSelector::build() {
    // Register each checkbox for live neighbors possibility (`0` through `8`).
    sdl::graphic::GridLayoutShPtr layout = std::make_shared<sdl::graphic::GridLayout>(
      "ruleset_layout",
      this,
      2u,
      12u,
      getGlobalMargins()
    );

    // Assign the layout to this widget.
    setLayout(layout);

    // Create labels.
    sdl::graphic::LabelWidget* descLabel = new sdl::graphic::LabelWidget(
      std::string("desc_label"),
      "Select cells to be...",
      getGeneralTextFont(),
      15u,
      sdl::graphic::LabelWidget::HorizontalAlignment::Center,
      sdl::graphic::LabelWidget::VerticalAlignment::Center,
      this,
      sdl::core::engine::Color::NamedColor::CorneFlowerBlue
    );
    if (descLabel == nullptr) {
      error(
        std::string("Could not create ruleset selector"),
        std::string("Main description label not created")
      );
    }

    sdl::graphic::PictureWidget* live = new sdl::graphic::PictureWidget(
      std::string("live_pic"),
      std::string("data/img/alive.bmp"),
      sdl::graphic::PictureWidget::Mode::Fit,
      this,
      sdl::core::engine::Color::NamedColor::CorneFlowerBlue
    );
    if (live == nullptr) {
      error(
        std::string("Could not create ruleset selector"),
        std::string("Live cells selection description pic not created")
      );
    }

    sdl::graphic::PictureWidget* dead = new sdl::graphic::PictureWidget(
      std::string("dead_pic"),
      std::string("data/img/dead.bmp"),
      sdl::graphic::PictureWidget::Mode::Fit,
      this,
      sdl::core::engine::Color::NamedColor::CorneFlowerBlue
    );
    if (dead == nullptr) {
      error(
        std::string("Could not create ruleset selector"),
        std::string("Dead cells selection description pic not created")
      );
    }

    descLabel->setFocusPolicy(sdl::core::FocusPolicy());
    live->setFocusPolicy(sdl::core::FocusPolicy());
    dead->setFocusPolicy(sdl::core::FocusPolicy());

    live->setMaxSize(getIconMaxSize());
    dead->setMaxSize(getIconMaxSize());

    layout->addItem(descLabel, 0u, 0u, 2u, 1u);
    layout->addItem(live,      0u, 1u, 1u, 1u);
    layout->addItem(dead,      1u, 1u, 1u, 1u);

    // Create checkboxes.
    for (unsigned val = 0u ; val < 9u ; ++val) {
      // Live ones.
      sdl::graphic::Checkbox* lcb = new sdl::graphic::Checkbox(
        generateNameForLiveNeighbors(val),
        std::to_string(val),
        getGeneralTextFont(),
        false,
        15u,
        this,
        utils::Sizef(),
        sdl::core::engine::Color::NamedColor::CorneFlowerBlue
      );
      if (lcb == nullptr) {
        error(
          std::string("Could not create ruleset selector"),
          std::string("Could not create checkbox for ") + std::to_string(val) + " live neighbor(s)"
        );
      }

      lcb->setMaxSize(
        utils::Sizef(
          std::numeric_limits<float>::max(),
          getCheckboxMaxHeight()
        )
      );

      // Handle default rules to be the game of life to match the value
      // we set in the `Colony`.
      if (val == 3u) {
        lcb->toggle(true);
      }

      layout->addItem(lcb, 0u, 2u + val, 1u, 1u);

      // Dead ones.
      sdl::graphic::Checkbox* dcb = new sdl::graphic::Checkbox(
        generateNameForDeadNeighbors(val),
        std::to_string(val),
        getGeneralTextFont(),
        false,
        15u,
        this,
        utils::Sizef(),
        sdl::core::engine::Color::NamedColor::CorneFlowerBlue
      );
      if (dcb == nullptr) {
        error(
          std::string("Could not create ruleset selector"),
          std::string("Could not create checkbox for ") + std::to_string(val) + " dead neighbor(s)"
        );
      }

      dcb->setMaxSize(
        utils::Sizef(
          std::numeric_limits<float>::max(),
          getCheckboxMaxHeight()
        )
      );

      // Handle default rules to be the game of life to match the value
      // we set in the `Colony`.
      if (val != 2u && val != 3u) {
        dcb->toggle(true);
      }

      layout->addItem(dcb, 1u, 2u + val, 1u, 1u);
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
        std::string("Could not create ruleset selector"),
        std::string("Could not create apply button")
      );
    }

    // Connect its `clicked` signal to the local slot.
    apply->onClick.connect_member<RulesetSelector>(
      this,
      &RulesetSelector::onApplyButtonClicked
    );

    // Add it to the layout.
    layout->addItem(apply, 0u, 11u, 2u, 1u);
  }

  void
  RulesetSelector::onApplyButtonClicked(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Create a new ruleset.
    CellEvolverShPtr evolver = std::make_shared<CellEvolver>();
    evolver->clear();

    // Traverse checkboxes and add each option.
    std::string name;
    sdl::graphic::Checkbox* cb = nullptr;

    for (unsigned count = 0u ; count < 8u ; ++count) {
      name = generateNameForLiveNeighbors(count);
      cb = getCheckboxFromName(name);

      if (cb == nullptr) {
        log(
          std::string("Could not fetch information for checkbox of ") + std::to_string(count) + " live neighbor(s)",
          utils::Level::Error
        );

        continue;
      }

      if (cb->toggled()) {
        evolver->addBornOption(count);
      }
    }

    for (unsigned count = 0u ; count < 8u ; ++count) {
      name = generateNameForDeadNeighbors(count);
      cb = getCheckboxFromName(name);

      if (cb == nullptr) {
        log(
          std::string("Could not fetch information for checkbox of ") + std::to_string(count) + " dead neighbor(s)",
          utils::Level::Error
        );

        continue;
      }

      if (!cb->toggled()) {
        evolver->addSurvivingOption(count);
      }
    }

    // Emit the signal.
    onRulesetChanged.safeEmit(
      std::string("onRulesetChanged(options)"),
      evolver
    );
  }

}
