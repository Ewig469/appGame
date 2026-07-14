/**
 * @file gui_logic.h
 * @brief Testable geometry, interaction, and status logic used by the GUI.
 * @author Hao Guo
 * @author Junke Pu
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "bruecken/board.h"
#include "move.h"

namespace bruecken {

/** A renderer-independent point in screen coordinates. */
struct GuiPoint {
    float x = 0.0F;
    float y = 0.0F;
};

/** Complete screen-space geometry of a possibly rotated board. */
struct GuiBoardGeometry {
    GuiPoint top_left;
    GuiPoint top_right;
    GuiPoint bottom_left;
    GuiPoint bottom_right;
    GuiPoint x_step;
    GuiPoint y_step;
    GuiPoint center;
};

/**
 * Calculate board corners and grid steps according to the school formula.
 * @par Primary contributor
 * Hao Guo (rotation and board rendering geometry).
 */
GuiBoardGeometry calculate_board_geometry(
    const Board& board,
    float screen_width,
    float screen_height);

/** Convert a logical board coordinate into a screen coordinate. */
GuiPoint board_coordinate_to_screen(
    const GuiBoardGeometry& geometry,
    int x,
    int y);

/**
 * Return the board point under the mouse, if one is close enough.
 * @par Primary contributor
 * Junke Pu (mouse interaction).
 */
std::optional<Position> find_nearest_board_position(
    const GuiBoardGeometry& geometry,
    const Board& board,
    GuiPoint mouse);

/** Convert a clicked coordinate into a valid move for the current player. */
std::optional<preset::Move> move_for_board_position(
    const Board& board,
    Position position);

/**
 * Return readable persistent axis labels, always including both endpoints.
 * Exact coordinates between these labels are exposed by the hover tooltip.
 * @par Primary contributor
 * Hao Guo (coordinate rendering).
 */
std::vector<int> coordinate_label_values(
    int coordinate_count,
    float point_spacing);

/**
 * Reconstruct one winning bridge path for a player.
 * @par Primary contributor
 * Junke Pu (winning-state visualization).
 */
std::vector<Position> calculate_winning_path(
    const Board& board,
    int player_id);

/**
 * Build the status text shown above the board.
 * @par Primary contributor
 * Junke Pu (game-state display).
 */
std::string game_status_text(
    const Board& board,
    const std::vector<std::string>& player_names);

}  // namespace bruecken
