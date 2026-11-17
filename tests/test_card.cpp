#include <catch2/catch_test_macros.hpp>

#include "freesueca/card.hpp"

#include <array>
#include <string_view>

using namespace freesueca;

TEST_CASE("Card stores its suit and rank", "[card]")
{
    const Card card(Suit::Hearts, Rank::Ace);

    REQUIRE(card.suit() == Suit::Hearts);
    REQUIRE(card.rank() == Rank::Ace);
}

TEST_CASE("Card calculates Sueca points", "[card][points]")
{
    struct TestCase
    {
        Rank rank;
        int expected_points;
    };

    constexpr std::array cases {
        TestCase{Rank::Ace, 11},
        TestCase{Rank::Seven, 10},
        TestCase{Rank::King, 4},
        TestCase{Rank::Jack, 3},
        TestCase{Rank::Queen, 2},
        TestCase{Rank::Six, 0},
        TestCase{Rank::Five, 0},
        TestCase{Rank::Four, 0},
        TestCase{Rank::Three, 0},
        TestCase{Rank::Two, 0}
    };

    for (const TestCase& test : cases)
    {
        const Card card(Suit::Hearts, test.rank);

        CHECK(card.points() == test.expected_points);
    }
}

TEST_CASE("Cards can be compared", "[card][comparison]")
{
    const Card first(Suit::Hearts, Rank::Ace);
    const Card second(Suit::Hearts, Rank::Ace);
    const Card third(Suit::Spades, Rank::Ace);

    REQUIRE(first == second);
    REQUIRE(first != third);
}
