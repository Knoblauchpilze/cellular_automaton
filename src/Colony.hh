#ifndef    COLONY_HH
# define   COLONY_HH

# include <mutex>
# include <memory>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Vector2.hh>
# include <core_utils/ThreadPool.hh>
# include "Cell.hh"

namespace cellulator {

  class Colony: utils::CoreObject {
    public:

      /**
       * @brief - Create a colony with the specified size. All cells will be
       *          initialized to a dead state. THe user can provide a name for
       *          the colony.
       *          The dimensions will be increased by one if needed if they are
       *          not multiple of `2`. This is to guarantee some consistency in
       *          the access to the cells.
       * @param dims - the dimensions of the colony.
       * @param ruleset - the set of rules to use to update the cells at each
       *                  iteration.
       * @param name - the lil' name of the colony.
       */
      Colony(const utils::Sizei& dims,
             const rules::Type& ruleset,
             const std::string& name = std::string("Daddy's lil monster"));

      /**
       * @brief - Destruction of the colony. Stops the execution if the colony
       *          is running.
       */
      ~Colony();

      /**
       * @brief - Used to retrieve the size of the colony as of now. This includes
       *          all the tiles generated so far even though they might not be
       *          rendered yet (or visible for that matter).
       *          The size is expressed in terms of cells.
       * @return - the size of the colony.
       */
      utils::Sizei
      getSize() noexcept;

      /**
       * @brief - Used to retrieve the cells from the area described in input into
       *          the specified vector. Note that any existing data in the vector
       *          will be erased. Also the area is clamped to match the dimensions
       *          of the colony if needed. The returned area describes the actual
       *          content of the `cells` vector.
       *          The input dimensions are clamped to the lowest which means that
       *          for example if the box spans `x: 50, w: 25`, the actual cells
       *          will be `[37; 62]`.
       * @param cells - output vector where cells will be saved.
       * @param area - the area for which cells should be retrieved.
       * @return - the actual box of the cells returned in the `cells` vector.
       */
      utils::Boxi
      fetchCells(std::vector<Cell>& cells,
                 const utils::Boxf& area);

      /**
       * @brief - Attempt to start the execution of the colony. Note that if the
       *          colony is already running, nothing happens.
       */
      void
      start();

      /**
       * @brief - Used to simulate a single step of the colony's life. Nothing is
       *          done in case the colony is already running.
       */
      void
      step();

      /**
       * @brief - Attempt to stop the execution of the colony. Note that if the
       *          colony is not started, nothing happens.
       */
      void
      stop();

      /**
       * @brief - Used to generate a random coolony without modifying the dimensions
       *          of the colony. Each cell will be assigned a random state among the
       *          available ones.
       *          An error is raised in case the simulation is started.
       */
      void
      generate();

    private:

      /**
       * @brief - Used to return the number of threads to create to process the jobs
       *          related to rendering the colony.
       * @return - a value used to create the thread pool associated to this renderer.
       */
      static
      unsigned
      getWorkerThreadCount() noexcept;

      /**
       * @brief - Used to convert the input box from a floating point semantic to a
       *          valid integer coordinates box. This will also make sure that the
       *          returned box completely encompasses the input box so that we are
       *          sure that all the data contained in the `in` box will also be there
       *          in the output box.
       * @param in - the box to convert to integer coordinates.
       * @return - a box containing the input `in` box with integer coordinates.
       */
      static
      utils::Boxi
      fromFPCoordinates(const utils::Boxf& in) noexcept;

      /**
       * @brief - Connect signals and build the scheduler to use to simulate the colony.
       */
      void
      build();

      /**
       * @brief - Reinitialize the colony with the specified dimensions. All cells will be
       *          assigned a `Dead` status. Assumes that the locker for this object has
       *          already been acquired.
       * @param dims - the dimensions of the cells array to allocate.
       */
      void
      reset(const utils::Sizei& dims);

      /**
       * @brief - Used to assign random values for each cell of the colony. This assumes that
       *          the colony is effectively stopped but no checks are performed to ensure it.
       *          Note that this method assumes that the locker is already acquired.
       */
      void
      randomize();

      /**
       * @brief - Used to schedule a rendering of the colony using the internal thread
       *          pool. Note that this function assumes that the locker on the options
       *          is already acquired before calling the method.
       */
      void
      scheduleRendering();

      /**
       * @brief - Internal slot used to handle the tiles computed by the thread
       *          pool. The goal is to trigger the creation of the needed repaint
       *          events to display the results of the computation.
       * @param tiles - a list of tiles that just completed.
       */
      void
      handleTilesComputed(const std::vector<utils::AsynchronousJobShPtr>& tiles);

    private:

      /**
       * @brief - Describe the possible state for the simulation.
       */
      enum class SimulationState {
        Stopped,
        Running,
        SingleStep
      };

      /**
       * @brief - Protect this colony from concurrent accesses.
       */
      std::mutex m_propsLocker;

      /**
       * @brief - Holds a description of the set of rules to apply to perform
       *          the computation of a cell to its descendant when evolving the
       *          colony. Different sets of rules will most likely lead to some
       *          distinct behaviors.
       *          This property is transmitted to the cells created during the
       *          evolution of the colony.
       */
      rules::Type m_ruleset;

      /**
       * @brief - Holds the dimensions of the colony. Note that these dimensions
       *          might be updated in case the colony is expanded and should reflect
       *          the total size of the colony (i.e. the farthest position a living
       *          cell has reached).
       */
      utils::Sizei m_dims;

      /**
       * @brief - The internal array of cells representing the colony.
       */
      std::vector<Cell> m_cells;

      /**
       * @brief - Holds the generation reached by this colony. Each call to `step`
       *          or a simulation step when `start` has been called triggers a new
       *          generation which is kept internally.
       */
      unsigned m_generation;

      /**
       * @brief - Convenience object allowing to schedule the simulation of the colony.
       */
      utils::ThreadPoolShPtr m_scheduler;

      /**
       * @brief - Holds the current simulation status. Checking this value allows to
       *          find the possible actions regarding the colony.
       */
      SimulationState m_simulationState;

      /**
       * @brief - Used to keep track of the tiles already rendered so far in the current
       *          rendering operation. This allows to compute some sort of percentage of
       *          completion of the task.
       */
      unsigned m_taskProgress;

      /**
       * @brief - Keep track of the total size of the batch of tasks that were generated
       *          to be scheduled.
       *          Useful in conjunction with the `m_taskProgress` to provide some sort of
       *          completion percentage.
       */
      unsigned m_taskTotal;

    public:

      /**
       * @brief - Signal emitted whenever a new generation has been computed by the pool.
       *          This is useful for listeners which would like to keep up with the current
       *          generation of cells displayed on screen.
       */
      utils::Signal<unsigned> onGenerationComputed;
  };

  using ColonyShPtr = std::shared_ptr<Colony>;
}

# include "Colony.hxx"

#endif    /* COLONY_HH */
