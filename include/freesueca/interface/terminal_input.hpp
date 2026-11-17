#pragma once

#include "freesueca/interface/player_action.hpp"

#include <cstddef>
#include <iosfwd>

namespace freesueca::interface
{
    PlayerAction read_player_action(
        std::istream& input,
        std::ostream& output,
        std::size_t card_count
    );
}