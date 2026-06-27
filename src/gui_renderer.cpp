/**
 * @file gui_renderer.cpp
 * @brief Fenster-, Brett-, Spielstein- und Brueckendarstellung mit Raylib.
 * @author Hao Guo
 */

#include "bruecken/gui_renderer.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "bruecken/common.h"
#include "raylib.h"

namespace bruecken {
namespace {

constexpr Color kBackground{246, 247, 250, 255};
constexpr Color kBoardBackground{255, 255, 255, 255};
constexpr Color kGridColor{169, 176, 190, 115};
constexpr Color kCoordinateColor{91, 99, 115, 255};
constexpr Color kPositionColor{105, 112, 126, 255};

/** Geometrie des gegebenenfalls gedrehten Spielfeld-Parallelogramms. */
struct BoardGeometry {
    Vector2 top_left;
    Vector2 top_right;
    Vector2 bottom_left;
    Vector2 bottom_right;
    Vector2 x_step;
    Vector2 y_step;
    Vector2 center;
};

Vector2 add(Vector2 left, Vector2 right) {
    return {left.x + right.x, left.y + right.y};
}

Vector2 subtract(Vector2 left, Vector2 right) {
    return {left.x - right.x, left.y - right.y};
}

Vector2 scale(Vector2 value, float factor) {
    return {value.x * factor, value.y * factor};
}

float vector_length(Vector2 value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}

/** Wandelt einen Hex-RGB-String wie #ff0015 in eine Raylib-Farbe um. */
Color parse_hex_color(const std::string& text, Color fallback) {
    std::string digits = text;
    if (!digits.empty() && digits.front() == '#') {
        digits.erase(digits.begin());
    }
    if (digits.size() != 6U) {
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
 * Berechnet die Brett-Eckpunkte nach der Formel aus SPIELREGELN.md.
 *
 * Der dort angegebene Ausdruck vereinfacht sich zu
 *   f = tan(rotation - 45 Grad) / 2 + 1/2.
 * Bei einem nicht quadratischen Brett entsteht dadurch das geforderte
 * Parallelogramm.
 */
BoardGeometry make_geometry(const Board& board) {
    constexpr float kPi = 3.14159265358979323846F;
    const float screen_width = static_cast<float>(GetScreenWidth());
    const float screen_height = static_cast<float>(GetScreenHeight());
    const float margin_x = std::max(54.0F, screen_width * 0.075F);
    const float margin_y = std::max(54.0F, screen_height * 0.075F);
    const float left = margin_x;
    const float right = screen_width - margin_x;
    const float top = margin_y;
    const float bottom = screen_height - margin_y;

    const float radians =
        static_cast<float>(board.get_rotation()) * kPi / 180.0F;
    const float formula_angle = radians - kPi / 4.0F;
    const float edge_fraction =
        std::clamp(std::tan(formula_angle) * 0.5F + 0.5F, 0.0F, 1.0F);
    const float width = right - left;
    const float height = bottom - top;

    BoardGeometry geometry{};
    geometry.top_left = {left + edge_fraction * width, top};
    geometry.top_right = {right, top + edge_fraction * height};
    geometry.bottom_right = {right - edge_fraction * width, bottom};
    geometry.bottom_left = {left, bottom - edge_fraction * height};
    geometry.x_step = scale(
        subtract(geometry.top_right, geometry.top_left),
        1.0F / static_cast<float>(board.get_width() - 1));
    geometry.y_step = scale(
        subtract(geometry.bottom_left, geometry.top_left),
        1.0F / static_cast<float>(board.get_height() - 1));
    geometry.center = scale(
        add(add(geometry.top_left, geometry.top_right),
            add(geometry.bottom_left, geometry.bottom_right)),
        0.25F);
    return geometry;
}

Vector2 board_to_screen(const BoardGeometry& geometry, int x, int y) {
    return add(geometry.top_left,
               add(scale(geometry.x_step, static_cast<float>(x)),
                   scale(geometry.y_step, static_cast<float>(y))));
}

bool is_corner(const Board& board, int x, int y) {
    return (x == 0 || x == board.get_width() - 1) &&
           (y == 0 || y == board.get_height() - 1);
}

void draw_centered_text(const std::string& text,
                        Vector2 center,
                        int font_size,
                        Color color) {
    const int text_width = MeasureText(text.c_str(), font_size);
    DrawText(text.c_str(),
             static_cast<int>(center.x) - text_width / 2,
             static_cast<int>(center.y) - font_size / 2,
             font_size,
             color);
}

int label_stride(float step_length) {
    return std::max(1, static_cast<int>(std::ceil(24.0F / step_length)));
}

Vector2 outward_label_position(Vector2 position,
                               Vector2 center,
                               float distance) {
    Vector2 direction = subtract(position, center);
    const float length = vector_length(direction);
    if (length > 0.001F) {
        direction = scale(direction, distance / length);
    }
    return add(position, direction);
}

}  // namespace

GuiRenderer::GuiRenderer(const Board& board,
                         std::vector<std::string> player_colors,
                         std::string title)
    : board_(board), player_colors_(std::move(player_colors)) {
    if (player_colors_.size() < static_cast<std::size_t>(kNumPlayers)) {
        player_colors_ = kDefaultPlayerColors;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(kDefaultWindowSize, kDefaultWindowSize, title.c_str());
    if (!IsWindowReady()) {
        throw std::runtime_error("Raylib-Fenster konnte nicht initialisiert werden");
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

void GuiRenderer::run() {
    while (!should_close()) {
        draw_frame();
    }
}

void GuiRenderer::draw_frame() {
    if (should_close()) {
        return;
    }

    const BoardGeometry geometry = make_geometry(board_);
    const Color player_colors[kNumPlayers] = {
        parse_hex_color(player_colors_[0], RED),
        parse_hex_color(player_colors_[1], BLUE)
    };
    const float step_length =
        std::min(vector_length(geometry.x_step),
                 vector_length(geometry.y_step));
    const float peg_radius = std::clamp(step_length * 0.29F, 2.2F, 9.5F);
    const float bridge_width =
        std::clamp(step_length * 0.18F, 1.5F, 5.0F);

    BeginDrawing();
    ClearBackground(kBackground);

    DrawTriangle(geometry.top_left, geometry.top_right,
                 geometry.bottom_right, kBoardBackground);
    DrawTriangle(geometry.top_left, geometry.bottom_right,
                 geometry.bottom_left, kBoardBackground);

    // N x M Raster zeichnen.
    for (int x = 0; x < board_.get_width(); ++x) {
        DrawLineEx(board_to_screen(geometry, x, 0),
                   board_to_screen(geometry, x, board_.get_height() - 1),
                   1.0F, kGridColor);
    }
    for (int y = 0; y < board_.get_height(); ++y) {
        DrawLineEx(board_to_screen(geometry, 0, y),
                   board_to_screen(geometry, board_.get_width() - 1, y),
                   1.0F, kGridColor);
    }

    // Spieler 1 besitzt oben/unten, Spieler 2 links/rechts.
    DrawLineEx(geometry.top_left, geometry.top_right,
               6.0F, player_colors[0]);
    DrawLineEx(geometry.bottom_left, geometry.bottom_right,
               6.0F, player_colors[0]);
    DrawLineEx(geometry.top_left, geometry.bottom_left,
               6.0F, player_colors[1]);
    DrawLineEx(geometry.top_right, geometry.bottom_right,
               6.0F, player_colors[1]);

    // Alle spielbaren Rasterpunkte anzeigen; die vier unspielbaren Ecken
    // werden als Kreuz markiert.
    const float point_radius =
        std::clamp(step_length * 0.065F, 0.9F, 2.2F);
    for (int y = 0; y < board_.get_height(); ++y) {
        for (int x = 0; x < board_.get_width(); ++x) {
            const Vector2 point = board_to_screen(geometry, x, y);
            if (is_corner(board_, x, y)) {
                const float arm = std::max(2.5F, point_radius * 2.0F);
                DrawLineEx({point.x - arm, point.y - arm},
                           {point.x + arm, point.y + arm},
                           1.5F, kPositionColor);
                DrawLineEx({point.x - arm, point.y + arm},
                           {point.x + arm, point.y - arm},
                           1.5F, kPositionColor);
            } else {
                DrawCircleV(point, point_radius, kPositionColor);
            }
        }
    }

    // Bruecken liegen optisch unter den Spielsteinen.
    for (const Bridge& bridge : board_.get_bridges()) {
        const int player =
            std::clamp(bridge.player_id, 0, kNumPlayers - 1);
        DrawLineEx(board_to_screen(geometry, bridge.from.x, bridge.from.y),
                   board_to_screen(geometry, bridge.to.x, bridge.to.y),
                   bridge_width, player_colors[player]);
    }

    for (const Peg& peg : board_.get_pegs()) {
        const int player = std::clamp(peg.player_id, 0, kNumPlayers - 1);
        const Vector2 point = board_to_screen(geometry, peg.pos.x, peg.pos.y);
        DrawCircleV(point, peg_radius + 1.5F, kBoardBackground);
        DrawCircleV(point, peg_radius, player_colors[player]);
    }

    // Koordinaten bleiben bei grossen Brettern lesbar: Bei wenig Platz wird
    // ein gleichmaessiger Beschriftungsschritt verwendet, der letzte Wert
    // wird jedoch immer angezeigt.
    const int x_stride = label_stride(vector_length(geometry.x_step));
    const int y_stride = label_stride(vector_length(geometry.y_step));
    for (int x = 0; x < board_.get_width(); x += x_stride) {
        draw_centered_text(
            std::to_string(x),
            outward_label_position(board_to_screen(geometry, x, 0),
                                   geometry.center, 17.0F),
            13, kCoordinateColor);
    }
    if ((board_.get_width() - 1) % x_stride != 0) {
        const int x = board_.get_width() - 1;
        draw_centered_text(
            std::to_string(x),
            outward_label_position(board_to_screen(geometry, x, 0),
                                   geometry.center, 17.0F),
            13, kCoordinateColor);
    }
    for (int y = y_stride; y < board_.get_height(); y += y_stride) {
        draw_centered_text(
            std::to_string(y),
            outward_label_position(board_to_screen(geometry, 0, y),
                                   geometry.center, 17.0F),
            13, kCoordinateColor);
    }
    if ((board_.get_height() - 1) % y_stride != 0) {
        const int y = board_.get_height() - 1;
        draw_centered_text(
            std::to_string(y),
            outward_label_position(board_to_screen(geometry, 0, y),
                                   geometry.center, 17.0F),
            13, kCoordinateColor);
    }

    EndDrawing();
}

}  // namespace bruecken
