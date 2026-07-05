/**
 * @file main.cpp
 * @brief Connects command-line settings, players, board, and GUI.
 * @author Hao Guo
 */

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "argument_parser.h"
#include "logger.h"
#include "move.h"
#include "player.h"
#include "player_type.h"

#include "bruecken/board.h"
#include "bruecken/gui_renderer.h"
#include "bruecken/human_player.h"
#include "bruecken/random_ai_player.h"

namespace bruecken {
namespace {

constexpr int kPlayerCount = 2;

/** Return whether a configured player needs mouse input from the GUI. */
bool has_human_player(const preset::Settings& settings) {
    return std::any_of(
        settings.player_types.begin(),
        settings.player_types.end(),
        [](preset::PlayerType type) {
            return type == preset::PlayerType::HUMAN;
        });
}

/**
 * Create one player behind the interface required by the school preset.
 * Unsupported optional player types are rejected explicitly.
 */
std::unique_ptr<preset::Player> create_player(
    preset::PlayerType type,
    preset::PlayerGuiAccess* gui) {

    switch (type) {
        case preset::PlayerType::HUMAN:
            if (gui == nullptr) {
                throw std::logic_error("A human player requires a GUI");
            }
            return std::make_unique<HumanPlayer>(*gui);

        case preset::PlayerType::RANDOM_AI:
            return std::make_unique<RandomAIPlayer>();

        case preset::PlayerType::SIMPLE_AI:
            throw std::invalid_argument(
                "SIMPLE_AI was selected but is not implemented");

        case preset::PlayerType::ADVANCED_AI:
            throw std::invalid_argument(
                "ADVANCED_AI was selected but is not implemented");
    }

    throw std::invalid_argument("Unknown player type");
}

/**
 * Wait after an AI move while keeping an optional GUI responsive.
 * @return false if the user requested that the GUI window close.
 */
bool wait_after_ai_move(GuiRenderer* gui, long delay_ms) {
    if (delay_ms <= 0) {
        return gui == nullptr || !gui->should_close();
    }

    if (gui == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        return true;
    }

    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(delay_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        if (gui->should_close()) {
            return false;
        }
        gui->draw_frame();
    }
    return !gui->should_close();
}

/** Run one complete two-player game. */
void run_game(const preset::Settings& settings) {
    if (settings.player_types.size() != kPlayerCount) {
        throw std::invalid_argument(
            "Bruecken requires exactly two player types");
    }

    Board board(
        settings.board_config.width,
        settings.board_config.height,
        settings.board_config.rotation);

    // A human always needs the GUI; --gui additionally enables it for AI games.
    const bool use_gui = settings.show_GUI || has_human_player(settings);
    std::unique_ptr<GuiRenderer> gui;
    if (use_gui) {
        gui = std::make_unique<GuiRenderer>(
            board,
            settings.player_colors,
            settings.player_names,
            "BrueckenSpiel");
    }

    std::array<std::unique_ptr<preset::Player>, kPlayerCount> players;
    for (int index = 0; index < kPlayerCount; ++index) {
        players[index] = create_player(
            settings.player_types[index],
            gui.get());

        // Preset player IDs are one-based, while array indexes are zero-based.
        players[index]->init(settings.board_config, index + 1);
        preset::Logger::info(
            "Initialized player " + std::to_string(index + 1) +
            ": " + std::string(players[index]->get_name()));
    }

    bool window_closed = false;
    while (board.get_phase() != GamePhase::kFinished &&
           board.get_phase() != GamePhase::kDraw) {

        if (gui != nullptr) {
            if (gui->should_close()) {
                window_closed = true;
                break;
            }
            gui->draw_frame();
        }

        const int current = board.get_current_player();
        const int opponent = (current + 1) % kPlayerCount;
        const auto move = players[current]->request();

        // A human may not have clicked yet. AI players only return no move
        // when their private board has no legal position left.
        if (!move.has_value()) {
            if (settings.player_types[current] == preset::PlayerType::HUMAN) {
                continue;
            }
            throw std::runtime_error(
                "The current AI could not provide a legal move");
        }

        if (move->get_id() != current + 1) {
            throw std::runtime_error(
                "Player returned a move with the wrong player ID");
        }
        if (!board.is_valid_move(*move)) {
            throw std::runtime_error(
                "Player returned a move rejected by the main board");
        }

        // The requesting player already applied the move to its private board.
        // The main board verifies it, then the opponent verifies it independently.
        board.apply_move(*move);
        players[opponent]->update(*move);

        preset::Logger::info(
            "Player " + std::to_string(current + 1) + " moved to (" +
            std::to_string(move->get_x()) + ", " +
            std::to_string(move->get_y()) + ")");

        if (settings.player_types[current] != preset::PlayerType::HUMAN &&
            !wait_after_ai_move(gui.get(), settings.delay)) {
            window_closed = true;
            break;
        }
    }

    if (window_closed) {
        preset::Logger::info("Game stopped because the window was closed");
        return;
    }

    if (board.get_phase() == GamePhase::kDraw) {
        preset::Logger::info("The game ended in a draw");
    } else {
        const int winner = board.check_win(0) ? 0 : 1;
        preset::Logger::info(
            "Player " + std::to_string(winner + 1) + " (" +
            settings.player_names[winner] + ") won the game");
    }

    // Keep the final board and highlighted winning path visible.
    while (gui != nullptr && !gui->should_close()) {
        gui->draw_frame();
    }
}

}  // namespace
}  // namespace bruecken

int main(int argc, char* argv[]) {
    const std::string project_name = "BrueckenSpiel";
    const std::string project_version = "0.1.0";
    const std::vector<std::string> project_authors = {
        "Zhibo Zhang",
        "Hao Guo",
        "Junke Pu",
        "Zhixin Fu"
    };

    // No optional preset feature is advertised until it is fully implemented.
    const std::vector<preset::OptionalFeature> implemented_features = {};

    try {
        preset::ArgumentParser parser(
            argc,
            argv,
            project_name,
            project_version,
            project_authors,
            implemented_features);

        bruecken::run_game(parser.get_settings());
    } catch (const std::exception& exception) {
        preset::Logger::error(
            std::string("Fatal error: ") + exception.what());
        return 1;
    }

    return 0;
}
