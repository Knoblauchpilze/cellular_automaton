# ifndef   ADD_ON_H_INCLUDED
#   define ADD_ON_H_INCLUDED

# include <SDL2/SDL.h>
# include <unordered_set>

namespace utils {

  struct Input {
    std::unordered_set<SDL_Keycode> keys;
    bool quit;

    bool click;
    SDL_Rect clickPos;

    bool
    operator[](const SDL_Keycode& code) const noexcept;

    void
    reset(const SDL_Keycode& code) noexcept;
  };

  typedef struct Input Input;

  enum class Cell {
    Newborn,
    Alive,
    Dying,
    Dead
  };

  struct Colony {
    Cell* cells;
    int width;
    int height;
    int generation;
  };

  struct Picture {
    SDL_Texture* pic;
    SDL_Rect pos;
  };

  struct Target {
    Picture display;
    int x;
    int y;
  };

  struct App {
    SDL_Window* win;
    SDL_Renderer* renderer;
    SDL_Texture* canvas;
    int width;
    int height;
    int hVisibleCellsCount;
    int vVisibleCellsCount;
  };

  /* FONCTIONS DE BASES : CHARGEMENT DE LA SDL, INITIALISATION, UPDATE EVENTS */
  App
  loadSDL(int width,
          int height,
          int hVisibleCells,
          int vVisibleCells);

  void
  updateEvents(Input& events);

  void
  initializeEvents(Input& events);

  void
  initializeColony(Colony& colony);

  bool
  loadTextures(Picture* textures,
               SDL_Renderer* renderer);

  bool
  initializeSights(Target& sights,
                   int displayedCellsHorizontally,
                   int displayedCellsVertically,
                   SDL_Renderer* renderer);

  void
  printColony(Colony& colony,
              const std::string& fileName);

  void
  loadColony(Colony& colony,
             const std::string& fileName);

  /* FONCTIONS D'AFFICHAGE DES CELLULES */
  void
  blitColony(Colony& colony,
             App& app,
             Picture* textures);

  /* FONCTIONS DE VIE DE LA COLONIE */
  void
  updateColony(Colony& colony);

  int
  countCellsAliveAround(Cell* cells,
                        int width,
                        int height,
                        int imin,
                        int imax,
                        int jmin,
                        int jmax,
                        int x,
                        int y);

}

#endif /* ADD_ON_H_INCLUDED */
