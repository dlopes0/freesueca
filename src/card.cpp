#include "freesueca/card.hpp"

namespace freesueca
{
    Card::Card(Suit suit, Rank rank)
        : suit_(suit), rank_(rank)
    {

    }

    Suit Card::suit() const noexcept
    {
        return suit_;
    }

    Rank Card::rank() const noexcept
    {
        return rank_;
    }

    int Card::points() const noexcept
    {
        switch (rank_)
        {
            case Rank::Ace: return 11;
            case Rank::Seven: return 10;
            case Rank::King: return 4;
            case Rank::Jack: return 3;
            case Rank::Queen: return 2;
            default: return 0;
        }
    }

    std::string Card::name() const
    {
        std::string rank_name;
        std::string suit_name;

        switch (rank_)
        {
            case Rank::Two: rank_name = "Two"; break;
            case Rank::Three: rank_name = "Three"; break;
            case Rank::Four: rank_name = "Four"; break;
            case Rank::Five: rank_name = "Five"; break;
            case Rank::Six: rank_name = "Six"; break;
            case Rank::Queen: rank_name = "Queen"; break;
            case Rank::Jack: rank_name = "Jack"; break;
            case Rank::King: rank_name = "King"; break;
            case Rank::Seven: rank_name = "Seven"; break;
            case Rank::Ace: rank_name = "Ace"; break;
        }

        switch (suit_)
        {
            case Suit::Hearts: suit_name = "Hearts"; break;
            case Suit::Diamonds: suit_name = "Diamonds"; break;
            case Suit::Clubs: suit_name = "Clubs"; break;
            case Suit::Spades: suit_name = "Spades"; break;
        }

        return rank_name + " of " + suit_name;
    }
}