/**
 * @file common.h
 * @brief Project-wide base types and constants in the bruecken namespace.
 * @author Zhibo Zhang
 */

#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace bruecken {

// =====================================================================
// Constants
// =====================================================================

/// Minimum board width/height
inline constexpr int kBoardMinSize = 5;
/// Maximum board width/height
inline constexpr int kBoardMaxSize = 96;
/// Minimum rotation in degrees
inline constexpr double kMinRotation = 0.0;
/// Maximum rotation in degrees
inline constexpr double kMaxRotation = 90.0;
/// Default GUI window size (square)
inline constexpr int kDefaultWindowSize = 720;
/// Number of players
inline constexpr int kNumPlayers = 2;

// =====================================================================
// Grid position
// =====================================================================

/**
 * @brief An (x, y) coordinate pair on the game board.
 *
 * x = column (starting at 0, left to right)
 * y = row    (starting at 0, top to bottom)
 */
struct Position {
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Position& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

// =====================================================================
// Direction / Board side
// =====================================================================

/// Which side of the board an area is assigned to.
enum class Direction {
    kTop,
    kBottom,
    kLeft,
    kRight,
    kNone  ///< neutral / overlapping zone
};

// =====================================================================
// Peg
// =====================================================================

/**
 * @brief A placed peg.
 *
 * Each peg belongs to exactly one player (player_id = 0 or 1).
 */
struct Peg {
    Position pos;
    int player_id = 0;  ///< 0 = player 1,  1 = player 2

    bool operator==(const Peg& other) const {
        return pos == other.pos && player_id == other.player_id;
    }
};

// =====================================================================
// Bridge
// =====================================================================

/**
 * @brief A bridge between two pegs owned by the same player.
 *
 * A bridge is created automatically when two pegs are placed at knight-move
 * distance (dx=1,dy=2 or dx=2,dy=1) and the connection does not cross any
 * other bridge.
 */
struct Bridge {
    Position from;
    Position to;
    int player_id = 0;

    bool operator==(const Bridge& other) const {
        return from == other.from && to == other.to &&
               player_id == other.player_id;
    }
};

// =====================================================================
// Game phase
// =====================================================================

/// Current game phase.
enum class GamePhase {
    kNotStarted,
    kInProgress,
    kFinished,   ///< a player has won
    kDraw        ///< draw
};

// =====================================================================
// Player colors (Hex-RGB strings)
// =====================================================================

/// Default colors: player 1 = red, player 2 = blue
inline const std::vector<std::string> kDefaultPlayerColors = {
    "#ff0000",
    "#0000ff"
};

/// Default names
inline const std::vector<std::string> kDefaultPlayerNames = {
    "Player 1",
    "Player 2"
};

}  // namespace bruecken
