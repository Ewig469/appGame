/**
 * @file gui_renderer.h
 * @brief Reine Raylib-Darstellung fuer das Bruecken-Spiel.
 * @author Hao Guo
 */

#pragma once

#include <string>
#include <vector>

#include "bruecken/board.h"

namespace bruecken {

/**
 * @brief Initialisiert das Fenster und zeichnet Brett, Steine und Bruecken.
 *
 * Der Renderer besitzt keine Eingabe- oder Spiellogik. Er liest bei jedem
 * Frame den aktuellen Zustand des uebergebenen Boards. Mausinteraktion,
 * Spielstatus und Spieleranbindung bleiben bewusst von der reinen
 * Darstellung getrennt.
 */
class GuiRenderer final {
public:
    /**
     * @param board Das anzuzeigende Spielbrett.
     * @param player_colors Spielerfarben als Hex-RGB-Strings.
     * @param title Titel des Betriebssystemfensters.
     */
    GuiRenderer(const Board& board,
                std::vector<std::string> player_colors,
                std::string title = "BrueckenSpiel");

    ~GuiRenderer();

    GuiRenderer(const GuiRenderer&) = delete;
    GuiRenderer& operator=(const GuiRenderer&) = delete;

    /** Fuehrt eine einfache, blockierende Raylib-Zeichenschleife aus. */
    void run();

    /** Zeichnet genau einen Frame aus dem aktuellen Board-Zustand. */
    void draw_frame();

    /** Gibt an, ob das Schliessen des Fensters angefordert wurde. */
    bool should_close() const;

private:
    const Board& board_;
    std::vector<std::string> player_colors_;
    bool window_open_ = false;
};

}  // namespace bruecken
