#pragma once

#include "freesueca/player_strategy.hpp"

namespace freesueca
{
    class CountingStrategy : public PlayerStrategy
    {
    public:
        [[nodiscard]] std::size_t choose_card(const PlayerDecisionContext& context) override;
    };
}
