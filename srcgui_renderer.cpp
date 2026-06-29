/**
 * @file gui_renderer.cpp
 * @brief Raylib drawing, status display and mouse input.
 * @author Hao Guo
 */

#include "bruecken/gui_renderer.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "bruecken/common.h"
#include "move.h"
#include "raylib.h"

namespace bruecken {
namespace {

constexpr Color kBackground{241, 245, 249, 255};
constexpr Color kBoardBackground{255, 255, 255, 255};
constexpr Color kGridColor{148, 163, 184, 130};
constexpr Color kTextColor{30, 41, 59, 255};
constexpr Color kSecondaryText{100, 116, 139, 255};
constexpr Color kPointColor{100, 116, 139, 255};
constexpr Color kWinningColor{250, 204, 21, 255};

struct BoardGeometry {
    Vector2 top_left;
    Vector2 top_right;
    Vector2 bottom_left;
    Vector2 bottom_right;
    Vector2 x_step;
    Vector2 y_step;
    Vector2 center;
};

Vector2 add(Vector2 a, Vector2 b) {
    return {a.x + b.x, a.y + b.y};
}

Vector2 subtract(Vector2 a, Vector2 b) {
    return {a.x - b.x, a.y - b.y};
}

Vector2 scale(Vector2 value, float factor) {
    return {value.x * factor, value.y * factor};
}

float length(Vector2 value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}

Color parse_color(const std::string& text, Color fallback) {
    std::string digits = text;

    if (!digits.empty() && digits.front() == '#') {
        digits.erase(digits.begin());
    }

    if (digits.size() != 6) {
        return fallback;
    }

    try {
        std::size_t parsed = 0;
        const unsigned long rgb = std::stoul(digits, &parsed, 16);

        if (parsed != digits.size()) {
            return fallback;
        }

        return {
            static_cast<unsigned char>((rgb >> 16U) & 0xffU),
            static_cast<unsigned char>((rgb >> 8U) & 0xffU),
            static_cast<unsigned char>(rgb & 0xffU),
            255
        };
    } catch (const std::exception&) {
        return fallback;
    }
}

/**
 * Calculates the rotated board geometry using the formula from
 * SPIELREGELN.md.
 */
BoardGeometry make_geometry(const Board& board) {
    constexpr float kPi = 3.14159265358979323846F;

    const float screen_width = static_cast<float>(GetScreenWidth());
    const float screen_height = static_cast<float>(GetScreenHeight());

    const float left = std::max(54.0F, screen_width * 0.075F);
    const float right = screen_width - left;
    const float top = std::max(116.0F, screen_height * 0.16F);
    const float bottom =
        screen_height - std::max(52.0F, screen_height * 0.07F);

    const float radians =
        static_cast<float>(board.get_rotation()) * kPi / 180.0F;

    const float formula_angle = radians - kPi / 4.0F;

    const float fraction = std::clamp(
        std::tan(formula_angle) * 0.5F + 0.5F,
        0.0F,
        1.0F);

    const float width = right - left;
    const float height = bottom - top;

    BoardGeometry result{};

    result.top_left = {
        left + fraction * width,
        top
    };

    result.top_right = {
        right,
        top + fraction * height
    };

    result.bottom_right = {
        right - fraction * width,
        bottom
    };

    result.bottom_left = {
        left,
        bottom - fraction * height
    };

    result.x_step = scale(
        subtract(result.top_right, result.top_left),
        1.0F / static_cast<float>(board.get_width() - 1));

    result.y_step = scale(
        subtract(result.bottom_left, result.top_left),
        1.0F / static_cast<float>(board.get_height() - 1));

    result.center = scale(
        add(
            add(result.top_left, result.top_right),
            add(result.bottom_left, result.bottom_right)),
        0.25F);

    return result;
}

Vector2 board_to_screen(
    const BoardGeometry& geometry,
    int x,
    int y) {

    return add(
        geometry.top_left,
        add(
            scale(geometry.x_step, static_cast<float>(x)),
            scale(geometry.y_step, static_cast<float>(y))));
}

/**
 * Finds the grid point nearest to the mouse.
 *
 * This is simpler than calculating an inverse transformation and still
 * works with rotated boards.
 */
std::optional<Position> find_clicked_position(
    const BoardGeometry& geometry,
    const Board& board) {

    const Vector2 mouse = GetMousePosition();

    const float step = std::min(
        length(geometry.x_step),
        length(geometry.y_step));

    const float radius = std::clamp(step * 0.48F, 5.0F, 14.0F);
    float best_distance = radius * radius;

    std::optional<Position> result;

    for (int y = 0; y < board.get_height(); ++y) {
        for (int x = 0; x < board.get_width(); ++x) {
            const Vector2 point = board_to_screen(geometry, x, y);

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

bool is_corner(const Board& board, int x, int y) {
    return (x == 0 || x == board.get_width() - 1) &&
           (y == 0 || y == board.get_height() - 1);
}

void draw_centered_text(
    const std::string& text,
    Vector2 center,
    int font_size,
    Color color) {

    const int width = MeasureText(text.c_str(), font_size);

    DrawText(
        text.c_str(),
        static_cast<int>(center.x) - width / 2,
        static_cast<int>(center.y) - font_size / 2,
        font_size,
        color);
}

Vector2 label_position(
    Vector2 point,
    Vector2 center,
    float distance) {

    Vector2 direction = subtract(point, center);
    const float direction_length = length(direction);

    if (direction_length > 0.001F) {
        direction = scale(direction, distance / direction_length);
    }

    return add(point, direction);
}

int label_stride(float step) {
    return std::max(
        1,
        static_cast<int>(std::ceil(24.0F / step)));
}

std::vector<Position> find_winning_path(
    const Board& board,
    int player_id) {

    std::vector<Peg> pegs;

    for (const Peg& peg : board.get_pegs()) {
        if (peg.player_id == player_id) {
            pegs.push_back(peg);
        }
    }

    if (pegs.empty()) {
        return {};
    }

    std::vector<std::vector<int>> neighbours(pegs.size());

    auto find_peg = [&](const Position& position) {
        for (int i = 0; i < static_cast<int>(pegs.size()); ++i) {
            if (pegs[i].pos == position) {
                return i;
            }
        }
        return -1;
    };

    for (const Bridge& bridge : board.get_bridges()) {
        if (bridge.player_id != player_id) {
            continue;
        }

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
        const Direction direction =
            board.get_direction(pegs[i].pos);

        const bool start =
            player_id == 0
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

        const Direction direction =
            board.get_direction(pegs[current].pos);

        const bool reached_goal =
            player_id == 0
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

    if (goal < 0) {
        return {};
    }

    std::vector<Position> path;

    for (int current = goal;
         current >= 0;
         current = parent[current]) {

        path.push_back(pegs[current].pos);
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void draw_status(
    const Board& board,
    const std::vector<std::string>& names,
    const Color colors[kNumPlayers],
    const std::string& feedback) {

    int displayed_player = board.get_current_player();
    std::string status;

    if (board.get_phase() == GamePhase::kFinished) {
        displayed_player = board.check_win(0) ? 0 : 1;
        status = names[displayed_player] + " gewinnt!";
    } else if (board.get_phase() == GamePhase::kDraw) {
        status = "Unentschieden";
    } else {
        status = names[displayed_player] + " ist am Zug";
    }

    DrawRectangleRounded(
        Rectangle{
            14.0F,
            12.0F,
            static_cast<float>(GetScreenWidth()) - 28.0F,
            82.0F},
        0.18F,
        8,
        Color{255, 255, 255, 255});

    DrawCircle(
        38,
        38,
        11.0F,
        colors[displayed_player]);

    DrawText(
        status.c_str(),
        58,
        23,
        21,
        kTextColor);

    const std::string round =
        "Runde " + std::to_string(board.get_turn() + 1);

    DrawText(
        round.c_str(),
        58,
        53,
        14,
        kSecondaryText);

    if (!feedback.empty()) {
        const int text_width =
            MeasureText(feedback.c_str(), 14);

        DrawText(
            feedback.c_str(),
            GetScreenWidth() - text_width - 28,
            54,
            14,
            kSecondaryText);
    }
}

}  // namespace

GuiRenderer::GuiRenderer(
    const Board& board,
    std::vector<std::string> player_colors,
    std::vector<std::string> player_names,
    std::string title)
    : board_(board),
      player_colors_(std::move(player_colors)),
      player_names_(std::move(player_names)) {

    if (player_colors_.size() < kNumPlayers) {
        player_colors_ = kDefaultPlayerColors;
    }

    if (player_names_.size() < kNumPlayers) {
        player_names_ = kDefaultPlayerNames;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(
        kDefaultWindowSize,
        kDefaultWindowSize,
        title.c_str());

    if (!IsWindowReady()) {
        throw std::runtime_error(
            "Raylib-Fenster konnte nicht initialisiert werden");
    }

    SetWindowMinSize(480, 480);
    SetTargetFPS(60);
    window_open_ = true;
}

GuiRenderer::~GuiRenderer() {
    if (window_open_ && IsWindowReady()) {
        CloseWindow();
    }
}

bool GuiRenderer::should_close() const {
    return !window_open_ || WindowShouldClose();
}

std::optional<preset::Move>
GuiRenderer::request_move_from_current_human_player() {
    auto result = pending_move_;
    pending_move_.reset();
    return result;
}

void GuiRenderer::draw_frame() {
    if (should_close()) {
        return;
    }

    const BoardGeometry geometry = make_geometry(board_);

    const Color colors[kNumPlayers] = {
        parse_color(player_colors_[0], RED),
        parse_color(player_colors_[1], BLUE)
    };

    const float step = std::min(
        length(geometry.x_step),
        length(geometry.y_step));

    const float peg_radius =
        std::clamp(step * 0.29F, 2.2F, 9.5F);

    const float bridge_width =
        std::clamp(step * 0.18F, 1.5F, 5.0F);

    // Remove clicks left over from the previous turn.
    if (input_turn_ != board_.get_turn()) {
        pending_move_.reset();
        input_turn_ = board_.get_turn();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        board_.get_phase() != GamePhase::kFinished &&
        board_.get_phase() != GamePhase::kDraw) {

        const auto position =
            find_clicked_position(geometry, board_);

        if (!position.has_value()) {
            feedback_ = "Bitte einen Rasterpunkt anklicken.";
        } else {
            preset::Move move(
                position->x,
                position->y,
                board_.get_current_player() + 1);

            if (board_.is_valid_move(move)) {
                pending_move_ = move;

                feedback_ =
                    "Ausgewählt: (" +
                    std::to_string(position->x) + ", " +
                    std::to_string(position->y) + ")";
            } else {
                feedback_ = "Dieses Feld ist nicht spielbar.";
            }
        }
    }

    BeginDrawing();
    ClearBackground(kBackground);

    draw_status(
        board_,
        player_names_,
        colors,
        feedback_);

    // Board background.
    DrawTriangle(
        geometry.top_left,
        geometry.top_right,
        geometry.bottom_right,
        kBoardBackground);

    DrawTriangle(
        geometry.top_left,
        geometry.bottom_right,
        geometry.bottom_left,
        kBoardBackground);

    // Grid.
    for (int x = 0; x < board_.get_width(); ++x) {
        DrawLineEx(
            board_to_screen(geometry, x, 0),
            board_to_screen(
                geometry,
                x,
                board_.get_height() - 1),
            1.0F,
            kGridColor);
    }

    for (int y = 0; y < board_.get_height(); ++y) {
        DrawLineEx(
            board_to_screen(geometry, 0, y),
            board_to_screen(
                geometry,
                board_.get_width() - 1,
                y),
            1.0F,
            kGridColor);
    }

    // Player 1: top/bottom. Player 2: left/right.
    DrawLineEx(
        geometry.top_left,
        geometry.top_right,
        6.0F,
        colors[0]);

    DrawLineEx(
        geometry.bottom_left,
        geometry.bottom_right,
        6.0F,
        colors[0]);

    DrawLineEx(
        geometry.top_left,
        geometry.bottom_left,
        6.0F,
        colors[1]);

    DrawLineEx(
        geometry.top_right,
        geometry.bottom_right,
        6.0F,
        colors[1]);

    // Board points.
    const float point_radius =
        std::clamp(step * 0.065F, 0.9F, 2.2F);

    for (int y = 0; y < board_.get_height(); ++y) {
        for (int x = 0; x < board_.get_width(); ++x) {
            const Vector2 point =
                board_to_screen(geometry, x, y);

            if (is_corner(board_, x, y)) {
                const float arm = 3.0F;

                DrawLineEx(
                    {point.x - arm, point.y - arm},
                    {point.x + arm, point.y + arm},
                    1.5F,
                    kPointColor);

                DrawLineEx(
                    {point.x - arm, point.y + arm},
                    {point.x + arm, point.y - arm},
                    1.5F,
                    kPointColor);
            } else {
                DrawCircleV(
                    point,
                    point_radius,
                    kPointColor);
            }
        }
    }

    // Normal bridges.
    for (const Bridge& bridge : board_.get_bridges()) {
        const int player =
            std::clamp(bridge.player_id, 0, kNumPlayers - 1);

        DrawLineEx(
            board_to_screen(
                geometry,
                bridge.from.x,
                bridge.from.y),
            board_to_screen(
                geometry,
                bridge.to.x,
                bridge.to.y),
            bridge_width,
            colors[player]);
    }

    // Winning path.
    if (board_.get_phase() == GamePhase::kFinished) {
        const int winner = board_.check_win(0) ? 0 : 1;
        const auto path = find_winning_path(board_, winner);

        for (std::size_t i = 1; i < path.size(); ++i) {
            DrawLineEx(
                board_to_screen(
                    geometry,
                    path[i - 1].x,
                    path[i - 1].y),
                board_to_screen(
                    geometry,
                    path[i].x,
                    path[i].y),
                bridge_width + 4.0F,
                kWinningColor);
        }
    }

    // Pegs.
    for (const Peg& peg : board_.get_pegs()) {
        const int player =
            std::clamp(peg.player_id, 0, kNumPlayers - 1);

        const Vector2 point =
            board_to_screen(
                geometry,
                peg.pos.x,
                peg.pos.y);

        DrawCircleV(
            point,
            peg_radius + 1.5F,
            kBoardBackground);

        DrawCircleV(
            point,
            peg_radius,
            colors[player]);
    }

    // Coordinates.
    const int x_stride = label_stride(length(geometry.x_step));
    const int y_stride = label_stride(length(geometry.y_step));

    for (int x = 0; x < board_.get_width(); x += x_stride) {
        draw_centered_text(
            std::to_string(x),
            label_position(
                board_to_screen(geometry, x, 0),
                geometry.center,
                17.0F),
            13,
            kSecondaryText);
    }

    for (int y = y_stride;
         y < board_.get_height();
         y += y_stride) {

        draw_centered_text(
            std::to_string(y),
            label_position(
                board_to_screen(geometry, 0, y),
                geometry.center,
                17.0F),
            13,
            kSecondaryText);
    }

    EndDrawing();
}

}  // namespace bruecken