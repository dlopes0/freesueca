#pragma once

#include "freesueca/player_strategy.hpp"

#include <cstdint>
#include <random>

namespace freesueca
{
    class RandomStrategy : public PlayerStrategy
    {
    public:
        RandomStrategy();
        explicit RandomStrategy(std::uint32_t seed);

        [[nodiscard]] std::size_t choose_card(const PlayerDecisionContext& context) override;

    private:
        std::mt19937 rng_;
    };

}