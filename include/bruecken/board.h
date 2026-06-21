/**
 * @file board.h
 * @brief Spielfeld-Klasse fuer Bruecken.
 * @author Ihr Name
 */

#pragma once

#include <cstdint>
#include <vector>

#include "bruecken/common.h"

namespace bruecken {

/**
 * @brief Modelliert das NxM Spielbrett inklusive Spielstand, Zugverifikation
 *        und Brueckengenerierung.
 *
 * Saemtliche Logik zur Spielregelpruefung sitzt in dieser Klasse.
 * Rotation wird derzeit als unrotierter Standardfall abgebildet.
 */
class Board {
public:
    // =================================================================
    // Konstruktion
    // =================================================================

    /**
     * @param width  Breite  (5..96)
     * @param height Hoehe   (5..96)
     * @param rotation Rotation in Grad (0..90), Standard = 0
     */
    Board(int width, int height, double rotation = 0.0);

    // =================================================================
    // Abfragen
    // =================================================================

    int  get_width()   const { return width_; }
    int  get_height()  const { return height_; }
    double get_rotation() const { return rotation_; }
    GamePhase get_phase() const { return phase_; }
    int  get_turn()    const { return turn_; }

    /// 0 = Spieler 1,  1 = Spieler 2
    int  get_current_player() const { return turn_ % kNumPlayers; }

    const std::vector<Peg>&    get_pegs()    const { return pegs_; }
    const std::vector<Bridge>& get_bridges() const { return bridges_; }

    // =================================================================
    // Koordinaten / Bereichszugehoerigkeit
    // =================================================================

    /** Liegt die Position im gueltigen NxM Raster? */
    bool is_in_bounds(const Position& pos) const;

    /**
     * @brief Gibt an, zu wessen Grenzgebiet diese Position gehoert.
     *
     *  - Innere (N-2)x(M-2)-Flaeche → kNone  (neutral, beide duerfen setzen)
     *  - Aeussere Randstreifen → kTop/kBottom (Spieler 1) / kLeft/kRight (Spieler 2)
     *  - Vier Ecken → kNone (Ueberlappung, niemand)
     */
    Direction get_direction(const Position& pos) const;

    /** Darf Spieler `player_id` auf dieses Feld setzen? */
    bool is_playable(const Position& pos, int player_id) const;

    /** Ist die Position bereits mit einem Stein belegt? */
    bool is_occupied(const Position& pos) const;

    // =================================================================
    // Zug
    // =================================================================

    /**
     * @brief Prueft, ob ein Zug regelkonform ist.
     *
     * Bedingungen:
     *  - Position im Raster
     *  - Position im spielbaren Bereich des Spielers
     *  - Feld nicht bereits belegt
     */
    bool is_valid_move(const Move& move) const;

    /**
     * @brief Fuehrt einen zuvor validierten Zug aus.
     *
     * Setzt den Stein, generiert ggf. neue Bruecken und prueft
     * auf Spielende. Wirft bei ungueltigem Zug.
     */
    void apply_move(const Move& move);

    // =================================================================
    // Spielende
    // =================================================================

    /** Prueft, ob Spieler `player_id` seine beiden Seiten verbunden hat. */
    bool check_win(int player_id) const;

    /** Prueft, ob ein Unentschieden vorliegt (beide blockiert). */
    bool check_draw() const;

private:
    // =================================================================
    // Interne Typen
    // =================================================================

    enum class Cell : uint8_t { kEmpty, kPegP1, kPegP2 };

    // =================================================================
    // Hilfsfunktionen
    // =================================================================

    /** Roesselsprung-Offsets (8 moegliche Nachbarpositionen). */
    static std::vector<Position> knight_offsets();

    /** Liefert alle Positionen im Roesselsprung-Abstand, die im Raster liegen. */
    std::vector<Position> knight_neighbors(const Position& pos) const;

    bool is_corner(const Position& pos) const;

    /**
     * @brief Prueft, ob zwei Bruecken sich kreuzen.
     *
     * Zwei Strecken (a1,a2) und (b1,b2) kreuzen sich, wenn sie sich
     * an einem Nicht-Endpunkt schneiden.
     */
    static bool bridges_cross(const Bridge& a, const Bridge& b);

    /**
     * @brief Prueft, ob eine neue Bruecke eine existierende kreuzt.
     */
    bool would_cross_existing(const Bridge& candidate) const;

    /**
     * @brief Baut neue Bruecken nach Setzen eines Steins.
     *
     * Durchmustert alle Steine des Spielers, prueft Roesselsprung-
     * Nachbarschaft und erzeugt Bruecken, die keine existierende kreuzen.
     */
    void generate_bridges(const Peg& new_peg);

    /**
     * @brief Graphbasierte Gewinnpruefung.
     *
     * Spieler 1 gewinnt, wenn eine durchgehende Brueckenkette
     * die obere mit der unteren Seite verbindet.
     * Spieler 2 analog fuer links↔rechts.
     */
    bool is_connected_across(int player_id) const;

    // =================================================================
    // Daten
    // =================================================================

    int width_;
    int height_;
    double rotation_;
    GamePhase phase_ = GamePhase::kNotStarted;
    int turn_ = 0;

    /// 2D-Raster: grid_[row][col]
    std::vector<std::vector<Cell>> grid_;

    std::vector<Peg>    pegs_;
    std::vector<Bridge> bridges_;

    // Nach Spieler-ID getrennte Listen fuer schnellen Zugriff
    std::vector<Peg>    pegs_by_player_[kNumPlayers];
    std::vector<Bridge> bridges_by_player_[kNumPlayers];
};

}  // namespace bruecken
