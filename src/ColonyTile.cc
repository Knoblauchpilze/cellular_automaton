
# include "ColonyTile.hh"

namespace cellulator {

  ColonyTile::ColonyTile(const utils::Boxi& area,
                         std::vector<Cell>& cells,
                         const Type& type):
    utils::AsynchronousJob(std::string("tile_") + area.toString(),
                           type == Type::Interior ? utils::Priority::Normal : utils::Priority::Low),

    m_area(area),
    m_data(cells),
    m_type(type)
  {
    setService("colony");
  }

  void
  ColonyTile::compute() {
    // Use the dedicated handler on the data.
    // TODO: Restore this.
  }

}
