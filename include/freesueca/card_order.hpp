
#pragma once

#include "freesueca/card.hpp"

namespace freesueca::card_order
{
    [[nodiscard]]
    constexpr int rank_strength(Rank rank) noexcept
    {
        switch (rank)
        {
            case Rank::Ace:   return 10;
            case Rank::Seven: return 9;
            case Rank::King:  return 8;
            case Rank::Jack:  return 7;
            case Rank::Queen: return 6;
            case Rank::Six:   return 5;
            case Rank::Five:  return 4;
            case Rank::Four:  return 3;
            case Rank::Three: return 2;
            case Rank::Two:   return 1;
        }

        return 0;
    }

    [[nodiscard]]
    constexpr int strength(
        const Card& card,
        Suit lead_suit,
        Suit trump_suit
    ) noexcept
    {
        const int rank = rank_strength(card.rank());

        if (card.suit() == trump_suit)
            return 200 + rank;

        if (card.suit() == lead_suit)
            return 100 + rank;

        return rank;
    }
}
