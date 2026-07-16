/**
 * @file board.h
 * @brief Board class for Knight Bridge.
 * @author Zhibo Zhang
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "bruecken/common.h"

namespace preset { class Move; }

namespace bruecken {

/**
 * @brief Models the NxM game board, including game state, move validation,
 *        and bridge generation.
 *
 * All game-rule validation logic lives in this class.
 * Rule-related coordinates are fixed integer positions in the NxM coordinate
 * frame. Rotation defines which of those fixed positions are inside the
 * rotated play field and which boundary area they belong to.
 */
class Board {
public:
    // =================================================================
    // Construction
    // =================================================================

    /**
     * @param width  Width  (5..96)
     * @param height Height (5..96)
     * @param rotation Rotation in degrees (0..90), default = 0
     */
    Board(int width, int height, double rotation = 0.0);

    // =================================================================
    // Queries
    // =================================================================

    int  get_width()   const { return width_; }
    int  get_height()  const { return height_; }
    double get_rotation() const { return rotation_; }
    double get_rotation_fraction() const;
    GamePhase get_phase() const { return phase_; }
    int  get_turn()    const { return turn_; }

    /// 0 = player 1,  1 = player 2
    int  get_current_player() const { return turn_ % kNumPlayers; }

    const std::vector<Peg>&    get_pegs()    const { return pegs_; }
    const std::vector<Bridge>& get_bridges() const { return bridges_; }

    // =================================================================
    // Coordinates / Area ownership
    // =================================================================

    /** Is the position inside the rotated board area within the NxM frame? */
    bool is_in_bounds(const Position& pos) const;

    /**
     * @brief Returns which boundary area this position belongs to.
     *
     *  - Rotated inner (N-2)x(M-2) area -> kNone
     *  - Rotated boundary strips -> kTop/kBottom/kLeft/kRight
     *  - Overlapping corner strips -> kNone
     */
    Direction get_direction(const Position& pos) const;

    /** May player `player_id` place on this field? */
    bool is_playable(const Position& pos, int player_id) const;

    /** Is the position already occupied by a peg? */
    bool is_occupied(const Position& pos) const;

    // =================================================================
    // Move
    // =================================================================

    /**
     * @brief Checks whether a move is legal.
     *
     * Conditions:
     *  - The game has not ended yet
     *  - The move belongs to the player whose turn it is
     *  - Position is inside the grid
     *  - Position is in the player's playable area
     *  - Field is not already occupied
     */
    bool is_valid_move(preset::Move move) const;

    /**
     * @brief Applies a previously validated move.
     *
     * Places the peg, generates new bridges if applicable, and checks
     * for game end. Throws for an invalid move.
     */
    void apply_move(preset::Move move);

    // =================================================================
    // Game end
    // =================================================================

    /** Checks whether player `player_id` has connected both target sides. */
    bool check_win(int player_id) const;

    /** Checks whether the game is a draw (both players are blocked). */
    bool check_draw() const;

private:
    // =================================================================
    // Internal types
    // =================================================================

    enum class Cell : uint8_t { kEmpty, kPegP1, kPegP2 };

    struct BoardPoint {
        double x = 0.0;
        double y = 0.0;
    };

    using BoardPolygon = std::array<BoardPoint, 4>;

    // =================================================================
    // Helper functions
    // =================================================================

    /** Knight-move offsets (8 possible neighboring positions). */
    static std::vector<Position> knight_offsets();

    /** Returns all knight-move positions that are inside the rotated board. */
    std::vector<Position> knight_neighbors(const Position& pos) const;

    bool is_grid_coordinate(const Position& pos) const;
    BoardPolygon rotated_polygon(double inset) const;
    bool point_in_polygon(const Position& pos, const BoardPolygon& polygon) const;
    std::array<double, 4> side_values(
        const Position& pos,
        const BoardPolygon& polygon) const;
    bool is_corner(const Position& pos) const;

    /**
     * @brief Checks whether two bridges cross.
     *
     * Two segments (a1,a2) and (b1,b2) cross if they intersect at a
     * non-endpoint.
     */
    static bool bridges_cross(const Bridge& a, const Bridge& b);

    /**
     * @brief Checks whether a new bridge crosses an existing one.
     */
    bool would_cross_existing(const Bridge& candidate) const;

    /**
     * @brief Builds new bridges after placing a peg.
     *
     * Scans all of the player's pegs, checks knight-move adjacency, and
     * creates bridges that do not cross any existing bridge.
     */
    void generate_bridges(const Peg& new_peg);

    /**
     * @brief Graph-based win check.
     *
     * Player 1 wins if a continuous chain of bridges connects the top side
     * to the bottom side. Player 2 wins analogously from left to right.
     */
    bool is_connected_across(int player_id) const;

    /**
     * @brief Checks whether any connection between a player's target sides
     *        is still possible.
     *
     * The player's own pegs and free fields playable by that player are
     * treated as potential nodes. Edges are only possible if a future
     * knight-move bridge would not cross an existing bridge.
     */
    bool has_potential_connection(int player_id) const;

    // =================================================================
    // Data
    // =================================================================

    int width_;
    int height_;
    double rotation_;
    GamePhase phase_ = GamePhase::kNotStarted;
    int turn_ = 0;

    /// 2D grid: grid_[row][col]
    std::vector<std::vector<Cell>> grid_;

    std::vector<Peg>    pegs_;
    std::vector<Bridge> bridges_;

    // Lists separated by player ID for fast access
    std::vector<Peg>    pegs_by_player_[kNumPlayers];
    std::vector<Bridge> bridges_by_player_[kNumPlayers];
};

}  // namespace bruecken
