# cellular_automaton

Contains an implementation of a cellular automaton. Most of the controls are assigned through keyboard keys and the mouse can be used to drop some cells on the canvas.
It is a porting of an old application of mine from `Win SDL 1.2` to `Ubuntu SDL 2.0`.
Not a very complex app but can still be fun to play with.

# Installation

- Clone the repo: `git clone git@github.com:Knoblauchpilze/cellular_automaton.git`
- Go to the project's directory `cd ~/path/to/the/repo`
- Compile: `make r`

# Usage

The user can provide a colony to load through a file to store in the `data` directory. Values are expected to be:
  - `1` for newborn cells
  - `2` for alive cells
  - `3` for dying cells
  - anything else is set to be a dead cell

The keys are as follows:
  - `n` to simulate next generation: keep simulating while the key is pressed
  - `LCtrl` allows to simulate a single step when pressing `n`
  - `KP 1` insert a newborn cell at the cursor position
  - `KP 2` insert an alive cell at the cursor position
  - `KP 3` insert a dying cell at the cursor position
  - `Delete` removes the cell at the cursor position
  - `Right, Up, Left, Down` moves the cursor around
  - The left button of the mouse drops alibe cells and update the position of the cursor. Keep dropping cells while the button is pressed.
  - `r` generates a random colony
  - `l` loads a colony from a file named `data/colony.txt`
  - `p` saves the current colony to a file named `data/colony.txt`
  - `Backspace` resets the colony to an empty one
