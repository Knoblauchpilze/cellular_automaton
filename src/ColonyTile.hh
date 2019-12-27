#ifndef    COLONY_TILE_HH
# define   COLONY_TILE_HH

# include <memory>
# include <maths_utils/Box.hh>
# include <core_utils/CoreObject.hh>
# include <core_utils/ThreadPool.hh>

namespace cellulator {

  class ColonyTile: public utils::CoreObject, public utils::AsynchronousJob {
    public:

      /**
       * @brief - Creates a new computation tile with the associated area. It
       *          will use the provided data to perform the computation of the
       *          cells in the specified area to the next generation.
       * @param area - the rendering area for which the computations should be
       *               done.
       */
      ColonyTile(const utils::Boxi& area);

      ~ColonyTile() = default;

    private:

      /**
       * @brief - The rendering area for which the computations should be performed.
       */
      utils::Boxi m_area;
  };

  using ColonyTileShPtr = std::shared_ptr<ColonyTile>;
}

# include "ColonyTile.hxx"

#endif    /* COLONY_TILE_HH */
