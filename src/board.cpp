/**
 * @file board.cpp
 * @brief Implements the Knight Bridge board model and core game rules.
 *
 * This file manages board state, move validation, bridge generation, and
 * win/draw detection for the logical game grid.
 * @author Zhibo Zhang
 */

#include "bruecken/board.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <queue>

#include "move.h"
#include "logger.h"

namespace bruecken {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kGeometryEpsilon = 1.0e-9;

Direction side_to_direction(int side) {
    switch (side) {
        case 0: return Direction::kTop;
        case 1: return Direction::kRight;
        case 2: return Direction::kBottom;
        case 3: return Direction::kLeft;
        default: return Direction::kNone;
    }
}

}  // namespace

// =====================================================================
// Construction
// =====================================================================

Board::Board(int width, int height, double rotation)
    : width_(width)
    , height_(height)
    , rotation_(rotation)
{
    if (width < kBoardMinSize || width > kBoardMaxSize ||
        height < kBoardMinSize || height > kBoardMaxSize) {
        throw std::invalid_argument(
            "Board size must be between " +
            std::to_string(kBoardMinSize) + " and " +
            std::to_string(kBoardMaxSize) + ".");
    }
    if (rotation < kMinRotation || rotation > kMaxRotation) {
        throw std::invalid_argument(
            "Rotation must be between " +
            std::to_string(kMinRotation) + " and " +
            std::to_string(kMaxRotation) + ".");
    }

    // Allocate the grid: height rows, width columns
    grid_.assign(height_, std::vector<Cell>(width_, Cell::kEmpty));
}

double Board::get_rotation_fraction() const {
    const double radians = rotation_ * kPi / 180.0;
    const double formula_angle = radians - kPi / 4.0;

    // School formula:
    // (sin(a-45) + (1 - cos(a-45)) * tan(a-45)) / 2 + 1 / 2
    // This simplifies to tan(a-45) / 2 + 1 / 2.
    return std::clamp(std::tan(formula_angle) * 0.5 + 0.5, 0.0, 1.0);
}

// =====================================================================
// Coordinates / Area ownership
// =====================================================================

bool Board::is_in_bounds(const Position& pos) const {
    return is_grid_coordinate(pos) &&
           point_in_polygon(pos, rotated_polygon(0.0));
}

bool Board::is_corner(const Position& pos) const {
    if (!is_in_bounds(pos)) return false;

    const auto sides = side_values(pos, rotated_polygon(1.0));
    int boundary_count = 0;
    for (const double value : sides) {
        if (value <= kGeometryEpsilon) {
            ++boundary_count;
        }
    }
    return boundary_count >= 2;
}

Direction Board::get_direction(const Position& pos) const {
    if (!is_in_bounds(pos)) return Direction::kNone;

    if (is_corner(pos)) return Direction::kNone;

    const auto sides = side_values(pos, rotated_polygon(1.0));
    int boundary_side = -1;
    int boundary_count = 0;

    for (int side = 0; side < static_cast<int>(sides.size()); ++side) {
        if (sides[side] <= kGeometryEpsilon) {
            boundary_side = side;
            ++boundary_count;
        }
    }

    if (boundary_count == 1) {
        return side_to_direction(boundary_side);
    }
    return Direction::kNone;
}

bool Board::is_playable(const Position& pos, int player_id) const {
    if (!is_in_bounds(pos)) return false;

    if (is_corner(pos)) return false;

    Direction dir = get_direction(pos);

    // Inner area may be played by both players
    if (dir == Direction::kNone) return true;

    // Player 0: top / bottom
    // Player 1: left / right
    if (player_id == 0) {
        return dir == Direction::kTop || dir == Direction::kBottom;
    } else {
        return dir == Direction::kLeft || dir == Direction::kRight;
    }
}

bool Board::is_occupied(const Position& pos) const {
    if (!is_grid_coordinate(pos)) return false;
    return grid_[pos.y][pos.x] != Cell::kEmpty;
}

bool Board::is_grid_coordinate(const Position& pos) const {
    return pos.x >= 0 && pos.x < width_ &&
           pos.y >= 0 && pos.y < height_;
}

Board::BoardPolygon Board::rotated_polygon(double inset) const {
    const double fraction = get_rotation_fraction();
    const double left = inset;
    const double top = inset;
    const double right = static_cast<double>(width_ - 1) - inset;
    const double bottom = static_cast<double>(height_ - 1) - inset;
    const double rect_width = std::max(0.0, right - left);
    const double rect_height = std::max(0.0, bottom - top);

    return BoardPolygon{
        BoardPoint{left + fraction * rect_width, top},
        BoardPoint{right, top + fraction * rect_height},
        BoardPoint{right - fraction * rect_width, bottom},
        BoardPoint{left, bottom - fraction * rect_height}
    };
}

bool Board::point_in_polygon(
    const Position& pos,
    const BoardPolygon& polygon) const {

    const auto sides = side_values(pos, polygon);
    return std::all_of(
        sides.begin(),
        sides.end(),
        [](double value) { return value >= -kGeometryEpsilon; });
}

std::array<double, 4> Board::side_values(
    const Position& pos,
    const BoardPolygon& polygon) const {

    const BoardPoint point{
        static_cast<double>(pos.x),
        static_cast<double>(pos.y)};
    auto cross = [](const BoardPoint& a, const BoardPoint& b, const BoardPoint& p) {
        return (b.x - a.x) * (p.y - a.y) -
               (b.y - a.y) * (p.x - a.x);
    };

    return {
        cross(polygon[0], polygon[1], point),
        cross(polygon[1], polygon[2], point),
        cross(polygon[2], polygon[3], point),
        cross(polygon[3], polygon[0], point)
    };
}

// =====================================================================
// Move
// =====================================================================

bool Board::is_valid_move(preset::Move move) const {
    int pid = move.get_id() - 1;  // preset::Move IDs are 1-based
    Position pos{move.get_x(), move.get_y()};

    if (pid < 0 || pid >= kNumPlayers) return false;
    if (phase_ == GamePhase::kFinished || phase_ == GamePhase::kDraw) return false;
    if (pid != get_current_player()) return false;
    if (!is_in_bounds(pos)) return false;
    if (!is_playable(pos, pid)) return false;
    if (is_occupied(pos)) return false;

    return true;
}

void Board::apply_move(preset::Move move) {
    if (phase_ == GamePhase::kFinished || phase_ == GamePhase::kDraw) {
        throw std::runtime_error("The game has already finished.");
    }
    if (!is_valid_move(move)) {
        throw std::invalid_argument("Invalid move: (" +
            std::to_string(move.get_x()) + "," +
            std::to_string(move.get_y()) + ")");
    }

    int pid = move.get_id() - 1;

    if (phase_ == GamePhase::kNotStarted) {
        phase_ = GamePhase::kInProgress;
    }

    // Place the peg
    Position pos{move.get_x(), move.get_y()};
    Cell cell = (pid == 0) ? Cell::kPegP1 : Cell::kPegP2;
    grid_[pos.y][pos.x] = cell;

    Peg peg{pos, pid};
    pegs_.push_back(peg);
    pegs_by_player_[pid].push_back(peg);

    preset::Logger::debug("Peg placed: " +
        std::to_string(pos.x) + "," + std::to_string(pos.y) +
        " by player " + std::to_string(pid));

    // Generate bridges
    generate_bridges(peg);

    // Check game end
    if (check_win(pid)) {
        phase_ = GamePhase::kFinished;
        preset::Logger::info("Player " + std::to_string(pid) + " has won!");
    } else if (check_draw()) {
        phase_ = GamePhase::kDraw;
        preset::Logger::info("Draw!");
    }

    turn_++;
}

// =====================================================================
// Knight moves
// =====================================================================

std::vector<Position> Board::knight_offsets() {
    return {
        { 1, -2}, { 2, -1}, { 2,  1}, { 1,  2},
        {-1,  2}, {-2,  1}, {-2, -1}, {-1, -2}
    };
}

std::vector<Position> Board::knight_neighbors(const Position& pos) const {
    std::vector<Position> result;
    for (const auto& off : knight_offsets()) {
        Position nb{pos.x + off.x, pos.y + off.y};
        if (is_in_bounds(nb)) {
            result.push_back(nb);
        }
    }
    return result;
}

// =====================================================================
// Bridges
// =====================================================================

bool Board::bridges_cross(const Bridge& a, const Bridge& b) {
    // A shared endpoint is not a crossing.
    if (a.from == b.from || a.from == b.to ||
        a.to   == b.from || a.to   == b.to) {
        return false;
    }

    // Orientation function (cross product)
    auto orient = [](const Position& p, const Position& q, const Position& r) -> int {
        int val = (q.x - p.x) * (r.y - p.y) -
                  (q.y - p.y) * (r.x - p.x);
        if (val > 0) return  1;   // counterclockwise
        if (val < 0) return -1;   // clockwise
        return 0;                 // collinear
    };

    int o1 = orient(a.from, a.to, b.from);
    int o2 = orient(a.from, a.to, b.to);
    int o3 = orient(b.from, b.to, a.from);
    int o4 = orient(b.from, b.to, a.to);

    // Standard case: different orientations mean the segments cross
    if (o1 != o2 && o3 != o4) return true;

    // Collinear case: on the same line, do they overlap?
    // This is extremely rare on the knight-move grid, but handled for completeness.
    auto on_segment = [](const Position& p, const Position& q, const Position& r) {
        return r.x <= std::max(p.x, q.x) && r.x >= std::min(p.x, q.x) &&
               r.y <= std::max(p.y, q.y) && r.y >= std::min(p.y, q.y);
    };

    if (o1 == 0 && on_segment(a.from, a.to, b.from)) return true;
    if (o2 == 0 && on_segment(a.from, a.to, b.to))   return true;
    if (o3 == 0 && on_segment(b.from, b.to, a.from)) return true;
    if (o4 == 0 && on_segment(b.from, b.to, a.to))   return true;

    return false;
}

bool Board::would_cross_existing(const Bridge& candidate) const {
    return std::any_of(bridges_.begin(), bridges_.end(),
        [&](const Bridge& existing) {
            return bridges_cross(candidate, existing);
        });
}

void Board::generate_bridges(const Peg& new_peg) {
    // Scan all own pegs and find knight-move pairs
    int pid = new_peg.player_id;
    for (const auto& other : pegs_by_player_[pid]) {
        if (other.pos == new_peg.pos) continue;

        // Knight-move distance?
        int dx = std::abs(new_peg.pos.x - other.pos.x);
        int dy = std::abs(new_peg.pos.y - other.pos.y);
        if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) continue;

        // Build a bridge if it does not cross any existing bridge
        Bridge candidate{new_peg.pos, other.pos, pid};
        if (!would_cross_existing(candidate)) {
            bridges_.push_back(candidate);
            bridges_by_player_[pid].push_back(candidate);
            preset::Logger::debug("Bridge created: (" +
                std::to_string(candidate.from.x) + "," +
                std::to_string(candidate.from.y) + ") -> (" +
                std::to_string(candidate.to.x) + "," +
                std::to_string(candidate.to.y) + ")");
        }
    }
}

// =====================================================================
// Game end
// =====================================================================

bool Board::is_connected_across(int player_id) const {
    const auto& pegs = pegs_by_player_[player_id];
    const auto& bridges = bridges_by_player_[player_id];

    if (pegs.empty()) return false;

    // Index of a peg in the pegs array
    // Adjacency list: peg index -> neighboring peg indices
    std::vector<std::vector<int>> adj(pegs.size());

    for (const auto& br : bridges) {
        // Find the indices of both endpoints
        int idx_from = -1, idx_to = -1;
        for (int i = 0; i < static_cast<int>(pegs.size()); i++) {
            if (pegs[i].pos == br.from) idx_from = i;
            if (pegs[i].pos == br.to)   idx_to   = i;
        }
        if (idx_from >= 0 && idx_to >= 0) {
            adj[idx_from].push_back(idx_to);
            adj[idx_to].push_back(idx_from);
        }
    }

    // Start nodes: all pegs that lie on one of the player's own sides
    std::queue<int> q;
    std::vector<bool> visited(pegs.size(), false);

    for (int i = 0; i < static_cast<int>(pegs.size()); i++) {
        Direction dir = get_direction(pegs[i].pos);
        bool is_start = false;
        if (player_id == 0) {
            is_start = (dir == Direction::kTop);
        } else {
            is_start = (dir == Direction::kLeft);
        }
        if (is_start) {
            q.push(i);
            visited[i] = true;
        }
    }

    // BFS
    while (!q.empty()) {
        int cur = q.front(); q.pop();

        Direction dir = get_direction(pegs[cur].pos);
        bool is_goal = false;
        if (player_id == 0) {
            is_goal = (dir == Direction::kBottom);
        } else {
            is_goal = (dir == Direction::kRight);
        }
        if (is_goal) return true;

        for (int nb : adj[cur]) {
            if (!visited[nb]) {
                visited[nb] = true;
                q.push(nb);
            }
        }
    }

    return false;
}

bool Board::check_win(int player_id) const {
    if (player_id < 0 || player_id >= kNumPlayers) return false;
    return is_connected_across(player_id);
}

bool Board::has_potential_connection(int player_id) const {
    if (player_id < 0 || player_id >= kNumPlayers) return false;

    const Cell opponent_cell = (player_id == 0) ? Cell::kPegP2 : Cell::kPegP1;

    auto is_available = [&](const Position& pos) {
        return is_playable(pos, player_id) && grid_[pos.y][pos.x] != opponent_cell;
    };

    std::queue<Position> q;
    std::vector<std::vector<bool>> visited(
        height_, std::vector<bool>(width_, false));

    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            Position pos{x, y};
            if (!is_available(pos)) continue;

            Direction dir = get_direction(pos);
            const bool is_start = (player_id == 0)
                ? dir == Direction::kTop
                : dir == Direction::kLeft;

            if (is_start) {
                q.push(pos);
                visited[y][x] = true;
            }
        }
    }

    while (!q.empty()) {
        Position cur = q.front();
        q.pop();

        Direction dir = get_direction(cur);
        const bool is_goal = (player_id == 0)
            ? dir == Direction::kBottom
            : dir == Direction::kRight;

        if (is_goal) return true;

        for (const Position& next : knight_neighbors(cur)) {
            if (visited[next.y][next.x] || !is_available(next)) continue;

            Bridge candidate{cur, next, player_id};
            if (would_cross_existing(candidate)) continue;

            visited[next.y][next.x] = true;
            q.push(next);
        }
    }

    return false;
}

bool Board::check_draw() const {
    if (check_win(0) || check_win(1)) return false;

    auto has_legal_position = [&](int player_id) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                const Position pos{x, y};
                if (is_playable(pos, player_id) &&
                    grid_[y][x] == Cell::kEmpty) {
                    return true;
                }
            }
        }
        return false;
    };

    const int next_player = (turn_ + 1) % kNumPlayers;
    if (!has_legal_position(next_player)) return true;

    return !has_potential_connection(0) && !has_potential_connection(1);
}

}  // namespace bruecken
