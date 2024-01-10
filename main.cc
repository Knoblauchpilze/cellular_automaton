
/**
 * @brief - Reimplementation of a program started in 05/2011 as a
 *          training and a tool to visualize the evolution of a
 *          cells colony following the rules of Conway's game of
 *          life.
 *          Implemented from:
 *            - 28/09/2019 - 03/10/2019
 *            - 17/12/2019 - 16/01/2020
 */

# include <core_utils/log/Locator.hh>
# include <core_utils/log/PrefixedLogger.hh>
# include <core_utils/log/StdLogger.hh>
# include <sdl_app_core/SdlApplication.hh>
# include <core_utils/CoreException.hh>
# include "InfoBar.hh"
# include "Colony.hh"
# include "ColonyStatus.hh"
# include "ColonyRenderer.hh"
# include "RulesetSelector.hh"
# include "RenderingProperties.hh"
# include "BrushSelector.hh"

namespace {
constexpr auto APP_NAME = "cellulator";
constexpr auto APP_TITLE = "Cellular Automaton: Welcome to the Jungle (Old: Cells' game)";
constexpr auto APP_ICON_PATH = "data/img/icon.bmp";
}

int main(int /*argc*/, char** /*argv*/) {
  // Create the logger.
  utils::log::StdLogger raw;
  raw.setLevel(utils::log::Severity::DEBUG);
  utils::log::PrefixedLogger logger("automaton", "main");
  utils::log::Locator::provide(&raw);

  try {
    auto app = std::make_shared<sdl::app::SdlApplication>(
      APP_NAME,
      APP_TITLE,
      APP_ICON_PATH,
      utils::Sizei(800, 600),
      true,
      utils::Sizef(0.4f, 0.5f),
      50.0f,
      60.0f
    );

    // Create the colony to simulate.
    cellulator::ColonyShPtr colony = std::make_shared<cellulator::Colony>(
      utils::Sizei(8, 8),
      std::string("Drop it like it's Hoth")
    );

    // Create the layout of the window: the main tab is a scrollable widget
    // allowing the display of the colony. The right dock widget allows to
    // control the computation parameters and the status bar displays some
    // general information about the colony.
    cellulator::ColonyRenderer* renderer = new cellulator::ColonyRenderer(colony);
    app->setCentralWidget(renderer);

    cellulator::ColonyStatus* status = new cellulator::ColonyStatus();
    app->addDockWidget(status, sdl::app::DockWidgetArea::TopArea);

    cellulator::InfoBar* bar = new cellulator::InfoBar();
    app->setStatusBar(bar);

    cellulator::RulesetSelector* rules = new cellulator::RulesetSelector();
    app->addDockWidget(rules, sdl::app::DockWidgetArea::RightArea, std::string("Ruleset"));

    cellulator::RenderingProperties* props = new cellulator::RenderingProperties();
    app->addDockWidget(props, sdl::app::DockWidgetArea::RightArea, std::string("Display"));

    cellulator::BrushSelector* brushes = new cellulator::BrushSelector();
    app->addDockWidget(brushes, sdl::app::DockWidgetArea::RightArea, std::string("Brushes"));

    // Connect the simulation's control button to the options panel slots.
    status->getFitToContentButton().onClick.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::fitToContent
    );
    status->onSimulationStarted.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::start
    );
    status->onSimulationStepped.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::nextStep
    );
    status->onSimulationStopped.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::stop
    );
    status->getGenerateColonyButton().onClick.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::generate
    );

    renderer->getScheduler()->onSimulationToggled.connect_member<cellulator::ColonyStatus>(
      status,
      &cellulator::ColonyStatus::onSimulationToggled
    );

    rules->onRulesetChanged.connect_member<cellulator::ColonyScheduler>(
      renderer->getScheduler().get(),
      &cellulator::ColonyScheduler::onRulesetChanged
    );

    props->onPaletteChanged.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::onPaletteChanded
    );

    brushes->onBrushChanged.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::onBrushChanged
    );

    bar->onGridDisplayChanged.connect_member<cellulator::ColonyRenderer>(
      renderer,
      &cellulator::ColonyRenderer::onGridDisplayToggled
    );

    // Connect changes in the colony to the status display.
    renderer->onCoordChanged.connect_member<cellulator::InfoBar>(
      bar,
      &cellulator::InfoBar::onSelectedCellChanged
    );
    renderer->onGenerationComputed.connect_member<cellulator::InfoBar>(
      bar,
      &cellulator::InfoBar::onGenerationComputed
    );
    renderer->onAliveCellsChanged.connect_member<cellulator::InfoBar>(
      bar,
      &cellulator::InfoBar::onAliveCellsChanged
    );

    // Run it.
    app->run();

    app.reset();
  }
  catch (const utils::CoreException& e) {
    logger.error("Caught internal exception while setting up application", e.what());
    return EXIT_FAILURE;
  }
  catch (const std::exception& e) {
    logger.error("Caught internal exception while setting up application", e.what());
    return EXIT_FAILURE;
  }
  catch (...) {
    logger.error("Unexpected error while setting up application");
    return EXIT_FAILURE;
  }

  // All is good.
  return EXIT_SUCCESS;
}
