/**
 * @file gui_renderer.h
 * @brief Declares the Raylib-based GUI for Knight Bridge.
 * @author Hao Guo
 * @author Junke Pu
 *
 * @par Contribution breakdown
 * - Hao Guo: window initialization and lifetime, board rendering, and the
 *   rendering of pegs and bridges.
 * - Junke Pu: mouse interaction, move placement, game-state display, and the
 *   integration of GuiRenderer with preset::PlayerGuiAccess.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "bruecken/board.h"
#include "player_gui_access.h"

namespace bruecken {

/**
 * @brief Displays the game and provides mouse moves to a human player.
 *
 * The class combines two GUI concerns. Hao Guo's rendering code owns the
 * Raylib window and visualizes the board, pegs, and bridges. Junke Pu's
 * interaction code interprets mouse clicks, displays the current game state,
 * and exposes valid moves through preset::PlayerGuiAccess.
 *
 * @par Primary contributors
 * Hao Guo (rendering) and Junke Pu (interaction and PlayerGuiAccess
 * integration).
 */
class GuiRenderer final : public preset::PlayerGuiAccess {
public:
    /**
     * @brief Creates the GUI and opens its Raylib window.
     * @param board Board whose current state is displayed. The board must
     *        outlive this renderer.
     * @param player_colors Player colors as hexadecimal RGB strings.
     * @param player_names Names shown in the game-state panel.
     * @param title Title of the operating-system window.
     *
     * @par Primary contributors
     * Hao Guo implemented window initialization and color setup; Junke Pu
     * integrated the player names used by the game-state display.
     */
    GuiRenderer(
        const Board& board,
        std::vector<std::string> player_colors,
        std::vector<std::string> player_names,
        std::string title = "Knight Bridge");

    /**
     * @brief Closes the Raylib window if it is still open.
     * @par Primary contributor
     * Hao Guo.
     */
    ~GuiRenderer();

    GuiRenderer(const GuiRenderer&) = delete;
    GuiRenderer& operator=(const GuiRenderer&) = delete;

    /**
     * @brief Processes mouse input and renders one complete GUI frame.
     *
     * Junke Pu's part handles clicks, move validation, and the status panel.
     * Hao Guo's part renders the board, coordinates, pegs, and bridges.
     * The corresponding blocks are marked in gui_renderer.cpp.
     */
    void draw_frame();

    /**
     * @brief Checks whether the GUI window should close.
     * @return `true` if the window is closed or a close request is pending.
     * @par Primary contributor
     * Hao Guo.
     */
    bool should_close() const;

    /**
     * @brief Gives the current human player's pending move to HumanPlayer.
     * @return The last valid clicked move, or std::nullopt if none is pending.
     *
     * A pending click is consumed by this call and is therefore returned at
     * most once. This override is the connection required by the school's
     * preset::PlayerGuiAccess interface.
     *
     * @par Primary contributor
     * Junke Pu.
     */
    std::optional<preset::Move>
    request_move_from_current_human_player() override;

private:
    /** Shared game model read by both rendering and interaction code. */
    const Board& board_;

    /** Colors used by Hao Guo's board, peg, and bridge rendering. */
    std::vector<std::string> player_colors_;

    /** Names used by Junke Pu's game-state display. */
    std::vector<std::string> player_names_;

    /** Valid move waiting to be consumed (Junke Pu: interaction). */
    std::optional<preset::Move> pending_move_;

    /** Message shown for the latest click (Junke Pu: interaction). */
    std::string feedback_;

    /** Turn associated with the pending click (Junke Pu: interaction). */
    int input_turn_ = -1;

    /** Tracks Raylib window ownership (Hao Guo: window lifetime). */
    bool window_open_ = false;
};

}  // namespace bruecken
