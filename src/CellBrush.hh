#ifndef    CELL_BRUSH_HH
# define   CELL_BRUSH_HH

# include <memory>
# include <core_utils/CoreObject.hh>
# include "CellsBlocks.hh"

namespace cellulator {

  // Forward declaration of a brush to be able to use `CellBrushShPtr`
  // right away.
  class CellBrush;

  using CellBrushShPtr = std::shared_ptr<CellBrush>;

  class CellBrush: public utils::CoreObject {
    public:

      /**
       * @brief - Create a brush and load the corresponding data in the specified
       *          file. The file should be a text file describing the live and dead
       *          cells existing in the configuration of the brush.
       *          The dimensions should be specified at the beginning of the file.
       * @param file - the name of the file from which the brush should be loaded.
       */
      CellBrush(const std::string& file);

      /**
       * @brief - Create a brush with the specified size filled with cells having a
       *          state equivalent to the input argument.
       * @param size - the size of the brush.
       * @param state - the state of the cells composing the brush.
       */
      CellBrush(const utils::Sizei& size,
                const State& state);

    private:
  };

}

# include "CellBrush.hxx"

#endif    /* CELL_BRUSH_HH */
