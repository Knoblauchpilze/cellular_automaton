#ifndef    COLONY_HH
# define   COLONY_HH

# include <mutex>
# include <memory>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include "ColonyTile.hh"
# include "CellsBlocks.hh"

namespace cellulator {

  class Colony: public utils::CoreObject {
    public:

      /**
       * @brief - Create a colony with the specified size. All cells will be
       *          initialized to a dead state. The user can provide a name for
       *          the colony.
       * @param dims - the dimensions of the colony.
       * @param ruleset - the set of rules to use to update the cells at each
       *                  iteration.
       * @param name - the lil' name of the colony.
       */
      Colony(const utils::Sizei& dims,
             const rules::Type& ruleset,
             const std::string& name = std::string("Daddy's lil monster"));

      /**
       * @brief - Destruction of the colony.
       */
      ~Colony() = default;

      /**
       * @brief - Used to retrieve the area encompassing all living cells in the
       *          colony. This only includes the tiles that contain at least one
       *          living cell.
       *          The size is expressed in terms of cells.
       * @return - the living area occupied by the colony.
       */
      utils::Boxi
      getArea() noexcept;

      /**
       * @brief - Used to retrieve the cells from the area described in input into
       *          the specified vector. Note that any existing data in the vector
       *          will be erased. Also the area is clamped to match the dimensions
       *          of the colony if needed. The returned area describes the actual
       *          content of the `cells` vector.
       *          The input dimensions are clamped to the lowest which means that
       *          for example if the box spans `x: 50, w: 25`, the actual cells
       *          will be `[37; 62]`.
       * @param cells - output vector where cells will be saved.
       * @param area - the area for which cells should be retrieved.
       * @return - the actual box of the cells returned in the `cells` vector.
       */
      utils::Boxi
      fetchCells(std::vector<State>& cells,
                 const utils::Boxf& area);

      /**
       * @brief - Used to simulate a single step of the colony's life. Nothing is
       *          done in case the colony is already running.
       *          Note that this method does not magically creates data and expect
       *          each individual cell to be updated with a consistent `next` state
       *          before calling it.
       *          Usually this is done through the `generateSchedule` interface to
       *          generate a list of jobs that can be executed to evolve each cell
       *          of the colony.
       *          Once this is done this method only applies the modifications so
       *          as to make them current.
       * @param alive - pointer which will be filled with the number of alive cells
       *                in the colony if needed.
       * @return - the new generation of the colony (typically starts at `0`): note
       *           that the first value returned by this method is `1` (as the `0`
       *           generation is actually the starting point).
       */
      unsigned
      step(unsigned* alive = nullptr);

      /**
       * @brief - Used to generate a random colony without modifying the dimensions
       *          of the colony. Each cell will be assigned a random state among the
       *          available ones.
       *          An error is raised in case the simulation is started.
       * @return - the number of live cells created during the process. This value
       *           can be used to update some external displays.
       */
      unsigned
      generate();

      /**
       * @brief - Used to generate a list of tiles to schedule for evolving the cells
       *          composing the colony. This schedule is by no means executed and is
       *          used to reflect the internal structure of the colony to divide the
       *          workload efficiently.
       *          The return value is a list of tiles that can be executed concurrently
       *          and which allow to evolve the colony one step further in time.
       * @return - a list of tiles to execute to evolve the colony.
       */
      std::vector<ColonyTileShPtr>
      generateSchedule();

    private:

      /**
       * @brief - Used to retrieve a suited size for a block of cells. Rather than having
       *          a single gigantic array where cells are stored we divide the colony into
       *          several smaller blocks containing a part of the cells. This helps having
       *          a better strategy when it comes to de/allocating elements and keeping
       *          track of living cells.
       *          The size of the block should not be too big (otherwise we might not be
       *          able to factorize correctly dead blocks) and not too small (otherwise
       *          we might end up with toog big an overhead managing all the blocks).
       * @return - a suited size for a cell block.
       */
      static
      utils::Sizei
      getCellBlockDims() noexcept;

      /**
       * @brief - Connect signals and build the scheduler to use to simulate the colony.
       *          Also perform the creation of the undelrying data used to keep the cells'
       *          data for this colony.
       * @param dims - the dimensions of the colony upon creation.
       * @param ruleset - the type of simulation to perform on the cells.
       */
      void
      build(const utils::Sizei& dims,
            const rules::Type& ruleset);

    private:

      /**
       * @brief - Protect this colony from concurrent accesses.
       */
      std::mutex m_propsLocker;

      /**
       * @brief - Holds the generation reached by this colony. Each call to `step`
       *          or a simulation step when `start` has been called triggers a new
       *          generation which is kept internally.
       */
      unsigned m_generation;

      /**
       * @brief - The internal container for the cells representing the colony.
       */
      CellsBlocksShPtr m_cells;
  };

  using ColonyShPtr = std::shared_ptr<Colony>;
}

# include "Colony.hxx"

#endif    /* COLONY_HH */
