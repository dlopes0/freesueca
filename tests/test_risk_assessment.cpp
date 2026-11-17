#include "freesueca/risk_assessment.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace freesueca;

TEST_CASE("Risk assessment considers trick value")
{
    const Card card(Suit::Clubs, Rank::Two);

    CHECK_FALSE(should_take_risk(40, 40, 10, card));
    CHECK(should_take_risk(40, 40, 25, card));
}

TEST_CASE("Risk assessment becomes more aggressive when behind")
{
    const Card card(Suit::Clubs, Rank::Two);

    CHECK_FALSE(should_take_risk(40, 40, 16, card));
    CHECK(should_take_risk(30, 60, 16, card));
}

TEST_CASE("Risk assessment becomes more conservative when ahead")
{
    const Card card(Suit::Clubs, Rank::Two);

    CHECK(should_take_risk(40, 40, 22, card));
    CHECK_FALSE(should_take_risk(70, 40, 22, card));
}

TEST_CASE("Risk assessment becomes more aggressive near 91 points")
{
    const Card card(Suit::Clubs, Rank::Two);

    CHECK_FALSE(should_take_risk(40, 40, 16, card));
    CHECK(should_take_risk(80, 40, 16, card));
}

TEST_CASE("Risk assessment accounts for candidate card strength")
{
    const Card low_card(Suit::Clubs, Rank::Two);
    const Card high_card(Suit::Clubs, Rank::Ace);

    CHECK(should_take_risk(40, 40, 25, low_card));
    CHECK_FALSE(should_take_risk(40, 40, 25, high_card));
}