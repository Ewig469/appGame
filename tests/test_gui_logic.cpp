/**
 * @file test_gui_logic.cpp
 * @brief Tests GUI geometry, coordinate interaction, status, and winning paths.
 * @author Hao Guo
 * @author Junke Pu
 */

#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "move.h"
#include "player_gui_access.h"

#include "bruecken/board.h"
#include "bruecken/gui_logic.h"
#include "bruecken/gui_renderer.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(float first, float second, float tolerance = 0.01F) {
    return std::abs(first - second) <= tolerance;
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

void test_player_gui_access_inheritance() {
    static_assert(
        std::is_base_of_v<
            preset::PlayerGuiAccess,
            bruecken::GuiRenderer>,
        "GuiRenderer must implement preset::PlayerGuiAccess");
}

void test_rotation_geometry() {
    constexpr float width = 720.0F;
    constexpr float height = 720.0F;

    const bruecken::Board board_zero(12, 16, 0.0);
    const auto zero =
        bruecken::calculate_board_geometry(board_zero, width, height);
    require(near(zero.top_left.y, zero.top_right.y),
            "At 0 degrees the top edge must be horizontal");
    require(near(zero.top_left.x, zero.bottom_left.x),
            "At 0 degrees the left edge must be vertical");

    const bruecken::Board board_diagonal(12, 16, 45.0);
    const auto diagonal =
        bruecken::calculate_board_geometry(board_diagonal, width, height);
    require(near(diagonal.top_left.x, diagonal.center.x),
            "At 45 degrees the top corner must be horizontally centered");
    require(near(diagonal.top_right.y, diagonal.center.y),
            "At 45 degrees the right corner must be vertically centered");

    const bruecken::Board board_ninety(12, 16, 90.0);
    const auto ninety =
        bruecken::calculate_board_geometry(board_ninety, width, height);
    require(near(ninety.top_left.x, ninety.top_right.x),
            "At 90 degrees the logical top edge must be vertical");
    require(near(ninety.top_left.y, ninety.bottom_left.y),
            "At 90 degrees the logical left edge must be horizontal");

    require_exception(
        [&] {
            bruecken::calculate_board_geometry(board_zero, 0.0F, height);
        },
        "GUI geometry must reject an invalid screen size");
}

void test_large_board_coordinates() {
    const bruecken::Board board(96, 96, 35.0);
    const auto geometry =
        bruecken::calculate_board_geometry(board, 720.0F, 720.0F);

    const auto x_labels = bruecken::coordinate_label_values(
        board.get_width(),
        std::hypot(geometry.x_step.x, geometry.x_step.y));
    const auto y_labels = bruecken::coordinate_label_values(
        board.get_height(),
        std::hypot(geometry.y_step.x, geometry.y_step.y));
    require(x_labels.front() == 0 && x_labels.back() == 95,
            "Large-board X labels must include both endpoints");
    require(y_labels.front() == 0 && y_labels.back() == 95,
            "Large-board Y labels must include both endpoints");

    const std::vector<bruecken::Position> samples = {
        {0, 0}, {95, 0}, {0, 95}, {95, 95}, {47, 63}, {80, 12}};
    for (const auto& expected : samples) {
        const auto screen = bruecken::board_coordinate_to_screen(
            geometry,
            expected.x,
            expected.y);
        const auto actual = bruecken::find_nearest_board_position(
            geometry,
            board,
            screen);
        require(actual.has_value() && *actual == expected,
                "Every sampled 96 x 96 coordinate must be discoverable");
    }

    require(
        !bruecken::find_nearest_board_position(
             geometry,
             board,
             bruecken::GuiPoint{0.0F, 0.0F})
             .has_value(),
        "A click far outside the board must not select a coordinate");
}

void test_click_to_move_mapping() {
    bruecken::Board board(5, 5);
    require(
        !bruecken::move_for_board_position(
             board,
             bruecken::Position{0, 0})
             .has_value(),
        "An unplayable corner must not become a GUI move");

    const auto first_move = bruecken::move_for_board_position(
        board,
        bruecken::Position{2, 2});
    require(first_move.has_value() && first_move->get_id() == 1,
            "A valid first click must create a Player 1 move");
    board.apply_move(*first_move);

    const auto second_move = bruecken::move_for_board_position(
        board,
        bruecken::Position{1, 1});
    require(second_move.has_value() && second_move->get_id() == 2,
            "The next valid click must create a Player 2 move");
    require(
        !bruecken::move_for_board_position(
             board,
             bruecken::Position{2, 2})
             .has_value(),
        "An occupied coordinate must not become a GUI move");
}

void test_player_one_status_and_winning_path() {
    bruecken::Board board(5, 5);
    const std::vector<std::string> names = {"Alice", "Bob"};
    require(bruecken::game_status_text(board, names) == "Alice to move",
            "Initial status must name Player 1");

    board.apply_move(preset::Move(1, 0, 1));
    require(bruecken::game_status_text(board, names) == "Bob to move",
            "Status must change after Player 1 moves");
    board.apply_move(preset::Move(4, 1, 2));
    board.apply_move(preset::Move(2, 2, 1));
    board.apply_move(preset::Move(4, 3, 2));
    board.apply_move(preset::Move(1, 4, 1));

    require(board.check_win(0), "Player 1 must have a winning connection");
    require(bruecken::game_status_text(board, names) == "Alice wins!",
            "Winning status must name Player 1");
    const auto path = bruecken::calculate_winning_path(board, 0);
    require(path.size() == 3,
            "Player 1 winning path must contain the three connected pegs");
    require(board.get_direction(path.front()) == bruecken::Direction::kTop,
            "Player 1 path must start at the top edge");
    require(board.get_direction(path.back()) == bruecken::Direction::kBottom,
            "Player 1 path must end at the bottom edge");
}

void test_player_two_winning_path() {
    bruecken::Board board(5, 5);
    board.apply_move(preset::Move(1, 0, 1));
    board.apply_move(preset::Move(0, 1, 2));
    board.apply_move(preset::Move(3, 0, 1));
    board.apply_move(preset::Move(2, 2, 2));
    board.apply_move(preset::Move(1, 4, 1));
    board.apply_move(preset::Move(4, 1, 2));

    require(board.check_win(1), "Player 2 must have a winning connection");
    const auto path = bruecken::calculate_winning_path(board, 1);
    require(path.size() == 3,
            "Player 2 winning path must contain the three connected pegs");
    require(board.get_direction(path.front()) == bruecken::Direction::kLeft,
            "Player 2 path must start at the left edge");
    require(board.get_direction(path.back()) == bruecken::Direction::kRight,
            "Player 2 path must end at the right edge");
}

}  // namespace

int main() {
    try {
        test_player_gui_access_inheritance();
        test_rotation_geometry();
        test_large_board_coordinates();
        test_click_to_move_mapping();
        test_player_one_status_and_winning_path();
        test_player_two_winning_path();
        std::cout << "All GUI logic tests passed.\n";
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "GUI logic test failed: " << exception.what() << '\n';
        return 1;
    }
}
