#include "freesueca/card_tracker.hpp"

#include <algorithm>

namespace freesueca
{
    namespace
    {
        std::size_t suit_index(Suit suit) noexcept
        {
            switch (suit)
            {
                case Suit::Hearts:   return 0;
                case Suit::Diamonds: return 1;
                case Suit::Clubs:    return 2;
                case Suit::Spades:   return 3;
            }

            return 0;
        }

        std::size_t rank_index(Rank rank) noexcept
        {
            switch (rank)
            {
                case Rank::Two:   return 0;
                case Rank::Three: return 1;
                case Rank::Four:  return 2;
                case Rank::Five:  return 3;
                case Rank::Six:   return 4;
                case Rank::Queen: return 5;
                case Rank::Jack:  return 6;
                case Rank::King:  return 7;
                case Rank::Seven: return 8;
                case Rank::Ace:   return 9;
            }

            return 0;
        }
    }

    std::size_t CardTracker::card_index(const Card& card) noexcept
    {
        return suit_index(card.suit()) * 10 +
               rank_index(card.rank());
    }

    void CardTracker::record(const Card& card) noexcept
    {
        const std::size_t index = card_index(card);

        // Avoid counting the same card twice.
        if (!played_[index])
        {
            played_[index] = true;
            ++cards_played_;
        }
    }

    void CardTracker::record(const Trick& trick) noexcept
    {
        for (const PlayedCard& played : trick.played_cards())
        {
            record(played.card);
        }
    }

    void CardTracker::record(const std::vector<Trick>& tricks) noexcept
    {
        for (const Trick& trick : tricks)
        {
            record(trick);
        }
    }

    bool CardTracker::has_been_played(const Card& card) const noexcept
    {
        return played_[card_index(card)];
    }

    std::size_t CardTracker::cards_played() const noexcept
    {
        return cards_played_;
    }


    std::vector<Card> CardTracker::unplayed_cards() const
    {
        constexpr std::array<Suit, 4> suits = {
            Suit::Hearts,
            Suit::Diamonds,
            Suit::Clubs,
            Suit::Spades
        };

        constexpr std::array<Rank, 10> ranks = {
            Rank::Two,
            Rank::Three,
            Rank::Four,
            Rank::Five,
            Rank::Six,
            Rank::Queen,
            Rank::Jack,
            Rank::King,
            Rank::Seven,
            Rank::Ace
        };

        std::vector<Card> result;
        result.reserve(40 - cards_played_);

        for (const Suit suit : suits)
        {
            for (const Rank rank : ranks)
            {
                const Card card(suit, rank);

                if (!has_been_played(card))
                    result.push_back(card);
            }
        }

        return result;
    }

    std::vector<Card> CardTracker::unknown_cards(
        const std::vector<Card>& own_hand) const
    {
        std::vector<Card> result;
        result.reserve(40 - cards_played_ - own_hand.size());

        for (const Card& card : unplayed_cards())
        {
            const bool in_own_hand = std::find(
                own_hand.begin(),
                own_hand.end(),
                card
            ) != own_hand.end();

            if (!in_own_hand)
                result.push_back(card);
        }

        return result;
    }
}
