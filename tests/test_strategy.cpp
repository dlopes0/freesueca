#include <catch2/catch_test_macros.hpp>

#include "freesueca/game.hpp"
#include "freesueca/random_strategy.hpp"
#include "freesueca/greedy_strategy.hpp"

#include <algorithm>

using namespace freesueca;

TEST_CASE("Random strategy always selects a legal card")
{
    Game game(12345);
    game.start();

    RandomStrategy strategy(42);

    const auto legal = game.legal_card_indices();

    REQUIRE_FALSE(legal.empty());

    const std::size_t selected = strategy.choose_card(game.decision_context());

    CHECK(
        std::find(legal.begin(), legal.end(), selected) != legal.end()
    );
}

TEST_CASE("Random strategy follows the lead suit")
{
    Game game(12345);
    game.start();

    // First player leads.
    game.play_card(0);

    RandomStrategy strategy(42);

    const auto legal = game.legal_card_indices();

    REQUIRE_FALSE(legal.empty());

    const Player& current = game.player(game.current_player_id());
    const Suit lead = game.current_trick().lead_suit();

    const std::size_t selected = strategy.choose_card(game.decision_context());

    REQUIRE(
        std::find(legal.begin(), legal.end(), selected) != legal.end()
    );

    if (current.has_suit(lead)) {
        CHECK(current.hand()[selected].suit() == lead);
    }
}

TEST_CASE("Random strategy can play a complete hand")
{
    Game game(12345);
    game.start();

    RandomStrategy strategy(42);

    while (!game.is_hand_over()) {
        const std::size_t selected = strategy.choose_card(game.decision_context());
        game.play_card(selected);
    }

    CHECK(game.tricks_played() == 10);
    CHECK(game.is_hand_over());
}

TEST_CASE("GreedyStrategy always chooses a legal card")
{
    Game game(1234);
    GreedyStrategy strategy;

    game.start();

    while (!game.is_hand_over())
    {
        const auto legal_indices = game.legal_card_indices();

        const std::size_t selection = strategy.choose_card(game.decision_context());

        REQUIRE(
            std::find(
                legal_indices.begin(),
                legal_indices.end(),
                selection
            ) != legal_indices.end()
        );

        game.play_card(selection);
    }
}