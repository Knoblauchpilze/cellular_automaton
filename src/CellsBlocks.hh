#ifndef    CELLS_BLOCKS_HH
# define   CELLS_BLOCKS_HH

# include <mutex>
# include <memory>
# include <vector>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Box.hh>

namespace cellulator {

  /**
   * @brief - Define the possible states of a cell.
   */
  enum class State {
    Dead,
    Alive
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

  class CellsBlocks: public utils::CoreObject {
    public:

      /**
       * @brief - Create a cells block with no associated cells. The goal of this object
       *          is to hide the complexity of managing the cells not as a single data
       *          array but rather as a collection of individual blocks with a reduced
       *          size.
       *          One can register or remove some blocks from the general vector along
       *          with accessing properties of a given cell. The dimensions of the blocks
       *          are specified at the construction of the object and cannot be changed
       *          afterwards. This will be used internally to build the atlas of blocks
       *          needed whenever the colony needs to be expanded.
       *          Note that this object does not have any notion of thread-safety so it
       *          should be handled with care.
       * @param ruleset - the set of rules to apply when evolving cells.
       * @param nodeDims - the dimensions of a single block of cells. This size will be
       *                   used for each allocation of a new cells block.
       */
      CellsBlocks(const rules::Type& ruleset,
                  const utils::Sizei& nodeDims);

      /**
       * @brief - Destruction of the cell block.
       */
      ~CellsBlocks() = default;

      /**
       * @brief - Resets the content of this cells blocks and allocate it again to cover
       *          the specified area. Note that the area is supposed to be centered at
       *          `0` and be made even if needed. Ideally we want to use multiple of `2`
       *          for most sizes as it ensures nice divisibility properties for lots of
       *          cases.
       *          In case the requested dimensions are not exactly divisible by the input
       *          `dims`, the blocks will be allocated to reach *at least* this size. This
       *          usually means allocating a larger area than expected.
       * @param dims - the minimum dimensions to reach. Will be reached when the node size
       *               divides perfectly this value.
       * @param state - the state to assign to each cell (default is `Dead`).
       * @return - the actual area allocated by this object. Should be a centered area with
       *           dimensions at least the ones of `dims`.
       */
      utils::Boxi
      allocateTo(const utils::Sizei& dims,
                 const State& state = State::Dead);

      /**
       * @brief - Used to assign random values and ages to all the cells currently allocated
       *          in the colony. All the blocks will be traversed but none will be created.
       * @return - the number of live cells created during the process.
       */
      unsigned
      randomize();

    private:
      
      /**
       * @brief - The set of rules associated to the cells handled by this block. The set
       *          described how and wehn cells should be born and die.
       */
      rules::Type m_ruleset;

      /**
       * @brief - Holds the dimensions of a single block of cells when allocated by this
       *          object. Each block of cells allocated by the object will be using this
       *          size.
       */
      utils::Sizei m_nodesDims;

      /**
       * @brief - The internal container for the state of cells representing the block.
       *          This array contain the current state of the cells (in contrast to the
       *          `m_nextStates` array).
       *          Note that contiguous patches of this vector might refer to very distinct
       *          locations in the colony's coordinate frame and some part might not be
       *          allocated (meaning no cells block refers to it).
       */
      std::vector<State> m_states;

      /**
       * @brief - Fills a similar purpose to `m_states` by holding the next states of the
       *          cells currently registered in the block. This state usually represent
       *          the future of the colony in a single step. It allows to keep the current
       *          generation available for computations and still schedule the evolution.
       */
      std::vector<State> m_nextStates;
  };

  using CellsBlocksShPtr = std::shared_ptr<CellsBlocks>;
}

# include "CellsBlocks.hxx"

#endif    /* CELLS_BLOCKS_HH */
