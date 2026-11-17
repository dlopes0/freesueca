#include "freesueca/monte_carlo_strategy.hpp"

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace freesueca
{
    namespace
    {
        constexpr std::uint64_t SPLITMIX_INCREMENT =
            0x9E3779B97F4A7C15ULL;

        constexpr std::uint64_t SPLITMIX_MULTIPLIER_1 =
            0xBF58476D1CE4E5B9ULL;

        constexpr std::uint64_t SPLITMIX_MULTIPLIER_2 =
            0x94D049BB133111EBULL;

        std::uint32_t make_random_seed()
        {
            std::random_device random_device;
            return random_device();
        }
    }

    MonteCarloStrategy::MonteCarloStrategy(
        std::size_t sample_count)
        : sample_count_(sample_count),
          sampler_(),
          base_seed_(make_random_seed())
    {
        if (sample_count_ == 0)
            throw std::invalid_argument(
                "Sample count must be positive");
    }

    MonteCarloStrategy::MonteCarloStrategy(
        std::size_t sample_count,
        std::uint32_t seed)
        : sample_count_(sample_count),
          sampler_(seed),
          base_seed_(seed)
    {
        if (sample_count_ == 0)
            throw std::invalid_argument(
                "Sample count must be positive");
    }

    MonteCarloStrategy::MonteCarloStrategy(
        std::size_t sample_count,
        std::uint32_t seed,
        std::size_t parallel_threads)
        : sample_count_(sample_count),
          sampler_(seed),
          base_seed_(seed),
          parallel_threads_(parallel_threads),
          parallel_samples_(true)
    {
        if (sample_count_ == 0)
            throw std::invalid_argument(
                "Sample count must be positive");

        if (parallel_threads_ == 0)
            throw std::invalid_argument(
                "Parallel thread count must be positive");
    }

    std::uint32_t MonteCarloStrategy::sample_seed(
        std::uint32_t base_seed,
        std::uint64_t decision_index,
        std::size_t sample_index) noexcept
    {
        // Derive a stable seed from the strategy seed,
        // decision number and sample index.

        std::uint64_t value =
            static_cast<std::uint64_t>(base_seed) ^
            (SPLITMIX_INCREMENT * (decision_index + 1)) ^
            (0xD1B54A32D192ED03ULL *
             (static_cast<std::uint64_t>(sample_index) + 1));

        value += SPLITMIX_INCREMENT;

        value = (value ^ (value >> 30)) *
                SPLITMIX_MULTIPLIER_1;

        value = (value ^ (value >> 27)) *
                SPLITMIX_MULTIPLIER_2;

        value ^= value >> 31;

        return static_cast<std::uint32_t>(
            value ^ (value >> 32));
    }

    std::size_t MonteCarloStrategy::choose_card(
        const PlayerDecisionContext& context)
    {
        const auto& legal = context.legal_card_indices;

        if (legal.empty())
            throw std::logic_error(
                "No legal cards available");

        const std::size_t our_team =
            context.player_id % 2;

        std::vector<std::uint64_t> riscos_totals(
            legal.size(), 0);

        std::vector<std::uint64_t> points_totals(
            legal.size(), 0);

        if (!parallel_samples_)
        {
            // Original sequential sampling path.

            for (std::size_t sample = 0;
                 sample < sample_count_;
                 ++sample)
            {
                const SampledHands hands = sampler_.sample(context);

                HandSimulator::Workspace workspace;

                for (std::size_t candidate = 0;
                     candidate < legal.size();
                     ++candidate)
                {
                    const SimulationResult result =
                        simulator_.simulate(
                            context,
                            hands,
                            legal[candidate],
                            workspace);

                    riscos_totals[candidate] +=
                        static_cast<std::uint64_t>(
                            result.team_riscos[our_team]);

                    points_totals[candidate] +=
                        static_cast<std::uint64_t>(
                            result.team_points[our_team]);
                }
            }
        }
        else
        {
            // Each sample writes to its own result slot.
            // This avoids concurrent writes to shared totals.

            const std::uint64_t current_decision =
                decision_index_++;

            std::vector<SampleEvaluation> evaluations(
                sample_count_);

            for (auto& evaluation : evaluations)
            {
                evaluation.riscos.resize(legal.size());
                evaluation.points.resize(legal.size());
            }

            const auto evaluate_sample =
                [&](std::size_t sample)
            {
                // Every sample owns its own RNG.
                HiddenHandSampler sample_sampler(
                    sample_seed(
                        base_seed_,
                        current_decision,
                        sample));


                    const SampledHands hands = sample_sampler.sample(context);

                    HandSimulator::Workspace workspace;

                    SampleEvaluation& evaluation =
                        evaluations[sample];

                    for (std::size_t candidate = 0;
                         candidate < legal.size();
                         ++candidate)
                    {
                        const SimulationResult result =
                            simulator_.simulate(
                                context,
                                hands,
                                legal[candidate],
                                workspace);

                    evaluation.riscos[candidate] =
                        static_cast<std::uint64_t>(
                            result.team_riscos[our_team]);

                    evaluation.points[candidate] =
                        static_cast<std::uint64_t>(
                            result.team_points[our_team]);
                }
            };

#ifdef _OPENMP
#pragma omp parallel for schedule(static) \
    num_threads(static_cast<int>(parallel_threads_))
#endif
            for (long long sample = 0;
                 sample < static_cast<long long>(sample_count_);
                 ++sample)
            {
                evaluate_sample(
                    static_cast<std::size_t>(sample));
            }

            // Reduce the results sequentially.

            for (const SampleEvaluation& evaluation : evaluations)
            {
                for (std::size_t candidate = 0;
                     candidate < legal.size();
                     ++candidate)
                {
                    riscos_totals[candidate] +=
                        evaluation.riscos[candidate];

                    points_totals[candidate] +=
                        evaluation.points[candidate];
                }
            }
        }

        // Maximize expected riscos.
        // Use card points as a secondary criterion.

        std::size_t best_candidate = 0;

        for (std::size_t candidate = 1;
             candidate < legal.size();
             ++candidate)
        {
            if (riscos_totals[candidate] >
                    riscos_totals[best_candidate] ||
                (riscos_totals[candidate] ==
                    riscos_totals[best_candidate] &&
                 points_totals[candidate] >
                    points_totals[best_candidate]))
            {
                best_candidate = candidate;
            }
        }

        return legal[best_candidate];
    }
}