/**
 * @file random_ai_player.cpp
 * @brief Implementation of the RandomAIPlayer class.
 *
 * This file implements a computer-controlled player that
 * selects valid moves randomly.
 *
 * The player maintains its own private copy of the game
 * board in order to validate moves and keep its internal
 * game state synchronized with the actual game.
 *
 * Author: Zhixin Fu
 */

#include "bruecken/random_ai_player.h"

#include <chrono>
#include <stdexcept>
#include <vector>
#include <random>
#include <memory>

namespace bruecken
{
    /**
     * @brief Constructs a random AI player.
     *
     * Initializes the random number generator using the
     * current system time.
     */
    RandomAIPlayer::RandomAIPlayer()
    {
        rng_.seed(
            static_cast<unsigned>(
                std::chrono::steady_clock::now()
                    .time_since_epoch()
                    .count()));
    }


    /**
     * @brief Returns the player's name.
     *
     * @return Name of the player.
     */
    std::string_view RandomAIPlayer::get_name()
    {
        return name_;
    }



    /**
     * @brief Initializes the player.
     *
     * Creates the player's private board and stores
     * the assigned player ID.
     *
     * @param board_config Board configuration.
     * @param player_id Player identifier (1 or 2).
     *
     * @throws std::runtime_error If the player has already been initialized.
     * @throws std::invalid_argument If the player ID is invalid.
     */
    void RandomAIPlayer::init(
        const preset::BoardConfig &board_config,
        int player_id)
    {
        if (initialized_)
        {
            throw std::runtime_error(
                "RandomAIPlayer already initialized");
        }

        if (player_id != 1 && player_id != 2)
        {
            throw std::invalid_argument(
                "player_id must be 1 or 2");
        }

        board_ = std::make_unique<Board>(
            board_config.width,
            board_config.height,
            board_config.rotation);

        player_id_ = player_id;

        initialized_ = true;
    }

    /**
     * @brief Selects a random valid move.
     *
     * All legal moves are collected from the current board.
     * One move is then selected uniformly at random and
     * applied to the player's private board.
     *
     * @return A randomly selected legal move or std::nullopt
     *         if no legal moves are available.
     */
    std::optional<preset::Move> RandomAIPlayer::request()
    {
        if (!initialized_)
        {
            throw std::runtime_error(
                "RandomAIPlayer not initialized");
        }

        std::vector<preset::Move> legal_moves;

        for (int y = 0; y < board_->get_height(); ++y)
        {
            for (int x = 0; x < board_->get_width(); ++x)
            {

                preset::Move move(
                    x,
                    y,
                    player_id_);

                if (board_->is_valid_move(move))
                {
                    legal_moves.push_back(move);
                }
            }
        }

        if (legal_moves.empty())
        {
            return std::nullopt;
        }

        std::uniform_int_distribution<std::size_t> dist(
            0,
            legal_moves.size() - 1);

        preset::Move move = legal_moves[dist(rng_)];

        board_->apply_move(move);

        return move;
    }

    /**
     * @brief Updates the local board with the opponent's move.
     *
     * The received move is validated before it is applied to
     * the player's private board.
     *
     * @param opponent_move The move performed by the opponent.
     *
     * @throws std::runtime_error If the player has not been initialized.
     * @throws std::runtime_error If the opponent move is invalid.
     */
    void RandomAIPlayer::update(
        preset::Move opponent_move)
    {
        if (!initialized_)
        {
            throw std::runtime_error(
                "RandomAIPlayer not initialized");
        }

        if (!board_->is_valid_move(opponent_move))
        {
            throw std::runtime_error(
                "Opponent move is invalid");
        }

        board_->apply_move(opponent_move);
    }

}
