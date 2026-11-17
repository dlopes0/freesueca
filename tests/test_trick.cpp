#include <catch2/catch_test_macros.hpp>

#include "freesueca/trick.hpp"

#include <stdexcept>

using namespace freesueca;

TEST_CASE("Trick starts empty", "[trick]")
{
    Trick trick(Suit::Hearts);

    CHECK(trick.card_count() == 0);
    CHECK_FALSE(trick.is_complete());
    CHECK(trick.trump_suit() == Suit::Hearts);
}

TEST_CASE("Trick records four played cards", "[trick]")
{
    Trick trick(Suit::Hearts);

    trick.play_card(0, Card(Suit::Clubs, Rank::Ace));
    trick.play_card(1, Card(Suit::Diamonds, Rank::Seven));
    trick.play_card(2, Card(Suit::Spades, Rank::King));
    trick.play_card(3, Card(Suit::Hearts, Rank::Two));

    CHECK(trick.card_count() == 4);
    CHECK(trick.is_complete());
}

TEST_CASE("Highest card of lead suit wins", "[trick]")
{
    Trick trick(Suit::Diamonds);

    trick.play_card(0, Card(Suit::Hearts, Rank::Six));
    trick.play_card(1, Card(Suit::Hearts, Rank::Ace));
    trick.play_card(2, Card(Suit::Clubs, Rank::Seven));
    trick.play_card(3, Card(Suit::Spades, Rank::King));

    CHECK(trick.winning_player() == 1);
}

TEST_CASE("Trump beats the lead suit", "[trick]")
{
    Trick trick(Suit::Diamonds);

    trick.play_card(0, Card(Suit::Hearts, Rank::Ace));
    trick.play_card(1, Card(Suit::Diamonds, Rank::Two));
    trick.play_card(2, Card(Suit::Hearts, Rank::Seven));
    trick.play_card(3, Card(Suit::Clubs, Rank::Ace));

    CHECK(trick.winning_player() == 1);
}

TEST_CASE("Highest trump wins", "[trick]")
{
    Trick trick(Suit::Diamonds);

    trick.play_card(0, Card(Suit::Hearts, Rank::Ace));
    trick.play_card(1, Card(Suit::Diamonds, Rank::Two));
    trick.play_card(2, Card(Suit::Diamonds, Rank::Seven));
    trick.play_card(3, Card(Suit::Hearts, Rank::King));

    CHECK(trick.winning_player() == 2);
}

TEST_CASE("Off-suit cards cannot win against the lead suit", "[trick]")
{
    Trick trick(Suit::Clubs);

    trick.play_card(0, Card(Suit::Hearts, Rank::Two));
    trick.play_card(1, Card(Suit::Spades, Rank::Ace));
    trick.play_card(2, Card(Suit::Hearts, Rank::Ace));
    trick.play_card(3, Card(Suit::Hearts, Rank::Four));

    CHECK(trick.winning_player() == 2);
}

TEST_CASE("Trick calculates its total points", "[trick]")
{
    Trick trick(Suit::Hearts);

    trick.play_card(0, Card(Suit::Hearts, Rank::Ace));
    trick.play_card(1, Card(Suit::Clubs, Rank::Seven));
    trick.play_card(2, Card(Suit::Spades, Rank::King));
    trick.play_card(3, Card(Suit::Diamonds, Rank::Jack));

    CHECK(trick.points() == 28);
}

TEST_CASE("Cannot determine winner before trick is complete", "[trick]")
{
    Trick trick(Suit::Hearts);

    trick.play_card(0, Card(Suit::Hearts, Rank::Ace));

    CHECK_THROWS_AS(trick.winning_player(), std::logic_error);
}

TEST_CASE("Cannot play twice with the same player", "[trick]")
{
    Trick trick(Suit::Hearts);

    trick.play_card(0, Card(Suit::Hearts, Rank::Ace));

    CHECK_THROWS_AS(trick.play_card(0, Card(Suit::Clubs, Rank::Seven)),
        std::logic_error
        );

    CHECK(trick.card_count() == 1);
}

TEST_CASE("Cannot play more than four cards", "[trick]")
{
    Trick trick(Suit::Hearts);

    trick.play_card(0, Card(Suit::Hearts, Rank::Ace));
    trick.play_card(1, Card(Suit::Clubs, Rank::Seven));
    trick.play_card(2, Card(Suit::Spades, Rank::King));
    trick.play_card(3, Card(Suit::Diamonds, Rank::Jack));

    CHECK_THROWS_AS(
        trick.play_card(0, Card(Suit::Hearts, Rank::Two)),
        std::logic_error
    );
}

TEST_CASE("Cannot play with an invalid player ID", "[trick]")
{
    Trick trick(Suit::Hearts);

    CHECK_THROWS_AS(
        trick.play_card(4, Card(Suit::Hearts, Rank::Ace)),
        std::out_of_range
        );
}