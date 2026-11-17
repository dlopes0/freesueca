#include "freesueca/deck.hpp"

#include <algorithm>
#include <utility>
#include <stdexcept>

namespace freesueca
{
    namespace
    {
        constexpr std::array suits {
            Suit::Hearts,
            Suit::Diamonds,
            Suit::Clubs,
            Suit::Spades
        };

        constexpr std::array ranks {
            Rank::Ace,
            Rank::Seven,
            Rank::King,
            Rank::Jack,
            Rank::Queen,
            Rank::Six,
            Rank::Five,
            Rank::Four,
            Rank::Three,
            Rank::Two
        };

        template <std::size_t... I>
        std::array<Card, sizeof...(I)> make_cards(std::index_sequence<I...>)
        {
            return {
                Card(suits[I / ranks.size()], ranks[I % ranks.size()])...
            };
        }
    }

    Deck::Deck() : Deck(std::random_device{}())
    {

    }

    Deck::Deck(std::uint32_t seed) : cards_(make_cards(std::make_index_sequence<SIZE>{})), rng_(seed)
    {

    }

    void Deck::reset()
    {
        cards_ = make_cards(std::make_index_sequence<SIZE>{});
        next_card_ = 0;
    }

    void Deck::shuffle()
    {
        std::shuffle(cards_.begin(), cards_.end(), rng_);
        next_card_ = 0;
    }

    Card Deck::deal()
    {
        if (next_card_ >= SIZE)
        {
            throw std::out_of_range("Deck is empty");
        }

        return cards_[next_card_++];
    }

    std::size_t Deck::remaining() const noexcept
    {
        return SIZE - next_card_;
    }
}