/**
 * @file test_invalid_move.cpp
 * @brief Test for invalid moves.
 *
 * This test verifies that a player correctly rejects
 * an invalid move received from the opponent.
 *
 * Responsibilities:
 * - Create two player instances
 * - Initialize both players
 * - Send an illegal move outside the board
 * - Verify that an exception is thrown
 *
 * @author Zhixin Fu
 */

#include <iostream>
#include <exception>

#include "bruecken/random_ai_player.h"
#include "board_config.h"
#include "move.h"

int main()
{
    try
    {
        preset::BoardConfig config;

        bruecken::RandomAIPlayer p1;
        bruecken::RandomAIPlayer p2;

        p1.init(config, 1);
        p2.init(config, 2);

        // completely outside the board
        preset::Move cheat(999, 999, 1);

        p2.update(cheat);

        std::cout
            << "ERROR: Invalid move was accepted!"
            << std::endl;

        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cout
            << "Exception caught:\n"
            << ex.what()
            << std::endl;

        return 0;
    }
}