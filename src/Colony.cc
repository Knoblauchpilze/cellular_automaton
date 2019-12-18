
# include "Colony.hh"

namespace cellulator {

  Colony::Colony(const utils::Sizei& dims,
                 const std::string& name):
    utils::CoreObject(name),

    m_propsLocker(),

    m_dims(dims)
  {
    // Check consistency.
    if (!m_dims.valid()) {
      error(
        std::string("Could not create colony"),
        std::string("Invalid dimensions ") + dims.toString()
      );
    }
  }

  void
  Colony::start() {
    // TODO: Start the colony.
  }

  void
  Colony::step() {
    // TODO: Simulate a single step of the colony.
  }

  void
  Colony::stop() {
    // TODO: Stop the colony.
  }

  void
  Colony::generate() {
    // TODO: Generate the colony.
  }

}

