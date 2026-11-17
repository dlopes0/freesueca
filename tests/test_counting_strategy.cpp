
#include "freesueca/counting_strategy.hpp"
#include "freesueca/game.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>

using namespace freesueca;

TEST_CASE("CountingStrategy chooses the weakest safe winning card")
{
    // Hearts are trump.
    Trick previous_trick(Suit::Hearts);

    previous_trick.play_card(0, Card(Suit::Hearts, Rank::Ace));
    previous_trick.play_card(1, Card(Suit::Hearts, Rank::Seven));
    previous_trick.play_card(2, Card(Suit::Hearts, Rank::Jack));
    previous_trick.play_card(3, Card(Suit::Spades, Rank::Two));

    // An opponent has led the Two of Clubs.
    Trick current_trick(Suit::Hearts);
    current_trick.play_card(1, Card(Suit::Clubs, Rank::Two));

    PlayerDecisionContext context{
        2,
        {
            Card(Suit::Hearts, Rank::King),
            Card(Suit::Hearts, Rank::Queen)
        },
        {0, 1},
        Suit::Hearts,
        current_trick,
        {previous_trick}
    };

    CountingStrategy strategy;

    const std::size_t selection = strategy.choose_card(context);

    // Both cards can win, but the Queen is sufficient.
    CHECK(selection == 1);
}

TEST_CASE("CountingStrategy always chooses a legal card")
{
    Game game(1234);
    CountingStrategy strategy;

    game.start();

    while (!game.is_hand_over())
    {
        const PlayerDecisionContext context =
            game.decision_context();

        const std::size_t selection =
            strategy.choose_card(context);

        REQUIRE(
            std::find(
                context.legal_card_indices.begin(),
                context.legal_card_indices.end(),
                selection
            ) != context.legal_card_indices.end()
        );

        game.play_card(selection);
    }
}
