#include "freesueca/game.hpp"
#include "freesueca/monte_carlo_strategy.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

using namespace freesueca;

TEST_CASE("MonteCarloStrategy chooses a legal card")
{
    Game game(1234);
    game.start();

    const PlayerDecisionContext context =
        game.decision_context();

    MonteCarloStrategy strategy(5, 42);

    const std::size_t selected =
        strategy.choose_card(context);

    CHECK(
        std::find(
            context.legal_card_indices.begin(),
            context.legal_card_indices.end(),
            selected
        ) != context.legal_card_indices.end()
    );
}

TEST_CASE("MonteCarloStrategy is reproducible with the same seed")
{
    Game game(1234);
    game.start();

    const PlayerDecisionContext context =
        game.decision_context();

    MonteCarloStrategy first_strategy(5, 42);
    MonteCarloStrategy second_strategy(5, 42);

    CHECK(
        first_strategy.choose_card(context) ==
        second_strategy.choose_card(context)
    );
}

TEST_CASE("MonteCarloStrategy rejects zero samples")
{
    CHECK_THROWS_AS(
        MonteCarloStrategy(0, 42),
        std::invalid_argument
    );
}
