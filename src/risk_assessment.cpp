#include "freesueca/risk_assessment.hpp"
#include "freesueca/card_order.hpp"

namespace freesueca
{
    bool should_take_risk(
        int our_team_points,
        int opposing_team_points,
        int trick_points,
        const Card& candidate) noexcept
    {
        int required_points = 20;

        // Adjust according to the current score difference
        const int score_difference =
            our_team_points - opposing_team_points;

        if (score_difference <= -20)
            required_points -= 5;
        else if (score_difference >= 20)
            required_points += 5;

        // Secure our hand if we are approaching 91 points
        if (our_team_points >= 80)
            required_points -= 10;

        // Prevent the opposing team from reaching 91 points
        if (opposing_team_points >= 80)
            required_points -= 5;

        // Stronger cards are more expensive to risk
        const int card_cost =
            card_order::rank_strength(candidate.rank()) / 2
            + candidate.points() / 2;

        required_points += card_cost;

        return trick_points >= required_points;
    }

}