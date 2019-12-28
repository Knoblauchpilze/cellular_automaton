#ifndef    CELLS_QUAD_TREE_NODE_HH
# define   CELLS_QUAD_TREE_NODE_HH

# include <memory>
# include <vector>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Box.hh>
# include "Cell.hh"
# include "ColonyTile.hh"

namespace cellulator {

  // Forward declaration to be able to use a shared pointer on a tree node
  // right away.
  class CellsQuadTreeNode;
  using CellsQuadTreeNodeShPtr = std::shared_ptr<CellsQuadTreeNode>;

  // Forward declaration to be able to use a shared pointer on a colony tile
  // right away.
  class ColonyTile;
  using ColonyTileShPtr = std::shared_ptr<ColonyTile>;

  class CellsQuadTreeNode: public utils::CoreObject {
    public:

      /**
       * @brief - Create a quad tree node representing the input area.
       *          The set of rules to use to evolve each cell of the
       *          node is defined through the `ruleset`. Upon creating
       *          the node all the cells will be assigned a `Dead` state.
       * @param area - the area of the node.
       * @param ruleset - the set of rules to use to update the cells to
       *                  the next iteration.
       */
      CellsQuadTreeNode(const utils::Boxi& area,
                        const rules::Type& ruleset);

      ~CellsQuadTreeNode() = default;

      /**
       * @brief - Used to assign random values for each cell of this node.
       */
      void
      randomize();

      /**
       * @brief - Used to retrieve the cells from the area described in input into
       *          the specified vector. Note that the vector is assumed to already
       *          be able to contain the data to represent the input `area`. If it
       *          is not the case undefined behavior will arise. This method will
       *          compare the input area with its internal area and fill in all the
       *          cells that are in the input `area`. If the area associated to this
       *          node does not intersect the input `area` the method returns and
       *          does not perform any modifications on the vector.
       * @param cells - output vector where cells will be saved.
       * @param area - the area for which cells should be retrieved.
       * @return - the actual box of the cells returned in the `cells` vector.
       */
      void
      fetchCells(std::vector<State>& cells,
                 const utils::Boxi& area);

      /**
       * @brief - Used to split this node into sub-nodes until the node size reaches
       *          the provided argument. It will split up the internal data into a
       *          set of children nodes recursively.
       *          Note that the internal data is erased upon splitting the node.
       * @param size - the maximum allowed size for nodes.
       */
      void
      splitUntil(const utils::Sizei& size);

      /**
       * @brief - Return the number of alive cells in this node. This can be used to
       *          speed up the computations and not compute anything if needed.
       * @return - the number of alive cells for this node.
       */
      unsigned
      getAliveCellsCount() noexcept;

      /**
       * @brief - Used to register the tiles needed to recompute the cells contained
       *          by this node or its children. Each needed tile is added to the input
       *          provided container. The tiles already contained in the vector are
       *          left unchanged.
       * @param tiles - an output array where the tiles needed to evolve this node
       *                will be registered.
       */
      void
      registerTiles(std::vector<ColonyTileShPtr>& tiles);

    private:

      /**
       * @brief - Creates the internal data needed to represent the input area. This
       *          usually requires allocating an array allowing to represent each of
       *          the cell defined by the area.
       *          All of them will be assigned a state as provided in input and the
       *          current rules set for the node. Note that the area is assumed to
       *          be valid and is not checked. Failure to comply will cause undefined
       *          behavior.
       * @param area - the area to associate to this node.
       * @param state - the state to assign to each created cell.
       */
      void
      initialize(const utils::Boxi& area,
                 const State& state);

      /**
       * @brief - Determine whether this tree node is a leaf or not.
       * @return - `true` if this node is a leaf and `false` otherwise.
       */
      bool
      isLeaf() const noexcept;

    private:

      /**
       * @brief - The area represented by this quad tree node. The area is expressed
       *          in cells coordinate frame.
       *          Note that no matter the content of the cells, all of them can be
       *          accessed as soon as this node is created.
       */
      utils::Boxi m_area;

      /**
       * @brief - Holds a description of the set of rules to apply to make each cell
       *          represented by this node evolve.
       */
      rules::Type m_ruleset;

      /**
       * @brief - The actual data of this node. Contains the cells and their current
       *          state. The size of this vector is directly derived from the area
       *          represented by it.
       */
      std::vector<Cell> m_cells;

      /**
       * @brief - Describes the number of alive cells in this node and its children.
       *          This count is updated upon each evolution of the colony.
       */
      unsigned m_aliveCount;

      /**
       * @brief - Contains the children of this node. This vector can either be empty
       *          or be assigned a subset of children. In this case the `m_area` is
       *          an aggregation of all the areas encompassed by children nodes and
       *          the `m_cells` array should be empty.
       */
      std::vector<CellsQuadTreeNodeShPtr> m_children;
  };

}

# include "CellsQuadTreeNode.hxx"

#endif    /* CELLS_QUAD_TREE_NODE_HH */
