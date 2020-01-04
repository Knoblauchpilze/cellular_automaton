#ifndef    COLONY_RENDERER_HH
# define   COLONY_RENDERER_HH

# include <mutex>
# include <maths_utils/Size.hh>
# include <sdl_core/SdlWidget.hh>
# include <core_utils/Signal.hh>
# include <sdl_graphic/ScrollableWidget.hh>
# include <sdl_engine/Brush.hh>
# include "Colony.hh"
# include "ColonyScheduler.hh"

namespace cellulator {

  class ColonyRenderer: public sdl::graphic::ScrollableWidget {
    public:

      /**
       * @brief - Creates a colony renderer with the specified size hint and
       *          parent widget. This widget is scrollable (meaning the user
       *          can scroll its content).
       *          The input colony is attached to the renderer and used for
       *          all the display.
       * @param colony - the colony to attach to this renderer.
       * @param hint - the size hint for this widget.
       * @param parent - the parent widget for this element.
       */
      ColonyRenderer(ColonyShPtr colony,
                     const utils::Sizef& sizeHint = utils::Sizef(),
                     sdl::core::SdlWidget* parent = nullptr);

      ~ColonyRenderer();

      /**
       * @brief - Used to request the viewing area to fit the content. This action is
       *          usually interesting for colonies not too large, otherwise the size
       *          of the individual cells can become too small. This method only uses
       *          the total size of the colony as a basis for the rendering window.
       *          Note that the aspect ratio of the cells is kept which means that one
       *          of the dimensions may display areas that do not belong to the colony.
       * @param dummy - the name of the component which requestes to fit the display
       *                to the content. Should not be used.
       */
      void
      fitToContent(const std::string& dummy);

      /**
       * @brief - Used to request the simulation to start immediately. This will
       *          fire a request to start simulating the colony and make cells
       *          live and die until a call to the `stop` method is issued.
       *          Note that nothing happens if the simulation is already started.
       */
      void
      start();

      /**
       * @brief - Used to request the simulation to stop immediately. Unlike the
       *          `start` method it will attemp to stop the simulation.
       *          Note that nothing happens if the simulation has not started yet.
       */
      void
      stop();

      /**
       * @brief - Used to request the simulation to simulate a single step and to
       *          display the results. Nothing happens if the simulation is already
       *          running (because the next step is obviously already requested).
       */
      void
      nextStep();

      /**
       * @brief - Used to request the colony to be generated randomly. Note that
       *          in case the simulation is started, nothing will happen. The user
       *          should first `stop` the simulation before trying to generate a
       *          new colony.
       * @param dummy - the name of the component which requested to generate a
       *                new colony. Should not be used.
       */
      void
      generate(const std::string& dummy);

    protected:

      /**
       * @brief - Specialziation of the parent method in order to perform the
       *          scrolling on this object. What we want is to move the real
       *          world area associated to the visual representation so that
       *          we get another perspective on the fractal.
       *          The interface is similar to what is expected by the parent
       *          class (see `ScrollableWidget` for more details).
       * @param posToFix - the position in local coordinate frame corresponding
       *                   to the position that should be fixed during the
       *                   scroll operation. Note that this corresponds to a
       *                   `cell` coordinate.
       * @param whereTo - the new position of the `posToFix`. Corresponds to
       *                  the `posToFix` where the `motion` has been applied.
       *                  Note that this corresponds to a `cell` coordinate.
       * @param motion - the motion to apply in cells coordinates.
       * @param notify - indication as to this method should emit some signals
       *                 like `onHorizontalAxisChanged`.
       * @return - `true` if the actual rendering area has been changed, and
       *           `false` otherwise.
       */
      bool
      handleContentScrolling(const utils::Vector2f& posToFix,
                             const utils::Vector2f& whereTo,
                             const utils::Vector2f& motion,
                             bool notify = true) override;

      /**
       * @brief - Reimplementation of the base class method to detect whenever the
       *          reset key is pressed, allowing to scroll the rendering area on
       *          the colony and thus display non visible parts yet.
       * @param e - the event to be interpreted.
       * @return - `true` if the event was recognized and `false` otherwise.
       */
      bool
      keyPressEvent(const sdl::core::engine::KeyEvent& e) override;

      /**
       * @brief - Reimplementation of the base class method to detect whenever the
       *          mouse moves inside the widget. This allows to provide notification
       *          to external listeners by converting the position into a real world
       *          coordinates.
       * @param e - the event to be interpreted.
       * @return - `true` if the event was recognized and `false` otherwise.
       */
      bool
      mouseMoveEvent(const sdl::core::engine::MouseEvent& e) override;

      /**
       * @brief - Reimplementation of the base class method to detect when the wheel
       *          is used: this should trigger the zooming behavior based on the factor
       *          defined for this renderer.
       * @param e - the event to be interpreted.
       * @return - `true` if the event was recognized and `false` otherwise.
       */
      bool
      mouseWheelEvent(const sdl::core::engine::MouseEvent& e) override;

      /**
       * @brief - Reimplementation of the base class method to handle the repaint
       *          of the texture representing the colony and its display.
       */
      void
      drawContentPrivate(const utils::Uuid& uuid,
                         const utils::Boxf& area) override;

    private:

      /**
       * @brief - Used to retrieve the default factor to use when zooming in.
       * @return - a factor suitable for zooming in operations.
       */
      static
      float
      getDefaultZoomInFactor() noexcept;

      /**
       * @brief - Used to retrieve the default factor to use when zooming out.
       * @return - a factor suitable for zooming out operations.
       */
      static
      float
      getDefaultZoomOutFactor() noexcept;

      /**
       * @brief - Used to retrieve the number of *pixel(s)* corresponding to a press
       *          of an arrow key. The large this value the more a single key stroke
       *          will shift the rendering area.
       *          Note that the value is expressed in pixels so that it stays relevant
       *          no matter the zoom level.
       * @return - a value indicating the number of pixels moved when an arrow key is
       *           pressed.
       */
      static
      float
      getArrowKeyMotion() noexcept;

      /**
       * @brief - Return a default viewing area for the colony. This value may or may
       *          not be suited to view very small or very large colony but given that
       *          one can zoom in or out it is not much of a problem.
       * @return - a default rendering area suited to see part of the colony displayed
       *           by this widget.
       */
      static
      utils::Boxf
      getDefaultRenderingArea() noexcept;

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

      /**
       * @brief - Computes the dimensions of a cell in terms of pixels. Note that this
       *          method assumes that the locker is acquired and *recomputes* the value
       *          at each call so use wisely and perform caching if needed.
       * @return - the dimensions of a single cell in pixels.
       */
      utils::Sizef
      getCellsDims();

      /**
       * @brief - Used to convert the input position expressed in a coordinate frame
       *          specified by the `global` boolean into a position expressed in real
       *          world coordinate. This will produce the return value in cells coord
       *          frame.
       *          Note that this method assumes that the locker is already acquired.
       * @param pos - the position to convert.
       * @param global - `true` if the position is expressed in global coordinate frame
       *                 and `false` if it expressed in local coordinate frame.
       * @return - the corresponding position in real world coordinate frame.
       */
      utils::Vector2f
      convertPosToRealWorld(const utils::Vector2f& pos,
                            bool global);

      /**
       * @brief - Internal slot used to handle the notification whenever a new generation
       *          has been computed by the colony. Used to both dispatch the information
       *          and also perform a repaint of the content.
       * @param generation - the index of the generation which was completed.
       * @param liveCells - the number of live cells in this generation.
       */
      void
      handleGenerationComputed(unsigned generation,
                               unsigned liveCells);

      /**
       * @brief - Used to create a brush representing the input cells given that it should
       *          represent the input `area`. This takes into account the size of the canvas
       *          for this object along with the number of cells visible.
       * @param cells - the cells data to use to create the visual representation.
       * @param area - the area represented by the cells.
       * @return - the brush representing the input cells given the cells' size for this item.
       */
      sdl::core::engine::BrushShPtr
      createBrushFromCells(const std::vector<State>& cells,
                           const utils::Boxi& area);

      /**
       * @brief - Perform a zoom which keeps the `center` at the specified location and
       *          with the specified factor. Note that theoretically the center could be
       *          outside of the visible range.
       *          Note that the center is expressed in local coordinate frame and *not*
       *          in cells coordinate frame. Assumes that the locker is already acquired.
       *          The new area is directly assigned to the internal `m_settings` prop.
       * @param center - the point to fix when performing the zoom.
       * @param factor - a measure of the ratio between the new size and the current size
       *                 of the rendering area.
       */
      void
      zoom(const utils::Vector2f& center,
           float factor = 2.0f);

      /**
       * @brief - Used to perform the notification of the cell's data pointed at assuming
       *          the mouse is at `pos` coordinates. The position can either be expressed
       *          in local or global coordinate frame as specified by the `global` input
       *          boolren.
       *          This method fetches the cell's data and notify the listeners through the
       *          dedicated signals.
       *          Note that we assume that the lock for this object is already acquired.
       * @param pos - the position pointed to by the mouse.
       * @param global - `true` if the position is expressed in global coordinate frame and
       *                 `false` if it is expresssed in local coordinate frame.
       */
      void
      notifyCoordinatePointedTo(const utils::Vector2f& pos,
                                bool global);

    private:

      /**
       * @brief - Describes the current display area for the colony. Helps to determine
       *          whether the events generated by the internal colony should trigger a
       *          repaint event or not.
       */
      struct RenderingWindow {
        utils::Boxf area;
      };

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
       * @brief - Used to represent the display settings of the colony. This indicates the
       *          area of the colony to repaint so that we can adapt the size of the cells
       *          and some other info based on the content to display.
       */
      RenderingWindow m_settings;

      /**
       * @brief - This value indicates whether the `m_tex` identifier is still valid or not.
       *          Each time a part of the cell is rendered this value is set to `true`
       *          indicating that the texture representing the colony needs to be updated.
       */
      bool m_colonyRendered;

      /**
       * @brief - The colony displayed in the renderer. Note that we still keep a reference
       *          on the colony itself to be able to access behaviors which are not related
       *          to the evolution of the colony.
       */
      ColonySchedulerShPtr m_scheduler;

      /**
       * @brief - The colony itself.
       */
      ColonyShPtr m_colony;

      /**
       * @brief - Contains the index of the signal registered on the colony to be notified
       *          when a new generation has been computed. In case we want to change the
       *          internal colony this value can be used to safely disconnect from the signal
       *          and be ready to attach a new one if needed.
       */
      int m_generationComputedSignalID;

      /**
       * @brief - Used to keep track internally of the last known position of the mouse inside
       *          this widget. Note that we have to use this in association with the base class
       *          method `isMouseInside` as we don't know using this value only whether the
       *          mouse is *still* inside the widget.
       *          The main reason of this attribute is to be able to update the cell's data to
       *          display when the user scroll the colony using the keyboard. In this case we
       *          want to pass on the information about the mouse position to the scrolling
       *          method which will handle the notification of external listeners.
       *          This position is saved in global coordinate frame.
       */
      utils::Vector2f m_lastKnownMousePos;

    public:

      /**
       * @brief - Signal emitted whenever a new generation has been computed by the renderer.
       *          This is useful for listeners which would like to keep up with the current
       *          generation of cells displayed on screen.
       */
      utils::Signal<unsigned> onGenerationComputed;

      /**
       * @brief - Signal emitted whenever the number of alive cells is modified in the colony
       *          displayed by the renderer. This is useful to listeners which would like to
       *          print this information.
       */
      utils::Signal<unsigned> onAliveCellsChanged;

      /**
       * @brief - Signal emitted whenever the coordinates of the point located under the mouse
       *          is changed. This is usually to keep track of said position (using a label for
       *          example).
       *          Along with the position this signal also indicate the age of the cell at the
       *          specified coordinate.
       */
      utils::Signal<utils::Vector2i, int> onCoordChanged;
  };

}

# include "ColonyRenderer.hxx"

#endif    /* COLONY_RENDERER_HH */
