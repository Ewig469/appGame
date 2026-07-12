/**
 * @file human_player.cpp
 * @brief Implementation of the HumanPlayer class.
 *
 * This file implements a human-controlled player for the Bruecken game.
 * The player receives moves from the graphical user interface (GUI)
 * and maintains a private copy of the game board in order to verify
 * and synchronize all moves independently.
 *
 * @author Zhixin Fu
 */

#include "bruecken/human_player.h"

#include <memory>
#include <stdexcept>
#include <string_view>

namespace bruecken
{
    /**
     * @brief Constructs a human player.
     *
     * Stores a reference to the GUI interface used to obtain
     * moves from the user.
     *
     * @param gui Reference to the GUI.
     */
    HumanPlayer::HumanPlayer(preset::PlayerGuiAccess &gui)
        : gui_(gui)
    {
    }

    /**
     * @brief Returns the player's name.
     *
     * @return Name of the player.
     */
    std::string_view HumanPlayer::get_name()
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
    void HumanPlayer::init(const preset::BoardConfig &board_config, int player_id)
    {
        if (initialized_)
        {
            throw std::runtime_error("HumanPlayer already initialized");
        }

        // Only player IDs 1 and 2 are valid.
        if (player_id != 1 && player_id != 2)
        {
            throw std::invalid_argument("player_id must be 1 or 2");
        }

        // Create the player's private copy of the game board.
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

        // Store the assigned player ID.
        player_id_ = player_id;

        // Initialize the internal player state.
        initialized_ = true;
        my_turn_ = (player_id == 1);
        game_finished_ = false;

        // At the beginning of the game no move has been played yet.
        expected_turn_ = 0;
    }

    /**
     * @brief Requests a move from the GUI.
     *
     * If the GUI provides a move, it is validated using the
     * player's private board. Valid moves are applied locally
     * before being returned.
     *
     * @return A valid move or std::nullopt if the GUI has no input yet.
     * @throws std::logic_error If called outside this player's turn or after
     *         the game has ended.
     * @throws std::runtime_error If the GUI returns an illegal move.
     */
    std::optional<preset::Move> HumanPlayer::request()
    {
        // The player must be initialized before requesting moves.
        if (!initialized_)
        {
            throw std::logic_error("Player not initialized.");
        }

        // No move can be requested after the game has finished.
        if (game_finished_)
        {
            throw std::logic_error("Game already finished.");
        }

        // request() may only be called during this player's turn.
        if (!my_turn_)
        {
            throw std::logic_error("It is not this player's turn.");
        }

        // Verify that the private board is synchronized.
        if (board_->get_turn() != expected_turn_)
        {
            throw std::logic_error("Board synchronization failed.");
        }

        // Ask the GUI for the next move.
        auto move = gui_.request_move_from_current_human_player();

        // No move has been entered yet.
        if (!move.has_value())
        {
            return std::nullopt;
        }

        // The GUI must return a move for this player.
        if (move->get_id() != player_id_)
        {
            throw std::runtime_error(
                "GUI returned a move for the wrong player");
        }

        // Validate the move using the private board.
        if (!board_->is_valid_move(*move))
        {
            throw std::runtime_error(
                "GUI returned an illegal move");
        }

        // Apply the move to the private board.
        board_->apply_move(*move);

        // One additional move has now been executed.
        expected_turn_++;

        // Verify that the board state is still synchronized.
        if (board_->get_turn() != expected_turn_)
        {
            throw std::logic_error(
                "Board synchronization failed.");
        }

        // Remember whether the game has finished.
        if (board_->get_phase() != GamePhase::kInProgress)
        {
            game_finished_ = true;
        }

        // The opponent moves next.
        my_turn_ = false;

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
     * @throws std::runtime_error If the player is not initialized.
     * @throws std::logic_error If called during this player's turn or after
     *         the game has ended.
     * @throws std::invalid_argument If the opponent move is invalid.
     */
    void HumanPlayer::update(preset::Move opponent_move)
    {
        // The player must be initialized first.
        if (!initialized_)
        {
            throw std::logic_error("Player not initialized.");
        }

        // No updates are accepted after the game has ended.
        if (game_finished_)
        {
            throw std::logic_error("Game already finished.");
        }

        // update() may only be called while waiting for the opponent.
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

        // Verify the legality of the opponent's move.
        if (!board_->is_valid_move(opponent_move))
        {
            throw std::invalid_argument(
                "Opponent move is invalid.");
        }

        // Apply the opponent's move to the private board.
        board_->apply_move(opponent_move);

        // One additional move has now been executed.
        expected_turn_++;

        // Verify that the board state is still synchronized.
        if (board_->get_turn() != expected_turn_)
        {
            throw std::logic_error(
                "Board synchronization failed.");
        }

        // Remember whether the game has finished.
        if (board_->get_phase() != GamePhase::kInProgress)
        {
            game_finished_ = true;
        }
        else
        {
            my_turn_ = true;
        }
    }

} // namespace bruecken
