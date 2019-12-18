#ifndef    CELL_HH
# define   CELL_HH

namespace cellulator {

  /**
   * @brief - Define the possible states of a cell.
   */
  enum class State {
    Dead,
    Newborn,
    Alive,
    Dying,
    Count
  };

  class Cell {
    public:

      /**
       * @brief - Create a cell with the provided state (default is a dead
       *          cell).
       * @param state - the state of the cell at initialization.
       */
      Cell(const State& state = State::Dead);

      ~Cell() = default;

      /**
       * @brief - Retrieves the current state of the cell.
       * @return - the state of the cell.
       */
      State
      state() const noexcept;

      /**
       * @brief - Used to assign a random state to the cell from the possible
       *          values.
       */
      void
      randomize();

    private:

      /**
       * @brief - The current state of the cell.
       */
      State m_state;
  };

}

# include "Cell.hxx"

#endif    /* CELL_HH */
