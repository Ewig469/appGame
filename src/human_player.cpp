/**
 * @file human_player.cpp
 * @brief Implementation of the HumanPlayer class.
 *
 * This file implements a human-controlled player for the Bruecken game.
 * The player receives moves from the graphical user interface (GUI)
 * and maintains a private copy of the game board in order to verify
 * and synchronize all moves independently.
 *
 * Author: Zhixin Fu
 */

#include "bruecken/human_player.h"

#include <memory>
#include <stdexcept>

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

        if (player_id != 1 && player_id != 2)
        {
            throw std::invalid_argument("player_id must be 1 or 2");
        }

        board_ = std::make_unique<Board>(
            board_config.width,
            board_config.height,
            board_config.rotation);

        player_id_ = player_id;

        initialized_ = true;
    }

    /**
     * @brief Requests a move from the GUI.
     *
     * If the GUI provides a move, it is validated using the
     * player's private board. Valid moves are applied locally
     * before being returned.
     *
     * @return A valid move or std::nullopt if no valid move is available.
     */
    std::optional<preset::Move> HumanPlayer::request()
    {
        if (!initialized_)
        {
            throw std::runtime_error("HumanPlayer not initialized");
        }

        auto move = gui_.request_move_from_current_human_player();

        if (!move.has_value())
        {
            return std::nullopt;
        }

        if (!board_->is_valid_move(*move))
        {
            return std::nullopt;
        }

        board_->apply_move(*move);

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
     * @throws std::runtime_error If the opponent move is invalid.
     */
    void HumanPlayer::update(preset::Move opponent_move)
    {
        if (!initialized_)
        {
            throw std::runtime_error("HumanPlayer not initialized");
        }

        if (!board_->is_valid_move(opponent_move))
        {
            throw std::runtime_error("Opponent move is invalid");
        }

        board_->apply_move(opponent_move);
    }

} // namespace bruecken
