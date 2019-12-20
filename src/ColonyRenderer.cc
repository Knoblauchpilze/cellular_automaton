
# include "ColonyRenderer.hh"
# include <sdl_engine/PaintEvent.hh>
# include <sdl_engine/Color.hh>

namespace cellulator {

  ColonyRenderer::ColonyRenderer(ColonyShPtr colony,
                                 const utils::Sizef& hint,
                                 sdl::core::SdlWidget* parent):
    sdl::graphic::ScrollableWidget(std::string("colony_renderer"),
                                   parent,
                                   hint),

    m_propsLocker(),

    m_tex(),
    m_settings(),
    m_colonyRendered(true),

    m_colony(colony),
    m_generationComputedSignalID(utils::Signal<unsigned>::NoID),

    onGenerationComputed()
  {
    setService(std::string("colony_renderer"));

    // Check consistency.
    if (colony == nullptr) {
      error(
        std::string("Could not create renderer"),
        std::string("Invalid null colony")
      );
    }

    build();
  }

  void
  ColonyRenderer::generate(const std::string& /*dummy*/) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Generate it at random.
    m_colony->generate();

    // Indicate that the colony changed so that we can repaint it.
    setColonyChanged();
  }

  void
  ColonyRenderer::drawContentPrivate(const utils::Uuid& uuid,
                                     const utils::Boxf& area)
  {
    // Acquire the lock on the attributes of this widget.
    Guard guard(m_propsLocker);

    // Load the colony: this should happen only if some cells have been
    // rendered since the last draw operation. This status is kept by
    // the `m_colonyRendered` boolean. Generally this indicates that a
    // compute operation is running and that some more tiles have been
    // received.
    if (colonyChanged()) {
      // Load the colony.
      loadColony();

      // The colony has been updated.
      m_colonyRendered = false;
    }

    // Check whether there's something to display.
    if (!m_tex.valid()) {
      return;
    }

    // Convert the area to local so that we blit only the right area of
    // the texture representing the fractal.
    utils::Boxf thisArea = LayoutItem::getRenderingArea().toOrigin();
    utils::Sizef sizeEnv = getEngine().queryTexture(uuid);
    utils::Sizef texSize = getEngine().queryTexture(m_tex);

    utils::Boxf srcArea = thisArea.intersect(area);
    utils::Boxf dstArea = thisArea.intersect(area);

    utils::Boxf srcEngine = convertToEngineFormat(srcArea, texSize);
    utils::Boxf dstEngine = convertToEngineFormat(dstArea, sizeEnv);

    getEngine().drawTexture(m_tex, &srcEngine, &uuid, &dstEngine);
  }

  void
  ColonyRenderer::build() {
    // Connect to the handler signal.
    m_generationComputedSignalID = m_colony->onGenerationComputed.connect_member<ColonyRenderer>(
      this,
      &ColonyRenderer::handleGenerationComputed
    );

    // Assign the rendering window: by default we consider that the whole area is visible.
    utils::Sizei s = m_colony->getSize();
    m_settings.area = utils::Boxf::fromSize(s, true);
  }

  void
  ColonyRenderer::loadColony() {
    // Clear any existing texture representing the colony.
    clearColony();

    // Create the brush representing the tile using the palette
    // provided by the user.
    // To do so we need to fetch the cells from the colony that
    // represent the visible area. Then we need to create a brush
    // from the cells with the specified dimensions and finally
    // a texture from the brush.

    // Fetch the cells that are visible. We need to convert the
    // input area.
    std::vector<Cell> cells;
    utils::Boxi out = m_colony->fetchCells(cells, m_settings.area);

    // Create the colors needed for the brush.
    sdl::core::engine::BrushShPtr brush = createBrushFromCells(cells, out);

    // check consistency.
    if (brush == nullptr) {
      error(
        std::string("Could not create texture to represent colony"),
        std::string("Failed to create brush data")
      );
    }

    // Use the brush to create a texture.
    m_tex = getEngine().createTextureFromBrush(brush);

    if (!m_tex.valid()) {
      error(
        std::string("Could not create texture to represent colony"),
        std::string("Failed to transform brush into texture")
      );
    }
  }

  void
  ColonyRenderer::handleGenerationComputed(unsigned generation) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Post a repaint event for each area that has been rendered.
    sdl::core::engine::PaintEventShPtr e = std::make_shared<sdl::core::engine::PaintEvent>();

    postEvent(e);

    // The colony need to be rendered again.
    setColonyChanged();

    // Notify listeners.
    onGenerationComputed.safeEmit(
      std::string("onGenerationComputed(") + std::to_string(generation) + ")",
      generation
    );
  }

  sdl::core::engine::BrushShPtr
  ColonyRenderer::createBrushFromCells(const std::vector<Cell>& /*cells*/,
                                       const utils::Boxi& /*area*/)
  {
    // Determine the size of a single cell.
    utils::Sizef env = LayoutItem::getRenderingArea().toSize();
    utils::Sizef cellsDims(env.w() / m_settings.area.w(), env.h() / m_settings.area.h());
    utils::Sizei iEnv(
      static_cast<int>(std::floor(env.w())),
      static_cast<int>(std::floor(env.h()))
    );

    // Create a canvas of the expected size.
    std::vector<sdl::core::engine::Color> colors(
      iEnv.area(),
      sdl::core::engine::Color::NamedColor::Black
    );

    // Fill in each cell with the corresponding color.
    // TODO: Implemenation.

    // Create the brush and return it.
    sdl::core::engine::BrushShPtr brush = std::make_shared<sdl::core::engine::Brush>(
      std::string("brush_for_") + getName(),
      false
    );

    brush->createFromRaw(iEnv, colors);

    return brush;
  }


      /**
       * @brief - Create a new brush that can be used to create a texture representing this
       *          colony. The current rendering area is rendered in the texture which might
       *          or might not include all the content of the colony.
       * @return - a pointer to a brush representing the colony.
       */
  /*    sdl::core::engine::BrushShPtr
      createBrush();
  sdl::core::engine::BrushShPtr
  Colony::createBrush() {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Traverse the internal array of cells and build an array of
    // colors to use to represent the colony.
    sdl::core::engine::Color def = sdl::core::engine::Color::NamedColor::Black;
    std::vector<sdl::core::engine::Color> colors(m_dims.area(), def);

    for (int y = 0 ; y < m_dims.h() ; ++y) {
      // Compute the coordinate of this pixel in the output canvas. Note that
      // we perform an inversion of the internal data array along the `y` axis:
      // indeed as we will use it to generate a surface we need to account for
      // the axis inversion that will be applied there.
      int offset = (m_dims.h() - 1 - y) * m_dims.w();

      for (int x = 0 ; x < m_dims.w() ; ++x) {
        // Determine the color for this cell.
        sdl::core::engine::Color c = def;
        switch (m_cells[offset + x].state()) {
          case State::Newborn:
            c = sdl::core::engine::Color::NamedColor::Green;
            break;
          case State::Alive:
            c = sdl::core::engine::Color::NamedColor::Blue;
            break;
          case State::Dying:
            c = sdl::core::engine::Color::NamedColor::Red;
            break;
          case State::Dead:
          default:
            // Keep the default color.
            break;
        }

        colors[offset + x] = c;
      }
    }

    // Create a brush from the array of colors.
    sdl::core::engine::BrushShPtr brush = std::make_shared<sdl::core::engine::Brush>(
      std::string("brush_for_") + getName(),
      false
    );

    brush->createFromRaw(m_dims, colors);

    return brush;
  }*/

}
