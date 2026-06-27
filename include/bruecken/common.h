/**
 * @file common.h
 * @brief Projektweite Basistypen und Konstanten im Namespace bruecken.
 * @author Zhibo Zhang
 */

#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace bruecken {

// =====================================================================
// Konstanten
// =====================================================================

/// Minimale Spielfeldbreite/-hoehe
inline constexpr int kBoardMinSize = 5;
/// Maximale Spielfeldbreite/-hoehe
inline constexpr int kBoardMaxSize = 96;
/// Vorgabe-Spielfeldgroesse
inline constexpr int kDefaultBoardSize = 24;
/// Minimale Rotation in Grad
inline constexpr double kMinRotation = 0.0;
/// Maximale Rotation in Grad
inline constexpr double kMaxRotation = 90.0;
/// GUI-Fenstergroesse (quadratisch)
inline constexpr int kDefaultWindowSize = 720;
/// Anzahl der Spieler
inline constexpr int kNumPlayers = 2;

// =====================================================================
// Position im Raster
// =====================================================================

/**
 * @brief Ein (x, y)-Koordinatenpaar auf dem Spielbrett.
 *
 * x = Spalte (ab 0, von links nach rechts)
 * y = Zeile  (ab 0, von oben nach unten)
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
// Himmelsrichtung / Brettseite
// =====================================================================

/// Welcher Seite des Spielfelds ein Bereich zugeordnet ist.
enum class Direction {
    kTop,
    kBottom,
    kLeft,
    kRight,
    kNone  ///< neutrale / überlappende Zone
};

/// Gibt die Gegenrichtung zurueck.
inline Direction opposite(Direction d) {
    switch (d) {
        case Direction::kTop:    return Direction::kBottom;
        case Direction::kBottom: return Direction::kTop;
        case Direction::kLeft:   return Direction::kRight;
        case Direction::kRight:  return Direction::kLeft;
        default:                 return Direction::kNone;
    }
}

// =====================================================================
// Spielstein
// =====================================================================

/**
 * @brief Ein gesetzter Spielstein.
 *
 * Jeder Stein gehoert genau einem Spieler (player_id = 0 oder 1).
 */
struct Peg {
    Position pos;
    int player_id = 0;  ///< 0 = Spieler 1,  1 = Spieler 2

    bool operator==(const Peg& other) const {
        return pos == other.pos && player_id == other.player_id;
    }
};

// =====================================================================
// Bruecke
// =====================================================================

/**
 * @brief Eine Bruecke zwischen zwei Spielsteinen desselben Spielers.
 *
 * Eine Bruecke entsteht automatisch, wenn zwei Steine im
 * Roesselsprung-Abstand gesetzt sind (dx=1,dy=2 oder dx=2,dy=1)
 * und die Verbindung keine andere Bruecke kreuzt.
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
// Spielphase
// =====================================================================

/// Aktuelle Phase des Spiels.
enum class GamePhase {
    kNotStarted,
    kInProgress,
    kFinished,   ///< ein Spieler hat gewonnen
    kDraw        ///< Unentschieden
};

// =====================================================================
// Spielerfarben (Hex-RGB strings)
// =====================================================================

/// Vorgabefarben: Spieler 1 = Rot, Spieler 2 = Blau
inline const std::vector<std::string> kDefaultPlayerColors = {
    "#ff0000",
    "#0000ff"
};

/// Vorgabenamen
inline const std::vector<std::string> kDefaultPlayerNames = {
    "Player 1",
    "Player 2"
};

}  // namespace bruecken
