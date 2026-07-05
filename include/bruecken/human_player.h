/**
 * @file human_player.h
 * @brief Declaration of the HumanPlayer class.
 *
 * This file defines the implementation of a human-controlled player
 * for the Bridges game.
 *
 * Responsibilities:
 * - Receive moves from the GUI
 * - Maintain a private copy of the game board
 * - Validate both own and opponent moves
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

namespace bruecken {

/**
 * @class HumanPlayer
 * @brief Human-controlled implementation of preset::Player.
 *
 * The HumanPlayer receives moves from the graphical user interface.
 * Each player owns a private copy of the game board, which is updated
 * after every move. This allows the player to independently verify the
 * legality of all moves without accessing the game's main board.
 */
class HumanPlayer : public preset::Player {
public:

    /**
     * @brief Creates a new HumanPlayer.
     *
     * @param gui Reference to the GUI interface used to request moves.
     */
    explicit HumanPlayer(
        preset::PlayerGuiAccess& gui);

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
     * This function may only be called once.
     *
     * @param board_config Board configuration.
     * @param player_id Player ID (1 or 2).
     */
    void init(
        const preset::BoardConfig& board_config,
        int player_id) override;

    /**
     * @brief Requests the next move from the GUI.
     *
     * The move is validated on the private board before it is returned.
     * The function may be polled while no GUI move is available. After a
     * move is accepted, another request is forbidden until the opponent has
     * moved.
     *
     * @return A valid move or std::nullopt if no move is available.
     * @throws std::logic_error If it is not this player's turn or the game
     *         has already ended.
     * @throws std::runtime_error If the GUI supplies an invalid move.
     */
    std::optional<preset::Move> request() override;

    /**
     * @brief Updates the private board with the opponent's move.
     *
     * @param opponent_move Move performed by the opponent.
     * @throws std::logic_error If called during this player's turn or after
     *         the game has ended.
     * @throws std::invalid_argument If the move has an invalid identity,
     *         position, or rule violation.
     */
    void update(
        preset::Move opponent_move) override;

private:

    /// Reference to the GUI interface.
    preset::PlayerGuiAccess& gui_;

    /// Private board used for local game state validation.
    std::unique_ptr<Board> board_;

    /// Indicates whether init() has already been called.
    bool initialized_ = false;

    /// ID of this player.
    int player_id_ = -1;

    /// Display name of the player.
    std::string name_ = "Human Player";
};

} // namespace bruecken
