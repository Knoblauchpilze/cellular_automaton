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

      /**
       * @brief - Unlike the other constructor this tile creates a dummy job
       *          which won't perform any computations but which can be used
       *          to signal that there are no other outstanding jobs to be
       *          processed by the colony.
       *          Whoever receives such a job should stop trying to create
       *          new jobs as it means that the colony has reached a steady
       *          state where nothing will change anymore: this can either
       *          mean that all cells have died or that the colony is only
       *          composed of still life forms.
       */
      ColonyTile();

      ~ColonyTile() = default;

      /**
       * @brief - Reimplementation of the interface method allowing to perform
       *          the evolution of the cells of the quadtree node attached to
       *          this tile.
       */
      void
      compute() override;

      /**
       * @brief - Used to determine whether this job is a closure job, indicating
       *          that no more changes can be sustained in the colony.
       * @return - `true` in case there's no need to start again some processing
       *           on the colony unless cells are added.
       */
      bool
      closure();

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
