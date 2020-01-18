#ifndef    RENDERING_PROPERTIES_HH
# define   RENDERING_PROPERTIES_HH

# include <sdl_core/SdlWidget.hh>
# include <sdl_graphic/TextBox.hh>
# include <sdl_graphic/SelectorWidget.hh>
# include "ColorPalette.hh"

namespace cellulator {

  class RenderingProperties: public sdl::core::SdlWidget {
    public:

      /**
       * @brief - Perform the creation of the panel allowing to choose the
       *          rendering options for the colony: this include the max age
       *          of cells that can be assigned a different color and the
       *          general palette.
       * @param hint - the size hint for this widget.
       * @param parent - the parent of this widget.
       */
      RenderingProperties(const utils::Sizef& hint = utils::Sizef(),
                          sdl::core::SdlWidget* parent = nullptr);

      ~RenderingProperties() = default;

    protected:

      static
      const char*
      getGeneralTextFont() noexcept;

      static
      const char*
      getMaxAgeTextboxName() noexcept;

      static
      sdl::core::engine::Color
      getDefaultColor() noexcept;

      static
      float
      getGlobalMargins() noexcept;

      static
      float
      getComponentMargins() noexcept;

      static
      float
      getMaxAgeSelectionHeight() noexcept;

      static
      float
      getApplyButtonHeight() noexcept;

      static
      utils::Sizef
      getPaletteMaxSize() noexcept;

      static
      unsigned
      getPaletteSteps() noexcept;

      static
      unsigned
      getDefaultMaxAge() noexcept;

      /**
       * @brief - Generate a default name for the component allowing to select
       *          a color for an age of a cell. Used internally to create the
       *          selectors and retrieve them to build the rendering options.
       * @param index - the index of the palette to create.
       * @return - a default string that can be used to guarantee consistent
       *           naming.
       */
      static
      std::string
      generateNameForPalette(unsigned index) noexcept;

      /**
       * @brief - Used to create the complete palette object used to represent
       *          a color to assign to a cell. This include populating the item
       *          with relevant colors.
       *          The colors are taken from the internal map and the `index-th`
       *          color is used as the active one.
       * @param index - the index of the palette to create.
       * @return - the object representing the palette.
       */
      sdl::graphic::SelectorWidget*
      createPaletteFromIndex(unsigned index);

      /**
       * @brief - Used to build the content of this widget so that it can be
       *          readily displayed.
       */
      void
      build();

      /**
       * @brief - Used to generate the default palette to use for this component.
       */
      void
      generatePalette() noexcept;

      /**
       * @brief - Used to retrieve the palette from the specified name.
       *          The return value is guaranteed to be not `null` if the method
       *          returns. Note that the locker is assumed to already be acquired
       *          upon calling this function.
       * @param name - the name of the palette to return.
       * @return - the palette associated to the input name.
       */
      sdl::graphic::SelectorWidget*
      getPaletteFromName(const std::string& name);

      /**
       * @brief - Used to retrieve the textbox holding the maximum age for a cell
       *          to be assigned a distinct color based on the palette.
       * @return - The textbox holding the maximum age for a cell.
       */
      sdl::graphic::TextBox*
      getMaxAgeTextbox();

      /**
       * @brief - Used to interpret the signal emitted by the `Apply` button so
       *          that the `onPaletteChanged` signal can be fired. This method
       *          gathers the options from the internal control elements and use
       *          them to build a valid options object to transmit through the
       *          signal.
       * @param dummy - the name of the element which was clicked, should always
       *                be the `Apply` button so we don't check it.
       */
      void
      onApplyButtonClicked(const std::string& dummy);

    private:

      /**
       * @brief - A mutex to protect the internal properties of this widget.
       */
      mutable std::mutex m_propsLocker;

      /**
       * @brief - The list of colors in order which are assigned to the item
       *          allowing to select colors. This ordered list is used when
       *          the rendering options should be packaged to make the link
       *          between the active element in a single palette and the color
       *          that is expected to be added to the props.
       */
      std::vector<sdl::core::engine::Color> m_colors;

    public:

      /**
       * @brief - Describe a signal that is issued whenever the palette is changed.
       */
      utils::Signal<ColorPaletteShPtr> onPaletteChanged;
  };

}

# include "RenderingProperties.hxx"

#endif    /* RENDERING_PROPERTIES_HH */
