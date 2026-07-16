/**
 * @file test_player.cpp
 * @brief Contract tests for HumanPlayer and RandomAIPlayer.
 * @author Zhixin Fu
 */

#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "board_config.h"
#include "move.h"
#include "player_gui_access.h"

#include "bruecken/board.h"
#include "bruecken/human_player.h"
#include "bruecken/random_ai_player.h"

namespace
{

    void require(bool condition, const std::string &message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void require_exception(
        const std::function<void()> &operation,
        const std::string &message)
    {

        try
        {
            operation();
        }
        catch (const std::exception &)
        {
            return;
        }
        throw std::runtime_error(message);
    }

    class MockGui final : public preset::PlayerGuiAccess
    {
    public:
        void provide(preset::Move move)
        {
            pending_move_ = move;
        }

        std::optional<preset::Move>
        request_move_from_current_human_player() override
        {
            auto result = pending_move_;
            pending_move_.reset();
            return result;
        }

    private:
        std::optional<preset::Move> pending_move_;
    };

    void test_initialization_contract(const preset::BoardConfig &config)
    {
        bruecken::RandomAIPlayer player;

        require(!player.get_name().empty(),
                "get_name must work before initialization");
        require_exception([&]
                          { player.request(); },
                          "request before init must throw");
        require_exception(
            [&]
            { player.update(preset::Move(1, 1, 1)); },
            "update before init must throw");

        require_exception(
            [&]
            {
                bruecken::RandomAIPlayer invalid_player;
                invalid_player.init(config, 0);
            },
            "init must reject player ID 0");
        require_exception(
            [&]
            {
                bruecken::RandomAIPlayer invalid_player;
                invalid_player.init(config, 3);
            },
            "init must reject player ID 3");

        player.init(config, 1);
        require_exception([&]
                          { player.init(config, 1); },
                          "init may only succeed once");
    }

    void test_random_ai_turn_contract(const preset::BoardConfig &config)
    {
        bruecken::RandomAIPlayer player_one;
        bruecken::RandomAIPlayer player_two;
        player_one.init(config, 1);
        player_two.init(config, 2);

        require_exception([&]
                          { player_two.request(); },
                          "Player 2 must not request on Player 1's turn");
        require_exception(
            [&]
            { player_one.update(preset::Move(1, 1, 2)); },
            "Player 1 must not update during its own turn");

        const auto first_move = player_one.request();
        require(first_move.has_value(), "Random AI must find an opening move");

        preset::Move tmp1 = *first_move;
        require(tmp1.get_id() == 1,
                "Random AI must return its own player ID");
        require_exception([&]
                          { player_one.request(); },
                          "A second successful request in one turn must throw");

        player_two.update(*first_move);
        require_exception(
            [&]
            { player_two.update(*first_move); },
            "A second update in one turn must throw");

        const auto second_move = player_two.request();
        require(second_move.has_value(), "Player 2 must find a legal move");
        
        preset::Move tmp2 = *second_move;
        require(tmp2.get_id() == 2,
                "Player 2 must return its own player ID");
        player_one.update(*second_move);
    }

    void test_human_player_contract(const preset::BoardConfig &config)
    {
        MockGui gui_one;
        MockGui gui_two;
        bruecken::HumanPlayer player_one(gui_one);
        bruecken::HumanPlayer player_two(gui_two);
        player_one.init(config, 1);
        player_two.init(config, 2);

        require(!player_one.request().has_value(),
                "No GUI input must produce nullopt");
        require(!player_one.request().has_value(),
                "Polling without a move must remain allowed");

        gui_one.provide(preset::Move(2, 2, 2));
        require_exception([&]
                          { player_one.request(); },
                          "HumanPlayer must reject a GUI move for another player");

        gui_one.provide(preset::Move(2, 2, 1));
        const auto first_move = player_one.request();
        require(first_move.has_value(), "HumanPlayer must return valid GUI input");
        require_exception([&]
                          { player_one.request(); },
                          "HumanPlayer must reject a repeated request");

        player_two.update(*first_move);
        gui_two.provide(preset::Move(999, 999, 2));
        require_exception([&]
                          { player_two.request(); },
                          "HumanPlayer must throw for illegal GUI input");

        gui_two.provide(preset::Move(0, 3, 2));
        const auto second_move = player_two.request();
        require(second_move.has_value(),
                "HumanPlayer 2 must accept legal GUI input");
        player_one.update(*second_move);
    }

    void test_terminal_state_contract(const preset::BoardConfig &config)
    {
        MockGui gui_one;
        MockGui gui_two;
        bruecken::HumanPlayer player_one(gui_one);
        bruecken::HumanPlayer player_two(gui_two);
        player_one.init(config, 1);
        player_two.init(config, 2);

        // Player 1 forms a top-to-bottom chain with two knight-move bridges.
        gui_one.provide(preset::Move(2, 0, 1));
        const auto p1_first = player_one.request();
        require(p1_first.has_value(), "Player 1 opening move was not returned");
        player_two.update(*p1_first);

        gui_two.provide(preset::Move(6, 2, 2));
        const auto p2_first = player_two.request();
        require(p2_first.has_value(), "Player 2 first move was not returned");
        player_one.update(*p2_first);

        gui_one.provide(preset::Move(3, 2, 1));
        const auto p1_second = player_one.request();
        require(p1_second.has_value(), "Player 1 second move was not returned");
        player_two.update(*p1_second);

        gui_two.provide(preset::Move(6, 4, 2));
        const auto p2_second = player_two.request();
        require(p2_second.has_value(), "Player 2 second move was not returned");
        player_one.update(*p2_second);

        gui_one.provide(preset::Move(4, 4, 1));
        const auto p1_third = player_one.request();
        require(p1_third.has_value(), "Player 1 third move was not returned");
        player_two.update(*p1_third);

        gui_two.provide(preset::Move(0, 2, 2));
        const auto p2_third = player_two.request();
        require(p2_third.has_value(), "Player 2 third move was not returned");
        player_one.update(*p2_third);

        gui_one.provide(preset::Move(3, 6, 1));
        const auto winning_move = player_one.request();
        require(winning_move.has_value(), "Winning move was not returned");
        player_two.update(*winning_move);

        require_exception([&]
                          { player_one.request(); },
                          "The winner must reject requests after game end");
        require_exception([&]
                          { player_two.request(); },
                          "The opponent must reject requests after game end");
        require_exception(
            [&]
            { player_two.update(preset::Move(3, 3, 1)); },
            "Players must reject updates after game end");
    }

    bool is_finished(const bruecken::Board &board)
    {
        const auto phase = board.get_phase();
        return phase == bruecken::GamePhase::kFinished ||
               phase == bruecken::GamePhase::kDraw;
    }

void test_random_ai_long_game(const preset::BoardConfig &config)
{
    bruecken::Board board(
        config.width,
        config.height,
        config.rotation);
    bruecken::RandomAIPlayer player_one;
    bruecken::RandomAIPlayer player_two;

    player_one.init(config, 1);
    player_two.init(config, 2);

    // Play until the game ends or the safety limit is reached.
    for (int turn = 0; turn < 5000; ++turn)
    {
        if (is_finished(board))
        {
            break;
        }

        auto move1 = player_one.request();

        if (!move1.has_value())
        {
            break;
        }

        board.apply_move(*move1);
        player_two.update(*move1);

        if (is_finished(board))
        {
            break;
        }

        auto move2 = player_two.request();

        if (!move2.has_value())
        {
            break;
        }

        board.apply_move(*move2);
        player_one.update(*move2);
    }

    require(is_finished(board),
            "Random AI long game must reach a terminal board state.");

    require_exception(
        [&]
        { player_one.request(); },
        "Player 1 must reject requests after game end.");

    require_exception(
        [&]
        { player_two.request(); },
        "Player 2 must reject requests after game end.");
}

void test_player_board_synchronization(const preset::BoardConfig &config)
{
    bruecken::Board board(
        config.width,
        config.height,
        config.rotation);
    bruecken::RandomAIPlayer player_one;
    bruecken::RandomAIPlayer player_two;

    player_one.init(config, 1);
    player_two.init(config, 2);

    for (int turn = 0; turn < 5000; ++turn)
    {
        if (is_finished(board))
        {
            break;
        }

        auto move1 = player_one.request();

        if (!move1.has_value())
        {
            break;
        }

        board.apply_move(*move1);
        player_two.update(*move1);

        if (is_finished(board))
        {
            break;
        }

        auto move2 = player_two.request();

        if (!move2.has_value())
        {
            break;
        }

        board.apply_move(*move2);
        player_one.update(*move2);
    }

    require(is_finished(board),
            "Player board synchronization test must reach a terminal state.");
}

void test_large_board()
{
    preset::BoardConfig config;

    config.width = 96;
    config.height = 96;
    config.rotation = 0;

    bruecken::RandomAIPlayer player_one;
    bruecken::RandomAIPlayer player_two;

    player_one.init(config, 1);
    player_two.init(config, 2);

    auto move = player_one.request();

    require(move.has_value(),
            "Random AI must find a move on a 96x96 board.");

    player_two.update(*move);
}

} // namespace

int main()
{
    try
    {
        preset::BoardConfig config;
        config.width = 7;
        config.height = 7;
        config.rotation = 0;

        test_initialization_contract(config);

        test_random_ai_turn_contract(config);

        test_human_player_contract(config);

        test_terminal_state_contract(config);

        test_random_ai_long_game(config);

        test_player_board_synchronization(config);

        // test_rotation_initialization();

        test_large_board();

        std::cout << "All player contract tests passed.\n";
        return 0;
    }
    catch (const std::exception &exception)
    {
        std::cerr << "Player contract test failed: "
                  << exception.what() << '\n';
        return 1;
    }
}
