/**
 * @file board.cpp
 * @brief Implementierung der Spielfeld-Klasse.
 * @author Ihr Name
 */

#include "bruecken/board.h"

#include <algorithm>
#include <stdexcept>
#include <queue>

#include "move.h"
#include "logger.h"

namespace bruecken {

// =====================================================================
// Konstruktion
// =====================================================================

Board::Board(int width, int height, double rotation)
    : width_(width)
    , height_(height)
    , rotation_(rotation)
{
    if (width < kBoardMinSize || width > kBoardMaxSize ||
        height < kBoardMinSize || height > kBoardMaxSize) {
        throw std::invalid_argument(
            "Board-Groesse muss zwischen " +
            std::to_string(kBoardMinSize) + " und " +
            std::to_string(kBoardMaxSize) + " liegen.");
    }
    if (rotation < kMinRotation || rotation > kMaxRotation) {
        throw std::invalid_argument(
            "Rotation muss zwischen " +
            std::to_string(kMinRotation) + " und " +
            std::to_string(kMaxRotation) + " liegen.");
    }

    // Raster anlegen: height Zeilen, width Spalten
    grid_.assign(height_, std::vector<Cell>(width_, Cell::kEmpty));
}

// =====================================================================
// Koordinaten / Bereichszugehoerigkeit
// =====================================================================

bool Board::is_in_bounds(const Position& pos) const {
    return pos.x >= 0 && pos.x < width_ &&
           pos.y >= 0 && pos.y < height_;
}

bool Board::is_corner(const Position& pos) const {
    return (pos.x == 0 && pos.y == 0) ||
           (pos.x == 0 && pos.y == height_ - 1) ||
           (pos.x == width_ - 1 && pos.y == 0) ||
           (pos.x == width_ - 1 && pos.y == height_ - 1);
}

Direction Board::get_direction(const Position& pos) const {
    if (!is_in_bounds(pos)) return Direction::kNone;

    // Ecken → ueberlappend, gehoert niemandem
    if (is_corner(pos)) return Direction::kNone;

    // Oberer Rand (Zeile 0)
    if (pos.y == 0) return Direction::kTop;
    // Unterer Rand (Zeile height-1)
    if (pos.y == height_ - 1) return Direction::kBottom;
    // Linker Rand (Spalte 0)
    if (pos.x == 0) return Direction::kLeft;
    // Rechter Rand (Spalte width-1)
    if (pos.x == width_ - 1) return Direction::kRight;

    // Innere (N-2)x(M-2)-Flaeche → neutral
    return Direction::kNone;
}

bool Board::is_playable(const Position& pos, int player_id) const {
    if (!is_in_bounds(pos)) return false;

    // Ecken duerfen von niemandem bespielt werden
    if (is_corner(pos)) return false;

    Direction dir = get_direction(pos);

    // Innere Flaeche → beide duerfen
    if (dir == Direction::kNone) return true;

    // Spieler 0: oben / unten
    // Spieler 1: links / rechts
    if (player_id == 0) {
        return dir == Direction::kTop || dir == Direction::kBottom;
    } else {
        return dir == Direction::kLeft || dir == Direction::kRight;
    }
}

bool Board::is_occupied(const Position& pos) const {
    if (!is_in_bounds(pos)) return false;
    return grid_[pos.y][pos.x] != Cell::kEmpty;
}

// =====================================================================
// Zug
// =====================================================================

bool Board::is_valid_move(const preset::Move& move) const {
    int pid = move.get_id() - 1;  // preset::Move IDs sind 1-basiert
    Position pos{move.get_x(), move.get_y()};

    if (!is_in_bounds(pos)) return false;
    if (!is_playable(pos, pid)) return false;
    if (is_occupied(pos)) return false;

    return true;
}

void Board::apply_move(const preset::Move& move) {
    if (phase_ == GamePhase::kFinished || phase_ == GamePhase::kDraw) {
        throw std::runtime_error("Spiel ist bereits beendet.");
    }
    if (!is_valid_move(move)) {
        throw std::invalid_argument("Ungueltiger Zug: (" +
            std::to_string(move.get_x()) + "," +
            std::to_string(move.get_y()) + ")");
    }

    int pid = move.get_id() - 1;

    if (phase_ == GamePhase::kNotStarted) {
        phase_ = GamePhase::kInProgress;
    }

    // Stein setzen
    Position pos{move.get_x(), move.get_y()};
    Cell cell = (pid == 0) ? Cell::kPegP1 : Cell::kPegP2;
    grid_[pos.y][pos.x] = cell;

    Peg peg{pos, pid};
    pegs_.push_back(peg);
    pegs_by_player_[pid].push_back(peg);

    preset::Logger::debug("Stein gesetzt: " +
        std::to_string(pos.x) + "," + std::to_string(pos.y) +
        " von Spieler " + std::to_string(pid));

    // Bruecken generieren
    generate_bridges(peg);

    // Spielende pruefen
    if (check_win(pid)) {
        phase_ = GamePhase::kFinished;
        preset::Logger::info("Spieler " + std::to_string(pid) + " hat gewonnen!");
    } else if (check_draw()) {
        phase_ = GamePhase::kDraw;
        preset::Logger::info("Unentschieden!");
    }

    turn_++;
}

// =====================================================================
// Roesselsprung
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
// Bruecken
// =====================================================================

bool Board::bridges_cross(const Bridge& a, const Bridge& b) {
    // Gemeinsamer Endpunkt → kein Kreuzen
    if (a.from == b.from || a.from == b.to ||
        a.to   == b.from || a.to   == b.to) {
        return false;
    }

    // Orientierungsfunktion (Kreuzprodukt)
    auto orient = [](const Position& p, const Position& q, const Position& r) -> int {
        int val = (q.x - p.x) * (r.y - p.y) -
                  (q.y - p.y) * (r.x - p.x);
        if (val > 0) return  1;   // gegen Uhrzeigersinn
        if (val < 0) return -1;   // im Uhrzeigersinn
        return 0;                 // kollinear
    };

    int o1 = orient(a.from, a.to, b.from);
    int o2 = orient(a.from, a.to, b.to);
    int o3 = orient(b.from, b.to, a.from);
    int o4 = orient(b.from, b.to, a.to);

    // Standardfall: unterschiedliche Orientierung → kreuzen
    if (o1 != o2 && o3 != o4) return true;

    // Kollinear-Fall: auf derselben Linie, ueberlappen sie?
    // (Im Roesselsprung-Raster extrem selten, aber der Vollstaendigkeit halber)
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
    // Alle eigenen Steine durchgehen, Roesselsprung-Paare finden
    int pid = new_peg.player_id;
    for (const auto& other : pegs_by_player_[pid]) {
        if (other.pos == new_peg.pos) continue;

        // Roesselsprung-Abstand?
        int dx = std::abs(new_peg.pos.x - other.pos.x);
        int dy = std::abs(new_peg.pos.y - other.pos.y);
        if (!((dx == 1 && dy == 2) || (dx == 2 && dy == 1))) continue;

        // Bruecke bauen, wenn sie keine existierende kreuzt
        Bridge candidate{new_peg.pos, other.pos, pid};
        if (!would_cross_existing(candidate)) {
            bridges_.push_back(candidate);
            bridges_by_player_[pid].push_back(candidate);
            preset::Logger::debug("Bruecke gebaut: (" +
                std::to_string(candidate.from.x) + "," +
                std::to_string(candidate.from.y) + ") -> (" +
                std::to_string(candidate.to.x) + "," +
                std::to_string(candidate.to.y) + ")");
        }
    }
}

// =====================================================================
// Spielende
// =====================================================================

bool Board::is_connected_across(int player_id) const {
    const auto& pegs = pegs_by_player_[player_id];
    const auto& bridges = bridges_by_player_[player_id];

    if (pegs.empty()) return false;

    // Index eines Steins im pegs-Array
    // Adjazenzliste: Stein-Index → Liste benachbarter Stein-Indizes
    std::vector<std::vector<int>> adj(pegs.size());

    for (const auto& br : bridges) {
        // Finde die Indizes der beiden Endpunkte
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

    // Startknoten: alle Steine, die auf einer der eigenen Seiten liegen
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
    return is_connected_across(player_id);
}

bool Board::check_draw() const {
    // Extrem selten. Einfache Heuristik: Brett voll und niemand gewinnt
    int total_playable = 0;
    for (int y = 0; y < height_; y++) {
        for (int x = 0; x < width_; x++) {
            Position pos{x, y};
            if (!is_corner(pos)) {
                total_playable++;
            }
        }
    }
    if (static_cast<int>(pegs_.size()) >= total_playable) {
        return !check_win(0) && !check_win(1);
    }
    return false;
}

}  // namespace bruecken
