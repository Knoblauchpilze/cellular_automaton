#ifndef    CELLS_QUAD_TREE_NODE_HH
# define   CELLS_QUAD_TREE_NODE_HH

# include <memory>
# include <vector>
# include <unordered_map>
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
       * @brief - Used to retrieve the area associated to this quadtree node.
       * @return - the area associated to this quad tree node.
       */
      utils::Boxi
      getArea() const noexcept;

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
      getAliveCellsCount() const noexcept;

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

      /**
       * @brief - Used to finalize the state of the cells and to swap their `next` state
       *          with their current one. This basically applies the evolution computed
       *          during a generation.
       *          We either swap the states for the internal cells or call the adequate
       *          method on children of this node.
       */
      void
      step();

      /**
       * @brief - Perform the evolution of the cells contained in this node. Note that
       *          in case the node is not a leaf, nothing happens.
       */
      void
      evolve();

      /**
       * @brief - Perform the evolution of the boundary of this node. Note that this method
       *          handles both the update of the internal boundaries (i.e. the boundaries of
       *          the children) and the boundaries of the exterior of the node. However the
       *          latter case is only handled when the node is the root, otherwise we consider
       *          that the boundaries will be handled by the parent node.
       */
      void
      evolveBoundaries();

      /**
       * @brief - Used to expand the input quadtree node to have an area four times as large
       *          as it currently have. The expansion process will split the current data into
       *          four children and create the parent root.
       *          All internal data will be copied, including the adjacency arrays. The data
       *          itself will be divided into the four children encompassing the current area
       *          and boundaries wrappers will be allocated as well.
       *          Note that in case the provided node does not need to be expanded, it will be
       *          returned as is. This allows to entirely rely on this method to handle the
       *          logic of expansion and call it no matter the real data contained in the tree.
       * @param root - the node to expand.
       * @return - the expanded version of the input `root` node.
       */
      CellsQuadTreeNodeShPtr
      expand(CellsQuadTreeNodeShPtr root);

    private:

      /**
       * @brief - Used to identify a child based on its location in the parent area.
       */
      enum class Child {
        NorthWest,
        NorthEast,
        SouthWest,
        SouthEast,
        None
      };

      /**
       * @brief - Similar to the public constructor but allows to specify the parent node in
       *          argument. This is the only way to create a children of a quadtree node and
       *          this is a protection so that only the class itself can expand and create
       *          its own children.
       *          The node can be marked for initialization or not. This will create internal
       *          arrays containing cells and adjacency data (so usually because the node will
       *          be a leaf) or not if the node is meant to be an intermediary node.
       *          Note that an orientation should be provided to indicate which role this node
       *          has in the parent's children.
       * @param area - the area of the node.
       * @param ruleset - the set of rules to use to update the cells to
       *                  the next iteration.
       * @param parent - the parent node to this quadtree node.
       * @param orientation - the role of this child within the parent's children.
       * @param allocate - `true` if the internal arrays should be initialized and `false`
       *                   otherwise.
       */
      CellsQuadTreeNode(const utils::Boxi& area,
                        const rules::Type& ruleset,
                        CellsQuadTreeNode* parent,
                        const Child& orientation,
                        bool allocate);

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
       * @param allocate - `true` if the internal array should be allocated (i.e. the
       *                   cells and adjacency values) and `false` otherwise.
       */
      void
      initialize(const utils::Boxi& area,
                 const State& state,
                 bool allocate);

      /**
       * @brief - Return `true` if this node is a root node (i.e. with no parent).
       * @return - `true` if the node does not have a parent, `false` otherwise.
       */
      bool
      isRoot() const noexcept;

      /**
       * @brief - Determine whether this tree node is a leaf or not.
       * @return - `true` if this node is a leaf and `false` otherwise.
       */
      bool
      isLeaf() const noexcept;

      /**
       * @brief - Used to determine whether this node has at least one live cell in
       *          the current generation. This is useful to filter out really early
       *          nodes where nothing will happen.
       * @return - `true` if at least one cell is not dead in this node.
       */
      bool
      hasLiveCells() const noexcept;

      /**
       * @brief - Exact opposite to the `hasLiveCells` method. This is just to have
       *          a nice semantic for the method instead of always checking for the
       *          `!hasLiveCells()` method.
       * @return - `true` if noe of the cells are alive in this node.
       */
      bool
      isDead() const noexcept;

      /**
       * @brief - Used to colletc the boundaries leaves nodes that are children of
       *          this node's hierarchy. This include this node if it matches (i.e.
       *          if it is a leaf) or all the matching children.
       *          Note that if this node is a leaf it will be added to the input
       *          vector as we consider that the parent should not have called this
       *          method on the child if it was not at least susceptible to be a
       *          leaf.
       *          Also note that the boundary nodes for this node will be appended
       *          to the input vector.
       * @param nodes - output vector which will contain the boundary nodes that
       *                belong to the hierarchy defined by this node.
       * @param includeEmpty - if `true` will collect all the boundaries including
       *                       the ones only containing dead cells. Note that not
       *                       allocated boundaries are never included.
       */
      void
      collectBoundaries(std::vector<CellsQuadTreeNode*>& nodes,
                        bool includeEmpty) noexcept;

      /**
       * @brief - Used to update the adjacency count for the cell at `coord` given that
       *          it is itself alive or dead (based on the value of the `alive` boolean).
       *          Note that the `m_nextAdjacency` will be updated and will not be persisted
       *          to the `m_adjacency` until the `step` method is called: this only holds
       *          as long as the `makeCurrent` boolean is `false`.
       *          Note also that if the coordinates are on the boundaries, this method does
       *          not try to update the contiguous children, we assume that another process
       *          will handle it.
       * @param coord - the coordinate of the cell to update.
       * @param alive - `true` if the cell is alive and `false` otherwise.
       * @param makeCurrent - `true` if the adjacency should be updated right away (i.e. if
       *                      the `m_adjacency` array is to be modified).
       */
      void
      updateAdjacencyFor(const utils::Vector2i& coord,
                         bool alive,
                         bool makeCurrent = false);

    private:

      /**
       * @brief - Convenience define to refer to the map of children.
       */
      using ChildrenMap = std::unordered_map<Child, CellsQuadTreeNodeShPtr>;

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
       * @brief - Holds a vector representing for each cell the number of alive cells
       *          in the neighborhood. This allows to find very quickly the current
       *          state of the cell and possibly its next state.
       *          We need to keep this value up-to-date so as to benefit from it and
       *          speed up the evolution process.
       */
      std::vector<unsigned> m_adjacency;

      /**
       * @brief - Used as a temporary buffer allowing to store the adjacency for the
       *          next step of the evolution of the cells. This value is swapped with
       *          the `m_adjacency` attribute when the `step` method is called on the
       *          node.
       */
      std::vector<unsigned> m_nextAdjacency;

      /**
       * @brief - Describes the number of alive cells in this node and its children.
       *          This count is updated upon each evolution of the colony.
       */
      unsigned m_aliveCount;

      /**
       * @brief - A reference to the parent noe of this quadtree element. May be `null`
       *          in case this node is a root node.
       */
      CellsQuadTreeNode* m_parent;

      /**
       * @brief - Used to hold the orientation of this child relatively to its parent.
       *          This indicates under which key this node can be found in the parent
       *          quadtree node. If the node has no parent, the orientation is set to
       *          `None`.
       */
      Child m_orientation;

      /**
       * @brief - Contains the children of this node. This vector can either be empty
       *          or be assigned a subset of children. In this case the `m_area` is
       *          an aggregation of all the areas encompassed by children nodes and
       *          the `m_cells` array should be empty.
       */
      ChildrenMap m_children;
  };

}

# include "CellsQuadTreeNode.hxx"

#endif    /* CELLS_QUAD_TREE_NODE_HH */
