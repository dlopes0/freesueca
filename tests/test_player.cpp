#include <catch2/catch_test_macros.hpp>

#include "freesueca/player.hpp"

#include <stdexcept>

using namespace freesueca;

TEST_CASE("Player has an identifier", "[player]")
{
    const Player player(1);

    REQUIRE(player.id() == 1);
}

TEST_CASE("Player starts with an empty hand", "[player]")
{
    const Player player(1);

    REQUIRE(player.card_count() == 0);
    REQUIRE(player.hand().empty());
}

TEST_CASE("Player can receive cards", "[player]")
{
    Player player(1);

    const Card card(Suit::Hearts, Rank::Ace);

    player.add_card(card);

    REQUIRE(player.card_count() == 1);
    CHECK(player.has_card(card));
}

TEST_CASE("Player can play a card", "[player]")
{
    Player player(1);

    const Card first(Suit::Hearts, Rank::Ace);
    const Card second(Suit::Spades, Rank::Seven);

    player.add_card(first);
    player.add_card(second);

    const Card played = player.play_card(0);

    CHECK(played == first);
    CHECK_FALSE(player.has_card(first));
    CHECK(player.has_card(second));

    REQUIRE(player.card_count() == 1);
}

TEST_CASE("Player cannot play an invalid card index", "[player]")
{
    Player player(1);

    player.add_card(Card(Suit::Hearts, Rank::Ace));

    REQUIRE_THROWS_AS(
        player.play_card(1),
        std::out_of_range
    );

    CHECK(player.card_count() == 1);
}

TEST_CASE("Player can check whether they have a suit", "[player]")
{
    Player player(1);

    player.add_card(Card(Suit::Hearts, Rank::Ace));
    player.add_card(Card(Suit::Spades, Rank::Seven));
    player.add_card(Card(Suit::Clubs, Rank::King));

    CHECK(player.has_suit(Suit::Hearts));
    CHECK(player.has_suit(Suit::Spades));
    CHECK(player.has_suit(Suit::Clubs));

    CHECK_FALSE(player.has_suit(Suit::Diamonds));
}

TEST_CASE("Player can inspect their hand", "[player]")
{
    Player player(1);

    const Card first(Suit::Hearts, Rank::Ace);
    const Card second(Suit::Clubs, Rank::King);

    player.add_card(first);
    player.add_card(second);

    const auto& hand = player.hand();

    REQUIRE(hand.size() == 2);

    CHECK(hand[0] == first);
    CHECK(hand[1] == second);
}

TEST_CASE("Player sorts hand by suit and rank", "[player]")
{
    Player player(0);

    player.add_card(Card(Suit::Spades, Rank::Ace));
    player.add_card(Card(Suit::Hearts, Rank::King));
    player.add_card(Card(Suit::Clubs, Rank::Seven));
    player.add_card(Card(Suit::Hearts, Rank::Three));
    player.add_card(Card(Suit::Diamonds, Rank::Two));
    player.add_card(Card(Suit::Hearts, Rank::Ace));

    player.sort_hand();

    const auto& hand = player.hand();

    REQUIRE(hand.size() == 6);

    CHECK(hand[0] == Card(Suit::Hearts, Rank::Three));
    CHECK(hand[1] == Card(Suit::Hearts, Rank::King));
    CHECK(hand[2] == Card(Suit::Hearts, Rank::Ace));
    CHECK(hand[3] == Card(Suit::Diamonds, Rank::Two));
    CHECK(hand[4] == Card(Suit::Clubs, Rank::Seven));
    CHECK(hand[5] == Card(Suit::Spades, Rank::Ace));
}