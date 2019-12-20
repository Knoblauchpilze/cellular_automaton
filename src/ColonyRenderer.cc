
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
  ColonyRenderer::createBrushFromCells(const std::vector<Cell>& cells,
                                       const utils::Boxi& area)
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
    for (int y = 0 ; y < iEnv.h() ; ++y) {
      for (int x = 0 ; x < iEnv.w() ; ++x) {
        // Compute the float cell coordinate for this pixel.
        float rX = x / cellsDims.w();
        float rY = y / cellsDims.h();

        // Compute the cell coordinate from the floating point coords.
        int cX = static_cast<int>(std::floor(m_settings.area.getLeftBound() + rX));
        int cY = static_cast<int>(std::floor(m_settings.area.getBottomBound() + rY));

        // Transform this using the provided area.
        utils::Vector2i c(cX - area.getLeftBound(), cY - area.getBottomBound());

        // Retrieve the state of this cell.
        const Cell& ce = cells[c.y() * area.w() + c.x()];

        // Assign the corresponding color to the pixel.
        sdl::core::engine::Color co = sdl::core::engine::Color::NamedColor::Pink;
        switch (ce.state()) {
          case State::Alive:
            co = sdl::core::engine::Color::NamedColor::Blue;
            break;
          case State::Newborn:
            co = sdl::core::engine::Color::NamedColor::Green;
            break;
          case State::Dying:
            co = sdl::core::engine::Color::NamedColor::Red;
            break;
          case State::Dead:
          default:
            co = sdl::core::engine::Color::NamedColor::Black;
            break;
        }

        colors[y * iEnv.w() + x] = co;
      }
    }

    // Create the brush and return it.
    sdl::core::engine::BrushShPtr brush = std::make_shared<sdl::core::engine::Brush>(
      std::string("brush_for_") + getName(),
      false
    );

    brush->createFromRaw(iEnv, colors);

    return brush;
  }

}
