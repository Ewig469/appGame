/**
 * @file test_invalid_move.cpp
 * @brief Tests rejection of invalid identities, turns, and coordinates.
 * @author Zhixin Fu
 */

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "board_config.h"
#include "move.h"

#include "bruecken/board.h"
#include "bruecken/random_ai_player.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void require_exception(
    const std::function<void()>& operation,
    const std::string& message) {

    try {
        operation();
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error(message);
}

}  // namespace

int main() {
    try {
        preset::BoardConfig config;
        config.width = 5;
        config.height = 5;

        bruecken::Board board(config.width, config.height, config.rotation);
        require(!board.is_valid_move(preset::Move(1, 1, 0)),
                "Board must reject player ID 0");
        require(!board.is_valid_move(preset::Move(1, 1, 3)),
                "Board must reject player ID 3");
        require(!board.is_valid_move(preset::Move(1, 1, 2)),
                "Board must reject a move made out of turn");
        require(!board.is_valid_move(preset::Move(999, 999, 1)),
                "Board must reject coordinates outside the board");
        require_exception(
            [&] { board.apply_move(preset::Move(1, 1, 0)); },
            "Board must throw when applying an invalid player ID");
        require(board.is_valid_move(preset::Move(1, 1, 1)),
                "A rejected move must not mutate the board");

        board.apply_move(preset::Move(1, 1, 1));
        require(!board.is_valid_move(preset::Move(2, 2, 1)),
                "Board must reject two consecutive moves by one player");
        require(board.is_valid_move(preset::Move(2, 2, 2)),
                "Board must accept the next player's legal move");

        bruecken::RandomAIPlayer player_two;
        player_two.init(config, 2);
        require_exception(
            [&] { player_two.update(preset::Move(1, 1, 0)); },
            "Player must reject an opponent ID outside 1..2");
        require_exception(
            [&] { player_two.update(preset::Move(999, 999, 1)); },
            "Player must reject an opponent move outside the board");
        require_exception(
            [&] { player_two.update(preset::Move(1, 1, 2)); },
            "Player must reject its own ID as an opponent move");

        player_two.update(preset::Move(1, 1, 1));
        require_exception(
            [&] { player_two.update(preset::Move(2, 2, 1)); },
            "Player must reject a duplicate update in the same turn");

        std::cout << "All invalid-move tests passed.\n";
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "Invalid-move test failed: "
                  << exception.what() << '\n';
        return 1;
    }
}
