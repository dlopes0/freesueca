#include "freesueca/random_strategy.hpp"

#include <stdexcept>

namespace freesueca {

    RandomStrategy::RandomStrategy()
        : rng_(std::random_device{}())
    {
    }

    RandomStrategy::RandomStrategy(std::uint32_t seed)
        : rng_(seed)
    {
    }

    std::size_t RandomStrategy::choose_card(
    const PlayerDecisionContext& context)
    {
        const auto& legal = context.legal_card_indices;

        if (legal.empty())
            throw std::logic_error("No legal cards available");

        std::uniform_int_distribution<std::size_t> distribution(
            0, legal.size() - 1
        );

        return legal[distribution(rng_)];
    }

}