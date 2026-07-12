# Knight Bridge

Knight Bridge is a two-player strategic board game and the final project for
the Programming Lab course (B.Inf.1802) in the 2026 summer semester. The goal
is to connect the two opposite sides assigned to your player with a continuous
chain of bridges.

## Team

| Name | Role and main responsibility |
|------|------------------------------|
| Hao Guo | Project lead; GUI window, board, peg and bridge rendering, and main program |
| Zhibo Zhang | Board model and core game logic |
| Zhixin Fu | Human Player, Random AI Player, and player state logic |
| Junke Pu | GUI interaction, game status, and `PlayerGuiAccess` integration |

## Requirements

- CMake 3.25 or newer
- A C++20-compatible compiler, for example GCC 13 or Clang 18
- Doxygen for generating the project documentation

Preset 1.0.2 and Raylib 6.0 are included in this repository and do not need to
be downloaded separately.

## Build

From the project directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The executable is usually created as `build/knight_bridge`. With
multi-configuration generators such as Visual Studio, it may be located at
`build/Release/knight_bridge.exe`.

Run the tests with:

```bash
ctest --test-dir build --output-on-failure
```

## Start The Game

Human vs. Human:

```bash
./build/knight_bridge
```

Human vs. Random AI:

```bash
./build/knight_bridge --playerTypes HUMAN RANDOM_AI
```

Random AI vs. Random AI with a visible GUI and a 500 ms delay:

```bash
./build/knight_bridge --playerTypes RANDOM_AI RANDOM_AI --delay 500 --gui
```

Full example with all implemented settings:

```bash
./build/knight_bridge \
    --size 24 24 \
    --rotation 15 \
    --playerTypes HUMAN RANDOM_AI \
    --playerNames "Alice" "Bot" \
    --playerColors "#ff0000" "#0000ff" \
    --delay 300 \
    --loglevel INFO
```

Only the required player types `HUMAN` and `RANDOM_AI` are implemented.
`SIMPLE_AI` and `ADVANCED_AI` are not implemented.

## GUI Usage

- The GUI opens automatically when at least one player is a Human Player.
- A peg is placed by left-clicking a valid grid point.
- The status area at the top shows the current player, the round, and the game
  result.
- Player sides, pegs, and bridges use the configured player colors.
- The coordinate system starts in the upper-left corner of the logical board.
- For large boards, the GUI uses sparse coordinate labels so that the board
  remains readable instead of covering the grid with overlapping numbers.
- The exact X/Y coordinate of every grid point is shown when the mouse hovers
  over that point. This also applies to large boards up to 96 x 96.
- After a win, the window remains open and the winning path is highlighted until
  the window is closed.
- For AI-only games, the `--gui` option opens the graphical display.

## Command Line Help

Command:

```bash
./build/knight_bridge --help
```

Output:

```text
=========================================================================
    Knight Bridge <0.1.0>
  Authors: Zhibo Zhang, Hao Guo, Junke Pu, Zhixin Fu
=========================================================================
Options:
 -d,--delay <DELAY>            Delay in milliseconds after an ai has
                               made its move.
 -g,--gui                      Show the GUI, even if no HUMAN player
                               exists.
 -s,--size <WIDTH HEIGHT>      Dimensions of the board. (5-96)
 -r,--rotation <ROTATION>      Degrees by which to rotate the board.
                               (0.0-90.0)
 -h,--help                     Print this help message and some extra
                               information about this program.
 -ll,--loglevel <LEVEL>        The maximum log level. [ERRORS,
                               WARNINGS, INFO, DEBUG]
 -pc,--playerColors <COLOR ...> The colors of the players. [#RRGGBB ...]
 -pn,--playerNames <NAME ...>  The names of the players.
 -pt,--playerTypes <TYPE ...>  The type of each player. [HUMAN,
                               RANDOM_AI, ...]
_________________________________________________________________________

Preset: v1.0.2 (28.06.2026)
Raylib: 6.0
  This project uses Raylib.
    Website: https://www.raylib.com/
    License: zlib
```

## Generate Documentation

```bash
doxygen Doxyfile
```

The generated HTML documentation is written to `doc/html/index.html`. The
`doc/` directory is a generated artifact and must not be committed to the
GitLab repository.
