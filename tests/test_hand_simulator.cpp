#include "freesueca/hand_simulator.hpp"
#include "freesueca/game.hpp"

#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

using namespace freesueca;

TEST_CASE("HandSimulator completes a hand")
{
    Game game(1234);
    game.start();

    // Play two complete tricks.
    for (int i = 0; i < 8; ++i)
    {
        const auto context = game.decision_context();
        game.play_card(context.legal_card_indices.front());
    }

    const PlayerDecisionContext context =
        game.decision_context();

    HiddenHandSampler sampler(42);
    const SampledHands hands = sampler.sample(context);

    HandSimulator simulator;

    const SimulationResult result =
        simulator.simulate(
            context,
            hands,
            context.legal_card_indices.front());

    CHECK(result.team_points[0] + result.team_points[1] == 120);
    CHECK(result.team_tricks[0] + result.team_tricks[1] == 10);
    CHECK(result.team_riscos[0] >= 0);
    CHECK(result.team_riscos[1] >= 0);
}

TEST_CASE("HandSimulator does not modify sampled hands")
{
    Game game(1234);
    game.start();

    const PlayerDecisionContext context =
        game.decision_context();

    HiddenHandSampler sampler(42);
    const SampledHands hands = sampler.sample(context);
    const SampledHands original_hands = hands;

    HandSimulator simulator;

    const SimulationResult result = simulator.simulate(
        context,
        hands,
        context.legal_card_indices.front());

    (void)result;
    CHECK(hands == original_hands);
}

TEST_CASE("HandSimulator rejects illegal candidate cards")
{
    Game game(1234);
    game.start();

    const PlayerDecisionContext context =
        game.decision_context();

    HiddenHandSampler sampler(42);
    const SampledHands hands = sampler.sample(context);

    HandSimulator simulator;

    CHECK_THROWS_AS(
        simulator.simulate(context, hands, context.hand.size()),
        std::invalid_argument);
}

TEST_CASE("HandSimulator is deterministic for the same sampled hands")
{
    Game game(1234);
    game.start();

    const PlayerDecisionContext context =
        game.decision_context();

    HiddenHandSampler sampler(42);
    const SampledHands hands = sampler.sample(context);

    HandSimulator simulator;

    const auto first = simulator.simulate(
        context, hands, context.legal_card_indices.front());

    const auto second = simulator.simulate(
        context, hands, context.legal_card_indices.front());

    CHECK(first.team_points == second.team_points);
    CHECK(first.team_tricks == second.team_tricks);
    CHECK(first.team_riscos == second.team_riscos);
}
