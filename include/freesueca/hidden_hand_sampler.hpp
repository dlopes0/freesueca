#pragma once

#include "freesueca/player_decision_context.hpp"

#include <array>
#include <cstdint>
#include <random>
#include <vector>

namespace freesueca
{
    using SampledHands = std::array<std::vector<Card>, 4>;

    class HiddenHandSampler
    {
    public:
        HiddenHandSampler();
        explicit HiddenHandSampler(std::uint32_t seed);

        [[nodiscard]] SampledHands sample(
            const PlayerDecisionContext& context);

    private:
        std::mt19937 rng_;
    };

}