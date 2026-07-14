/**
 * @file random_ai_player.h
 * @brief Declaration of the RandomAIPlayer class.
 *
 * This file defines a computer-controlled player that selects
 * valid moves randomly.
 *
 * Responsibilities:
 * - Generate random valid moves
 * - Maintain a private copy of the game board
 * - Validate both own and opponent moves
 *
 * @author Zhixin Fu
 */

#pragma once

#include <memory>
#include <optional>
#include <random>
#include <string>

#include "player.h"
#include "bruecken/board.h"

namespace bruecken
{

    /**
     * @class RandomAIPlayer
     * @brief Random computer-controlled implementation of preset::Player.
     *
     * The player searches all legal moves on its private board
     * and randomly selects one of them.
     */
    class RandomAIPlayer : public preset::Player
    {
    public:
        RandomAIPlayer();

        std::string_view get_name() override;

        void init(
            const preset::BoardConfig &board_config,
            int player_id) override;

        std::optional<preset::Move> request() override;

        void update(preset::Move opponent_move) override;

    private:
        /// Private board.
        std::unique_ptr<Board> board_;

        /// Random number generator.
        std::mt19937 rng_;

        /// Player name.
        std::string name_ = "Random AI";

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