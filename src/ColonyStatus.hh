#ifndef    COLONY_STATUS_HH
# define   COLONY_STATUS_HH

# include <maths_utils/Size.hh>
# include <sdl_core/SdlWidget.hh>
# include <sdl_engine/Color.hh>
# include <sdl_graphic/Button.hh>

namespace cellulator {

  class ColonyStatus: public sdl::core::SdlWidget {
    public:

      /**
       * @brief - Used to create a colony status widget allowing to display
       *          information about the current state of the colony being
       *          rendered in the application or general information about
       *          the cells being simulated.
       * @param hint - the size hint for this widget.
       * @param parent - the parent of this widget.
       */
      ColonyStatus(const utils::Sizef& hint = utils::Sizef(),
                      sdl::core::SdlWidget* parent = nullptr);

      ~ColonyStatus() = default;

      /**
       * @brief - Used to retrive the fit to content button associated to this
       *          status. This method is only meant as a way to connect elements
       *          to the `onClick` signal of this button, lacking of a better way.
       *          The method may raise an error in case the start simulation
       *          button is not defined.
       * @return - a reference to the fit to content button  associated to this
       *           status.
       */
      sdl::graphic::Button&
      getFitToContentButton();

      /**
       * @brief - Used to retrive the start simulation button associated to this
       *          status. This method is only meant as a way to connect elements
       *          to the `onClick` signal of this button, lacking of a better way.
       *          The method may raise an error in case the start simulation
       *          button is not defined.
       * @return - a reference to the start simulation button associated to this
       *           status.
       */
      sdl::graphic::Button&
      getStartSimilationButton();

      /**
       * @brief - Similar to the `getStartSimulationButton` but used to retrieve
       *          the stop simulation button.
       * @return - a reference to the stop simulation button.
       */
      sdl::graphic::Button&
      getStopSimilationButton();

      /**
       * @brief - Similar to the `getStartSimulationButton` but used to retrieve
       *          the next step button.
       * @return - a reference to the next step button.
       */
      sdl::graphic::Button&
      getNextStepButton();

      /**
       * @brief - Similar to the `getStartSimulationButton` but used to retrieve
       *          the generate random colony button.
       * @return - a reference to the generate colony button.
       */
      sdl::graphic::Button&
      getGenerateColonyButton();

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
       * @brief - Used to define the margins of the layout applied around the whole
       *          widget. Usually `0`.
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
       * @brief - Used to retrieve the default name for the fit to content button.
       * @return - a string that should be used to provide consistent naming for
       *           the fit to content button.
       */
      static
      const char*
      getFitToContentButtonName() noexcept;

      /**
       * @brief - Used to retrieve the default name for the start simulation button.
       * @return - a string that should be used to provide consistent naming for the
       *           start simulation button.
       */
      static
      const char*
      getStartSimulationButtonName() noexcept;

      /**
       * @brief - Similar to the `getStartSimulationButtonName` but retrieves the
       *          default name to use for the button allowing to stop the simulation.
       * @return - a string that should be used to provide consistent naming for the
       *           stop simulation button.
       */
      static
      const char*
      getStopSimulationButtonName() noexcept;

      /**
       * @brief - Used to retrieve a button allowing to simulate a single step of the
       *          colony. This name can be used for consistent naming of this button
       *          across the API.
       * @return - a string that should be used to provide consistent naming for the
       *           next step button.
       */
      static
      const char*
      getNextStepButtonName() noexcept;

      /**
       * @brief - Used to retrieve the default name for the random generation
       *          button.
       * @return - a string that should be used to provide consistent naming
       *           for the button to create a random colony.
       */
      static
      const char*
      getRandomGenerationButtonName() noexcept;

      /**
       * @brief - Retrieve a font name that can be used for buttons in this widget.
       * @return - a string representing the font to use for labels of buttons in
       *           this widget.
       */
      static
      const char*
      getButtonFontName() noexcept;

      /**
       * @brief - Retrieves the maximum size for the simulation control buttons. As we
       *          only want the buttons to be large enough in order for the icon to be
       *          visible. Growing them more would not bring anything.
       * @return - a value describing the maximum size for the simulation button.
       */
      static
      utils::Sizef
      getSimulationButtonMaxSize() noexcept;

      /**
       * @brief - Used to build the content of this widget so that it can be
       *          readily displayed.
       */
      void
      build();

    private:

      /**
       * @brief - A mutex to protect the internal properties of this widget.
       */
      mutable std::mutex m_propsLocker;
  };

}

# include "ColonyStatus.hxx"

#endif    /* COLONY_STATUS_HH */
