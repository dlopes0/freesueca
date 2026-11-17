#include "freesueca/trick.hpp"
#include "freesueca/card_order.hpp"

#include <algorithm>
#include <stdexcept>

namespace freesueca
{
    Trick::Trick(Suit trump_suit) : trump_suit_(trump_suit)
    {
        played_cards_.reserve(PLAYER_COUNT);
    }

    void Trick::play_card(PlayerId player_id, Card card)
    {
        if (player_id >= PLAYER_COUNT)
        {
            throw std::out_of_range("Invalid player ID");
        }

        if (is_complete())
        {
            throw std::logic_error("Trick is already complete");
        }

        const auto already_played = std::find_if(
          played_cards_.begin(),
          played_cards_.end(),
          [player_id](const PlayedCard& played)
          {
              return played.player_id == player_id;
          }
        );

        if (already_played != played_cards_.end())
        {
            throw std::logic_error("Player has already played in this trick");
        }

        played_cards_.push_back({player_id, card});
    }

    bool Trick::is_complete() const noexcept
    {
        return played_cards_.size() == PLAYER_COUNT;
    }

    std::size_t Trick::card_count() const noexcept
    {
        return played_cards_.size();
    }


    PlayerId Trick::winning_player() const
    {
        if (!is_complete())
        {
            throw std::logic_error(
                "Cannot determine winner before trick is complete"
            );
        }

        const Suit lead = lead_suit();

        const auto winner = std::max_element(
            played_cards_.begin(),
            played_cards_.end(),
            [this, lead](const PlayedCard& a, const PlayedCard& b)
            {
                return card_order::strength(
                    a.card, lead, trump_suit_
                ) < card_order::strength(
                    b.card, lead, trump_suit_
                );
            }
        );

        return winner->player_id;
    }


    int Trick::points() const noexcept
    {
        int total = 0;

        for (const auto& played: played_cards_)
        {
            total += played.card.points();
        }

        return total;
    }

    Suit Trick::lead_suit() const
    {
        if (played_cards_.empty())
        {
            throw std::logic_error("No cards have been played");
        }

        return played_cards_.front().card.suit();
    }

    Suit Trick::trump_suit() const noexcept
    {
        return trump_suit_;
    }

    const std::vector<PlayedCard>& Trick::played_cards() const noexcept
    {
        return played_cards_;
    }
}
