#include "bruecken/board.h"

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

    bruecken::Board tall_board(5, 96, 90.0);
    require(tall_board.get_height() == 96, "5x96 board should be accepted");

    bruecken::Board wide_board(96, 5, 45.0);
    require(wide_board.get_width() == 96, "96x5 board should be accepted");

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

    require_throws([&] { play(board, 2, 2, 0); },
                   "applying player id 0 should throw");
    require_throws([&] { play(board, 2, 2, 3); },
                   "applying player id 3 should throw");
}

void test_crossing_bridge_is_not_created() {
    bruecken::Board board(5, 5);

    play(board, 1, 0, 1);
    play(board, 2, 2, 1);
    require(board.get_bridges().size() == 1,
            "first knight connection should create one bridge");

    play(board, 0, 2, 2);
    play(board, 2, 1, 2);
    require(board.get_bridges().size() == 1,
            "crossing bridge should not be created");
}

void test_shared_endpoint_is_not_a_crossing() {
    bruecken::Board board(5, 5);

    play(board, 1, 0, 1);
    play(board, 2, 2, 1);
    play(board, 3, 1, 1);

    require(board.get_bridges().size() == 2,
            "bridges sharing an endpoint should both be created");
}

void test_non_crossing_bridge_is_created() {
    bruecken::Board board(5, 5);

    play(board, 0, 1, 2);
    play(board, 2, 2, 2);
    require(board.get_bridges().size() == 1,
            "first player 2 bridge should be created");

    play(board, 1, 0, 1);
    play(board, 3, 1, 1);
    require(board.get_bridges().size() == 2,
            "non-crossing bridge should be created");
}

void test_draw_when_both_routes_are_impossible_before_board_is_full() {
    bruecken::Board board(5, 5);

    const std::vector<preset::Move> moves = {
        {0, 3, 2}, {3, 1, 2}, {1, 3, 1}, {2, 0, 1},
        {3, 3, 2}, {1, 2, 2}, {2, 4, 1}, {1, 0, 1},
        {4, 2, 2}, {4, 3, 2}, {0, 1, 2}, {3, 0, 1},
        {0, 2, 2}, {2, 1, 2}, {2, 2, 2}, {3, 2, 1},
        {2, 3, 2}, {1, 1, 1}, {3, 4, 1},
    };

    for (const auto& move : moves) {
        board.apply_move(move);
        if (board.get_phase() == bruecken::GamePhase::kDraw) {
            break;
        }
    }

    require(board.get_phase() == bruecken::GamePhase::kDraw,
            "blocked routes should produce a draw");
    require(board.get_pegs().size() < 21,
            "draw test should not rely on a completely full 5x5 board");
}

}  // namespace

int main() {
    const std::vector<void (*)()> tests = {
        test_board_boundary_values,
        test_invalid_player_id_is_rejected,
        test_crossing_bridge_is_not_created,
        test_shared_endpoint_is_not_a_crossing,
        test_non_crossing_bridge_is_created,
        test_draw_when_both_routes_are_impossible_before_board_is_full,
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
