#include "freesueca/interface/terminal_input.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using namespace freesueca::interface;

TEST_CASE("Terminal input parses card selection", "[terminal_input]")
{
    std::istringstream input("3");
    std::ostringstream output;

    const PlayerAction action =
        read_player_action(input, output, 10);

    CHECK(action.type == PlayerActionType::PlayCard);
    CHECK(action.card_index == 2);
}

TEST_CASE("Terminal input parses sort command", "[terminal_input]")
{
    std::istringstream input("o");
    std::ostringstream output;

    const PlayerAction action =
        read_player_action(input, output, 10);

    CHECK(action.type == PlayerActionType::SortHand);
}

TEST_CASE("Terminal input rejects selection outside hand", "[terminal_input]")
{
    std::istringstream input("10");
    std::ostringstream output;

    const PlayerAction action =
        read_player_action(input, output, 9);

    CHECK(action.type == PlayerActionType::Invalid);
}

TEST_CASE("Terminal input parses quit command", "[terminal_input]")
{
    std::istringstream input("q");
    std::ostringstream output;

    const PlayerAction action =
        read_player_action(input, output, 10);

    CHECK(action.type == PlayerActionType::Quit);
}

TEST_CASE("Terminal input handles end of input", "[terminal_input]")
{
    std::istringstream input;
    std::ostringstream output;

    const PlayerAction action =
        read_player_action(input, output, 10);

    CHECK(action.type == PlayerActionType::Quit);
}