
#include "freesueca/card_order.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace freesueca;

TEST_CASE("Card rank strength follows Sueca order")
{
    CHECK(card_order::rank_strength(Rank::Ace) == 10);
    CHECK(card_order::rank_strength(Rank::Seven) == 9);
    CHECK(card_order::rank_strength(Rank::King) == 8);
    CHECK(card_order::rank_strength(Rank::Jack) == 7);
    CHECK(card_order::rank_strength(Rank::Queen) == 6);
    CHECK(card_order::rank_strength(Rank::Six) == 5);
    CHECK(card_order::rank_strength(Rank::Five) == 4);
    CHECK(card_order::rank_strength(Rank::Four) == 3);
    CHECK(card_order::rank_strength(Rank::Three) == 2);
    CHECK(card_order::rank_strength(Rank::Two) == 1);
}

TEST_CASE("Trump cards are stronger than lead-suit cards")
{
    const Card trump_card(Suit::Hearts, Rank::Two);
    const Card lead_card(Suit::Clubs, Rank::Ace);

    CHECK(
        card_order::strength(trump_card, Suit::Clubs, Suit::Hearts) >
        card_order::strength(lead_card, Suit::Clubs, Suit::Hearts)
    );
}

TEST_CASE("Lead-suit cards are stronger than off-suit cards")
{
    const Card lead_card(Suit::Clubs, Rank::Two);
    const Card off_suit_card(Suit::Diamonds, Rank::Ace);

    CHECK(
        card_order::strength(lead_card, Suit::Clubs, Suit::Hearts) >
        card_order::strength(off_suit_card, Suit::Clubs, Suit::Hearts)
    );
}
