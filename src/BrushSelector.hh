#ifndef    BRUSH_SELECTOR_HH
# define   BRUSH_SELECTOR_HH

# include <mutex>
# include <unordered_map>
# include <sdl_core/SdlWidget.hh>
# include <sdl_graphic/Button.hh>
# include <core_utils/Signal.hh>
# include <sdl_graphic/Slider.hh>
# include "CellBrush.hh"

namespace cellulator {

  class BrushSelector: public sdl::core::SdlWidget {
    public:

      /**
       * @brief - Perform the creation of a panel that allows to select and
       *          apply a brush to add some cells to the colony.
       *          The selection itself is done through toggle buttons.
       * @param hint - the size hint for this widget.
       * @param parent - the parent of this widget.
       */
      BrushSelector(const utils::Sizef& hint = utils::Sizef(),
                    sdl::core::SdlWidget* parent = nullptr);

      ~BrushSelector() = default;

    protected:

      static
      const char*
      getGeneralTextFont() noexcept;

      static
      float
      getGlobalMargins() noexcept;

      static
      float
      getComponentMargins() noexcept;

      static
      float
      getBrushSelectMaxHeight() noexcept;

      static
      sdl::core::engine::Color
      getDefaultColor() noexcept;

      static
      const char*
      getBrushSizeSliderName() noexcept;

      /**
       * @brief - Used to build the content of this widget so that it can be
       *          readily displayed.
       */
      void
      build();

      sdl::graphic::Button*
      getBrushButtonFromName(const std::string& name);

      sdl::graphic::Slider&
      getBrushSizeSlize();

      /**
       * @brief - Create a brush from the input name. The name will be scanned
       *          against matching patterns to allow the creation of the brush
       *          either from a file or from any other mean necessary.
       * @param name - the name of the brush to create.
       * @param size - the size of the brush. Note that this parameter is used
       *               mostly for monotonic brushes and not ones loaded from
       *               a file (as the file obviously describes everything).
       * @return - the created brush or `null` if the brush cannot be created.
       */
      CellBrushShPtr
      createBrushFromName(const std::string& name,
                          const utils::Sizei& size);

      /**
       * @brief - Used to create the button representing the input brush. The
       *          button will display the name of the brush and the specified
       *          icon. The button will be a toggle button with a default font
       *          and color.
       * @param name - the name of the brush (which will be used as the text
       *               of the button).
       * @param icon - the icon to use for this button.
       * @return - a pointer to the created button. Note that this value is
       *           guaranteed to be *not* `null` in case the method returns.
       */
      sdl::graphic::Button*
      createButtonFromBrushName(const std::string& name,
                                const std::string& icon);

      /**
       * @brief - Used to interpret the signal emitted by any of the brush
       *          toggle buttons to reset the other buttons but also to send
       *          a signal indicating to external listeners that the active
       *          brush has been changed.
       * @param brushName - the name of the brush which has been selected from
       *                    the panel. Note that based on the `toggled` button
       *                    it might indicate that the brush is no longer active.
       * @param toggled - `true` if the button which sent the signal is now
       *                  toggled and `false` otherwise.
       */
      void
      onBrushSelected(std::string brushName,
                      bool toggled);

      /**
       * @brief - Local slot connected to the slider's value changed signal.
       *          This will trigger a new signal emitted to propagate the new
       *          brush.
       * @param size - the new brush' size.
       */
      void
      onBrushSizeChanged(float size);

      /**
       * @brief - Used internally to perform the creation and update of the internal
       *          properties to notify that the new active brush is described by the
       *          input attributes.
       * @param brushName - the name of the brush that should be made active.
       * @param brushSize - the size of the brush to create.
       */
      void
      notifyBrushChanged(const std::string& brushName,
                         int brushSize);

    private:

      /**
       * @brief - Convenience typedef to refer to the map of association between
       *          a brush and its button name.
       */
      using BrushesTable = std::unordered_map<std::string, std::string>;

      /**
       * @brief - Convenience structure holding the properties to describe a brush.
       */
      struct BrushDesc {
        bool valid;
        std::string name;
        int dim;
      };

      /**
       * @brief - Protects this object from concurrent accesses.
       */
      std::mutex m_propsLocker;

      /**
       * @brief - A list of the brushes registered in this selector. This array
       *          allows to keep track of all the brushes that we handle and thus
       *          untoggle the current active one when a new one is activated.
       *          This map makes the link between the button's name and the brush
       *          name.
       */
      BrushesTable m_brushes;

      /**
       * @brief - Information about the last brush notified through the local signal.
       */
      BrushDesc m_currentBrush;

    public:

      /**
       * @brief - Describe a signal that is issued whenever the active brush
       *          is changed. This describes the brush to be used.
       */
      utils::Signal<CellBrushShPtr> onBrushChanged;
  };

}

# include "BrushSelector.hxx"

#endif    /* BRUSH_SELECTOR_HH */
