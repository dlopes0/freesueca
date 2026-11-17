#include <catch2/catch_test_macros.hpp>

#include "freesueca/deck.hpp"

#include <algorithm>
#include <array>
#include <vector>
#include <stdexcept>

using namespace freesueca;

TEST_CASE("Deck starts with 40 cards", "[deck]")
{
    Deck deck;

    REQUIRE(deck.remaining() == 40);
}

TEST_CASE("Deck contains every card exactly once", "[deck]")
{
    Deck deck;

    std::vector<Card> cards;

    while (deck.remaining() > 0)
    {
        cards.push_back(deck.deal());
    }

    REQUIRE(cards.size() == 40);

    constexpr std::array suits {
        Suit::Hearts,
        Suit::Diamonds,
        Suit::Clubs,
        Suit::Spades
    };

    constexpr std::array ranks {
        Rank::Ace,
        Rank::Seven,
        Rank::King,
        Rank::Jack,
        Rank::Queen,
        Rank::Six,
        Rank::Five,
        Rank::Four,
        Rank::Three,
        Rank::Two
    };

    for (const auto suit : suits)
    {
        for (const auto rank : ranks)
        {
            const Card expected(suit, rank);

            CHECK(
                std::find(cards.begin(), cards.end(), expected)
                != cards.end()
            );
        }
    }
}

TEST_CASE("Dealing removes cards from the deck", "[deck]")
{
    Deck deck;

    const Card first = deck.deal();

    REQUIRE(deck.remaining() == 39);

    const Card second = deck.deal();

    REQUIRE(deck.remaining() == 38);
    CHECK(first != second);
}

TEST_CASE("Dealing from an empty deck throws", "[deck]")
{
    Deck deck;

    for (int i = 0; i < 40; ++i)
    {
        static_cast<void>(deck.deal());
    }

    REQUIRE(deck.remaining() == 0);

    REQUIRE_THROWS_AS(
      deck.deal(),
      std::out_of_range
    );
}

TEST_CASE("Reset restores all 40 cards", "[deck]")
{
    Deck deck;

    for (int i = 0; i < 10; ++i)
    {
        static_cast<void>(deck.deal());
    }

    REQUIRE(deck.remaining() == 30);

    deck.reset();

    REQUIRE(deck.remaining() == 40);
}

TEST_CASE("Shuffling resets the dealing position", "[deck]")
{
    Deck deck(12345);

    for (int i = 0; i < 10; ++i)
    {
        static_cast<void>(deck.deal());
    }

    REQUIRE(deck.remaining() == 30);

    deck.shuffle();

    REQUIRE(deck.remaining() == 40);
}

TEST_CASE("Decks with the same seed shuffle identically", "[deck]")
{
    Deck first(12345);
    Deck second(12345);

    first.shuffle();
    second.shuffle();

    for (int i = 0; i < 40; ++i)
    {
        CHECK(first.deal() == second.deal());
    }
}