/*
    15/05/2011 - Restored on 28/09/2019
    PROJET AUTOMATE CELLULAIRE
*/

#include <iostream>
#include <SDL2/SDL.h>
#include <time.h>
#include "add_on.hh"

int main (int argc, char** argv) {
  // Define constants.
  const int appWidth = 1920;
  const int appHeight = 1080;

  const int cellSize = 2;

  const int hVisibleCells = appWidth / cellSize;
  const int vVisibleCells = appHeight / cellSize;

  const std::uint32_t fps = 20u;
  const std::uint32_t expectedFrameDuration = 1000u / fps;

  const std::string colonyFile = "data/colony.txt";

  // Create the SDL window.
  utils::App app = utils::loadSDL(appWidth, appHeight, hVisibleCells, vVisibleCells);

  // Initialize events handling system.
  utils::Input events;
  utils::initializeEvents(events);

  // Initialize textures.
  utils::Picture textures[3];
  utils::Target sights;

  bool validT = utils::loadTextures(textures, cellSize, cellSize, app.renderer);
  bool validS = utils::initializeSights(sights, cellSize, cellSize, app.hVisibleCellsCount, app.vVisibleCellsCount, app.renderer);

  if (!validT || !validS) {
    std::cerr << "[main] Could not allocate one or more textures, aborting" << std::endl;
    return EXIT_FAILURE;
  }

  // Load a colony.
  utils::Colony colony;
  bool valid = utils::createColony(colony, hVisibleCells, vVisibleCells);
  if (!valid) {
    std::cerr << "[main] Could not create colony with dimensions " << hVisibleCells << "x" << vVisibleCells << std::endl;
    return EXIT_FAILURE;
  }

  utils::initializeColony(colony);

  // Proceed with main loop.
  while (events.quit == false && events[SDLK_ESCAPE] == false) {
    std::uint32_t startFrame = SDL_GetTicks();

    // Poll events.
    utils::updateEvents(events);

    // Update game logic.
    if (events[SDLK_l]) {
      events.reset(SDLK_l);
      std::cout << "[main] Loading colony from \"" << colonyFile << "\"" << std::endl;
      utils::loadColony(colony, colonyFile);
    }
    if (events[SDLK_p]) {
      events.reset(SDLK_p);
      std::cout << "[main] Saving colony to \"" << colonyFile << "\"" << std::endl;
      utils::printColony(colony, colonyFile);
    }
    if (events[SDLK_BACKSPACE]) {
      events.reset(SDLK_BACKSPACE);
      std::cout << "[main] Reset colony to empty state" << std::endl;
      utils::initializeColony(colony);
    }
    if (events[SDLK_n]) {
      // Move on to next generation.
      if (events[SDLK_LCTRL]) {
        events.reset(SDLK_n);
      }

      std::cout << "[main] Evolving colony to generation " << colony.generation + 1 << std::endl;
      utils::updateColony(colony);
    }
    if (events[SDLK_r]) {
      events.reset(SDLK_r);

      std::cout << "[main] Generating new random colony" << std::endl;
      randomizeColony(colony);
    }
    if (events[SDLK_RIGHT]) {
      events.reset(SDLK_RIGHT);

      if (sights.x < app.hVisibleCellsCount - 1) {
        ++sights.x;
      }
      std::cout << "[main] Target is now cell " << sights.x << "x" << sights.y << std::endl;
    }
    if (events[SDLK_UP]) {
      events.reset(SDLK_UP);

      if (sights.y > 0) {
        --sights.y;
      }
      std::cout << "[main] Target is now cell " << sights.x << "x" << sights.y << std::endl;
    }
    if (events[SDLK_LEFT]) {
      events.reset(SDLK_LEFT);

      if (sights.x > 0) {
        --sights.x;
      }
      std::cout << "[main] Target is now cell " << sights.x << "x" << sights.y << std::endl;
    }
    if (events[SDLK_DOWN]) {
      events.reset(SDLK_DOWN);

      if (sights.y < app.vVisibleCellsCount - 1) {
        ++sights.y;
      }
      std::cout << "[main] Target is now cell " << sights.x << "x" << sights.y << std::endl;
    }
    if (events[SDLK_KP_1]) {
      events.reset(SDLK_KP_1);
      std::cout << "[main] Adding newborn cell at " << sights.x << "x" << sights.y << std::endl;
      colony.cells[sights.y * colony.width + sights.x] = utils::Cell::Newborn;
    }
    if (events[SDLK_KP_2]) {
      events.reset(SDLK_KP_2);
      std::cout << "[main] Adding alive cell at " << sights.x << "x" << sights.y << std::endl;
      colony.cells[sights.y * colony.width + sights.x] = utils::Cell::Alive;
    }
    if (events.click) {

      // Retrieve the coordinate of the cell targeted by the mouse.
      const int cellX = events.clickPos.x / cellSize;
      const int cellY = events.clickPos.y / cellSize;

      // Mouse the sights over there.
      sights.x = cellX;
      sights.y = cellY;

      // Add an alive cell.
      std::cout << "[main] Adding alive cell at " << sights.x << "x" << sights.y << std::endl;
      colony.cells[sights.y * colony.width + sights.x] = utils::Cell::Alive;
    }
    if (events[SDLK_KP_3]) {
      events.reset(SDLK_KP_3);
      std::cout << "[main] Adding dying cell at " << sights.x << "x" << sights.y << std::endl;
      colony.cells[sights.y * colony.width + sights.x] = utils::Cell::Dying;
    }
    if (events[SDLK_DELETE]) {
      events.reset(SDLK_DELETE);
      std::cout << "[main] Reset cell at " << sights.x << "x" << sights.y << std::endl;
      colony.cells[sights.y * colony.width + sights.x] = utils::Cell::Dead;
    }

    sights.display.pos.x = sights.x * app.width / app.hVisibleCellsCount;
    sights.display.pos.y = sights.y * app.height / app.vVisibleCellsCount;

    utils::blitColony(colony, app, textures);
    SDL_SetRenderTarget(app.renderer, app.canvas);
    SDL_RenderCopy(app.renderer, sights.display.pic, nullptr, &sights.display.pos);

    SDL_SetRenderTarget(app.renderer, nullptr);
    SDL_RenderCopy(app.renderer, app.canvas, nullptr, nullptr);

    SDL_RenderPresent(app.renderer);

    std::uint32_t endFrame = SDL_GetTicks();
    std::uint32_t duration = endFrame - startFrame;
    if (duration >= expectedFrameDuration - 3u) {
      if (duration > expectedFrameDuration) {
        std::cerr << "[main] Frame lasted " << duration
                  << "ms which is greater than the expected duration of "
                  << expectedFrameDuration
                  << std::endl;
      }
    }
    else {
      // std::cout << "Frame lasted " << duration << "ms, sleeping for " << expectedFrameDuration - duration << std::endl;
      SDL_Delay(expectedFrameDuration - duration);
    }
  }

  delete[] colony.cells;
  delete[] colony.old;

  SDL_DestroyTexture(sights.display.pic);

  SDL_DestroyTexture(textures[0].pic);
  SDL_DestroyTexture(textures[1].pic);
  SDL_DestroyTexture(textures[2].pic);

  SDL_Quit();

  return EXIT_SUCCESS;
}
