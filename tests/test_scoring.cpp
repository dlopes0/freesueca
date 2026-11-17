#include <catch2/catch_test_macros.hpp>

#include "freesueca/scoring.hpp"

using namespace freesueca;

TEST_CASE("A team earns no riscos when it loses")
{
    CHECK(calculate_riscos(50, 70, 4) == 0);
}

TEST_CASE("A draw awards no riscos")
{
    CHECK(calculate_riscos(60, 60, 5) == 0);
}

TEST_CASE("A team earns one risco with 61 to 90 points")
{
    CHECK(calculate_riscos(61, 59, 5) == 1);
    CHECK(calculate_riscos(90, 30, 6) == 1);
}

TEST_CASE("A team earns two riscos with 91 or more points")
{
    CHECK(calculate_riscos(91, 29, 7) == 2);
    CHECK(calculate_riscos(119, 1, 9) == 2);
}

TEST_CASE("A team earns four riscos for a capote")
{
    CHECK(calculate_riscos(120, 0, 10) == 4);
}