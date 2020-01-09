#ifndef    RULESET_SELECTOR_HH
# define   RULESET_SELECTOR_HH

# include <sdl_core/SdlWidget.hh>
# include <sdl_graphic/Checkbox.hh>
# include <core_utils/Signal.hh>
# include "CellEvolver.hh"

namespace cellulator {

  class RulesetSelector: public sdl::core::SdlWidget {
    public:

      /**
       * @brief - Perform the creation of the selector allowing to choose between
       *          various rules for the cells in the colony. The main goal is to
       *          allow for the selection of the number of neighbors needed for a
       *          cell to be born and the number of neighbors allowing it not to
       *          die.
       *          The selection itself is done through checkboxes.
       * @param hint - the size hint for this widget.
       * @param parent - the parent of this widget.
       */
      RulesetSelector(const utils::Sizef& hint = utils::Sizef(),
                      sdl::core::SdlWidget* parent = nullptr);

      ~RulesetSelector() = default;

    protected:

      static
      const char*
      getGeneralTextFont() noexcept;

      static
      float
      getGlobalMargins() noexcept;

      static
      float
      getCheckboxMaxHeight() noexcept;

      static
      utils::Sizef
      getIconMaxSize() noexcept;

      /**
       * @brief - Generate a default name for the component representing the
       *          specified number. This can be used to assign a name for the
       *          checkboxes representing the individual cells count for live
       *          cells.
       * @param number - the number of cells represented by the checkbox.
       * @return - a default string that can be used to guarantee consistent
       *           naming.
       */
      static
      std::string
      generateNameForLiveNeighbors(unsigned number) noexcept;

      /**
       * @brief - Fill a similar purpose to `generateNameForLiveNeighbors` but
       *          for dead cells.
       * @param number - the number of cells represented by the checkbox.
       * @return - a default string that can be used to guarantee consistent
       *           naming.
       */
      static
      std::string
      generateNameForDeadNeighbors(unsigned number) noexcept;

      /**
       * @brief - Used to build the content of this widget so that it can be
       *          readily displayed.
       */
      void
      build();

      /**
       * @brief - Used to retrieve the checkbox with the specified name from the
       *          internal children. This can be used after all the checkboxes
       *          have been registered and yet the user needs to query info on
       *          it.
       *          The return value is guaranteed to be not `null` if the method
       *          returns. Note that the locker is assumed to already be acquired
       *          upon calling this function.
       * @param name - the name of the checkbox to return.
       * @return - the checkbox associated to the input name.
       */
      sdl::graphic::Checkbox*
      getCheckboxFromName(const std::string& name);

      /**
       * @brief - Used to interpret the signal emitted by the `Apply` button so
       *          that the `onRulesetChanged` signal can be fired. This method
       *          gathers the options from the internal checkboxes and build a
       *          valid `CellEvolver` object to transmit through the signal.
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

    public:

      /**
       * @brief - Describe a signal that is issued whenever the `Apply` button is
       *          pressed in this panel.
       */
      utils::Signal<CellEvolverShPtr> onRulesetChanged;
  };

}

# include "RulesetSelector.hxx"

#endif    /* RULESET_SELECTOR_HH */
