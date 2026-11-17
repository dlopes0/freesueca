#pragma once

#include "freesueca/player_decision_context.hpp"

#include <cstddef>

namespace freesueca
{
    class PlayerStrategy
    {
    public:
        virtual ~PlayerStrategy() = default;

        [[nodiscard]] virtual std::size_t choose_card(const PlayerDecisionContext& context) = 0;
    };
}
