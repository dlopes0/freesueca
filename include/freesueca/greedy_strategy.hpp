#pragma once

#include "freesueca/player_strategy.hpp"

#include <span>
#include <vector>

namespace freesueca
{
    class GreedyStrategy : public PlayerStrategy
    {
    public:
        [[nodiscard]] std::size_t choose_card(
            const PlayerDecisionContext& context) override;

        [[nodiscard]] std::size_t choose_card(
            PlayerId player_id,
            const std::vector<Card>& hand,
            std::span<const std::size_t> legal_cards,
            Suit trump_suit,
            const Trick& trick) const;
    };
}