/**
 * @file board_tests.cpp
 * @brief Tests for the board class.
 * @author Zhibo Zhang
 */

#include "bruecken/board.h"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "move.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(double first, double second, double tolerance = 0.000001) {
    return std::abs(first - second) <= tolerance;
}

template <typename Fn>
void require_throws(Fn&& fn, const std::string& message) {
    try {
        fn();
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error(message);
}

void play(bruecken::Board& board, int x, int y, int player_id) {
    board.apply_move(preset::Move(x, y, player_id));
}

void test_board_boundary_values() {
    bruecken::Board min_board(5, 5, 0.0);
    require(min_board.get_width() == 5, "5x5 board should be accepted");
    require(near(min_board.get_rotation_fraction(), 0.0),
            "0 degree rotation should place the board on the unrotated corners");

    bruecken::Board tall_board(5, 96, 90.0);
    require(tall_board.get_height() == 96, "5x96 board should be accepted");
    require(near(tall_board.get_rotation_fraction(), 1.0),
            "90 degree rotation should place the board on the rotated corners");

    bruecken::Board wide_board(96, 5, 45.0);
    require(wide_board.get_width() == 96, "96x5 board should be accepted");
    require(near(wide_board.get_rotation_fraction(), 0.5),
            "45 degree rotation should place the board corners halfway");

    bruecken::Board max_board(96, 96, 90.0);
    require(max_board.get_height() == 96, "96x96 board should be accepted");

    require_throws([] { bruecken::Board board(4, 5); },
                   "width below 5 should be rejected");
    require_throws([] { bruecken::Board board(5, 4); },
                   "height below 5 should be rejected");
    require_throws([] { bruecken::Board board(97, 5); },
                   "width above 96 should be rejected");
    require_throws([] { bruecken::Board board(5, 97); },
                   "height above 96 should be rejected");
    require_throws([] { bruecken::Board board(5, 5, -0.1); },
                   "negative rotation should be rejected");
    require_throws([] { bruecken::Board board(5, 5, 90.1); },
                   "rotation above 90 should be rejected");
}

void test_invalid_player_id_is_rejected() {
    bruecken::Board board(5, 5);

    require(!board.is_valid_move(preset::Move(2, 2, 0)),
            "player id 0 should be invalid");
    require(!board.is_valid_move(preset::Move(2, 2, 3)),
            "player id 3 should be invalid");
    require(!board.is_valid_move(preset::Move(2, 2, 2)),
            "player 2 should not be accepted on player 1's turn");

    require_throws([&] { play(board, 2, 2, 0); },
                   "applying player id 0 should throw");
    require_throws([&] { play(board, 2, 2, 3); },
                   "applying player id 3 should throw");
}

void test_unrotated_corner_areas_are_not_playable() {
    bruecken::Board board(5, 5);

    const std::vector<bruecken::Position> corner_positions = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1},
        {3, 0}, {4, 0}, {3, 1}, {4, 1},
        {0, 3}, {1, 3}, {0, 4}, {1, 4},
        {3, 3}, {4, 3}, {3, 4}, {4, 4},
    };

    for (const bruecken::Position& pos : corner_positions) {
        require(!board.is_playable(pos, 0),
                "Corner areas must not be playable by player 1");
        require(!board.is_playable(pos, 1),
                "Corner areas must not be playable by player 2");
    }

    require(board.get_direction({2, 0}) == bruecken::Direction::kTop,
            "The top edge strip must still belong to player 1");
    require(board.get_direction({2, 1}) == bruecken::Direction::kTop,
            "The inner top edge line belongs to the top boundary strip");
    require(board.get_direction({0, 2}) == bruecken::Direction::kLeft,
            "The left edge strip must still belong to player 2");
    require(board.get_direction({1, 2}) == bruecken::Direction::kLeft,
            "The inner left edge line belongs to the left boundary strip");

    require(board.is_playable({2, 0}, 0),
            "Player 1 must be allowed to play on the top boundary");
    require(!board.is_playable({2, 0}, 1),
            "Player 2 must not play on player 1's top boundary");
    require(board.is_playable({0, 2}, 1),
            "Player 2 must be allowed to play on the left boundary");
    require(!board.is_playable({0, 2}, 0),
            "Player 1 must not play on player 2's left boundary");
}

void test_rotation_scales_board_inside_fixed_grid() {
    bruecken::Board board(15, 15, 30.0);

    require(!board.is_in_bounds({0, 0}),
            "A fixed-grid outer corner can be outside the rotated board");
    require(!board.is_playable({0, 0}, 0),
            "A point outside the rotated board must not be playable");
    require(board.is_in_bounds({7, 7}),
            "The fixed-grid center must remain inside the rotated board");
    require(board.is_playable({7, 7}, 0),
            "A neutral center point must be playable by player 1");
    require(board.is_playable({7, 7}, 1),
            "A neutral center point must be playable by player 2");
}

void test_crossing_bridge_is_not_created() {
    bruecken::Board board(7, 7);

    play(board, 2, 1, 1);
    play(board, 2, 2, 2);
    play(board, 3, 3, 1);
    require(board.get_bridges().size() == 1,
            "first knight connection should create one bridge");

    play(board, 4, 3, 2);
    require(board.get_bridges().size() == 1,
            "crossing bridge should not be created");
}

void test_shared_endpoint_is_not_a_crossing() {
    bruecken::Board board(7, 7);

    play(board, 2, 1, 1);
    play(board, 0, 2, 2);
    play(board, 3, 3, 1);
    play(board, 0, 3, 2);
    play(board, 4, 1, 1);

    require(board.get_bridges().size() == 2,
            "bridges sharing an endpoint should both be created");
}

void test_non_crossing_bridge_is_created() {
    bruecken::Board board(7, 7);

    play(board, 2, 1, 1);
    play(board, 0, 2, 2);
    play(board, 4, 1, 1);
    play(board, 2, 3, 2);
    require(board.get_bridges().size() == 1,
            "first player 2 bridge should be created");

    play(board, 3, 3, 1);
    require(board.get_bridges().size() == 3,
            "non-crossing bridges should be created");
}

void test_finished_board_rejects_more_moves() {
    bruecken::Board board(7, 7);

    play(board, 2, 0, 1);
    play(board, 6, 2, 2);
    play(board, 3, 2, 1);
    play(board, 6, 4, 2);
    play(board, 4, 4, 1);
    play(board, 0, 2, 2);
    play(board, 3, 6, 1);

    require(board.get_phase() == bruecken::GamePhase::kFinished,
            "the prepared position should finish the game");
    require(!board.is_valid_move(preset::Move(2, 3, 2)),
            "no move should be valid after the game is finished");
    require_throws([&] { play(board, 2, 3, 2); },
                   "applying a move after game end should throw");
}

}  // namespace

int main() {
    const std::vector<void (*)()> tests = {
        test_board_boundary_values,
        test_invalid_player_id_is_rejected,
        test_unrotated_corner_areas_are_not_playable,
        test_rotation_scales_board_inside_fixed_grid,
        test_crossing_bridge_is_not_created,
        test_shared_endpoint_is_not_a_crossing,
        test_non_crossing_bridge_is_created,
        test_finished_board_rejects_more_moves,
    };

    try {
        for (const auto& test : tests) {
            test();
        }
    } catch (const std::exception& ex) {
        std::cerr << "board_tests failed: " << ex.what() << '\n';
        return 1;
    }

    std::cout << "board_tests passed\n";
    return 0;
}
