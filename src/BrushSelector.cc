
# include "BrushSelector.hh"
# include <sdl_graphic/LinearLayout.hh>

namespace cellulator {

  BrushSelector::BrushSelector(const utils::Sizef& hint,
                               sdl::core::SdlWidget* parent):
    sdl::core::SdlWidget(std::string("brush_selector"),
                         hint,
                         parent,
                         getDefaultColor()),

    m_propsLocker(),

    m_brushes(),
    m_currentBrush(BrushDesc{
      true,
      "Standard",
      1
    }),

    onBrushChanged()
  {
    build();
  }

  void
  BrushSelector::build() {
    // Create the layout to receive all the registered brushes.
    sdl::graphic::LinearLayoutShPtr layout = std::make_shared<sdl::graphic::LinearLayout>(
      "brush_select_layout",
      this,
      sdl::graphic::LinearLayout::Direction::Vertical,
      getGlobalMargins(),
      getComponentMargins()
    );

    layout->allowLog(false);

    // Assign the layout to this widget.
    setLayout(layout);

    // Create the brush' size selector.
    sdl::graphic::Slider* size = new sdl::graphic::Slider(
      getBrushSizeSliderName(),
      1.0f,
      utils::Vector2f(1.0f, 10.0f),
      9u,
      0u,
      getGeneralTextFont(),
      15u,
      this
    );
    if (size == nullptr) {
      error(
        std::string("Could not create brush selection panel"),
        std::string("Could not create brush size slider")
      );
    }

    size->onValueChanged.connect_member<BrushSelector>(
      this,
      &BrushSelector::onBrushSizeChanged
    );

    layout->setMaxSize(
      utils::Sizef(
        std::numeric_limits<float>::max(),
        getBrushSelectMaxHeight()
      )
    );
    layout->addItem(size);

    // Create each brush.
    sdl::graphic::Button* b = nullptr;

    b = createButtonFromBrushName("Standard", "");
    b->toggle(true);
    layout->addItem(b);

    b = createButtonFromBrushName("Eraser", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Gosper glider gun", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Backrake", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Backrake2", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Ecologist", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Halfmax", "");
    layout->addItem(b);

    b = createButtonFromBrushName("LWSW", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Puffer", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Spacerake", "");
    layout->addItem(b);

    b = createButtonFromBrushName("Shick engine", "");
    layout->addItem(b);
  }

  CellBrushShPtr
  BrushSelector::createBrushFromName(const std::string& name,
                                     const utils::Sizei& size)
  {
    // Detect known brush names and perform the creation of each one.
    if (name == "Standard") {
      return std::make_shared<CellBrush>(size, State::Alive);
    }

    if (name == "Eraser") {
      return std::make_shared<CellBrush>(size, State::Dead);
    }

    if (name == "Gosper glider gun") {
      return CellBrush::fromFile("data/brushes/golgun.brush");
    }

    if (name == "Backrake") {
      return CellBrush::fromFile("data/brushes/backRake.brush");
    }

    if (name == "Backrake2") {
      return CellBrush::fromFile("data/brushes/backRake2.brush");
    }

    if (name == "Ecologist") {
      return CellBrush::fromFile("data/brushes/ecologist.brush");
    }

    if (name == "Halfmax") {
      return CellBrush::fromFile("data/brushes/halfmax.brush");
    }

    if (name == "LWSW") {
      return CellBrush::fromFile("data/brushes/LWSW.brush");
    }

    if (name == "Puffer") {
      return CellBrush::fromFile("data/brushes/puffer2.brush");
    }

    if (name == "Spacerake") {
      return CellBrush::fromFile("data/brushes/spaceRake.brush");
    }

    if (name == "Shick engine") {
      return CellBrush::fromFile("data/brushes/shickEngine.brush");
    }

    // Unknown brush.
    log(
      std::string("Could not create brush from unknown name ") + name,
      utils::Level::Warning
    );

    return nullptr;
  }

  void
  BrushSelector::onBrushSelected(std::string brushName,
                                 bool toggled)
  {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Untoggle any other brush in case this brush has been toggled.
    if (toggled) {
      for (BrushesTable::const_iterator it = m_brushes.cbegin() ;
          it != m_brushes.cend() ;
          ++it)
      {
        if (it->first == brushName) {
          continue;
        }

        sdl::graphic::Button* b = getBrushButtonFromName(it->first);
        b->toggle(false);
      }
    }

    // Check whether we should deactivate the brush or create it.
    if (!toggled) {
      // Deactivate the current brush.
      log("Deactivating brush " + m_currentBrush.name);
      m_currentBrush.valid = false;

      // Notify listeners right here.
      onBrushChanged.safeEmit(
        std::string("onBrushChanged(deactivation)"),
        nullptr
      );

      return;
    }

    // Try to find the corresponding brush in the internal list.
    BrushesTable::const_iterator name = m_brushes.find(brushName);
    if (name == m_brushes.cend()) {
      log(
        std::string("Could not find data for brush \"") + brushName + "\" in local data",
        utils::Level::Error
      );

      return;
    }

    // Retrieve the size to assign to the brush.
    sdl::graphic::Slider& s = getBrushSizeSlize();
    int dim = s.getValue();

    notifyBrushChanged(name->second, dim);
  }

  void
  BrushSelector::onBrushSizeChanged(float size) {
    // Protect from concurrent accesses.
    Guard guard(m_propsLocker);

    // Convert the input size to an integer and compare it to the
    // last emitted brush.
    int iSize = static_cast<int>(std::round(size));

    if (!m_currentBrush.valid || m_currentBrush.dim == iSize) {
      // The current brush is either not valid or has the same size
      // as the one we received: no need to notify anyone.
      return;
    }

    notifyBrushChanged(m_currentBrush.name, iSize);
  }

  void
  BrushSelector::notifyBrushChanged(const std::string& brushName,
                                    int brushSize)
  {
    // Create dimensions from the input brush size.
    utils::Sizei size(brushSize, brushSize);

    CellBrushShPtr nb = createBrushFromName(brushName, size);

    // Register this brush with the new info.
    m_currentBrush = BrushDesc{
      true,
      brushName,
      brushSize
    };

    // Notify external listeners.
    onBrushChanged.safeEmit(
      std::string("onBrushChanged(") + brushName + ")",
      nb
    );
  }

}
