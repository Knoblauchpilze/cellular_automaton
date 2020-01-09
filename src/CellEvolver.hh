#ifndef    CELL_EVOLVER_HH
# define   CELL_EVOLVER_HH

# include <memory>
# include <vector>
# include <unordered_set>
# include <core_utils/CoreObject.hh>

namespace cellulator {

  class CellEvolver: public utils::CoreObject {
    public:

      /**
       * @brief - Create an object allowing to perform the evolution of cells
       *          based on a set of rules describing how many of the neighbors
       *          are allowed to be alive or dead for a cell to be born or to
       *          die.
       * @param live - the number of live cells for a dead cell to be born.
       * @param survive - the number of live cells for a live cell to survive.
       */
      CellEvolver(const std::vector<unsigned>& live = std::vector<unsigned>{3},
                  const std::vector<unsigned>& survive = std::vector<unsigned>{2, 3});

      ~CellEvolver() = default;

      /**
       * @brief - Clear any existing option for cells to be born or surviving.
       */
      void
      clear() noexcept;

      /**
       * @brief - Used to register a new valid number allowing a dead cell to
       *          be born on the next generation. Nothing happens if this count
       *          is already registered.
       * @param neighbor - the number of live cells for a dead one to be born.
       * @return - `true` if the count was not yet existing.
       */
      bool
      addBornOption(unsigned neighbor) noexcept;

      /**
       * @brief - Fill a similar purpose to `addBornOption` but describe the
       *          number of live cells that a live cell should have in order
       *          to stay alive in the next generation.
       * @param neighbor - the number of live cells for a live cell to stay
       *                   alive.
       * @return - `true` if the count was not yet existing.
       */
      bool
      addSurvivingOption(unsigned neighbor) noexcept;

      /**
       * @brief - Used to determine whether a cell with the specified number
       *          of neighbots will be born in the next generation given the
       *          internal set of rules for this object.
       * @param neighbor - the number of live neighbors to the cell.
       * @return - `true` if the cell will be born and `false` otherwise (the
       *           cell stays dead).
       */
      bool
      isBorn(unsigned neighbor);

      /**
       * @brief - Used to determine whether a cell with the specified number
       *          of neighbors will survive in the next generation given the
       *          internal set of rules for this object.
       * @param neighbor - the number of live neighbors to the cell.
       * @return - `true` if the cell survives, `false` otherwise (so the
       *           cell dies).
       */
      bool
      survives(unsigned neighbor);

    protected:

      /**
       * @brief - Register a vector into the internal sets.
       * @param vec - the vector to register.
       * @param live - `true` if the vector should be assigned to the `m_born`
       *               set and `false` if the vector should be added to the
       *               `m_survive` vector.
       */
      void
      registerVector(const std::vector<unsigned>& vec,
                     bool live) noexcept;

    private:

      /**
       * @brief - Holds the set describing how many cells should be surrounding
       *          a dead cell for it to become alive.
       */
      std::unordered_set<unsigned> m_born;

      /**
       * @brief - Holds the set describing how many living cells should surround
       *          a live cell for it to stay alive.
       */
      std::unordered_set<unsigned> m_survive;
  };

  using CellEvolverShPtr = std::shared_ptr<CellEvolver>;
}

# include "CellEvolver.hxx"

#endif    /* CELL_EVOLVER_HH */
