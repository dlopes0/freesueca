#pragma once

#include "freesueca/hand_simulator.hpp"
#include "freesueca/hidden_hand_sampler.hpp"
#include "freesueca/player_strategy.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace freesueca
{
    class MonteCarloStrategy : public PlayerStrategy
    {
    public:
        explicit MonteCarloStrategy(std::size_t sample_count = 250);
        MonteCarloStrategy(std::size_t sample_count, std::uint32_t seed);

        // Enables sample-level parallelism
        MonteCarloStrategy(
            std::size_t sample_count,
            std::uint32_t seed,
            std::size_t parallel_threads);

        [[nodiscard]] std::size_t choose_card(
            const PlayerDecisionContext& context) override;

    private:
        struct SampleEvaluation
        {
            std::vector<std::uint64_t> riscos;
            std::vector<std::uint64_t> points;
        };

        [[nodiscard]] static std::uint32_t sample_seed(
            std::uint32_t base_seed,
            std::uint64_t decision_index,
            std::size_t sample_index) noexcept;

        std::size_t sample_count_;
        HiddenHandSampler sampler_;
        HandSimulator simulator_;

        std::uint32_t base_seed_ = 0;
        std::uint64_t decision_index_ = 0;
        std::size_t parallel_threads_ = 1;
        bool parallel_samples_ = false;
    };
}