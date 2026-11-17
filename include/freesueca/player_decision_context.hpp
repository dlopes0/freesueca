#pragma once

#include "freesueca/card.hpp"
#include "freesueca/trick.hpp"
#include "freesueca/types.hpp"

#include <cstddef>
#include <vector>

namespace freesueca
{
    struct PlayerDecisionContext
    {
        PlayerId player_id;

        // Private information belonging to this player
        std::vector<Card> hand;

        // Public game information
        std::vector<std::size_t> legal_card_indices;
        Suit trump_suit;
        Trick current_trick;
        std::vector<Trick> trick_history;

        // Current-hand score from this player's team's perspective
        // Default values preserve compatibility with existing aggregate initializers
        int our_team_points = 0;
        int opposing_team_points = 0;
    };
}
