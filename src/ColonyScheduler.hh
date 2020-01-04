#ifndef    COLONY_SCHEDULER_HH
# define   COLONY_SCHEDULER_HH

# include <mutex>
# include <memory>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <maths_utils/Vector2.hh>
# include <core_utils/ThreadPool.hh>
# include "Colony.hh"

namespace cellulator {

  class ColonyScheduler: utils::CoreObject {
    public:

      /**
       * @brief - Create a colony scheduler for the specified colony. Note that
       *          this method is merely a wrapper to allow for easy scheduling
       *          of the execution and evolution of the colony along with some
       *          utilities method to acces general properties of the colony
       *          and notifications.
       * @param colony - the colony to simulate.
       */
      ColonyScheduler(ColonyShPtr colony);

      /**
       * @brief - Destruction of the colony. Stops the execution if the colony
       *          is running.
       */
      ~ColonyScheduler();

      /**
       * @brief - Wrapper to the corresponding method from the `Colony` class. This
       *          allows to query the current size of the colony based on the extreme
       *          position reached by cells.
       *          The size is expressed in terms of cells.
       * @return - the living area occupied by the colony.
       */
      utils::Boxi
      getArea() noexcept;

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
      fetchCells(std::vector<State>& cells,
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
       * @brief - Wraps a call to the internal colony to generate random cells on
       *          all the available element.
       *          An error is raised in case the simulation is started (usually we
       *          want the user to call `stop` before calling this method).
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
       * @brief - Connect signals and build the scheduler to use to simulate the colony.
       */
      void
      build();

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

      /**
       * @brief - The internal colony which is scheduled by this object.
       */
      ColonyShPtr m_colony;

    public:

      /**
       * @brief - Signal emitted whenever a new generation has been computed by the pool.
       *          This is useful for listeners which would like to keep up with the current
       *          generation of cells displayed on screen.
       */
      utils::Signal<unsigned> onGenerationComputed;
  };

  using ColonySchedulerShPtr = std::shared_ptr<ColonyScheduler>;
}

# include "ColonyScheduler.hxx"

#endif    /* COLONY_SCHEDULER_HH */
