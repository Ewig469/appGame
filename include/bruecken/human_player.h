/**
 * @file human_player.h
 * @brief Declaration of the HumanPlayer class.
 *
 * This file defines a human-controlled player for the Bridges game.
 *
 * @author Zhixin Fu
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "player.h"
#include "player_gui_access.h"
#include "bruecken/board.h"

namespace bruecken
{

    /**
     * @class HumanPlayer
     * @brief Human-controlled implementation of preset::Player.
     *
     * The player receives moves from the GUI and keeps a private
     * copy of the game board to verify all moves independently.
     */
    class HumanPlayer : public preset::Player
    {
    public:
        explicit HumanPlayer(preset::PlayerGuiAccess &gui);

        std::string_view get_name() override;

        void init(
            const preset::BoardConfig &board_config,
            int player_id) override;

        std::optional<preset::Move> request() override;

        void update(preset::Move opponent_move) override;

    private:
        /// GUI interface.
        preset::PlayerGuiAccess &gui_;

        /// Private board.
        std::unique_ptr<Board> board_;

        /// Player name.
        std::string name_ = "Human Player";

        /// Player ID (1 or 2).
        int player_id_ = -1;

        /// True after init() has been called.
        bool initialized_ = false;

        /// True if it is currently this player's turn.
        bool my_turn_ = false;

        /// True after the game has ended.
        bool game_finished_ = false;

        /// Expected number of processed turns.
        int expected_turn_ = 0;
    };

} // namespace bruecken