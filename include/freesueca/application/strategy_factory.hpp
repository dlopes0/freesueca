#pragma once

#include "freesueca/application/cpu_difficulty.hpp"
#include "freesueca/player_strategy.hpp"

#include <memory>

namespace freesueca::application
{
    // Creates a separate strategy instance for one CPU player
    [[nodiscard]] std::unique_ptr<PlayerStrategy> make_cpu_strategy(CpuDifficulty difficulty);
}
