#ifndef    COLONY_TILE_HH
# define   COLONY_TILE_HH

# include <memory>
# include <maths_utils/Box.hh>
# include <core_utils/CoreObject.hh>
# include <core_utils/ThreadPool.hh>
# include "CellsQuadTreeNode.hh"

namespace cellulator {

  // Forward declaration to be able to use a quadtree node right away.
  class CellsQuadTreeNode;

  class ColonyTile: public utils::CoreObject, public utils::AsynchronousJob {
    public:

      /**
       * @brief - Creates a new computation tile with the associated area. It
       *          will use the provided data to perform the computation of the
       *          cells in the specified area to the next generation.
       * @param area - the rendering area for which the computations should be
       *               done.
       * @param cells - the cells data to make evolve.
       */
      ColonyTile(const utils::Boxi& area,
                 CellsQuadTreeNode* cells);

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
  };

  using ColonyTileShPtr = std::shared_ptr<ColonyTile>;
}

# include "ColonyTile.hxx"

#endif    /* COLONY_TILE_HH */
