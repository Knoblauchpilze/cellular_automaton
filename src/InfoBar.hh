#ifndef    INFO_BAR_HH
# define   INFO_BAR_HH

# include <sdl_core/SdlWidget.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Vector2.hh>
# include <sdl_graphic/LabelWidget.hh>
# include <sdl_graphic/Button.hh>

namespace cellulator {

  class InfoBar: public sdl::core::SdlWidget {
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
      InfoBar(const utils::Sizef& hint = utils::Sizef(),
              sdl::core::SdlWidget* parent = nullptr);

      ~InfoBar() = default;

      /**
       * @brief - Used to receive notification about a generation being computed. This
       *          will be used to update the related label.
       * @param generation - the new generation that has just been computed.
       */
      void
      onGenerationComputed(unsigned generation);

      /**
       * @brief - Used to receive notification when the cell pointed at by the mouse is
       *          changed so that we can update the related label.
       *          The selected cell is provided along its age (which usually changes from
       *          cell to another). Note that in case the `age` is negative it means that
       *          there is no cell at the specified coordinates. It is up to this object
       *          to determine the best way to display this.
       * @param coords - the coordinates of the cell which is now pointed at by the mouse.
       * @param age - the age of the cell that is now selected (i.e. the one at `coords`).
       */
      void
      onSelectedCellChanged(utils::Vector2i coords,
                            int age);

      /**
       * @brief - Used to receive notifications about the number of alive cells in the
       *          colony currently displayed. This provide an indication of the `health`
       *          of the colony.
       * @param count - the number of alive cells in the colony at the moment.
       */
      void
      onAliveCellsChanged(unsigned count);

      /**
       * @brief - Used to retrieve the grid display button registered in this info bar. This
       *          method is only meant as a way to connect elements to the `onClick` signal
       *          of this button, lacking of a better way.
       *          The method may raise an error in case the grid display button cannot be
       *          found.
       * @return - a reference to the grid display button associated to this bar.
       */
      sdl::graphic::Button&
      getDisplayGridButton();

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
       * @brief - Used to retrieve the default name to associate to the button
       *          controlling whether the grid is displayed.
       * @return - a string that should be used to provide consistent naming for
       *           the grid display button.
       */
      static
      const char*
      getDisplayGridButtonName() noexcept;

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

# include "InfoBar.hxx"

#endif    /* INFO_BAR_HH */
