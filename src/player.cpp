#include "freesueca/player.hpp"

#include <algorithm>
#include <stdexcept>

namespace freesueca
{
    Player::Player(PlayerId id) : id_(id)
    {
        hand_.reserve(10);
    }

    PlayerId Player::id() const noexcept
    {
        return id_;
    }

    void Player::add_card(Card card)
    {
        hand_.push_back(card);
    }

    Card Player::play_card(std::size_t index)
    {
        if (index >= hand_.size())
        {
            throw std::out_of_range("Invalid card index");
        }

        const Card card = hand_[index];

        hand_.erase(hand_.begin() + static_cast<std::ptrdiff_t>(index));

        return card;
    }

    void Player::sort_hand()
    {
        std::sort(
            hand_.begin(),
            hand_.end(),
            [](const Card& lhs, const Card& rhs)
            {
                if (lhs.suit() != rhs.suit())
                {
                    return lhs.suit() < rhs.suit();
                }

                return lhs.rank() < rhs.rank();
            }
        );
    }

    bool Player::has_suit(Suit suit) const noexcept
    {
        return std::any_of(
            hand_.begin(),
            hand_.end(),
            [suit](const Card& card)
            {
                return card.suit() == suit;
            }
        );
    }

    bool Player::has_card(const Card& card) const noexcept
    {
        return std::find(hand_.begin(), hand_.end(), card)
            != hand_.end();
    }

    std::size_t Player::card_count() const noexcept
    {
        return hand_.size();
    }

    const std::vector<Card>& Player::hand() const noexcept
    {
        return hand_;
    }

    void Player::clear_hand() noexcept
    {
        hand_.clear();
    }
}
