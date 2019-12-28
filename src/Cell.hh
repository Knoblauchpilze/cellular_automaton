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

  namespace rules {

    /**
     * @brief - The ruleset to use to compute the next state of a cell
     *          based on both its current state and the number of alive
     *          neighboring cells.
     */
    enum class Type {
      GameOfLife
    };
  }

  class Cell {
    public:

      /**
       * @brief - Create a cell with the provided state (default is a dead
       *          cell) and ruleset (default is game of life).
       * @param state - the state of the cell at initialization.
       * @param ruleset - the set of rules to apply to compute the state of
       *                  a cell based on its current state and the number
       *                  of alive cells in the neighborhood.
       */
      Cell(const State& state = State::Dead,
           const rules::Type& ruleset = rules::Type::GameOfLife);

      ~Cell() = default;

      /**
       * @brief - Retrieves the current state of the cell.
       * @return - the state of the cell.
       */
      State
      state() const noexcept;

      /**
       * @brief - Used to assign a random state to the cell from the possible
       *          values. Note that the new state is applied right away and
       *          does not need to be `validated` through the `step` method
       *          like in the case of an update.
       * @return - the new state for this cell.
       */
      State
      randomize();

      /**
       * @brief - Used to save the current value of the `next` step into the
       *          current state of the cell. This method allows to effectively
       *          change the current state of the cell so that it can be used
       *          in later computations.
       * @return - the new state for this cell.
       */
      State
      step();

      /**
       * @brief - Update the value of the `next` step with the evolution of the
       *          current state given that there are `livingNeighbors` alive
       *          cells around this one. The value is not applied directly to
       *          the current state (which means that `state` will still return
       *          the same value) but saved in the `next` internal attribute to
       *          be then applied by the `step` function.
       *          Returns the predicted state of the cell.
       * @param livingNeighbors - the number of alive cells in the neighboorhood
       *                           of the cell.
       * @return - the predicted state of the cell.
       */
      State
      update(unsigned livingNeigbors);

    private:

      /**
       * @brief - Used to compute the evolution of the current cell with a living
       *          neighboring cells and to return the corresponding state.
       * @param current - the current state of the cell.
       * @param living - the number of living neighboring cells.
       * @return - a the state of the cell at the next iteration.
       */
      static
      State
      evolveGameOfLife(const State& current,
                       unsigned living) noexcept;

    private:

      /**
       * @brief - The set of rules to use to compute the next state of the cell
       *          based on its current one. Each ruleset has its own handling
       *          method. Failure to find the associated execution kernel will
       *          raise an error.
       */
      rules::Type m_ruleset;

      /**
       * @brief - The current state of the cell.
       */
      State m_state;

      /**
       * @brief - Holds the next state of the cell as computed during the `update`
       *          method. This is only applied to the `m_state` attribute upon a
       *          call to the `step` method.
       */
      State m_next;
  };

}

# include "Cell.hxx"

#endif    /* CELL_HH */
