#ifndef    CELLS_QUAD_TREE_NODE_BORDER_HXX
# define   CELLS_QUAD_TREE_NODE_BORDER_HXX

# include "CellsQuadTreeNode_border.hh"

namespace cellulator {
  namespace borders {

    inline
    std::string
    getNameFromDirection(const Direction& dir) noexcept {
      switch (dir) {
        case Direction::East:
          return std::string("East");
        case Direction::North:
          return std::string("North");
        case Direction::West:
          return std::string("West");
        case Direction::South:
          return std::string("South");
        default:
          return std::string("Unknown");
      }
    }

  }

  inline
  Border
  createDirectionFromName(const borders::Name& name) noexcept {
    Border b;

    switch (name) {
      case borders::Name::NorthWest:
        b.set(borders::Direction::North);
        b.set(borders::Direction::West);
        break;
      case borders::Name::NorthEast:
        b.set(borders::Direction::North);
        b.set(borders::Direction::East);
        break;
      case borders::Name::SouthWest:
        b.set(borders::Direction::South);
        b.set(borders::Direction::West);
        break;
      case borders::Name::SouthEast:
        b.set(borders::Direction::South);
        b.set(borders::Direction::East);
        break;
      case borders::Name::None:
      default:
        // Unhandled name, keep the default border directions.
        break;
    }

    return b;
  }

}

namespace utils {

  template <>
  inline
  std::string
  getNameForKey(const cellulator::borders::Direction& dir) {
    return cellulator::borders::getNameFromDirection(dir);
  }

}

#endif    /* CELLS_QUAD_TREE_NODE_BORDER_HXX */
