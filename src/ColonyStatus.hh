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
       * @brief - Used to retrieve the fit to content button associated to this
       *          status. This method is only meant as a way to connect elements
       *          to the `onClick` signal of this button, lacking of a better way.
       *          The method may raise an error in case the start simulation
       *          button is not defined.
       * @return - a reference to the fit to content button associated to this
       *           status.
       */
      sdl::graphic::Button&
      getFitToContentButton();

      /**
       * @brief - Similar to the `getFitToContentButton` but used to retrieve the
       *          generate random colony button.
       * @return - a reference to the generate colony button.
       */
      sdl::graphic::Button&
      getGenerateColonyButton();

      /**
       * @brief - Used to react to the simulation state being toggled. Basically the
       *          simulation can either be running, which means that it will try to
       *          continuously update the cells until further notice or halted in
       *          which case no changes will happen.
       *          In most cases this component provides information and commands to
       *          make the state of the simulation change. But the renderer also has
       *          some internal means to control the state of the simulation. It is
       *          important that such events are notified to this object so that we
       *          can display a consistent state for ths simulation button.
       *          This method is thus expected to be called whenever some external
       *          component tempers with the state of the simulation: this allows
       *          this component to update the internal state of the simulation's
       *          started button to be consistent with it.
       * @param running - `true` if the simulation is now running.
       */
      void
      onSimulationToggled(bool running);

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
       * @brief - A suited value to use for the border size of the buttons used in
       *          this status.
       * @return - a suited value for the size of the borders for buttons.
       */
      static
      float
      getButtonBorderSize() noexcept;

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

      /**
       * @brief - Used to retrieve the start simulation button associated to this
       *          status. This method is used when a `onClick` signal is received
       *          in order to retrieve the source of the event. This allows to
       *          set the needed states if needed (typically untoggling the `Start`
       *          simulation button when the simulation is stopped.
       *          The method may raise an error in case the start simulation button
       *          is not defined.
       * @return - a reference to the start simulation button associated to this
       *           status.
       */
      sdl::graphic::Button&
      getStartSimulationButton();

      /**
       * @brief - Similar to the `getStartSimulationButton` but used to retrieve the
       *          stop simulation button.
       * @return - a reference to the stop simulation button.
       */
      sdl::graphic::Button&
      getStopSimulationButton();

      /**
       * @brief - Similar to the `getStartSimulationButton` but used to retrieve the
       *          next step button.
       * @return - a reference to the next step button.
       */
      sdl::graphic::Button&
      getNextStepButton();

      /**
       * @brief - Signal handling a click on any button of this status. We handle internal
       *          modifications (like untoggling of the `Start` button if needed) and then
       *          transmit the correct signal through the public interface.
       * @param buttonName - the name of the button which has just been pressed.
       */
      void
      onButtonClicked(const std::string& buttonName);

    private:

      /**
       * @brief - A mutex to protect the internal properties of this widget.
       */
      mutable std::mutex m_propsLocker;

      /**
       * @brief - This boolean helps to keep track of the current simulation
       *          state. Note that is only tracks the requests made so far
       *          through the `onSimulationStarted` signal but does not tell
       *          whether the relevant elements have actually started the
       *          simulation.
       */
      bool m_started;

    public:

      /**
       * @brief - Signal emitted whenver the simulation of the colony should start. This is
       *          triggered by a click on the `Start` button.
       */
      utils::Signal<> onSimulationStarted;

      /**
       * @brief - Signal emitted whenver the simulation of the colony should move one step
       *          forward. This is triggered by a click on the `Step` button.
       */
      utils::Signal<> onSimulationStepped;

      /**
       * @brief - Signal emitted whenver the simulation of the colony should stop. This is
       *          triggered by a click on the `Stop` button.
       */
      utils::Signal<> onSimulationStopped;
  };

}

# include "ColonyStatus.hxx"

#endif    /* COLONY_STATUS_HH */
