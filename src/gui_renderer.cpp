/**
 * @file gui_renderer.cpp
 * @brief Implements Raylib rendering, status display, and mouse input.
 * @author Hao Guo
 * @author Junke Pu
 *
 * @par Contribution breakdown
 * - Hao Guo: GUI window initialization and lifetime; board, peg, and bridge
 *   rendering.
 * - Junke Pu: GUI interaction and move placement; game-state display; the
 *   preset::PlayerGuiAccess implementation.
 */

#include "bruecken/gui_renderer.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "bruecken/common.h"
#include "bruecken/gui_logic.h"
#include "move.h"
#include "raylib.h"

namespace bruecken {
namespace {

// Visual palette used by Hao Guo's renderer and Junke Pu's status panel.
constexpr Color kBackground{241, 245, 249, 255};
constexpr Color kBoardBackground{255, 255, 255, 255};
constexpr Color kGridColor{148, 163, 184, 130};
constexpr Color kTextColor{30, 41, 59, 255};
constexpr Color kSecondaryText{100, 116, 139, 255};
constexpr Color kPointColor{100, 116, 139, 255};
constexpr Color kWinningColor{250, 204, 21, 255};

using BoardGeometry = GuiBoardGeometry;

/** Convert renderer-independent GUI coordinates to a Raylib vector. */
Vector2 to_vector(GuiPoint point) {
    return {point.x, point.y};
}

/** @brief Returns the Euclidean length of a screen-space vector. */
float length(Vector2 value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}

/**
 * @brief Converts a hexadecimal RGB string such as `#ff0015` to a color.
 * @param text Color string to parse.
 * @param fallback Color returned when parsing fails.
 * @return The parsed opaque Raylib color, or @p fallback.
 * @par Primary contributor
 * Hao Guo (rendering configuration).
 */
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
 * @brief Calculates the rotated board geometry.
 * @param board Board whose dimensions and rotation define the geometry.
 * @return Corner points, grid steps, and center in screen coordinates.
 *
 * The transformation follows the formula specified in SPIELREGELN.md.
 *
 * @par Primary contributor
 * Hao Guo (board rendering).
 */
BoardGeometry make_geometry(const Board& board) {
    return calculate_board_geometry(
        board,
        static_cast<float>(GetScreenWidth()),
        static_cast<float>(GetScreenHeight()));
}

/**
 * @brief Maps an integer board coordinate to a screen-space point.
 * @par Primary contributor
 * Hao Guo (board rendering).
 */
Vector2 board_to_screen(
    const BoardGeometry& geometry,
    int x,
    int y) {

    return to_vector(board_coordinate_to_screen(geometry, x, y));
}

/**
 * @brief Finds the grid point nearest to the mouse cursor.
 * @return The clicked board position, or std::nullopt when no point is close.
 *
 * This is simpler than calculating an inverse transformation and still
 * works with rotated boards.
 *
 * @par Primary contributor
 * Junke Pu (mouse interaction and move placement).
 */
std::optional<Position> find_clicked_position(
    const BoardGeometry& geometry,
    const Board& board) {

    const Vector2 mouse = GetMousePosition();
    return find_nearest_board_position(
        geometry,
        board,
        GuiPoint{mouse.x, mouse.y});
}

/**
 * @brief Checks whether a coordinate is one of the four unplayable corners.
 * @par Primary contributor
 * Hao Guo (board rendering).
 */
bool is_corner(const Board& board, int x, int y) {
    return (x == 0 || x == board.get_width() - 1) &&
           (y == 0 || y == board.get_height() - 1);
}

/** @brief Draws text centered around the supplied screen position. */
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

/** @brief Places a coordinate label outside the board, away from its center. */
Vector2 label_position(
    Vector2 point,
    Vector2 center,
    float distance) {

    Vector2 direction{
        point.x - center.x,
        point.y - center.y};
    const float direction_length = length(direction);

    if (direction_length > 0.001F) {
        const float factor = distance / direction_length;
        direction.x *= factor;
        direction.y *= factor;
    }

    return {point.x + direction.x, point.y + direction.y};
}

/**
 * @brief Draws the current player, round, result, and interaction feedback.
 * @par Primary contributor
 * Junke Pu (game-state display).
 */
void draw_status(
    const Board& board,
    const std::vector<std::string>& names,
    const Color colors[kNumPlayers],
    const std::string& feedback) {

    int displayed_player = board.get_current_player();
    if (board.get_phase() == GamePhase::kFinished) {
        displayed_player = board.check_win(0) ? 0 : 1;
    }
    const std::string status = game_status_text(board, names);

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
        "Round " + std::to_string(board.get_turn() + 1);

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

    // Hao Guo: normalize the colors consumed by the visual renderer.
    if (player_colors_.size() < kNumPlayers) {
        player_colors_ = kDefaultPlayerColors;
    }

    // Junke Pu: normalize names consumed by the game-state panel.
    if (player_names_.size() < kNumPlayers) {
        player_names_ = kDefaultPlayerNames;
    }

    // Hao Guo: configure and initialize the GUI window.
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
    // Hao Guo: release the Raylib window owned by this renderer.
    if (window_open_ && IsWindowReady()) {
        CloseWindow();
    }
}

bool GuiRenderer::should_close() const {
    // Hao Guo: expose the window-lifetime state to the main game loop.
    return !window_open_ || WindowShouldClose();
}

std::optional<preset::Move>
GuiRenderer::request_move_from_current_human_player() {
    // Junke Pu: implement the school's PlayerGuiAccess hand-off. Copying the
    // optional and then clearing it ensures that each click is consumed once.
    auto result = pending_move_;
    pending_move_.reset();
    return result;
}

void GuiRenderer::draw_frame() {
    if (should_close()) {
        return;
    }

    // Hao Guo: derive all geometry and drawing sizes for this frame.
    const BoardGeometry geometry = make_geometry(board_);

    // Junke Pu: resolve the exact coordinate currently under the mouse. This
    // keeps every coordinate discoverable even on the largest supported board.
    const auto hovered_position =
        find_clicked_position(geometry, board_);

    const Color colors[kNumPlayers] = {
        parse_color(player_colors_[0], RED),
        parse_color(player_colors_[1], BLUE)
    };

    const float step = std::min(
        length(to_vector(geometry.x_step)),
        length(to_vector(geometry.y_step)));

    const float peg_radius =
        std::clamp(step * 0.29F, 2.2F, 9.5F);

    const float bridge_width =
        std::clamp(step * 0.18F, 1.5F, 5.0F);

    // Junke Pu: discard interaction state left over from the previous turn.
    if (input_turn_ != board_.get_turn()) {
        pending_move_.reset();
        input_turn_ = board_.get_turn();
    }

    // Junke Pu: translate a click into a valid game move and queue it for
    // request_move_from_current_human_player(). This is the piece-placement
    // interaction; Board applies the move later through HumanPlayer.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        board_.get_phase() != GamePhase::kFinished &&
        board_.get_phase() != GamePhase::kDraw) {

        const auto& position = hovered_position;

        if (!position.has_value()) {
            feedback_ = "Bitte einen Rasterpunkt anklicken.";
        } else {
            const auto move =
                move_for_board_position(board_, *position);

            if (move.has_value()) {
                pending_move_ = *move;

                feedback_ =
                    "Selected: (" +
                    std::to_string(position->x) + ", " +
                    std::to_string(position->y) + ")";
            } else {
                feedback_ = "This field is not playable.";
            }
        }
    }

    // Hao Guo: begin the visual rendering pass.
    BeginDrawing();
    ClearBackground(kBackground);

    // Junke Pu: display the current turn, game result, and click feedback.
    draw_status(
        board_,
        player_names_,
        colors,
        feedback_);

    // Hao Guo: render the board background.
    DrawTriangle(
        to_vector(geometry.top_left),
        to_vector(geometry.top_right),
        to_vector(geometry.bottom_right),
        kBoardBackground);

    DrawTriangle(
        to_vector(geometry.top_left),
        to_vector(geometry.bottom_right),
        to_vector(geometry.bottom_left),
        kBoardBackground);

    // Hao Guo: render the board grid.
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

    // Hao Guo: render the goal edges. Player 1 owns top/bottom; player 2
    // owns left/right.
    DrawLineEx(
        to_vector(geometry.top_left),
        to_vector(geometry.top_right),
        6.0F,
        colors[0]);

    DrawLineEx(
        to_vector(geometry.bottom_left),
        to_vector(geometry.bottom_right),
        6.0F,
        colors[0]);

    DrawLineEx(
        to_vector(geometry.top_left),
        to_vector(geometry.bottom_left),
        6.0F,
        colors[1]);

    DrawLineEx(
        to_vector(geometry.top_right),
        to_vector(geometry.bottom_right),
        6.0F,
        colors[1]);

    // Hao Guo: render playable board points and mark unplayable corners.
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

    // Hao Guo: render normal bridges below the pegs.
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

    // Junke Pu: visualize the final game state by highlighting the winner's
    // connected bridge path.
    if (board_.get_phase() == GamePhase::kFinished) {
        const int winner = board_.check_win(0) ? 0 : 1;
        const auto path = calculate_winning_path(board_, winner);

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

    // Hao Guo: render the players' pegs above the bridges.
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

    // Hao Guo: render readable board-coordinate labels.
    const auto x_labels = coordinate_label_values(
        board_.get_width(),
        length(to_vector(geometry.x_step)));
    const auto y_labels = coordinate_label_values(
        board_.get_height(),
        length(to_vector(geometry.y_step)));

    for (const int x : x_labels) {
        draw_centered_text(
            std::to_string(x),
            label_position(
                board_to_screen(geometry, x, 0),
                to_vector(geometry.center),
                17.0F),
            13,
            kSecondaryText);
    }

    for (const int y : y_labels) {
        if (y == 0) continue;

        draw_centered_text(
            std::to_string(y),
            label_position(
                board_to_screen(geometry, 0, y),
                to_vector(geometry.center),
                17.0F),
            13,
            kSecondaryText);
    }

    // Hao Guo and Junke Pu: render a readable coordinate tooltip for every
    // grid point. Persistent labels provide orientation; this tooltip provides
    // the exact non-skipped coordinate on boards up to 96 x 96.
    if (hovered_position.has_value()) {
        const Vector2 point = board_to_screen(
            geometry,
            hovered_position->x,
            hovered_position->y);

        DrawCircleLines(
            static_cast<int>(point.x),
            static_cast<int>(point.y),
            std::max(6.0F, step * 0.42F),
            kWinningColor);

        const std::string coordinate =
            "X: " + std::to_string(hovered_position->x) +
            "  Y: " + std::to_string(hovered_position->y);
        constexpr int kTooltipFontSize = 16;
        const int tooltip_width =
            MeasureText(coordinate.c_str(), kTooltipFontSize) + 18;
        constexpr int kTooltipHeight = 28;
        const Vector2 mouse = GetMousePosition();
        const float tooltip_x = std::clamp(
            mouse.x + 14.0F,
            8.0F,
            static_cast<float>(GetScreenWidth() - tooltip_width - 8));
        const float tooltip_y = std::clamp(
            mouse.y + 14.0F,
            8.0F,
            static_cast<float>(GetScreenHeight() - kTooltipHeight - 8));

        DrawRectangleRounded(
            Rectangle{
                tooltip_x,
                tooltip_y,
                static_cast<float>(tooltip_width),
                static_cast<float>(kTooltipHeight)},
            0.25F,
            6,
            kTextColor);
        DrawText(
            coordinate.c_str(),
            static_cast<int>(tooltip_x) + 9,
            static_cast<int>(tooltip_y) + 6,
            kTooltipFontSize,
            kBoardBackground);
    }

    // Hao Guo: present the completed frame.
    EndDrawing();
}

}  // namespace bruecken
