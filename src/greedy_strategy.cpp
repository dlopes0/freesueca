#include "freesueca/greedy_strategy.hpp"
#include "freesueca/card_order.hpp"

#include <algorithm>
#include <stdexcept>

namespace freesueca
{
    namespace
    {
        bool weaker_card(const Card& a, const Card& b)
        {
            if (a.points() != b.points())
                return a.points() < b.points();

            return card_order::rank_strength(a.rank()) <
                   card_order::rank_strength(b.rank());
        }
    }

    std::size_t GreedyStrategy::choose_card(
        const PlayerDecisionContext& context)
    {
        return choose_card(
            context.player_id,
            context.hand,
            context.legal_card_indices,
            context.trump_suit,
            context.current_trick);
    }

    std::size_t GreedyStrategy::choose_card(
        PlayerId player_id,
        const std::vector<Card>& hand,
        std::span<const std::size_t> legal,
        Suit trump,
        const Trick& trick) const
    {
        if (legal.empty())
            throw std::logic_error("No legal cards available");

        // Leading a trick: play a low-value card.
        if (trick.card_count() == 0)
        {
            return *std::min_element(
                legal.begin(),
                legal.end(),
                [&](std::size_t a, std::size_t b)
                {
                    return weaker_card(hand[a], hand[b]);
                });
        }

        const Suit lead = trick.lead_suit();

        // Find the player currently winning the trick.
        const auto& played_cards = trick.played_cards();

        const PlayedCard* current_winner = &played_cards.front();

        for (const auto& played : played_cards)
        {
            if (card_order::strength(played.card, lead, trump) >
                card_order::strength(current_winner->card, lead, trump))
            {
                current_winner = &played;
            }
        }

        const bool partner_is_winning =
            current_winner->player_id % 2 == player_id % 2;

        // Find the weakest winning and non-winning cards.
        bool has_winning_card = false;
        bool has_non_winning_card = false;

        std::size_t weakest_winning_card = 0;
        std::size_t weakest_non_winning_card = 0;

        for (const std::size_t index : legal)
        {
            const Card& candidate = hand[index];

            if (card_order::strength(candidate, lead, trump) >
                card_order::strength(current_winner->card, lead, trump))
            {
                if (!has_winning_card ||
                    card_order::strength(candidate, lead, trump) <
                    card_order::strength(
                        hand[weakest_winning_card], lead, trump))
                {
                    weakest_winning_card = index;
                    has_winning_card = true;
                }
            }
            else
            {
                if (!has_non_winning_card ||
                    weaker_card(candidate, hand[weakest_non_winning_card]))
                {
                    weakest_non_winning_card = index;
                    has_non_winning_card = true;
                }
            }
        }

        // Partner is winning: avoid overtaking them.
        if (partner_is_winning && has_non_winning_card)
            return weakest_non_winning_card;

        // Opponent is winning: capture a worthwhile trick
        // with the weakest card that can win.
        if (!partner_is_winning &&
            trick.points() >= 10 &&
            has_winning_card)
        {
            return weakest_winning_card;
        }

        // Otherwise, conserve stronger cards.
        return *std::min_element(
            legal.begin(),
            legal.end(),
            [&](std::size_t a, std::size_t b)
            {
                return weaker_card(hand[a], hand[b]);
            });
    }
}