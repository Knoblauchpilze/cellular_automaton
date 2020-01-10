#ifndef    COLOR_PALETTE_HH
# define   COLOR_PALETTE_HH

# include <memory>
# include <vector>
# include <core_utils/CoreObject.hh>
# include <sdl_engine/Gradient.hh>

namespace cellulator {

  class ColorPalette: public utils::CoreObject {
    public:

      /**
       * @brief - Create an object allowing to query rendering properties for
       *          colony cells. This basically define the way each cell should
       *          be rendered to the user.
       *          Coloring for now include some sort of gradient indicating
       *          the age of the cell.
       *          The default parameters allow to define a colony which gradient
       *          ranges on `[1; 10]` for the age of the cell and with a palette
       *          commonly used to represent temperature gradient.
       * @param maxAge - the minimum age of a cell for it to be assigned the last
       *                 color of the palette.
       */
      ColorPalette(unsigned maxAge = 10u);

      ~ColorPalette() = default;

      /**
       * @brief - Used to assign the palette to be used by this object. Note that
       *          the palette will be covering the range `[1; m_maxAge]`. The max
       *          age is provided and construction time and cannot be changed.
       * @param palette - the gradient to assign to this palette.
       */
      void
      setGradient(sdl::core::engine::GradientShPtr palette);

      /**
       * @brief - Determine the color to assign to the cell specified by the age
       *          in argument.
       *          Note that an error is raised if the internal palette is not set.
       * @param age - the age of the cell for which the color is to be determined.
       * @return - the color assigned to the cell.
       */
      sdl::core::engine::Color
      colorize(unsigned age);

    private:

      /**
       * @brief - Used to retrieve a default palette defining nice coloring to
       *          represent cells.
       * @return - a default palette to use in case none is provided.
       */
      static
      sdl::core::engine::GradientShPtr
      getDefaultPalette() noexcept;

    private:

      /**
       * @brief - Defines the minimum age for a cell to be assigned to the last
       *          color of the palette. This allows toonly consider cells that
       *          are quite young to be displayed differently and to group all
       *          the `old` ones under a common color (something like adulthood
       *          for cells).
       */
      unsigned m_maxAge;

      /**
       * @brief - The gradient to use to represent cells until age `m_maxAge`.
       */
      sdl::core::engine::GradientShPtr m_gradient;
  };

  using ColorPaletteShPtr = std::shared_ptr<ColorPalette>;
}

# include "ColorPalette.hxx"

#endif    /* COLOR_PALETTE_HH */
