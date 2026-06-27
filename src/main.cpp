/**
 * @file main.cpp
 * @brief Haupteinstiegspunkt des Bruecken-Spiels.
 * @author 
 */

#include <iostream>
#include <stdexcept>

#include "argument_parser.h"
#include "logger.h"
#include "move.h"
#include "bruecken/bruecken.h"

int main(int argc, char* argv[]) {
    // Projekt-Metadaten — an euer Projekt anpassen
    const std::string project_name = "BrueckenSpiel";
    const std::string project_version = "0.1.0";
    const std::vector<std::string> project_authors = {
        "Autor 1",
        "Autor 2",
        "Autor 3",
        "Autor 4"
    };

    // Bisher keine optionalen Features implementiert
    const std::vector<preset::OptionalFeature> implemented_features = {};

    try {
        preset::ArgumentParser parser(argc, argv,
                                      project_name,
                                      project_version,
                                      project_authors,
                                      implemented_features);

        const auto& settings = parser.get_settings();

        preset::Logger::info("Starte " + project_name + " v" + project_version);
        preset::Logger::info("Spielfeld: " +
                             std::to_string(settings.board_config.width) + "x" +
                             std::to_string(settings.board_config.height) +
                             ", Rotation: " +
                             std::to_string(settings.board_config.rotation) +
                             "°");

        preset::Logger::info("Board-Grenzen: " +
                             std::to_string(bruecken::kBoardMinSize) + ".." +
                             std::to_string(bruecken::kBoardMaxSize));

        // --- Board-Test ---
        bruecken::Board board(
            settings.board_config.width,
            settings.board_config.height,
            settings.board_config.rotation);

        preset::Logger::info("Board erstellt: " +
                             std::to_string(board.get_width()) + "x" +
                             std::to_string(board.get_height()));

        // Einfacher Zug-Test
        preset::Move m1(3, 3, 1);
        if (board.is_valid_move(m1)) {
            board.apply_move(m1);
            preset::Logger::info("Zug 1 ok: (3,3)");
        }

        preset::Move m2(4, 5, 2);
        if (board.is_valid_move(m2)) {
            board.apply_move(m2);
            preset::Logger::info("Zug 2 ok: (4,5)");
        }

        // Roesselsprung-Test
        preset::Move m3(5, 4, 1);
        if (board.is_valid_move(m3)) {
            board.apply_move(m3);
            preset::Logger::info("Zug 3 ok: (5,4) — sollte Bruecke bauen");
        }

        preset::Logger::info("Steine: " + std::to_string(board.get_pegs().size()));
        preset::Logger::info("Bruecken: " + std::to_string(board.get_bridges().size()));

        std::string phase_str;
        switch (board.get_phase()) {
            case bruecken::GamePhase::kNotStarted: phase_str = "Nicht gestartet"; break;
            case bruecken::GamePhase::kInProgress: phase_str = "Laeuft"; break;
            case bruecken::GamePhase::kFinished:   phase_str = "Zu Ende"; break;
            case bruecken::GamePhase::kDraw:       phase_str = "Unentschieden"; break;
        }
        preset::Logger::info("Spielphase: " + phase_str);

        // TODO: Players erstellen (via settings.player_types)
        // TODO: GUI initialisieren (wenn HUMAN beteiligt oder --gui)
        // TODO: Spielschleife starten

    } catch (const std::exception& ex) {
        preset::Logger::error(std::string("Fehler: ") + ex.what());
        return 1;
    }

    return 0;
}
