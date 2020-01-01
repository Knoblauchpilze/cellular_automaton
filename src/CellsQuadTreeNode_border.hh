#ifndef    CELLS_QUAD_TREE_NODE_BORDER_HH
# define   CELLS_QUAD_TREE_NODE_BORDER_HH

# include <string>
# include <core_utils/CoreFlag.hh>

namespace cellulator {
  namespace borders {

    /**
     * @brief - Types of border direction.
     */
    enum class Direction {
      East        = 0,
      North       = 1,
      West        = 2,
      South       = 3,
      ValuesCount = 4
    };

    /**
     * @brief - Retrieves a human readable name from the input focus type.
     * @param dir - a flag containing several modifiers for which a name
     *              should be provided.
     * @return - a string describing the names of the border direction that
     *           is registered in the input argument.
     */
    std::string
    getNameFromDirection(const Direction& dir) noexcept;

    /**
     * @brief - Convenience enumeration allowing to create complex border
     *          direction including two directions at once.
     */
    enum class Name {
      NorthWest,
      NorthEast,
      SouthWest,
      SouthEast,
      None
    };

  }

  using Border = utils::CoreFlag<borders::Direction>;

  /**
   * @brief - Used to create the border direction's flag from the input
   *          combination. This only combines the two possible direction
   *          described by the input name.
   * @param name - the name describing the border direction.
   * @return - a flag with all border directions activated.
   */
  Border
  createDirectionFromName(const borders::Name& name) noexcept;

}

# include "CellsQuadTreeNode_border.hxx"

#endif    /* CELLS_QUAD_TREE_NODE_BORDER_HH */
