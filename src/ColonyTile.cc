
# include "ColonyTile.hh"

namespace cellulator {

  ColonyTile::ColonyTile(const utils::Boxi& area):
    utils::CoreObject(std::string("tile_") + area.toString()),
    utils::AsynchronousJob(),

    m_area(area)
  {}

}
