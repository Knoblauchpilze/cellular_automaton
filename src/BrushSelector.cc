
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

    // Create each brush.
    sdl::graphic::Button* b = nullptr;

    b = createButtonFromBrushName("Standard", "");
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
  BrushSelector::createBrushFromName(const std::string& name) {
    // Detect known brush names and perform the creation of each one.
    if (name == "Standard") {
      return std::make_shared<CellBrush>(utils::Sizei(1, 1), State::Alive);
    }

    if (name == "Eraser") {
      return std::make_shared<CellBrush>(utils::Sizei(1, 1), State::Dead);
    }

    if (name == "Gosper glider gun") {
      return std::make_shared<CellBrush>(
        "data/brushes/golgun.brush"
      );
    }

    if (name == "Backrake") {
      return std::make_shared<CellBrush>(
        "data/brushes/backRake.brush"
      );
    }

    if (name == "Backrake2") {
      return std::make_shared<CellBrush>(
        "data/brushes/backRake2.brush"
      );
    }

    if (name == "Ecologist") {
      return std::make_shared<CellBrush>(
        "data/brushes/ecologist.brush"
      );
    }

    if (name == "Halfmax") {
      return std::make_shared<CellBrush>(
        "data/brushes/halfmax.brush"
      );
    }

    if (name == "LWSW") {
      return std::make_shared<CellBrush>(
        "data/brushes/LWSW.brush"
      );
    }

    if (name == "Puffer") {
      return std::make_shared<CellBrush>(
        "data/brushes/puffer2.brush"
      );
    }

    if (name == "Spacerake") {
      return std::make_shared<CellBrush>(
        "data/brushes/spaceRake.brush"
      );
    }

    if (name == "Shick engine") {
      return std::make_shared<CellBrush>(
        "data/brushes/shickEngine.brush"
      );
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

    // Create the brush to provide to external listeners if any.
    CellBrushShPtr nb = nullptr;

    if (toggled) {
      BrushesTable::const_iterator name = m_brushes.find(brushName);
      if (name == m_brushes.cend()) {
        log(
          std::string("Could not find data for brush \"") + brushName + "\" in local data",
          utils::Level::Error
        );
      }
      else {
        nb = createBrushFromName(name->second);
      }
    }

    // Notify external listeners.
    onBrushChanged.safeEmit(
      std::string("onBrushChanged(") + brushName + ", " + std::to_string(toggled) + ")",
      nb
    );
  }

}
