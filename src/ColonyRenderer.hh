#ifndef    COLONY_RENDERER_HH
# define   COLONY_RENDERER_HH

# include <mutex>
# include <maths_utils/Size.hh>
# include <sdl_core/SdlWidget.hh>
# include <sdl_graphic/ScrollableWidget.hh>

namespace cellulator {

  class ColonyRenderer: public sdl::graphic::ScrollableWidget {
    public:

      /**
       * @brief - Creates a colony renderer with the specified size hint and
       *          parent widget. This widget is scrollable (meaning the user
       *          can scroll its content).
       * @param hint - the size hint for this widget.
       * @param parent - the parent widget for this element.
       */
      ColonyRenderer(const utils::Sizef& sizeHint = utils::Sizef(),
                     sdl::core::SdlWidget* parent = nullptr);

      ~ColonyRenderer();

    protected:

      /**
       * @brief - Reimplementation of the base class method to provide update of
       *          the rendering window when a resize is requested.
       * @param window - the available size to perform the update.
       */
      void
      updatePrivate(const utils::Boxf& window) override;

      /**
       * @brief - Reimplementation of the base class method to handle the repaint
       *          of the texture representing the colony and its display.
       */
      void
      drawContentPrivate(const utils::Uuid& uuid,
                         const utils::Boxf& area) override;

    private:

      /**
       * @brief - Connect signals and build the renderer in a more general way.
       */
      void
      build();

      /**
       * @brief - Used to clear the texture associated to this colony.
       */
      void
      clearColony();

      /**
       * @brief - Used to determine whether the cells have changed since the creation
       *          of the texture representing them. If this is the case it means that
       *          the `m_tex` should be recreated.
       *          Assumes that the locker is already acquired.
       */
      bool
      colonyChanged() const noexcept;

      /**
       * @brief - Used to specify that the tiles have changed and thus that the `m_tex`
       *          texture should be recreated on the next call to `drawContentPrivate`.
       *          Assumes that the locker is already acquired.
       */
      void
      setColonyChanged() noexcept;

      /**
       * @brief - Performs the creation of the texture representing this colony from
       *          the data associated to it. Assumes that the locker is already acquired.
       */
      void
      loadColony();

    private:

      /**
       * @brief - A mutex allowing to protect this widget from concurrent accesses.
       */
      mutable std::mutex m_propsLocker;

      /**
       * @brief - The index returned by the engine for the texture representing the colony
       *          on screen. It is rendered from the cells' data computed internally and is
       *          valid as long as the `m_colonyRendered` boolean is set to `false`.
       */
      utils::Uuid m_tex;

      /**
       * @brief - This value indicates whether the `m_tex` identifier is still valid or not.
       *          Each time a part of the cell is rendered this value is set to `true`
       *          indicating that the texture representing the colony needs to be updated.
       */
      bool m_colonyRendered;
  };

}

# include "ColonyRenderer.hxx"

#endif    /* COLONY_RENDERER_HH */
