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
 * @author Zhixin Fu
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
     * Creates the player's private board, stores the assigned player ID,
     * and initializes the internal player state.
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

        // Only player IDs 1 and 2 are valid.
        if (player_id != 1 && player_id != 2)
        {
            throw std::invalid_argument(
                "player_id must be 1 or 2");
        }

        board_ = std::make_unique<Board>(
            board_config.width,
            board_config.height,
            board_config.rotation);

        // Verify that the board was initialized correctly.
        if (board_->get_width() != board_config.width ||
            board_->get_height() != board_config.height)
        {
            throw std::runtime_error("Board initialization failed.");
        }

        player_id_ = player_id;

        // Initialize the internal player state.
        initialized_ = true;
        my_turn_ = (player_id == 1);
        game_finished_ = false;

        // At the beginning of the game no move has been played yet.
        expected_turn_ = 0;
    }

    /**
     * @brief Generates the next move.
     *
     * Searches all legal moves on the private board,
     * randomly selects one of them, applies it to the
     * private board and returns it.
     *
     * If no legal move exists, std::nullopt is returned.
     *
     * @return A randomly selected legal move or std::nullopt.
     *
     * @throws std::logic_error
     *         If the player has not been initialized,
     *         the game has already finished,
     *         or it is not this player's turn.
     */
    std::optional<preset::Move> RandomAIPlayer::request()
    {
        // The player must be initialized before requesting moves.
        if (!initialized_)
        {
            throw std::logic_error("Player not initialized.");
        }

        // No further moves are allowed after the game has ended.
        if (game_finished_)
        {
            throw std::logic_error("Game already finished.");
        }

        // request() may only be called during this player's turn.
        if (!my_turn_)
        {
            throw std::logic_error("It is not this player's turn.");
        }

        // request() may only be called during this player's turn.
        if (board_->get_turn() != expected_turn_)
        {
            throw std::logic_error("Board synchronization failed.");
        }

        // Collect every legal move on the current board.
        std::vector<preset::Move> legal_moves;

        for (int y = 0; y < board_->get_height(); ++y)
        {
            for (int x = 0; x < board_->get_width(); ++x)
            {

                preset::Move move(
                    x,
                    y,
                    player_id_);

                // Only legal moves are considered.
                if (board_->is_valid_move(move))
                {
                    legal_moves.push_back(move);
                }
            }
        }

        // No legal move is available.
        if (legal_moves.empty())
        {
            return std::nullopt;
        }

        // Select one legal move uniformly at random.
        std::uniform_int_distribution<std::size_t> dist(
            0,
            legal_moves.size() - 1);

        preset::Move move = legal_moves[dist(rng_)];

        // Apply the selected move to the private board.
        board_->apply_move(move);

        // One move has been executed successfully.
        expected_turn_++;

        // Verify that the private board remains synchronized.
        if (board_->get_turn() != expected_turn_)
        {
            throw std::logic_error("Board synchronization failed.");
        }

        // Check whether the move finished the game.
        if (board_->get_phase() != GamePhase::kInProgress)
        {
            game_finished_ = true;
        }

        // The opponent moves next.
        my_turn_ = false;

        return move;
    }

    /**
     * @brief Applies the opponent's move to the private board.
     *
     * The move is verified before being applied. If the move
     * violates the game rules or belongs to the wrong player,
     * an exception is thrown.
     *
     * @param opponent_move Move performed by the opponent.
     *
     * @throws std::logic_error
     *         If the player has not been initialized,
     *         the game has already finished,
     *         or update() is called during this player's turn.
     *
     * @throws std::invalid_argument
     *         If the opponent move is invalid.
     */
    void RandomAIPlayer::update(preset::Move opponent_move)
    {
        // The player must be initialized.
        if (!initialized_)
        {
            throw std::logic_error("Player not initialized.");
        }

        // Ignore updates after the game has finished.
        if (game_finished_)
        {
            throw std::logic_error("Game already finished.");
        }

        // update() is only valid while waiting for the opponent.
        if (my_turn_)
        {
            throw std::logic_error("Update called during own turn.");
        }

        // The received move must belong to the opponent.
        if (opponent_move.get_id() == player_id_)
        {
            throw std::invalid_argument(
                "Received own move as opponent move.");
        }

        // Verify the move using the private board.
        if (!board_->is_valid_move(opponent_move))
        {
            throw std::invalid_argument(
                "Opponent move is invalid");
        }

        // Synchronize the private board.
        board_->apply_move(opponent_move);

        // One additional move has now been executed.
        expected_turn_++;

        // Verify that the board state is still synchronized.
        if (board_->get_turn() != expected_turn_)
        {
            throw std::logic_error("Board synchronization failed.");
        }

        // Update the internal game state.
        if (board_->get_phase() != GamePhase::kInProgress)
        {
            game_finished_ = true;
        }
        else
        {
            my_turn_ = true;
        }

    }

}
