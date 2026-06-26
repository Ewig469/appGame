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

namespace bruecken {

/**
 * @class RandomAIPlayer
 * @brief Random computer-controlled implementation of preset::Player.
 *
 * The RandomAIPlayer searches all legal moves on the board
 * and randomly selects one of them.
 *
 * Like every player implementation, it maintains a private
 * copy of the game board in order to independently verify
 * the legality of all moves.
 */
class RandomAIPlayer : public preset::Player {
public:

    /**
     * @brief Creates a new RandomAIPlayer.
     *
     * Initializes the random number generator.
     */
    RandomAIPlayer();

    /**
     * @brief Returns the player's display name.
     *
     * @return Player name.
     */
    std::string_view get_name() override;

    /**
     * @brief Initializes the player.
     *
     * Creates the private board and stores the player's ID.
     *
     * @param board_config Board configuration.
     * @param player_id Player ID (1 or 2).
     */
    void init(
        const preset::BoardConfig& board_config,
        int player_id) override;


    /**
     * @brief Selects a random valid move.
     *
     * All legal moves are collected and one of them is
     * chosen uniformly at random.
     *
     * @return Selected move or std::nullopt if no legal move exists.
     */
    std::optional<preset::Move> request() override;



    /**
     * @brief Updates the private board with the opponent's move.
     *
     * @param opponent_move Move performed by the opponent.
     */
    void update(
        preset::Move opponent_move) override;

private:

    /// Private board used for local game state validation.
    std::unique_ptr<Board> board_;

    /// Indicates whether init() has already been called.
    bool initialized_ = false;

    /// ID of this player.
    int player_id_ = -1;

    /// Random number generator, Mersenne Twister
    std::mt19937 rng_;

    /// Display name of the player.
    std::string name_ = "Random AI";
};

} // namespace bruecken