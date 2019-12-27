#ifndef    CELLS_QUAD_TREE_HH
# define   CELLS_QUAD_TREE_HH

# include <memory>
# include <vector>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Box.hh>
# include "Cell.hh"
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  class CellsQuadTree: public utils::CoreObject {
    public:

      /**
       * @brief - Create a quad tree to contain cells where each node can reach
       *          the specified dimensions before being split into several nodes.
       *          This threshold should not be too small as to keep some level of
       *          performance but not too big neither.
       *          The quad tree is built with an initial size which will be set
       *          and allocated given the required cells dimensions. All the cells
       *          will be assigned a `Dead` state.
       * @param dims - the dimensions of the colony.
       * @param nodesDims - the dimensions of nodes containing cells.
       * @param ruleset - the set of rules to use to update the cells to the next
       *                  iteration.
       * @param name - the lil' name of the colony.
       */
      CellsQuadTree(const utils::Sizei& dims,
                    const utils::Sizei& nodeDims,
                    const rules::Type& ruleset,
                    const std::string& name = std::string("Daddy's lil monster"));

      ~CellsQuadTree() = default;

      /**
       * @brief - Used to retrieve the size of the quad tree as of now. This includes
       *          all the tiles generated so far even though they might not be rendered
       *          yet (or visible for that matter).
       *          The size is expressed in terms of cells.
       * @return - the size of the quad tree.
       */
      utils::Sizei
      getSize() noexcept;

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
       * @brief - Reinitialize the quadtree with the specified dimensions. All nodes will
       *          be destroyed and rebuilt with the specified size.
       * @param dims - the dimensions of the cells array to allocate.
       */
      void
      reset(const utils::Sizei& dims);

      /**
       * @brief - Used to assign random values for each cell of the colony. This assumes that
       *          the colony is effectively stopped but no checks are performed to ensure it.
       *          Note that this method assumes that the locker is already acquired.
       */
      void
      randomize();

    private:

      /**
       * @brief - Used to convert the input box from a floating point semantic to a
       *          valid integer coordinates box. This will also make sure that the
       *          returned box completely encompasses the input box so that we are
       *          sure that all the data contained in the `in` box will also be there
       *          in the output box.
       * @param in - the box to convert to integer coordinates.
       * @return - a box containing the input `in` box with integer coordinates.
       */
      utils::Boxi
      fromFPCoordinates(const utils::Boxf& in) const noexcept;

    private:

      /**
       * @brief - Protect this colony from concurrent accesses.
       */
      std::mutex m_propsLocker;

      /**
       * @brief - Holds a description of the set of rules to apply to perform
       *          the computation of a cell to its descendant when evolving the
       *          colony. Different sets of rules will most likely lead to some
       *          distinct behaviors.
       *          This property is transmitted to the cells created during the
       *          evolution of the colony.
       */
      rules::Type m_ruleset;

      /**
       * @brief - The maximum dimensions of a single node of the quad tree. This
       *          will represent a contiguous patch of cells of the specified dims
       *          and trigger the creation of new nodes if the dimensions of the
       *          colony exceeds the dimensions of a single node.
       */
      utils::Sizei m_nodesSize;

      /**
       * @brief - The total size of the colony. Note that this corresponds to the
       *          aggregation of all the nodes created so far which still contain
       *          at least a living cell.
       *          This allows for fast requesting of the dimensions of a colony.
       *          The colony is always centered at the origin, hence the size and
       *          not the box used to represent the dimensions of it.
       */
      utils::Sizei m_size;

      /**
       * @brief - Contains the top level child of the quad tree. This element can
       *          be changed in case the quad tree need to be extended to cover
       *          some other area. It is guaranteed to be valid during the life
       *          of the quad tree.
       */
      CellsQuadTreeNodeShPtr m_root;
  };

  using CellsQuadTreeShPtr = std::shared_ptr<CellsQuadTree>;
}

# include "CellsQuadTree.hxx"

#endif    /* CELLS_QUAD_TREE_HH */
