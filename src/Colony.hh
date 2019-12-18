#ifndef    COLONY_HH
# define   COLONY_HH

# include <mutex>
# include <memory>
# include <core_utils/CoreObject.hh>
# include <maths_utils/Size.hh>
# include <sdl_engine/Brush.hh>
# include "Cell.hh"

namespace cellulator {

  class Colony: utils::CoreObject {
    public:

      /**
       * @brief - Create a colony with the specified size. All cells will be
       *          initialized to a dead state. THe user can provide a name for
       *          the colony.
       * @param dims - the dimensions of the colony.
       * @param name - the lil' name of the colony.
       */
      Colony(const utils::Sizei& dims,
             const std::string& name = std::string("Daddy's lil monster"));

      /**
       * @brief - Destruction of the colony. Stops the execution if the colony
       *          is running.
       */
      ~Colony();

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

      /**
       * @brief - Create a new brush that can be used to create a texture representing this
       *          colony. The current rendering area is rendered in the texture which might
       *          or might not include all the content of the colony.
       * @return - a pointer to a brush representing the colony.
       */
      sdl::core::engine::BrushShPtr
      createBrush();

    private:

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

    private:

      /**
       * @brief - Protect this colony from concurrent accesses.
       */
      std::mutex m_propsLocker;

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
  };

  using ColonyShPtr = std::shared_ptr<Colony>;
}

# include "Colony.hxx"

#endif    /* COLONY_HH */
