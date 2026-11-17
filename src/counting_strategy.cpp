#include "freesueca/counting_strategy.hpp"

#include "freesueca/card_order.hpp"
#include "freesueca/card_tracker.hpp"
#include "freesueca/game.hpp"
#include "freesueca/void_tracker.hpp"
#include "freesueca/risk_assessment.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

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

        const PlayedCard& find_current_winner(
            const Trick& trick,
            Suit lead,
            Suit trump)
        {
            const auto& played = trick.played_cards();

            return *std::max_element(
                played.begin(),
                played.end(),
                [lead, trump](const PlayedCard& a,
                              const PlayedCard& b)
                {
                    return card_order::strength(
                        a.card, lead, trump
                    ) < card_order::strength(
                        b.card, lead, trump
                    );
                }
            );
        }

        bool can_be_overtaken(
            const Card& candidate,
            Suit lead_suit,
            Suit trump_suit,
            const std::vector<Card>& unknown_cards,
            const std::vector<PlayerId>& players_yet_to_play,
            const VoidTracker& void_tracker)
        {
            for (const Card& card : unknown_cards)
            {
                if (card_order::strength(card, lead_suit, trump_suit)
                    <= card_order::strength(candidate, lead_suit, trump_suit))
                {
                    continue;
                }

                for (const PlayerId player_id : players_yet_to_play)
                {
                    if (!void_tracker.is_void(player_id, card.suit()))
                        return true;
                }
            }

            return false;
        }

        std::vector<PlayerId> players_yet_to_play(
            const PlayerDecisionContext& context)
        {
            std::vector<PlayerId> players;

            const std::size_t cards_played =
                context.current_trick.card_count();

            for (std::size_t offset = 1;
                 offset < Game::PLAYER_COUNT - cards_played;
                 ++offset)
            {
                players.push_back(
                    (context.player_id + offset) % Game::PLAYER_COUNT
                );
            }

            return players;
        }
    }

    std::size_t CountingStrategy::choose_card(
        const PlayerDecisionContext& context)
    {
        const auto& hand = context.hand;
        const auto& legal = context.legal_card_indices;
        const Trick& trick = context.current_trick;

        if (legal.empty())
            throw std::logic_error("No legal cards available");

        // Reconstruct all cards that have already been played.
        CardTracker tracker;

        tracker.record(context.trick_history);
        tracker.record(context.current_trick);

        VoidTracker void_tracker;
        void_tracker.record(context.trick_history);
        void_tracker.record(context.current_trick);

        // Cards held by the other three players are unknown.
        const auto unknown_cards =
            tracker.unknown_cards(hand);

        // Leading a trick: initially use the GreedyStrategy rule.
        if (trick.card_count() == 0)
        {
            return *std::min_element(
                legal.begin(),
                legal.end(),
                [&](std::size_t a, std::size_t b)
                {
                    return weaker_card(hand[a], hand[b]);
                }
            );
        }

        const Suit lead = trick.lead_suit();
        const Suit trump = context.trump_suit;

        const PlayedCard& winner =
            find_current_winner(trick, lead, trump);

        const bool partner_is_winning =
            winner.player_id % 2 == context.player_id % 2;

        std::vector<std::size_t> winning_cards;
        std::vector<std::size_t> non_winning_cards;
        std::vector<std::size_t> safe_winning_cards;

        const std::vector<PlayerId> remaining_players =
            players_yet_to_play(context);

        for (const std::size_t index : legal)
        {
            const Card& candidate = hand[index];

            const bool beats_winner =
                card_order::strength(candidate, lead, trump) >
                card_order::strength(winner.card, lead, trump);

            if (!beats_winner)
            {
                non_winning_cards.push_back(index);
                continue;
            }

            winning_cards.push_back(index);

            // If the current player is last to play, no opponent
            // can overtake the card during this trick.
            const bool last_to_play = remaining_players.empty();

            if (last_to_play ||
                !can_be_overtaken(
                    candidate,
                    lead,
                    trump,
                    unknown_cards,
                    remaining_players,
                    void_tracker))
            {
                safe_winning_cards.push_back(index);
            }
        }

        // Our partner is winning: avoid overtaking them.
        if (partner_is_winning)
        {
            if (!non_winning_cards.empty())
            {
                return *std::min_element(
                    non_winning_cards.begin(),
                    non_winning_cards.end(),
                    [&](std::size_t a, std::size_t b)
                    {
                        return weaker_card(hand[a], hand[b]);
                    }
                );
            }

            // Every legal card overtakes our partner.
            return *std::min_element(
                legal.begin(),
                legal.end(),
                [&](std::size_t a, std::size_t b)
                {
                    return weaker_card(hand[a], hand[b]);
                }
            );
        }

        // Prefer a winning card that cannot be overtaken.
        if (!safe_winning_cards.empty())
        {
            return *std::min_element(
                safe_winning_cards.begin(),
                safe_winning_cards.end(),
                [&](std::size_t a, std::size_t b)
                {
                    return card_order::strength(
                        hand[a], lead, trump
                    ) < card_order::strength(
                        hand[b], lead, trump
                    );
                }
            );
        }

        // No guaranteed winner. Consider taking a risk if the
        // current trick is valuable enough.
        if (!winning_cards.empty())
        {
            const auto best_risk_card = *std::min_element(
                winning_cards.begin(),
                winning_cards.end(),
                [&](std::size_t a, std::size_t b)
                {
                    return card_order::strength(
                        hand[a], lead, trump
                    ) < card_order::strength(
                        hand[b], lead, trump
                    );
                }
            );

            if (should_take_risk(
                context.our_team_points,
                context.opposing_team_points,
                trick.points(),
                hand[best_risk_card]))
            {
                return best_risk_card;
            }
        }

        // Otherwise, conserve strong cards.
        return *std::min_element(
            legal.begin(),
            legal.end(),
            [&](std::size_t a, std::size_t b)
            {
                return weaker_card(hand[a], hand[b]);
            }
        );
    }
}
