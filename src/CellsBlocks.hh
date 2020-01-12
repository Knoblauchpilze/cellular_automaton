#ifndef    CELLS_BLOCKS_HH
# define   CELLS_BLOCKS_HH

# include <mutex>
# include <memory>
# include <vector>
# include <unordered_map>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Vector2.hh>
# include "CellEvolver.hh"
# include "CellBrush.hh"

namespace cellulator {

  /**
   * @brief - Define the possible states of a cell.
   */
  enum class State {
    Dead,
    Alive
  };

  // Forward declaration to be able to use a shared pointer on a colony tile
  // right away.
  class ColonyTile;
  using ColonyTileShPtr = std::shared_ptr<ColonyTile>;

  // Forward declaration of the `CellBrush` class as it also includes this
  // file for the `State` declaration.
  class CellBrush;

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
       * @param nodeDims - the dimensions of a single block of cells. This size will be
       *                   used for each allocation of a new cells block.
       */
      CellsBlocks(const utils::Sizei& nodeDims);

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
       *          This method is mostly a wrapper which interpret the input dimensions to
       *          build a consistent area and then calls the internal wrapper `allocate`.
       * @param dims - the minimum dimensions to reach. Will be reached when the node size
       *               divides perfectly this value.
       * @return - the actual area allocated by this object. Should be a centered area with
       *           dimensions at least the ones of `dims`.
       */
      utils::Boxi
      allocateTo(const utils::Sizei& dims);

      /**
       * @brief - Used to assign random values and ages to all the cells currently allocated
       *          in the colony. All the blocks will be traversed but none will be created.
       * @return - the number of live cells created during the process.
       */
      unsigned
      randomize();

      /**
       * @brief - Wrapper to the internal `stepPrivate` method. ALlows for external elements
       *          to trigger the next step of the colony. Basically just acquire the internal
       *          locker and transmit the call to `stepPrivate`.
       * @return - the number of alive cells at the current generation.
       */
      unsigned
      step();

      /**
       * @brief - Used to generate a schedule of all the blocks currently registered in this
       *          object and wrap them in the input vector as `ColonyTile`s. This can then
       *          be used for example to perform the evolution of the colony.
       *          Note that no nodes are created nor destroyed during this operation.
       * @param tiles - the output vector to use to register the tiles.
       */
      void
      generateSchedule(std::vector<ColonyTileShPtr>& tiles);

      /**
       * @brief - Used to perform the evolution of the block represetned by the input index.
       *          Nothing happens if the block does not exist internally. Otherwise the cells
       *          composing the block are updated so that their `m_nextStates` and adjacency
       *          is changed to rerepsent their future version.
       *          Note that the state is not actually applied in order to allow other blocks
       *          to get evolved: this only happens upon calling the `step` method which will
       *          evolve all the blocks at once.
       * @param blockID - the index of the block to evolve.
       */
      void
      evolve(unsigned blockID);

      /**
       * @brief - Used to retrieve the current live area for this object. This encompasses any
       *          live cell in the colony allowing for example to fit to content as tightly as
       *          possible.
       *          This area is cached from evolution step and thus is not expensive to retrieve.
       *          Note that it is only up-to-date as the last call to `step`.
       * @return - the area covering all live cells for this object.
       */
      utils::Boxf
      getLiveArea() noexcept;

      /**
       * @brief - Used to retrieve the state and age of the cell at `coord` if any. In case the
       *          cell is dead or is not part of the colony a `Dead` state and a negative age
       *          (usually `-1`) will be returned.
       * @param coord - the coordinate of the cell to retrieve.
       * @return - the status of the cell at specified coordinate.
       */
      std::pair<State, int>
      getCellStatus(const utils::Vector2i& coord);

      /**
       * @brief - Used to retrieve the cells from the area described in input into the specified
       *          vector. Internal blocks will be scanned for any matching the corresponding area
       *          and filled in the output `cells` vector.
       *          As we store the cells in contiguous blocks we can be a bit more efficient and
       *          try to assign all the cells registered in a single block before moving on to
       *          the next one.
       *          Also there's no need to traverse the individual cells requetsed by the input
       *          area because they will automatically be discovered when scanning the internal
       *          list of blocks.
       *          The default state of non-existing cells is `Dead`.
       * @param cells - output vector where cells will be saved.
       * @param area - the area for which cells should be retrieved.
       */
      void
      fetchCells(std::vector<std::pair<State, unsigned>>& cells,
                 const utils::Boxi& area);

      /**
       * @brief - Used by external providers to update the ruleset used by this colony to perform
       *          the evolution of the cells.
       * @param ruleset - the rules to use to evolve cells.
       */
      void
      setRuleset(CellEvolverShPtr ruleset);

      /**
       * @brief - Used to paint the input `brush` on this blocks of cells. The area covered by the
       *          brush is scanned in order to add the required cells. If blocks need to be created
       *          to accomodate with the brush they are.
       *          The brush is not checked for validity so failure to guarantee that may cause some
       *          undefined behavior.
       *          Note that the modifications are applied to the current state of the colony so that
       *          it can be directly used when computing the next generation of cells.
       * @param brush - the brush to repaint.
       * @param coord - the position at which the brush should be repainted.
       */
      void
      paint(const CellBrush& brush,
            const utils::Vector2i& coord);

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

        bool active;      //< `true` if the block is registered in the `m_blocks` array
                          //< and `false` if this is not the case. Such cases indicate
                          //< blocks which have been destroyed and are waiting to be
                          //< reused.
        unsigned alive;   //< The number of alive cells.
        unsigned nAlive;  //< The number of alive cells in the next state of this block.
        unsigned changed; //< The number of cells which changed in the previous iteration.
                          //< If this value is `0` and `alive > 0` it means that only still
                          //< life forms are present in the block so we can skip it.

        int west;         //< The index of the block directly on the left of this one.
                          //< The value is set to `-1` if the block does not exist.
        int east;         //< The index of the block directly on the right of this one.
        int south;        //< The index of the block directly on the bottom of this one.
        int north;        //< The index of the block directly on the top of this one.

        int nw;           //< Index of the north west block if any.
        int ne;           //< Index of the north east block if any.
        int sw;           //< Index of the south west block if any.
        int se;           //< Index of the south east block if any.
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
       * @brief - Perform the allocation of the internal buffer arrays to match the input
       *          dimensions and assign a `Dead` state to each created cell. Note that the
       *          input area is assumed to be a perfect multiple of the internal nodes size
       *          and failure to comply will cause undefined behavior.
       *          All the needed block are allocated right away.
       * @param area - the area to allocate. Must be a multiple of `m_nodesDims`.
       */
      void
      allocate(const utils::Boxi& area);

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
       * @return - the description of the created block.
       */
      BlockDesc
      registerNewBlock(const utils::Boxi& area);

      /**
       * @brief - Used to unregister the block specified by the input id. Note that
       *          the input index is both the identifier of the block and its index
       *          in the `m_blocks` index.
       *          We will set the `active` boolean to `false` for the block and set
       *          it as available in the `m_freeBlocks` array.
       *          Nothing happens if the block is already inactive.
       *          Note that the internal mutex is assumed to be locked upon calling
       *          this method.
       * @param blockID - the index of the block to destroy.
       * @return - `true` if the block was effectively destroyed and `false` otherwise
       *           (usually indicating that it was already inactive).
       */
      bool
      destroyBlock(unsigned blockID);

      /**
       * @brief - Clears any data registered in this cells blocks excluding the
       *          internal node dimensions and ruleset.
       *          This allows to start fresh and should theoretically followed
       *          at some point by a call to the `allocateTo` method.
       *          Assumes that the internal locker is already acquired.
       */
      void
      clear();

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
       * @brief - Compute the coordinate to access the cell's data in the internal
       *          arrays from the input coord, given that the coordinates are within
       *          the block. Failure to guarantee that will cause undefined behavior.
       *          Note that the coordinates should be expressed in *global* coordinate
       *          frame.
       * @param block - the block to which the coordinate belongs.
       * @param coord - the coordinate expressed in absolute coordinate frame. This
       *                method converts it to local block's coordinate frame and then
       *                uses it to extract the data index if needed.
       * @param global - `true` if the `coord` is expressed in absolute coordinate frame
       *                 and `false` if it is already expressed in local block's frame.
       *                 Note that local block's frame means `[0, w[x[0, h[`.
       * @return - an index which can be used in the internal arrays (like `m_state`
       *           for example) and which corresponds to the cell's data.
       */
      int
      indexFromCoord(const BlockDesc& block,
                     const utils::Vector2i& coord,
                     bool global) const;

      /**
       * @brief - Compute the real world coordinates of the `coord` cell in the input
       *          block. Basically the reverse method from `indexFromCoord`. This means
       *          that `coordFromIndex(block, indexFromCoord(block, coord) = coord`.
       *          No checks are performed to verify whether the `coord` is effectively
       *          a valid index for the block, nor whether the block is valid.
       *          We assume that the block is layout in such a way that the coord `0`
       *          corresponds to the `area.getBottomLeft()` point and that the `max`
       *          coord (i.e. `block.end - block.start`) corresponds to the top right.
       * @param block - the description of the block the `coord` is referring to.
       * @param coord - an index representing a cell in the internal data of the block.
       * @param global - `true` if the output coordinate should be expressed in global
       *                 coordinate frame and `false` if it should be expressed in the
       *                 block's coordinate frame (i.e. `[0, w[x[0, h[`).
       * @return - the real world coordinate of the `coord` index in the block.
       */
      utils::Vector2i
      coordFromIndex(const BlockDesc& block,
                     unsigned coord,
                     bool global) const;

      /**
       * @brief - Perform the update of the adjacency for the cell at `coord` in the
       *          specified block. The coordinates should be expressed in the block's
       *          coordinate frame (i.e. `x in [0; block.area.w()]` and same for `y`)
       *          to be considered valid: failure to do so will terminate the method
       *          early (so it's safe but not recommended).
       *          This method will add one to all the adjacency count of cells that
       *          are neighboring the input coordinate to the next adjacency step.
       *          In case the coordinate are located on the boundary of the block the
       *          neighboring blocks are scanned and updated but are not created if
       *          they do not exist.
       * @param block - the block related to the coordinate.
       * @param coord - the coordinate within the block to update. The coordinate area
       *                assumed to lie in the range `[0, w[x[0, h[`.
       * @param makeCurrent - `true` if the current adjacency should be updated.
       */
      void
      updateAdjacency(const BlockDesc& block,
                      const utils::Vector2i& coord,
                      bool makeCurrent);

      /**
       * @brief - Used to randomize the content of the block described in input. The
       *          cells composing this node will be assigned random state based on
       *          the input probability.
       *          Note that the adjacency is also updated once all the cells have
       *          been generated.
       *          Assumes that the internal locker is already acquired.
       * @param desc - the block description to be randomized.
       * @param deadProb - the probability to generate a dead cell. Should be in the
       *                   range `[0; 1]` but we don't check it.
       */
      void
      makeRandom(BlockDesc& desc,
                 float deadProb);

      /**
       * @brief - Used to update the age of all the cells registered in the blocks.
       *          Will traverse the `m_states` array and use it to increase the age
       *          of live cells.
       *          Assumes that the internal locker is already acquired.
       */
      void
      updateCellsAge() noexcept;

      /**
       * @brief - Used to update the live area to encompass the live cells in this
       *          colony. All active blocks are scanned for live cells and accounted
       *          for in the `m_liveArea`.
       *          Assumes that the internal locker is already acquired.
       */
      void
      updateLiveArea() noexcept;

      /**
       * @brief - Used to determine whether this block is a boundary node and perform
       *          the needed allocation should it be the case. We consider that a node
       *          is a boundary as long as one of its neighboring node is not registered
       *          in the internal blocks list.
       *          Note that we don't scan the blocks list but rather rely on the props
       *          of the block: indeed if any of the `east`, `west`, etc. point is set
       *          to `-1` it means that the corresponding block is not allocated and
       *          thus we should do it.
       *          The allocation only occurs in case the block is active and has at least
       *          one live cells (otherwise there's no need to allocate it) or if the
       *          `force` boolean is set to `true`.
       * @param blockID - the index of the block for which boundaries should be allocated.
       * @param force - `true` if the boundary for the input node should be created even
       *                though the node does not have any live cells in it. Note that an
       *                inactive block will still be left unchanged.
       * @return - `true` if some nodes have been allocated for this block.
       */
      bool
      allocateBoundary(unsigned blockID,
                       bool force) noexcept;

      /**
       * @brief - Try to find a block spanning the `area`. Note that the block is considered
       *          to be spanning the input `area` if it contains all of it: partially covering
       *          the area is not considered a valid block.
       *          This is quite a constraint but we figured that most of the area will have
       *          been generated by this object anyways so it is not super-complicated to use
       *          areas exactly covering blocks in this situation.
       *          Assumes that the locker for this object is already acquired.
       * @param area - the area to find in the registered blocks.
       * @param id - the output argument which should be populated with the block's index.
       * @return - `true` if a block spanning the input area was found and `false` otherwise.
       *           Note that in case the return value is `false` the vaule of `desc` should
       *           be ignored (it will be set to `-1`).
       */
      bool
      find(const utils::Boxi& area,
           int& id);

      /**
       * @brief - Used to retrieve the block containing the input coordinate. There's at most one
       *          block that can contain it as blocks do not overlap. In case the corresponding
       *          block is not allocated yet the return value should be ignored. This is indicated
       *          by the `found` boolean being `false`.
       *          Note that the locker is assumed to already be acquired. The input `coord` should
       *          be expressed in local coordinate frame.
       * @param coord - the coordinate that should be included in a block: should be expressed in
       *                real world coordinate frame.
       * @param found - output boolean indicating whether the return value is relevant (i.e. if
       *                the block has been found).
       * @return - the index of the block containing the input coordinate. Should be ignored if the
       *           `found` boolean is `false`.
       */
      unsigned
      findBlock(const utils::Vector2i& coord,
                bool& found);

      /**
       * @brief - Used to perform the necessary modification to the internal blocks so
       *          that the `from` node is related to its neighbors. We will scan the
       *          internal `m_blocksLinks` table through the `find` interface in order
       *          to link the block to its neighbors.
       *          We will also perform the needed attachements for the neighboring nodes
       *          to reflect this new state (so for example if `block.sw == from` we should
       *          also have `from.ne == block` etc.).
       *          Note that only nodes that already exists will be attached but none will
       *          be created.
       * @param from - the index of the element which should be attached to neighboring
       *               blocks.
       */
      void
      attach(int from);

      /**
       * @brief - Used to perform the exact opposite operation than `attach`. It should
       *          be called whenever a block is destroyed so that it is marked as not
       *          registered in its neighbors: this will guarantee that it gets created
       *          again if needed.
       * @param from - the index of the element which should be detached from neighboring
       *               blocks.
       */
      void
      detach(int from);

      /**
       * @brief - Used to move the cells one step forward in time. This basically means swapping
       *          the internal arrays representing the next step with the current one. We also
       *          need to perform the expansion of the colony in case some cells are now on the
       *          boundaries so that we can continue simulating them properly.
       * @return - the number of alive cells at the current generation.
       */
      unsigned
      stepPrivate();

    private:

      /**
       * @brief - Convenience typedef to be able to refer to a map storing a unique identifier
       *          for a coordinate and its associated position in the `m_blocksIndex` array.
       */
      using AreaToBlockIndex = std::unordered_map<unsigned, unsigned>;

      /**
       * @brief - Protect this object from concurrent accesses.
       */
      std::mutex m_propsLocker;

      /**
       * @brief - The set of rules associated to the cells handled by this block. The set
       *          described how and when cells should be born and die.
       */
      CellEvolverShPtr m_ruleset;

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
       * @brief - As we want to allow the computation of blocks in parellel, it means that
       *          we need to find a way to make sure that only a single thread is accessing
       *          the cells data at any time.
       *          Regarding the cells' state it is quite easy as the only information that
       *          is needed to evolve a cell is the current cell. But it's not so easy for
       *          the adjacency. While most of the accesses are fine (i.e. accesses where
       *          the data is *inside* a block), some might concurrently modify the values
       *          of certain cells: namely the ones that lie on the boundaries of blocks.
       *          In order to not greatly decrease the efficiency of the worker threads, we
       *          will use this locker to protect concurrent accesses to adjacency values
       *          for any cells on the boundary of a block.
       *          Actually we do want to protect cells which can influence the boundary so
       *          it also cvers the second to boundary cells of a node.
       */
      std::mutex m_adjacencyLocker;

      /**
       * @brief - Holds an array representing the age of each cells. Note that this array
       *          should only be interpreted in case the `m_states` value indicates a live
       *          cell at this point.
       */
      std::vector<int> m_ages;

      /**
       * @brief - Holds a count of the number of active blocks currently registered in the
       *          object. When this value drops to `0` it means that there are no remaining
       *          live cells anywhere in the colony.
       */
      unsigned m_liveBlocks;

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

      /**
       * @brief - Used to receive the list of blocks currently registered in the `m_blocks`
       *          array. This allows to easily get from the area representing the block to
       *          the block's index.
       *          The key is a value computed from the area associated to the block and is
       *          unique so it can be used to identify a given block. This is really very
       *          interesting in case we want to easily find whether an area already has a
       *          associated block for example in the case of linking a block to another.
       */
      AreaToBlockIndex m_blocksIndex;

      /**
       * @brief - Represents the total area covered by the block currently allocated. Note
       *          that this area might not be contiguous in the sense that part of its
       *          interior can be composed of dead blocks. Also only part of the boundary
       *          might not be allocate. It is still useful as an upper bound on the size
       *          reached by the colony.
       */
      utils::Boxi m_totalArea;

      /**
       * @brief - Similar to `m_totalArea` but limits the area to the region where at least
       *          a live cell can be found. What this area represents is that on any border
       *          one can find a live cell (i.e. there exists a live cell in the border from
       *          `m_liveArea.getTopLeft()` and `m_liveArea.getTopRight()` and so on).
       *          This area is at most equal to the `m_totalArea` and can be useful when a
       *          fit to content operation needs to be performed.
       */
      utils::Boxf m_liveArea;
  };

  using CellsBlocksShPtr = std::shared_ptr<CellsBlocks>;
}

# include "CellsBlocks.hxx"

#endif    /* CELLS_BLOCKS_HH */
