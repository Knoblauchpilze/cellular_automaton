#ifndef    STATUS_BAR_HH
# define   STATUS_BAR_HH

# include <sdl_core/SdlWidget.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Vector2.hh>
# include <sdl_graphic/LabelWidget.hh>

namespace cellulator {

  class StatusBar: public sdl::core::SdlWidget {
    public:

      /**
       * @brief - Used to create a status bar widget allowing to display the mouse
       *          coordinates using cells semantic and an indication of the current
       *          number of alive cells, etc.
       *          This is helpful to provide more context to the user about what is
       *          currently displayed in the renderer.
       * @param hint - the size hint for this widget.
       * @param parent - the parent of this widget.
       */
      StatusBar(const utils::Sizef& hint = utils::Sizef(),
                sdl::core::SdlWidget* parent = nullptr);

      ~StatusBar() = default;

    protected:

      /**
       * @brief - Retrieves the maximum height for this component. This is usually
       *          assigned at construction and considered enough to display all the
       *          info needed.
       * @return - a value describing the maximum height of this component.
       */
      static
      float
      getStatusMaxHeight() noexcept;

      /**
       * @brief - Defines the font to use for the labels displayed in this status
       *          bar. THis includes both the current mouse position and the area
       *          defined for rendering.
       * @return - a name identifying the font to use for the information panels.
       */
      static
      const char*
      getInfoLabelFont() noexcept;

      /**
       * @brief - Used to define the margins of the layout applied around the whole
       *          widget.
       * @return - a value representing the global margins for this widget.
       */
      static
      float
      getGlobalMargins() noexcept;

      /**
       * @brief - Used to define the margins between the component of this status.
       * @return - a value representing the margins between each component of the
       *           widget.
       */
      static
      float
      getComponentMargins() noexcept;

      /**
       * @brief - Used to retrieve the default name for the label displaying mouse
       *          coordinates in real world frame.
       * @return - a string that should be used to provide consistent naming for
       *           the mouse coordinates label.
       */
      static
      const char*
      getMouseCoordsLabelName() noexcept;

      /**
       * @brief - Used to retrieve the default name for the generation display.
       * @return - a string that should be used to provide consistent naming for
       *           the generation label.
       */
      static
      const char*
      getGenerationLabelName() noexcept;

      /**
       * @brief - Used to retrieve the default name for the alive cells label.
       * @return - a string that should be used to provide consistent naming for
       *           the alive cells label.
       */
      static
      const char*
      getAliveCellsLabelName() noexcept;

      /**
       * @brief - Used to build the content of this widget so that it can be
       *          readily displayed.
       */
      void
      build();

      /**
       * @brief - Used to retrieve the label widget holding the mouse coordinates
       *          in real world frame. Typically used when the coordinates need to
       *          be changed/updated.
       *          The return value is guaranteed to be not `null` if the method
       *          returns. Note that the locker is assumed to already be acquired
       *          upon calling this function.
       * @return - the label widget associated to the mouse coordinates.
       */
      sdl::graphic::LabelWidget*
      getMouseCoordsLabel();

      /**
       * @brief - Used to retrieve the label displaying the current generation reached
       *          by the colony. The return value is guaranteed to be not `null` if the
       *          method returns. Note that the locker is assumed to already be acquired
       *          upon calling this function.
       * @return - the label displaying generation.
       */
      sdl::graphic::LabelWidget*
      getGenerationLabel();

      /**
       * @brief - Used to retrieve the label widget holding the alive cells count.
       *          Typically used when the alive cells count should be changed. The
       *          return value is guaranteed to be not `null` if the method returns.
       *          Note that the locker is assumed to already be acquired upon calling
       *          this function.
       * @return - the label widget associated to this status.
       */
      sdl::graphic::LabelWidget*
      getAliveCellsLabel();

    private:

      /**
       * @brief - A mutex to protect the internal properties of this widget.
       */
      mutable std::mutex m_propsLocker;
  };

}

# include "StatusBar.hxx"

#endif    /* STATUS_BAR_HH */
