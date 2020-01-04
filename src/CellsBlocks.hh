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
       * @brief - Describe a cell block with all its associated properties. Note that we
       *          have convenience attributes which can speed up the access and fetching
       *          of information about the block. The drawback is that we need to update
       *          these properties when needed.
       */
      struct BlockDesc {
        unsigned id;      //< A unique identifier for this block.

        utils::Boxi area; //< The area represented by this block.
        unsigned start;   //< The starting index of this block in the general `m_cells`
                          //< array.
        unsigned end;     //< The end index of this block. Should be equal to the start
                          //< index plus the size of the block.

        unsigned alive;   //< The number of alive cells.
        unsigned changed; //< The number of cells which changed in the previous iteration.
                          //< If this value is `0` and `alive > 0` it means that only still
                          //< life forms are present in the block so we can skip it.
      };

      /**
       * @brief - Generate a default probability to create a `Dead` cell. This can be used
       *          for example when randomizing the cells of individual blocks.
       * @return - a value in the range `[0; 1]` defining the probability to generate a
       *           `Dead` cell. Note that `0` indicates no chance at all to generate `Dead`
       *           cells.
       */
      static
      float
      getDeadCellProbability() noexcept;

      /**
       * @brief - Try to find a block spanning the `area`. Note that the block is considered
       *          to be spanning the input `area` if it contains all of it: partially covering
       *          the area is not considered a valid block.
       *          This is quite a constraint but we figured that most of the area will have
       *          been generated by this object anyways so it is not super-complicated to use
       *          areas exactly covering blocks in this situation.
       * @param area - the area to find in the registered blocks.
       * @param desc - the output argument which should be populated with the block's info.
       * @return - `true` if a block spanning the input area was found and `false` otherwise.
       *           Note that in case the return value is `false` the vaule of `desc` should
       *           be ignored.
       */
      bool
      find(const utils::Boxi& area,
           BlockDesc& desc);

      /**
       * @brief - Register a block corresponding to the input area. All remaining
       *          information will be populated automatically. Note that no checks
       *          are performed to verify whether a block spanning this area does
       *          already exist or not.
       *          One can use the `find` method to attempt to retrieve any existing
       *          block for said area.
       *          Note that the locker is assumed to be locked upon calling this
       *          method.
       * @param area - the area to associate to the block.
       * @param state - the state of cells to assign to the newly created block.
       * @return - the description of the created block.
       */
      BlockDesc
      registerNewBlock(const utils::Boxi& area,
                       const State& state = State::Dead);

      /**
       * @brief - Used to retrieve the index at which the data for a block with the
       *          specified index should begin in the internal vectors. This value
       *          is usually stored directly in the block (see `start`).
       *          Note that the locker is assumed to be locked upon calling this
       *          method.
       * @param blockID - the index of the block for which the data index should be
       *                  computed.
       * @return - an index representing the value at which the data for the block
       *           with the specified index is stored.
       */
      unsigned
      dataIDFromBlock(unsigned blockID) const noexcept;

      /**
       * @brief - Convenience method to retrieve the number of cells contained in a
       *          single block. This uses internally the `m_nodeDims` to provide the
       *          values count.
       * @return - the number of cells contained in a single cells block.
       */
      unsigned
      sizeOfBlock() const noexcept;

      /**
       * @brief - Used to randomize the content of the block described in input. The
       *          cells composing this node will be assigned random state based on
       *          the input probability.
       *          Note that the adjacency is also updated once all the cells have
       *          been generated.
       * @param desc - the block description to be randomized.
       * @param deadProb - the probability to generate a dead cell. Should be in the
       *                   range `[0; 1]` but we don't check it.
       * @param makeCurrent - `true` if the state of the cell to generate should be
       *                      assigned to the current state or to the next state.
       */
      void
      makeRandom(BlockDesc& desc,
                 float deadProb,
                 bool makeCurrent);

    private:

      /**
       * @brief - Protect this object from concurrent accesses.
       */
      std::mutex m_propsLocker;
      
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
       * @brief - Stores the number of alive cells in the neighborhood of any cell in the
       *          colony. This array iallows a nice speed boost compared to the classical
       *          method (i.e. determining for each cell the number of neighbors) at some
       *          extra memory cost. It is updated for each evolution and can be used to
       *          compute the next state of each cell.
       */
      std::vector<unsigned> m_adjacency;

      /**
       * @brief - Fills a similar purpose to `m_states` by holding the next states of the
       *          cells currently registered in the block. This state usually represent
       *          the future of the colony in a single step. It allows to keep the current
       *          generation available for computations and still schedule the evolution.
       */
      std::vector<State> m_nextStates;

      /**
       * @brief - Fills a similar purpose to `m_nextStates`: holds the adjacency for cells
       *          for their next state. Ideally this vector should be swapped with the other
       *          `m_adjacency` in order to make it current.
       */
      std::vector<unsigned> m_nextAdjacency;

      /**
       * @brief - The list of registered cells blocks so far. This list is the heart of the
       *          classification system as it allows to interpret the raw data contained in
       *          all the above vectors.
       *          This vector should never be shrinking but might contain a lot of dead
       *          blocks where there are either no alive cells or only still life forms.
       */ 
      std::vector<BlockDesc> m_blocks;

      /**
       * @brief - Used to contain the list of free blocks in the general vectors. This array
       *          might be empty in which case it indicates that the `m_states` and similar
       *          states are not sparse at all (all the data is still relevant). In this case
       *          the `registerBlock` method should append data to the end of vectors shall
       *          a block be created.
       *          If this vector contains some values, it indicates id of blocks that can be
       *          recycled in the internal vectors: this is done in an attempt to reduce the
       *          memory footprint of this object.
       */
      std::vector<unsigned> m_freeBlocks;
  };

  using CellsBlocksShPtr = std::shared_ptr<CellsBlocks>;
}

# include "CellsBlocks.hxx"

#endif    /* CELLS_BLOCKS_HH */
