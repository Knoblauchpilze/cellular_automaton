#ifndef    COLONY_TILE_HH
# define   COLONY_TILE_HH

# include <memory>
# include <vector>
# include <maths_utils/Box.hh>
# include <core_utils/AsynchronousJob.hh>
# include "CellsBlocks.hh"

namespace cellulator {

  class ColonyTile: public utils::AsynchronousJob {
    public:

      /**
       * @brief - Creates a new computation tile with the block index. This
       *          class is merely a wrapper to allow the parallel computation
       *          of several blocks. We don't actually want to do anything we
       *          just use it as a convenient wrapper to handle scheduling of
       *          blocks through the `CellsBlocks` interface.
       * @param blockID - the block attached to this colony tile: it will be
       *                  scheduled for evolution when this tile is executed.
       * @param cells - the cells data from which we can make sense of the
       *                `blockID`.
       */
      ColonyTile(unsigned blockID,
                 CellsBlocks* cells);

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
       * @brief - The index of the block attached to this tile: will be scheduled
       *          for evolution when this tile is executed. We don't actually have
       *          to know to which area this block is related, this is all managed
       *          internally by the `CellsBlocks` data.
       */
      unsigned m_blockID;

      /**
       * @brief - The data containing the cells to evolve.
       */
      CellsBlocks* m_data;
  };

  using ColonyTileShPtr = std::shared_ptr<ColonyTile>;
}

# include "ColonyTile.hxx"

#endif    /* COLONY_TILE_HH */
