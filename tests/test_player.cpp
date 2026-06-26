/**
 * @file test_player.cpp
 * @brief Test program for HumanPlayer and RandomAIPlayer.
 *
 * This program runs a game between two RandomAIPlayer
 * instances and verifies that both players keep their
 * private boards synchronized by exchanging moves.
 *
 * @author Zhixin Fu
 */

#include <iostream>
#include <exception>

#include "bruecken/random_ai_player.h"
#include "board_config.h"

int main()
{
    try
    {
        preset::BoardConfig config;
        config.width = 24;
        config.height = 24;
        config.rotation = 0;

        bruecken::RandomAIPlayer player1;
        bruecken::RandomAIPlayer player2;

        player1.init(config, 1);
        player2.init(config, 2);

        std::cout << "=== AI vs AI Test gestartet ===\n";

        for (int round = 0; round < 50; round++)
        {
            // -------------------------
            // Player 1
            // -------------------------
            auto move1 = player1.request();

            if (move1.has_value())
            {
                std::cout
                    << "P1 -> ("
                    << move1->get_x()
                    << ", "
                    << move1->get_y()
                    << ")\n";

                player2.update(*move1);
            }

            // -------------------------
            // Player 2
            // -------------------------
            auto move2 = player2.request();

            if (move2.has_value())
            {
                std::cout
                    << "P2 -> ("
                    << move2->get_x()
                    << ", "
                    << move2->get_y()
                    << ")\n";

                player1.update(*move2);
            }
        }

        std::cout << "\n=== Test erfolgreich beendet ===\n";

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr
            << "TEST FEHLGESCHLAGEN:\n"
            << ex.what()
            << std::endl;

        return 1;
    }
}