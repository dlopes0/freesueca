#pragma once

#include "freesueca/hidden_hand_sampler.hpp"

#include <array>
#include <cstddef>

namespace freesueca
{
    struct SimulationResult {
        std::array<int, 2> team_points{};
        std::array<int, 2> team_tricks{};
        std::array<int, 2> team_riscos{};
    };

    class HandSimulator
    {
    public:
        struct Workspace
        {
            SampledHands hands;
        };

        [[nodiscard]] SimulationResult simulate(
            const PlayerDecisionContext& context,
            const SampledHands& hands,
            std::size_t candidate_card_index) const;

        [[nodiscard]] SimulationResult simulate(
            const PlayerDecisionContext& context,
            const SampledHands& hands,
            std::size_t candidate_card_index,
            Workspace& workspace) const;
    };

}