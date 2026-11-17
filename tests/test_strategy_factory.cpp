#include "freesueca/application/strategy_factory.hpp"

#include "freesueca/greedy_strategy.hpp"
#include "freesueca/monte_carlo_strategy.hpp"
#include "freesueca/random_strategy.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace freesueca;
using namespace freesueca::application;

TEST_CASE("CPU difficulty creates the corresponding strategy")
{
    SECTION("Easy creates a random strategy")
    {
        const auto strategy = make_cpu_strategy(CpuDifficulty::Easy);
        REQUIRE(dynamic_cast<RandomStrategy*>(strategy.get()) != nullptr);
    }

    SECTION("Medium creates a greedy strategy")
    {
        const auto strategy = make_cpu_strategy(CpuDifficulty::Medium);
        REQUIRE(dynamic_cast<GreedyStrategy*>(strategy.get()) != nullptr);
    }

    SECTION("Hard creates a Monte Carlo strategy")
    {
        const auto strategy = make_cpu_strategy(CpuDifficulty::Hard);
        REQUIRE(dynamic_cast<MonteCarloStrategy*>(strategy.get()) != nullptr);
    }
}

TEST_CASE("CPU players receive independent strategy instances")
{
    const auto player_1 = make_cpu_strategy(CpuDifficulty::Hard);
    const auto player_2 = make_cpu_strategy(CpuDifficulty::Hard);

    REQUIRE(player_1 != player_2);
}

TEST_CASE("Unknown CPU difficulty throws an exception")
{
    const auto invalid_difficulty = static_cast<CpuDifficulty>(-1);

    REQUIRE_THROWS(
        make_cpu_strategy(invalid_difficulty)
    );
}