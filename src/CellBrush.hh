#ifndef    CELL_BRUSH_HH
# define   CELL_BRUSH_HH

# include <memory>
# include <core_utils/CoreObject.hh>
# include "CellsBlocks.hh"

namespace cellulator {

  // Forward declaration of a brush to be able to use `CellBrushShPtr`
  // right away.
  class CellBrush;
  using CellBrushShPtr = std::shared_ptr<CellBrush>;

  // Forward declaration of a cell state as this file is also needed
  // in the `CellsBlocks` class where the `State` is defined.
  enum class State;

  class CellBrush: public utils::CoreObject {
    public:

      /**
       * @brief - Create a brush and load the corresponding data in the specified
       *          file. The file should be a text file describing the live and dead
       *          cells existing in the configuration of the brush.
       *          The dimensions should be specified at the beginning of the file.
       * @param file - the name of the file from which the brush should be loaded.
       * @param invertY - `true` if the `y` axis should be inverted compared to what
       *                  is described in the file.
       */
      CellBrush(const std::string& file,
                bool invertY);

      /**
       * @brief - Create a brush with the specified size filled with cells having a
       *          state equivalent to the input argument.
       * @param size - the size of the brush.
       * @param state - the state of the cells composing the brush.
       */
      CellBrush(const utils::Sizei& size,
                const State& state);

      /**
       * @brief - Used to create a brush pointer from the specified file name. This
       *          factory method allows to abstract away the precise way a brush is
       *          created from a file.
       * @param file - the name of the file describing the data for the brush.
       * @return - a pointer to the created brush.
       */
      static
      CellBrushShPtr
      fromFile(const std::string& file);

      /**
       * @brief - Determine whether this brush is valid. We consider that the brush
       *          is valid if its associated size is not empty and if its internal
       *          data has the expected dimensions to represent all the area of the
       *          size.
       * @return - `true` if the brush is valid and `false` otherwise.
       */
      bool
      valid() const noexcept;

      /**
       * @brief - Retrieve the size associated to this brush. This either represent
       *          the provided size at construction or the size extracted from the
       *          file describing the brush.
       *          This method helps to determine which queries to `getStateAt` is
       *          susceptible to return a non `Dead` value.
       *          Note that if the `valid` returns `false` this size will be empty.
       * @return - the size associated to this brush.
       */
      utils::Sizei
      getSize() const noexcept;

      /**
       * @brief - Retrieve the state of the cell at the specified coordinate for
       *          this brush. The coordinates are expected to be expressed in the
       *          local coordinate frame where `[0; 0]` refers to the bottom left
       *          corner of the brush and `[getSize().w(); getSize().h()]` refers
       *          to the top right corner.
       *          If the brush is not valid, any return value will be `Dead`. Any
       *          input coordinate outside of the area covered by a valid brush
       *          will return `Dead`.
       * @param coord - the coordinate of the cell whose state should be retrieved.
       * @return - the state of said coordinate or `Dead` if such a cell does not
       *           exist (or if the brush is not valid).
       */
      State
      getStateAt(const utils::Vector2i& coord) const noexcept;

      /**
       * @brief - Similar method to `getStateAt` but takes two integer coordinates
       *          rather than a single vector.
       * @param x - the coordinate along the `x` axis to retrieve.
       * @param y - the coordinate along the `y` axis to retrieve.
       * @return - the state of said coordinate or `Dead` if such a cell does not
       *           exist (or if the brush is not valid).
       */
      State
      getStateAt(int x,
                 int y) const noexcept;

    private:

      /**
       * @brief - Return the value of the character to use in a brush file to
       *          represent a `Dead` cell.
       * @return - a character representing a `Dead` cell.
       */
      static
      constexpr char
      getDeadCellCharacter() noexcept;

      /**
       * @brief - Similar to `getDeadCellCharacter` but for live cells.
       * @return - a character representing a `Live` cell.
       */
      static
      constexpr char
      getLiveCellCharacter() noexcept;

      /**
       * @brief - Default constructor creating a one by one `Alive` cell. This is
       *          used as a default initializer for other constructors but is only
       *          allowed to be called internally.
       * @param name - the name of the brush.
       */
      CellBrush(const std::string& name = std::string("default"));

      /**
       * @brief - Return the monotonic status of this brush. Should be used before
       *          any usage of the `m_monotonicState` variables.
       * @return - `true` if the brush is monotonic and `false` otherwise.
       */
      bool
      isMonotonic() const noexcept;

      /**
       * @brief - Used to populate the internal data from the file in argument. This
       *          will reset the monotonic status of the brush and try to interpret
       *          the data contained in the file to build the brush. In case the file
       *          does not describe valid data, the initial state of the brush is not
       *          destroyed (in fact it is preserved until the last moment).
       * @param file - the name of the file from which the data for this brush should
       *               be fetched.
       * @param invertY - `true` if the data contianed in the file should be registered
       *                  upside-down in the internal array (effectively reversing the
       *                  `y` axis).
       */
      void
      loadFromFile(const std::string& file,
                   bool invertY);

    private:

      /**
       * @brief - The size of this brush. This is either provided at construction or
       *          extracted from the file describing the brush.
       */
      utils::Sizei m_size;

      /**
       * @brief - This value is `true` if the brush is composed of a single state for
       *          all its size. This is a small optimization for large brushes only
       *          composed of a single cell's state (like the standard brush or the
       *          eraser).
       *          If this value is `true` the `m_monotonicState` can be used. It should
       *          be ignored otherwise.
       */
      bool m_monotonic;

      /**
       * @brief - A state which represent the entirety of the cells described by the
       *          `m_size`. This allows for optimizations in the case of very large
       *          brushes describing a single cell (like the standard or eraser brush).
       *          This value should not be used unless `m_monotonic` is `true`.
       */
      State m_monotonicState;

      /**
       * @brief - The vector containing the individual cells for the brush. This array
       *          contains data from bottom left to top right and should have a size of
       *          `m_size.area`. If this is not the case the brush is not considered to
       *          be valid. That is except it is monotonic.
       *          This array is populated in the case of brushes created from a data
       *          file.
       */
      std::vector<State> m_data;
  };

}

# include "CellBrush.hxx"

#endif    /* CELL_BRUSH_HH */
