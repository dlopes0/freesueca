#include "freesueca/hand_simulator.hpp"

#include "freesueca/greedy_strategy.hpp"
#include "freesueca/scoring.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <array>
#include <span>

namespace freesueca
{
    namespace
    {
        constexpr std::size_t PLAYER_COUNT = 4;

        constexpr std::size_t MAX_HAND_SIZE = 10;

        struct LegalCards
        {
            std::array<std::size_t, MAX_HAND_SIZE> indices{};
            std::size_t count = 0;

            [[nodiscard]] std::span<const std::size_t> view() const
            {
                return {indices.data(), count};
            }
        };

        LegalCards legal_indices(
            const std::vector<Card>& hand,
            const Trick& trick)
        {
            if (hand.size() > MAX_HAND_SIZE)
                throw std::length_error("Hand exceeds maximum Sueca hand size");

            LegalCards legal;

            if (trick.card_count() == 0)
            {
                for (std::size_t i = 0; i < hand.size(); ++i)
                    legal.indices[legal.count++] = i;

                return legal;
            }

            const Suit lead = trick.lead_suit();

            const bool has_lead_suit =
                std::any_of(
                    hand.begin(),
                    hand.end(),
                    [lead](const Card& card)
                    {
                        return card.suit() == lead;
                    });

            for (std::size_t i = 0; i < hand.size(); ++i)
            {
                if (!has_lead_suit || hand[i].suit() == lead)
                    legal.indices[legal.count++] = i;
            }

            return legal;
        }

        bool hands_are_empty(const SampledHands& hands)
        {
            return std::all_of(
                hands.begin(),
                hands.end(),
                [](const auto& hand)
                {
                    return hand.empty();
                });
        }
    } // namespace

    SimulationResult HandSimulator::simulate(
    const PlayerDecisionContext& context,
    const SampledHands& sampled_hands,
    std::size_t candidate_card_index) const
    {
        Workspace workspace;

        return simulate(
            context,
            sampled_hands,
            candidate_card_index,
            workspace);
    }

    SimulationResult HandSimulator::simulate(
        const PlayerDecisionContext& context,
        const SampledHands& sampled_hands,
        std::size_t candidate_card_index,
        Workspace& workspace) const
    {
        if (context.player_id >= PLAYER_COUNT)
            throw std::out_of_range("Invalid player ID");

        // Restore the original deal. Vector assignment reuses capacity
        // when the workspace already has sufficient storage.
        workspace.hands = sampled_hands;

        SampledHands& hands = workspace.hands;

        if (hands[context.player_id] != context.hand)
            throw std::invalid_argument(
                "Sampled hand does not match the current player's hand");

        const auto& legal = context.legal_card_indices;

        if (std::find(
                legal.begin(),
                legal.end(),
                candidate_card_index) == legal.end())
        {
            throw std::invalid_argument("Candidate card is not legal");
        }

        SimulationResult result;

        const std::size_t our_team = context.player_id % 2;

        result.team_points[our_team] =
            context.our_team_points;

        result.team_points[1 - our_team] =
            context.opposing_team_points;

        // Recover the number of tricks already won.
        for (const Trick& completed_trick : context.trick_history)
        {
            const std::size_t team =
                completed_trick.winning_player() % 2;

            ++result.team_tricks[team];
        }

        Trick trick = context.current_trick;

        GreedyStrategy strategy;

        PlayerId current_player = context.player_id;

        // Play the candidate card first.
        const Card candidate =
            hands[current_player][candidate_card_index];

        hands[current_player].erase(
            hands[current_player].begin() + candidate_card_index);

        trick.play_card(current_player, candidate);

        // The next player acts after the candidate unless the trick
        // has already been completed.
        if (!trick.is_complete())
            current_player = (current_player + 1) % PLAYER_COUNT;

        // Complete the current hand.
        while (!hands_are_empty(hands))
        {
            if (trick.is_complete())
            {
                const PlayerId winner = trick.winning_player();
                const std::size_t winning_team = winner % 2;

                result.team_points[winning_team] += trick.points();
                ++result.team_tricks[winning_team];

                trick = Trick(context.trump_suit);
                current_player = winner;

                continue;
            }

            const LegalCards legal_cards =
    legal_indices(hands[current_player], trick);

            const auto legal_view = legal_cards.view();

            if (legal_view.empty())
                throw std::logic_error("Player has no legal cards");

            const std::size_t selected_index =
                strategy.choose_card(
                    current_player,
                    hands[current_player],
                    legal_view,
                    context.trump_suit,
                    trick);

            if (std::find(
                    legal_view.begin(),
                    legal_view.end(),
                    selected_index) == legal_view.end())
            {
                throw std::logic_error(
                    "Strategy selected an illegal card");
            }

            const Card selected_card =
                hands[current_player][selected_index];

            hands[current_player].erase(
                hands[current_player].begin() + selected_index);

            trick.play_card(current_player, selected_card);

            if (!trick.is_complete())
                current_player = (current_player + 1) % PLAYER_COUNT;
        }

        // The last play must have completed the final trick.
        if (!trick.is_complete())
            throw std::logic_error("Simulation ended with an incomplete trick");

        const PlayerId final_winner = trick.winning_player();
        const std::size_t final_team = final_winner % 2;

        result.team_points[final_team] += trick.points();
        ++result.team_tricks[final_team];

        // Calculate the hand's riscos.
        for (std::size_t team = 0; team < 2; ++team)
        {
            const std::size_t opposing_team = 1 - team;

            result.team_riscos[team] = calculate_riscos(
                result.team_points[team],
                result.team_points[opposing_team],
                result.team_tricks[team]);
        }

        return result;
    }
} // namespace freesueca
