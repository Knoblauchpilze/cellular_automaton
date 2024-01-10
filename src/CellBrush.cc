
# include "CellBrush.hh"
# include <fstream>
# include <sstream>
# include <algorithm>

namespace {

  inline
  void
  ltrim(std::string& s) {
    s.erase(
      s.begin(),
      std::find_if(s.begin(), s.end(),
        [](int ch) {
          return !std::isspace(ch);
        }
      )
    );
  }

  inline
  void
  rtrim(std::string& s) {
    s.erase(
      std::find_if(s.rbegin(), s.rend(),
        [](int ch) {
          return !std::isspace(ch);
        }
      ).base(),
      s.end()
    );
  }

  inline
  void
  trim(std::string& s) {
    rtrim(s);
    ltrim(s);
  }

}

namespace cellulator {

  CellBrush::CellBrush(const std::string& name):
    utils::CoreObject(name),

    m_size(1, 1),

    m_monotonic(true),
    m_monotonicState(State::Alive),

    m_data()
  {
    setService("brush");
  }

  CellBrush::CellBrush(const std::string& file,
                       bool invertY):
    CellBrush(file)
  {
    // Load the data from the file.
    loadFromFile(file, invertY);
  }

  CellBrush::CellBrush(const utils::Sizei& size,
                       const State& state):
    CellBrush(size.toString() + "_" + std::to_string(static_cast<int>(state)))
  {
    // Check whether the input size is valid.
    if (!size.valid()) {
      error(
        std::string("Could not create cell brush"),
        std::string("Invalid input size ") + size.toString()
      );
    }

    // Reset monotonic brush' properties.
    m_size = size;
    m_monotonicState = state;
  }

  State
  CellBrush::getStateAt(int x,
                        int y) const noexcept
  {

    if (!valid()) {
      return State::Dead;
    }

    // In case the coordinate is not inside the area covered by this
    // brush return `Dead` as well.
    if (x < 0 || y < 0 || x >= m_size.w() || y >= m_size.h()) {
      return State::Dead;
    }

    // If the brush is monotonic, return the associated state.
    if (isMonotonic()) {
      return m_monotonicState;
    }

    // Convert the coordinate to a one-dimensional offset. We can be
    // certain that this will yield a valid index as we checked that
    // the coordinate was inside the area covered by this brush.
    int off = y * m_size.w() + x;

    return m_data[off];
  }

  void
  CellBrush::loadFromFile(const std::string& file,
                          bool invertY)
  {
    // Open the file associated to the brush' data.
    std::ifstream in(file.c_str());

    if (!in.good()) {
      error(
        std::string("Could not perform loading of brush from \"") + file + "\"",
        std::string("Cannot open file")
      );
    }

    // The first line should describe the dimensions of the brush.
    std::string dims;
    std::getline(in, dims);

    trim(dims);

    // The dimensions should be separated by a 'x' character. We
    // will use a string stream to separate both dimension.
    std::stringstream dss(dims);

    std::string wAsStr;
    std::getline(dss, wAsStr, 'x');

    std::string hAsStr;
    std::getline(dss, hAsStr, 'x');

    if (wAsStr.empty() || hAsStr.empty()) {
      error(
        std::string("Could not perform loading of brush from \"") + file + "\"",
        std::string("Cannot interpret invalid dimensions \"") + dims + "\""
      );
    }

    unsigned w;
    std::istringstream converter(wAsStr);
    converter >> w;

    unsigned h;
    converter.clear();
    converter.str(hAsStr);
    converter >> h;

    if (w == 0u || h == 0u) {
      error(
        std::string("Could not perform loading of brush from \"") + file + "\"",
        std::string("Interpreted invalid dimensions ") + std::to_string(w) + "x" + std::to_string(h)
      );
    }

    // Now that we know the dimensions of the brush we can start parsing the rest
    // of the data. It should be lined up in consecutive lines each one having
    // exactly `w` characters.
    // We don't consider empty lines as an error. Also in case a character does
    // not represent neither a live nor dead cell we don't interpret it but still
    // continue the process. This will most likely result in a corrupted brush
    // but it's up to the user to provide correct data.
    std::vector<State> brush(w * h, State::Dead);
    std::string line;

    unsigned curW = 0u, curH = 0u, offset = 0u, l = 0u, id = 0u;

    while (!in.eof() && curH < h) {
      // Retrieve the line of data.
      std::getline(in, line);

      // Trim spaces if any.
      trim(line);

      ++l;

      // If the line is empty, move on to the next.
      if (line.empty()) {
        warn("Detected empty line in file \"" + file + "\"");
        continue;
      }

      // In any other case, try to interpret each line
      offset = (invertY ? (h - 1u - curH) * w : curH * w);
      id = 0u;
      curW = 0u;

      while (id < w && curW < line.size()) {
        // Interpret this character.
        switch (line[id]) {
          case getDeadCellCharacter():
            // The `brush` is already filled with `Dead` cells, do nothing.
            ++curW;
            break;
          case getLiveCellCharacter():
            brush[offset + curW] = State::Alive;
            ++curW;
            break;
          default:
            warn(std::string("Detected invalid character '") + line[id] + "' in file \"" + file + "\"");
            break;
        }

        ++id;
      }

      // Check whether we could fill all the cells for this line.
      if (curW < w) {
        warn(
          "Could only parse " + std::to_string(curW) + " / " + std::to_string(w) +
          " character(s) in line " + std::to_string(curH) + " in file \"" + file + "\""
        );
      }

      ++curH;
    }

    if (curH < h) {
      warn(
        "Could only parse " + std::to_string(curH) + " / " + std::to_string(h) +
        " line(s) in file \"" + file + "\""
      );
    }

    // Assign the parsed data.
    m_size = utils::Sizei(w, h);
    m_data.swap(brush);
    m_monotonic = false;
  }

}

