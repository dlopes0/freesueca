#include "freesueca/void_tracker.hpp"

#include <stdexcept>

namespace freesueca
{
    std::size_t VoidTracker::suit_index(Suit suit)
    {
        switch (suit)
        {
            case Suit::Hearts:   return 0;
            case Suit::Diamonds: return 1;
            case Suit::Clubs:    return 2;
            case Suit::Spades:   return 3;
        }

        throw std::invalid_argument("Invalid suit");
    }

    void VoidTracker::record(const Trick& trick)
    {
        if (trick.card_count() == 0)
            return;

        const Suit lead = trick.lead_suit();

        for (const PlayedCard& played : trick.played_cards())
        {
            if (played.card.suit() != lead)
            {
                void_suits_[played.player_id][suit_index(lead)] = true;
            }
        }
    }

    void VoidTracker::record(const std::vector<Trick>& tricks)
    {
        for (const Trick& trick : tricks)
        {
            record(trick);
        }
    }

    bool VoidTracker::is_void(
        PlayerId player_id,
        Suit suit) const
    {
        if (player_id >= PLAYER_COUNT)
            throw std::out_of_range("Invalid player ID");

        return void_suits_[player_id][suit_index(suit)];
    }
}