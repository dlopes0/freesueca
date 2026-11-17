#pragma once

#include "freesueca/card.hpp"

namespace freesueca
{
    [[nodiscard]] bool should_take_risk(
        int our_team_points,
        int opposing_team_points,
        int trick_points,
        const Card& candidate) noexcept;
}