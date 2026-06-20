/**
 * @file main.cpp
 * @brief Haupteinstiegspunkt des Bruecken-Spiels.
 * @author Ihr Name
 */

#include <iostream>
#include <stdexcept>

#include "argument_parser.h"
#include "logger.h"
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

        // Kurzer Test der Basistypen
        bruecken::Position test_pos{3, 7};
        preset::Logger::debug("Test-Position: " +
                              std::to_string(test_pos.x) + "," +
                              std::to_string(test_pos.y));

        preset::Logger::info("Board-Grenzen: " +
                             std::to_string(bruecken::kBoardMinSize) + ".." +
                             std::to_string(bruecken::kBoardMaxSize));
        preset::Logger::info("Standard-Fenster: " +
                             std::to_string(bruecken::kDefaultWindowSize) + "px");

        // TODO: Board erstellen
        // TODO: Players erstellen (via settings.player_types)
        // TODO: GUI initialisieren (wenn HUMAN beteiligt oder --gui)
        // TODO: Spielschleife starten

    } catch (const std::exception& ex) {
        preset::Logger::error(std::string("Fehler: ") + ex.what());
        return 1;
    }

    return 0;
}
