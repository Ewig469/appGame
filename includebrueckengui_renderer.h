/**
 * @file gui_renderer.h
 * @brief Raylib GUI for the Bruecken game.
 * @author Hao Guo
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "bruecken/board.h"
#include "player_gui_access.h"

namespace bruecken {

/**
 * @brief Draws the board and provides mouse input to HumanPlayer.
 */
class GuiRenderer final : public preset::PlayerGuiAccess {
public:
    GuiRenderer(
        const Board& board,
        std::vector<std::string> player_colors,
        std::vector<std::string> player_names,
        std::string title = "BrueckenSpiel");

    ~GuiRenderer();

    GuiRenderer(const GuiRenderer&) = delete;
    GuiRenderer& operator=(const GuiRenderer&) = delete;

    /** Draws one frame and processes a possible mouse click. */
    void draw_frame();

    /** Returns true if the user wants to close the window. */
    bool should_close() const;

    /**
     * Returns the last valid click to HumanPlayer.
     * Every click is returned only once.
     */
    std::optional<preset::Move>
    request_move_from_current_human_player() override;

private:
    const Board& board_;
    std::vector<std::string> player_colors_;
    std::vector<std::string> player_names_;

    std::optional<preset::Move> pending_move_;
    std::string feedback_;

    int input_turn_ = -1;
    bool window_open_ = false;
};

}  // namespace bruecken