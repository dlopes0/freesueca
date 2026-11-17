#include "freesueca/hidden_hand_sampler.hpp"
#include "freesueca/card_tracker.hpp"
#include "freesueca/game.hpp"
#include "freesueca/void_tracker.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>

using namespace freesueca;

TEST_CASE("HiddenHandSampler preserves own hand and distributes unknown cards")
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

    CHECK(hands[context.player_id] == context.hand);

    // Reconstruct the unknown cards.
    CardTracker tracker;
    tracker.record(context.trick_history);
    tracker.record(context.current_trick);

    const auto unknown_cards =
        tracker.unknown_cards(context.hand);

    std::vector<Card> distributed_cards;

    for (PlayerId player = 0; player < Game::PLAYER_COUNT; ++player)
    {
        if (player == context.player_id)
            continue;

        distributed_cards.insert(
            distributed_cards.end(),
            hands[player].begin(),
            hands[player].end());
    }

    REQUIRE(distributed_cards.size() == unknown_cards.size());

    // Every unknown card must be distributed exactly once.
    for (const Card& card : unknown_cards)
    {
        CHECK(
            std::count(
                distributed_cards.begin(),
                distributed_cards.end(),
                card
            ) == 1
        );
    }
}

TEST_CASE("HiddenHandSampler respects known void suits")
{
    Game game(1234);
    game.start();

    bool found_void = false;

    // Play until we have observed at least one player
    // failing to follow suit.
    while (!game.is_hand_over() && !found_void)
    {
        const auto context = game.decision_context();

        game.play_card(context.legal_card_indices.front());

        VoidTracker tracker;
        tracker.record(game.trick_history());
        tracker.record(game.current_trick());

        for (PlayerId player = 0;
             player < Game::PLAYER_COUNT;
             ++player)
        {
            for (const Suit suit : {
                Suit::Hearts,
                Suit::Diamonds,
                Suit::Clubs,
                Suit::Spades
            })
            {
                if (tracker.is_void(player, suit))
                    found_void = true;
            }
        }
    }

    REQUIRE(found_void);
    REQUIRE_FALSE(game.is_hand_over());

    const PlayerDecisionContext context =
        game.decision_context();

    HiddenHandSampler sampler(42);
    const SampledHands hands = sampler.sample(context);

    VoidTracker tracker;
    tracker.record(context.trick_history);
    tracker.record(context.current_trick);

    for (PlayerId player = 0;
         player < Game::PLAYER_COUNT;
         ++player)
    {
        for (const Suit suit : {
            Suit::Hearts,
            Suit::Diamonds,
            Suit::Clubs,
            Suit::Spades
        })
        {
            if (!tracker.is_void(player, suit))
                continue;

            for (const Card& card : hands[player])
                CHECK(card.suit() != suit);
        }
    }
}

TEST_CASE("HiddenHandSampler is reproducible with the same seed")
{
    Game game(1234);
    game.start();

    const PlayerDecisionContext context =
        game.decision_context();

    HiddenHandSampler sampler_a(42);
    HiddenHandSampler sampler_b(42);

    CHECK(
        sampler_a.sample(context) ==
        sampler_b.sample(context)
    );
}