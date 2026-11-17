#include <catch2/catch_test_macros.hpp>

#include "freesueca/deck.hpp"
#include "freesueca/player.hpp"

#include <algorithm>
#include <array>
#include <vector>

using namespace freesueca;

TEST_CASE("Four players can receive a complete deck", "[integration]")
{
    Deck deck(12345);
    deck.shuffle();

    std::array<Player, 4> players {
        Player(0),
        Player(1),
        Player(2),
        Player(3)
    };

    for (int round = 0; round < 10; ++round)
    {
        for (auto& player : players)
        {
            player.add_card(deck.deal());
        }
    }

    for (auto& player : players)
    {
        CHECK(player.card_count() == 10);
    }

    CHECK(deck.remaining() == 0);

    std::vector<Card> all_cards;
    all_cards.reserve(Deck::SIZE);

    for (const auto& player : players)
    {
        for (const auto& card : player.hand())
        {
            const auto it = std::find(
            all_cards.begin(),
            all_cards.end(),
                card
            );

            CHECK(it == all_cards.end());

            all_cards.push_back(card);
        }
    }

    CHECK(all_cards.size() == Deck::SIZE);
}