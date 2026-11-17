#include "freesueca/card_tracker.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

using namespace freesueca;

TEST_CASE("New CardTracker contains no played cards")
{
    CardTracker tracker;

    REQUIRE(tracker.cards_played() == 0);

    CHECK_FALSE(
        tracker.has_been_played(Card(Suit::Hearts, Rank::Ace))
    );
}

TEST_CASE("CardTracker records individual cards")
{
    CardTracker tracker;

    const Card ace(Suit::Hearts, Rank::Ace);

    tracker.record(ace);

    CHECK(tracker.has_been_played(ace));
    CHECK(tracker.cards_played() == 1);
}

TEST_CASE("CardTracker does not count duplicate cards")
{
    CardTracker tracker;

    const Card ace(Suit::Hearts, Rank::Ace);

    tracker.record(ace);
    tracker.record(ace);

    CHECK(tracker.has_been_played(ace));
    CHECK(tracker.cards_played() == 1);
}

TEST_CASE("CardTracker records all cards in a trick")
{
    CardTracker tracker;

    Trick trick(Suit::Hearts);

    trick.play_card(0, Card(Suit::Clubs, Rank::Ace));
    trick.play_card(1, Card(Suit::Clubs, Rank::Seven));
    trick.play_card(2, Card(Suit::Clubs, Rank::King));
    trick.play_card(3, Card(Suit::Hearts, Rank::Two));

    tracker.record(trick);

    CHECK(tracker.cards_played() == 4);

    CHECK(tracker.has_been_played(
        Card(Suit::Clubs, Rank::Ace)
    ));

    CHECK(tracker.has_been_played(
        Card(Suit::Hearts, Rank::Two)
    ));

    CHECK_FALSE(tracker.has_been_played(
        Card(Suit::Spades, Rank::Ace)
    ));
}

TEST_CASE("CardTracker records multiple tricks")
{
    CardTracker tracker;

    Trick first(Suit::Hearts);
    first.play_card(0, Card(Suit::Clubs, Rank::Ace));
    first.play_card(1, Card(Suit::Clubs, Rank::Seven));
    first.play_card(2, Card(Suit::Clubs, Rank::King));
    first.play_card(3, Card(Suit::Hearts, Rank::Two));

    Trick second(Suit::Hearts);
    second.play_card(0, Card(Suit::Diamonds, Rank::Ace));
    second.play_card(1, Card(Suit::Diamonds, Rank::Seven));
    second.play_card(2, Card(Suit::Diamonds, Rank::King));
    second.play_card(3, Card(Suit::Spades, Rank::Two));

    const std::vector<Trick> history = {first, second};

    tracker.record(history);

    CHECK(tracker.cards_played() == 8);

    CHECK(tracker.has_been_played(
        Card(Suit::Diamonds, Rank::Ace)
    ));
}


TEST_CASE("CardTracker identifies all unplayed cards")
{
    CardTracker tracker;

    const auto remaining = tracker.unplayed_cards();

    REQUIRE(remaining.size() == 40);
}

TEST_CASE("CardTracker excludes played cards from unplayed cards")
{
    CardTracker tracker;

    const Card ace(Suit::Hearts, Rank::Ace);
    const Card seven(Suit::Clubs, Rank::Seven);

    tracker.record(ace);
    tracker.record(seven);

    const auto remaining = tracker.unplayed_cards();

    REQUIRE(remaining.size() == 38);

    CHECK_FALSE(
        std::find(remaining.begin(), remaining.end(), ace)
        != remaining.end()
    );

    CHECK_FALSE(
        std::find(remaining.begin(), remaining.end(), seven)
        != remaining.end()
    );
}

TEST_CASE("CardTracker identifies cards unknown to the player")
{
    CardTracker tracker;

    const std::vector<Card> own_hand = {
        Card(Suit::Hearts, Rank::Ace),
        Card(Suit::Clubs, Rank::Seven),
        Card(Suit::Diamonds, Rank::King)
    };

    tracker.record(Card(Suit::Spades, Rank::Ace));

    const auto unknown = tracker.unknown_cards(own_hand);

    REQUIRE(unknown.size() == 36);

    CHECK(
        std::find(
            unknown.begin(),
            unknown.end(),
            Card(Suit::Hearts, Rank::Ace)
        ) == unknown.end()
    );

    CHECK(
        std::find(
            unknown.begin(),
            unknown.end(),
            Card(Suit::Spades, Rank::Ace)
        ) == unknown.end()
    );
}

