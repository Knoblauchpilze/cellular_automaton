#ifndef    COLONY_TILE_HH
# define   COLONY_TILE_HH

# include <memory>
# include <maths_utils/Box.hh>
# include <core_utils/AsynchronousJob.hh>
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  // Forward declaration to be able to use a quadtree node right away.
  class CellsQuadTreeNode;

  class ColonyTile: public utils::AsynchronousJob {
    public:

      /**
       * @brief - Describe the type of operation which should be performed by this
       *          tile.
       */
      enum class Type {
        Border,
        Interior
      };

    public:

      /**
       * @brief - Creates a new computation tile with the associated area. It
       *          will use the provided data to perform the computation of the
       *          cells in the specified area to the next generation.
       *          Based on the `type` of the tile the computation process will
       *          either call the computation methods on the interior of the
       *          `cells` or call the boundaries update. Depending on the type
       *          the priority is also not the same (typically borders jobs are
       *          assigned a low priority).
       * @param area - the rendering area for which the computations should be
       *               done.
       * @param cells - the cells data to make evolve.
       * @param type - the type of job associated to this tile.
       */
      ColonyTile(const utils::Boxi& area,
                 CellsQuadTreeNode* cells,
                 const Type& type);

      ~ColonyTile() = default;

      /**
       * @brief - Reimplementation of the interface method allowing to perform
       *          the evolution of the cells of the quadtree node attached to
       *          this tile.
       */
      void
      compute() override;

    private:

      /**
       * @brief - The rendering area for which the computations should be performed.
       */
      utils::Boxi m_area;

      /**
       * @brief - The quadtree node containing the cells to evolve.
       */
      CellsQuadTreeNode* m_data;

      /**
       * @brief - The type of this tile, which determine the processing to call on the
       *          internal `m_data` node.
       */
      Type m_type;
  };

  using ColonyTileShPtr = std::shared_ptr<ColonyTile>;
}

# include "ColonyTile.hxx"

#endif    /* COLONY_TILE_HH */
