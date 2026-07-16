/**
 * @file gui_logic.cpp
 * @brief Implements testable logic shared by GuiRenderer and GUI tests.
 * @author Hao Guo
 * @author Junke Pu
 */

#include "bruecken/gui_logic.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>

namespace bruecken {
namespace {

GuiPoint add(GuiPoint first, GuiPoint second) {
    return {first.x + second.x, first.y + second.y};
}

GuiPoint subtract(GuiPoint first, GuiPoint second) {
    return {first.x - second.x, first.y - second.y};
}

GuiPoint scale(GuiPoint point, float factor) {
    return {point.x * factor, point.y * factor};
}

float point_length(GuiPoint point) {
    return std::sqrt(point.x * point.x + point.y * point.y);
}

}  // namespace

GuiBoardGeometry calculate_board_geometry(
    const Board& board,
    float screen_width,
    float screen_height) {

    if (screen_width <= 0.0F || screen_height <= 0.0F) {
        throw std::invalid_argument("Screen dimensions must be positive");
    }

    const float available_left = std::max(54.0F, screen_width * 0.075F);
    const float available_right = screen_width - available_left;
    const float available_top = std::max(116.0F, screen_height * 0.16F);
    const float available_bottom =
        screen_height - std::max(52.0F, screen_height * 0.07F);
    const float fraction =
        static_cast<float>(board.get_rotation_fraction());

    const float available_width = available_right - available_left;
    const float available_height = available_bottom - available_top;
    const float unit = std::min(
        available_width / static_cast<float>(board.get_width() - 1),
        available_height / static_cast<float>(board.get_height() - 1));
    const float width = unit * static_cast<float>(board.get_width() - 1);
    const float height = unit * static_cast<float>(board.get_height() - 1);
    const float left = available_left + (available_width - width) * 0.5F;
    const float top = available_top + (available_height - height) * 0.5F;
    const float right = left + width;
    const float bottom = top + height;

    GuiBoardGeometry result{};
    result.grid_top_left = {left, top};
    result.grid_top_right = {right, top};
    result.grid_bottom_right = {right, bottom};
    result.grid_bottom_left = {left, bottom};
    result.top_left = {left + fraction * width, top};
    result.top_right = {right, top + fraction * height};
    result.bottom_right = {right - fraction * width, bottom};
    result.bottom_left = {left, bottom - fraction * height};
    result.x_step = scale(
        subtract(result.grid_top_right, result.grid_top_left),
        1.0F / static_cast<float>(board.get_width() - 1));
    result.y_step = scale(
        subtract(result.grid_bottom_left, result.grid_top_left),
        1.0F / static_cast<float>(board.get_height() - 1));
    result.center = scale(
        add(
            add(result.grid_top_left, result.grid_top_right),
            add(result.grid_bottom_left, result.grid_bottom_right)),
        0.25F);
    return result;
}

GuiPoint board_coordinate_to_screen(
    const GuiBoardGeometry& geometry,
    int x,
    int y) {

    return add(
        geometry.grid_top_left,
        add(
            scale(geometry.x_step, static_cast<float>(x)),
            scale(geometry.y_step, static_cast<float>(y))));
}

std::optional<Position> find_nearest_board_position(
    const GuiBoardGeometry& geometry,
    const Board& board,
    GuiPoint mouse) {

    const float step = std::min(
        point_length(geometry.x_step),
        point_length(geometry.y_step));
    const float radius = std::clamp(step * 0.48F, 5.0F, 14.0F);
    float best_distance = radius * radius;
    std::optional<Position> result;

    for (int y = 0; y < board.get_height(); ++y) {
        for (int x = 0; x < board.get_width(); ++x) {
            if (!board.is_in_bounds(Position{x, y})) continue;

            const GuiPoint point =
                board_coordinate_to_screen(geometry, x, y);
            const float dx = mouse.x - point.x;
            const float dy = mouse.y - point.y;
            const float distance = dx * dx + dy * dy;

            if (distance < best_distance) {
                best_distance = distance;
                result = Position{x, y};
            }
        }
    }
    return result;
}

std::optional<preset::Move> move_for_board_position(
    const Board& board,
    Position position) {

    preset::Move move(
        position.x,
        position.y,
        board.get_current_player() + 1);
    if (!board.is_valid_move(move)) return std::nullopt;
    return move;
}

std::vector<int> coordinate_label_values(
    int coordinate_count,
    float point_spacing) {

    if (coordinate_count <= 0 || point_spacing <= 0.0F) {
        throw std::invalid_argument(
            "Coordinate count and point spacing must be positive");
    }

    const int stride = std::max(
        1,
        static_cast<int>(std::ceil(24.0F / point_spacing)));
    std::vector<int> labels;
    for (int value = 0; value < coordinate_count; value += stride) {
        labels.push_back(value);
    }

    const int final_value = coordinate_count - 1;
    if (labels.back() != final_value) {
        labels.push_back(final_value);
    }
    return labels;
}

std::vector<Position> calculate_winning_path(
    const Board& board,
    int player_id) {

    if (player_id < 0 || player_id >= kNumPlayers) {
        throw std::invalid_argument("player_id must be 0 or 1");
    }

    std::vector<Peg> pegs;
    for (const Peg& peg : board.get_pegs()) {
        if (peg.player_id == player_id) {
            pegs.push_back(peg);
        }
    }
    if (pegs.empty()) return {};

    std::vector<std::vector<int>> neighbours(pegs.size());
    auto find_peg = [&](const Position& position) {
        for (int i = 0; i < static_cast<int>(pegs.size()); ++i) {
            if (pegs[i].pos == position) return i;
        }
        return -1;
    };

    for (const Bridge& bridge : board.get_bridges()) {
        if (bridge.player_id != player_id) continue;
        const int from = find_peg(bridge.from);
        const int to = find_peg(bridge.to);
        if (from >= 0 && to >= 0) {
            neighbours[from].push_back(to);
            neighbours[to].push_back(from);
        }
    }

    std::queue<int> queue;
    std::vector<int> parent(pegs.size(), -1);
    std::vector<bool> visited(pegs.size(), false);
    for (int i = 0; i < static_cast<int>(pegs.size()); ++i) {
        const Direction direction = board.get_direction(pegs[i].pos);
        const bool start = player_id == 0
            ? direction == Direction::kTop
            : direction == Direction::kLeft;
        if (start) {
            queue.push(i);
            visited[i] = true;
        }
    }

    int goal = -1;
    while (!queue.empty()) {
        const int current = queue.front();
        queue.pop();
        const Direction direction = board.get_direction(pegs[current].pos);
        const bool reached_goal = player_id == 0
            ? direction == Direction::kBottom
            : direction == Direction::kRight;
        if (reached_goal) {
            goal = current;
            break;
        }
        for (const int next : neighbours[current]) {
            if (!visited[next]) {
                visited[next] = true;
                parent[next] = current;
                queue.push(next);
            }
        }
    }

    if (goal < 0) return {};
    std::vector<Position> path;
    for (int current = goal; current >= 0; current = parent[current]) {
        path.push_back(pegs[current].pos);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::string game_status_text(
    const Board& board,
    const std::vector<std::string>& player_names) {

    if (player_names.size() < kNumPlayers) {
        throw std::invalid_argument("Two player names are required");
    }
    if (board.get_phase() == GamePhase::kFinished) {
        const int winner = board.check_win(0) ? 0 : 1;
        return player_names[winner] + " wins!";
    }
    if (board.get_phase() == GamePhase::kDraw) {
        return "Draw";
    }
    return player_names[board.get_current_player()] + " to move";
}

}  // namespace bruecken
