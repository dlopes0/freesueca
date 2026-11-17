#include "freesueca/scoring.hpp"

namespace freesueca
{

    int calculate_riscos(
        int team_points,
        int opposing_team_points,
        int team_tricks
    )
    {
        if (team_points <= opposing_team_points)
        {
            return 0;
        }

        if (team_tricks == 10)
        {
            return 4;
        }

        if (team_points >= 91)
        {
            return 2;
        }

        return 1;
    }

}