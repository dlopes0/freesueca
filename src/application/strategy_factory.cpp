#include "freesueca/application/strategy_factory.hpp"

#include "freesueca/greedy_strategy.hpp"
#include "freesueca/monte_carlo_strategy.hpp"
#include "freesueca/random_strategy.hpp"

#include <memory>
#include <stdexcept>
#include <cstddef>

namespace freesueca::application
{
    namespace
    {
        constexpr std::size_t HARD_DIFFICULTY_SAMPLE_COUNT = 250;
    }

    std::unique_ptr<PlayerStrategy> make_cpu_strategy(
        const CpuDifficulty difficulty)
    {
        switch (difficulty)
        {
            case CpuDifficulty::Easy:
                return std::make_unique<RandomStrategy>();

            case CpuDifficulty::Medium:
                return std::make_unique<GreedyStrategy>();

            case CpuDifficulty::Hard:
                return std::make_unique<MonteCarloStrategy>(HARD_DIFFICULTY_SAMPLE_COUNT);
        }

        throw std::invalid_argument("Unknown CPU difficulty");
    }
}
