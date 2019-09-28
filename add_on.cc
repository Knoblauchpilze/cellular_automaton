
# include <string>
# include <iostream>
# include <stdexcept>
# include <SDL2/SDL.h>
# include <time.h>
# include <cstdlib>
# include "add_on.hh"

namespace {

  SDL_Texture*
  loadToTex(const std::string& path,
            SDL_Renderer* renderer)
  {
    SDL_Surface* imageAsSurface = SDL_LoadBMP(path.c_str());
    if (imageAsSurface == nullptr) {
      std::cerr << "Unable to create surface from file \"" << path << "\" (err: \"" << SDL_GetError() << "\")" << std::endl;
      return nullptr;
    }

    SDL_SetColorKey(imageAsSurface, SDL_TRUE, SDL_MapRGB(imageAsSurface->format, 255, 255, 255));

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, imageAsSurface);

    SDL_FreeSurface(imageAsSurface);

    return tex;
  }

  SDL_Texture*
  createTex(int width,
            int height,
            SDL_Color color,
            SDL_Renderer* renderer)
  {
    SDL_Texture* tex = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_TEXTUREACCESS_TARGET,
      width,
      height
    );

    if (tex == nullptr) {
      std::cerr << "Could not create texture with dimensions " << width << "x" << height
                << " and color (r: " << color.r << ", g: " << color.g << ", b: " << color.b
                << ", a: " << color.a << ")"
                << std::endl;

      return nullptr;
    }

    SDL_SetRenderTarget(renderer, tex);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_RenderClear(renderer);

    return tex;
  }

}

namespace utils {
  /* FONCTIONS DE BASES : CHARGEMENT DE LA SDL, INITIALISATION, UPDATE EVENTS */

  bool
  Input::operator[](const SDL_Keycode& code) const noexcept {
    return keys.count(code) > 0;
  }

  void
  Input::reset(const SDL_Keycode& code) noexcept {
    keys.erase(code);
  }

  App
  loadSDL(int width,
          int height,
          int hVisibleCells,
          int vVisibleCells)
  {
    // Initialize RNG.
    srand(time(nullptr));

    // Initialize SDL.
    if (!SDL_WasInit(SDL_INIT_VIDEO) && SDL_Init(SDL_INIT_VIDEO) != 0) {
      throw std::logic_error(
        std::string("Could not initialize SDL lib (err: \"") + SDL_GetError() + "\""
      );
    }

    App app = App{
      nullptr,
      nullptr,
      nullptr,
      width,
      height,
      hVisibleCells,
      vVisibleCells
    };

    // Create a SDL window.
    app.win = SDL_CreateWindow(
      "Cellular Automaton: Welcome to the Jungle (Old: Cells' game)",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      width,
      height,
      SDL_WINDOW_SHOWN
    );

    // Initialize the icon of this application.
    SDL_Surface* iconAsSurface = SDL_LoadBMP("data/img/65px-Stop_hand.svg[1].bmp");
    if (iconAsSurface == nullptr) {
      throw std::logic_error(
        std::string("Could not load icon \"data/img/65px-Stop_hand.svg[1].bmp\"")
      );
    }

    // Set the icon to the window.
    SDL_SetWindowIcon(app.win, iconAsSurface);

    // Release resources used to create the icon.
    SDL_FreeSurface(iconAsSurface);

    // Create the renderer associated to this app.
    app.renderer = SDL_CreateRenderer(app.win, -1, SDL_RENDERER_ACCELERATED);
    if (app.renderer == nullptr) {
      throw std::logic_error(
        std::string("Could not create renderer associated to main window (err: \"") + SDL_GetError() + "\""
      );
    }

    // Create the main canvas to use to display content.
    app.canvas = SDL_CreateTexture(
      app.renderer,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_TEXTUREACCESS_TARGET,
      app.width,
      app.height
    );
    if (app.canvas == nullptr) {
      throw std::logic_error(
        std::string("Could not create main canvas for this application (err: \"") + SDL_GetError() + "\""
      );
    }

    // Return the created window.
    return app;
  }

  void
  updateEvents(Input& events) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          events.quit = true;
          break;
        case SDL_KEYDOWN:
          events.keys.insert(event.key.keysym.sym);
          break;
        case SDL_KEYUP:
          events.keys.erase(event.key.keysym.sym);
          break;
        case SDL_MOUSEMOTION:
          if (events.click) {
            events.clickPos.x = event.motion.x;
            events.clickPos.y = event.motion.y;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            events.click = true;
            events.clickPos.x = event.button.x;
            events.clickPos.y = event.button.y;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (event.button.button == SDL_BUTTON_LEFT) {
            events.click = false;
          }
          break;
        default:
          break;
      }
    }
  }

  void
  initializeEvents(Input& events) {
    events.keys.clear();

    events.quit = false;

    events.click = false;
    events.clickPos.x = -1;
    events.clickPos.y = -1;
  }

  bool
  createColony(Colony& colony,
               int width,
               int height)
  {
    colony = Colony{
      nullptr,
      nullptr,
      width,
      height,
      0
    };

    colony.cells = new Cell[width * height];
    colony.old = new Cell[width * height];

    if (colony.cells == nullptr || colony.cells == nullptr) {
      std::cerr << "Could not create colony with dimensions " << width << "x" << height << std::endl;
      return false;
    }

    initializeColony(colony);

    return true;
  }

  void
  initializeColony(Colony& colony) {
    for (int i = 0 ; i < colony.height ; i++) {
      for (int j = 0 ; j < colony.width ; j++) {
        colony.cells[i * colony.width + j] = Cell::Dead;
      }
    }

    colony.generation = 0;
  }

  void
  randomizeColony(Colony& colony) {
    const int probNewborn = 10;
    const int probAlive = 25;
    const int probDying = 5;

    for (int i = 0 ; i < colony.height ; i++) {
      for (int j = 0 ; j < colony.width ; j++) {
        int rnd = std::rand() % 100;

        if (rnd < probNewborn) {
          colony.cells[i * colony.width + j] = Cell::Newborn;
        }
        else if (rnd < probNewborn + probAlive) {
          colony.cells[i * colony.width + j] = Cell::Alive;
        }
        else if (rnd < probNewborn + probAlive + probDying) {
          colony.cells[i * colony.width + j] = Cell::Dying;
        }
        else {
          colony.cells[i * colony.width + j] = Cell::Dead;
        }
      }
    }
  }

  bool
  loadTextures(Picture* textures,
               int sizeX,
               int sizeY,
               SDL_Renderer* renderer)
  {
    textures[0].pic = createTex(sizeX, sizeY, SDL_Color{0, 255, 0, SDL_ALPHA_OPAQUE}, renderer);
    textures[1].pic = createTex(sizeX, sizeY, SDL_Color{0, 0, 255, SDL_ALPHA_OPAQUE}, renderer);
    textures[2].pic = createTex(sizeX, sizeY, SDL_Color{255, 0, 0, SDL_ALPHA_OPAQUE}, renderer);

    if (textures[0].pic != nullptr) {
      SDL_QueryTexture(textures[0].pic, nullptr, nullptr, &textures[0].pos.w, &textures[0].pos.h);
    }
    if (textures[1].pic != nullptr) {
      SDL_QueryTexture(textures[1].pic, nullptr, nullptr, &textures[1].pos.w, &textures[1].pos.h);
    }
    if (textures[2].pic != nullptr) {
      SDL_QueryTexture(textures[2].pic, nullptr, nullptr, &textures[2].pos.w, &textures[2].pos.h);
    }

    return textures[0].pic != nullptr && textures[1].pic != nullptr && textures[2].pic != nullptr;
  }

  bool
  initializeSights(Target& sights,
                   int sizeX,
                   int sizeY,
                   int displayedCellsHorizontally,
                   int displayedCellsVertically,
                   SDL_Renderer* renderer)
  {
    sights.display.pic = createTex(sizeX, sizeY, SDL_Color{255, 255, 255, SDL_ALPHA_OPAQUE}, renderer);

    sights.x = displayedCellsHorizontally / 2;
    sights.y = displayedCellsVertically / 2;

    SDL_QueryTexture(sights.display.pic, nullptr, nullptr, &sights.display.pos.w, &sights.display.pos.h);

    return sights.display.pic != nullptr;
  }

  void
  printColony(Colony& colony,
              const std::string& fileName)
  {
    FILE* out = fopen(fileName.c_str(),"w+");
    fprintf(out, "Cellular automaton, gen %d:\n", colony.generation);

    if (out == nullptr) {
      std::cerr << "Unable to print colony to file \"" << fileName << "\"" << std::endl;

      return;
    }

    for (int i = 0 ; i < colony.height ; i++) {
      for (int j = 0 ; j < colony.width ; j++) {
        if (colony.cells[i * colony.width + j] == Cell::Newborn) {
          fputc('1', out);
        }
        else if (colony.cells[i * colony.width + j] == Cell::Alive) {
          fputc('2',out);
        }
        else if (colony.cells[i * colony.width + j] == Cell::Dying) {
          fputc('3',out);
        }
        else {
          fputc('0',out);
        }
      }

      fputc('\n',out);
    }

    fclose(out);
  }

  void
  loadColony(Colony& colony,
             const std::string& fileName)
  {
    FILE* in = nullptr;
    in = fopen(fileName.c_str(), "r");

    if (in == nullptr) {
      initializeColony(colony);

      std::cerr << "Unable to load colony from file \"" << fileName << "\"" << std::endl;

      return;
    }

    int readCells = 0, i = 0, j = 0, total = colony.width * colony.height;
    char c;

    do {
      char c = fgetc(in);

      if (c != '\n' && c != EOF) {
        if (c == '1') {
          colony.cells[i * colony.width + j]= Cell::Newborn;
        }
        else if(c=='2') {
          colony.cells[i * colony.width + j] = Cell::Alive;
        }
        else if(c=='3') {
          colony.cells[i * colony.width + j] = Cell::Dying;
        }
        else {
          colony.cells[i * colony.width + j] = Cell::Dead;
        }
        
        ++readCells;
      }

      j++;
      if (j == colony.width) {
        j = 0;
        ++i;
      }
    } while(readCells < total && i < colony.height && c != EOF);

    fclose(in);
  }

  /* FONCTIONS D'AFFICHAGE DES CELLULES */
  void
  blitColony(Colony& colony,
             App& app,
             Picture* textures)
  {
    SDL_SetRenderTarget(app.renderer, app.canvas);
    SDL_SetRenderDrawColor(app.renderer, 10, 10, 10, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app.renderer);

    for (int i = 0 ; i < colony.height ; i++) {
      for(int j = 0 ; j < colony.width ; j++) {
        if (colony.cells[i * colony.width + j] == Cell::Newborn) {
          textures[0].pos.x = j * app.width / app.hVisibleCellsCount;
          textures[0].pos.y = i * app.height / app.vVisibleCellsCount;

          SDL_RenderCopy(app.renderer, textures[0].pic, nullptr, &textures[0].pos);
        }
        else if (colony.cells[i * colony.width + j] == Cell::Alive) {
          textures[1].pos.x =j * app.width / app.hVisibleCellsCount;
          textures[1].pos.y =i * app.height / app.vVisibleCellsCount;

          SDL_RenderCopy(app.renderer, textures[1].pic, nullptr, &textures[1].pos);
        }
        else if(colony.cells[i * colony.width + j] == Cell::Dying) {
          textures[2].pos.x = j * app.width / app.hVisibleCellsCount;
          textures[2].pos.y = i * app.height / app.vVisibleCellsCount;

          SDL_RenderCopy(app.renderer, textures[2].pic, nullptr, &textures[2].pos);
        }
      }
    }
  }

  /* FONCTIONS DE VIE DE LA COLONIE */
  void
  updateColony(Colony& colony) {
    // Copy old version of the colony in order not to alias the update process.
    for (int i = 0 ; i < colony.height ; i++) {
      for (int j = 0 ; j < colony.width ; j++) {
        colony.old[i * colony.width + j] = colony.cells[i * colony.width + j];
      }
    }

    int imin, imax, jmin, jmax;

    for (int i = 0 ; i < colony.height ; i++) {
      for (int j = 0 ; j < colony.width ; j++) {
        // Determine bounds based on the current index of the cell.
        if (i == 0) {
          imin = 0;
          imax = 1;
        }
        else if (i < colony.height - 1) {
          imin = i - 1;
          imax = i + 1;
        }
        else {
          imin = i - 1;
          imax = colony.height - 1;
        }

        if (j == 0) {
          jmin = 0;
          jmax = 1;
        }
        else if (j < colony.width - 1) {
          jmin = j - 1;
          jmax = j + 1;
        }
        else {
          jmin = j - 1;
          jmax = colony.width - 1;
        }

        // Count the number of cells still alive in the neighbourhood.
        int aliveCount = countCellsAliveAround(colony.old, colony.width, colony.height, imin, imax, jmin, jmax, j, i);

        // Update the state of the current cell.
        if (colony.old[i * colony.width + j] == Cell::Newborn) {
          if (aliveCount > 1 && aliveCount < 4) {
            colony.cells[i * colony.width + j] = Cell::Alive;
          }
          else {
            colony.cells[i * colony.width + j] = Cell::Dying;
          }
        }
        else if (colony.old[i * colony.width + j] == Cell::Alive) {
          if (aliveCount < 2) {
            colony.cells[i * colony.width + j] = Cell::Dying;
          }
          else if (aliveCount > 3) {
            colony.cells[i * colony.width + j] = Cell::Dying;
          }
        }
        else if (colony.old[i * colony.width + j] == Cell::Dying) {
          if (aliveCount == 3) {
            colony.cells[i * colony.width + j] = Cell::Newborn;
          }
          else {
            colony.cells[i * colony.width + j] = Cell::Dead;
          }
        }
        else {
          if (aliveCount == 3) {
            colony.cells[i * colony.width + j] = Cell::Newborn;
          }
        }
      }
    }

    // One more generation has been computed.
    ++colony.generation;
  }

  int
  countCellsAliveAround(Cell* cells,
                        int width,
                        int height,
                        int imin,
                        int imax,
                        int jmin,
                        int jmax,
                        int x,
                        int y)
  {
    int aliveCount = 0;

    for (int i = imin ; i <= imax ; i++) {
      for (int j = jmin ; j <= jmax ; j++) {
        if (i != y || j != x) {
          if (cells[i * width + j] != Cell::Dead &&
              cells[i * width + j] != Cell::Dying)
          {
            ++aliveCount;
          }
        }
      }
    }

    return aliveCount;
  }

}
