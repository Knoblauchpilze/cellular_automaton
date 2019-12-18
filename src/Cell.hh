#ifndef    CELL_HH
# define   CELL_HH

namespace cellulator {

  class Cell {
    public:

      /**
       * @brief - Create a cell with the provided state (default is a dead
       *          cell).
       * @param state - the state of the cell at initialization.
       */
      Cell();

      ~Cell() = default;

    private:
  };

}

# include "Cell.hxx"

#endif    /* CELL_HH */
