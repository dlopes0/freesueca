#include <catch2/catch_test_macros.hpp>

#include "freesueca/void_tracker.hpp"

using namespace freesueca;

TEST_CASE("New VoidTracker has no known void suits", "[void_tracker]")
{
    VoidTracker tracker;

    for (PlayerId player = 0; player < 4; ++player)
    {
        CHECK_FALSE(tracker.is_void(player, Suit::Hearts));
        CHECK_FALSE(tracker.is_void(player, Suit::Diamonds));
        CHECK_FALSE(tracker.is_void(player, Suit::Clubs));
        CHECK_FALSE(tracker.is_void(player, Suit::Spades));
    }
}

TEST_CASE("Playing off-suit reveals a void suit", "[void_tracker]")
{
    VoidTracker tracker;

    Trick trick(Suit::Spades);

    trick.play_card(0, Card(Suit::Hearts, Rank::Two));
    trick.play_card(1, Card(Suit::Clubs, Rank::Three));

    tracker.record(trick);

    CHECK(tracker.is_void(1, Suit::Hearts));

    CHECK_FALSE(tracker.is_void(0, Suit::Hearts));
    CHECK_FALSE(tracker.is_void(1, Suit::Clubs));
}

TEST_CASE("Following suit does not reveal a void", "[void_tracker]")
{
    VoidTracker tracker;

    Trick trick(Suit::Spades);

    trick.play_card(0, Card(Suit::Hearts, Rank::Two));
    trick.play_card(1, Card(Suit::Hearts, Rank::Three));

    tracker.record(trick);

    CHECK_FALSE(tracker.is_void(0, Suit::Hearts));
    CHECK_FALSE(tracker.is_void(1, Suit::Hearts));
}

TEST_CASE("Void information persists across tricks", "[void_tracker]")
{
    VoidTracker tracker;

    Trick first(Suit::Spades);
    first.play_card(0, Card(Suit::Hearts, Rank::Two));
    first.play_card(1, Card(Suit::Clubs, Rank::Three));

    tracker.record(first);

    Trick second(Suit::Spades);
    second.play_card(2, Card(Suit::Diamonds, Rank::Four));
    second.play_card(1, Card(Suit::Spades, Rank::Five));

    tracker.record(second);

    CHECK(tracker.is_void(1, Suit::Hearts));
}

TEST_CASE("Recording multiple tricks preserves void information",
          "[void_tracker]")
{
    VoidTracker tracker;

    std::vector<Trick> tricks;

    Trick first(Suit::Spades);
    first.play_card(0, Card(Suit::Hearts, Rank::Two));
    first.play_card(1, Card(Suit::Clubs, Rank::Three));

    Trick second(Suit::Spades);
    second.play_card(2, Card(Suit::Diamonds, Rank::Four));
    second.play_card(3, Card(Suit::Hearts, Rank::Five));

    tricks.push_back(first);
    tricks.push_back(second);

    tracker.record(tricks);

    CHECK(tracker.is_void(1, Suit::Hearts));
    CHECK(tracker.is_void(3, Suit::Diamonds));
}