#include "freesueca/game.hpp"
#include "freesueca/player_strategy.hpp"
#include "freesueca/greedy_strategy.hpp"
#include "freesueca/monte_carlo_strategy.hpp"

#include <array>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace freesueca;

enum class ParallelMode
{
    Games,
    Samples
};

struct SimulationResults
{
    std::array<long long, 2> points{};
    std::array<long long, 2> tricks{};
    std::array<long long, 2> riscos{};

    long long wins_team_0 = 0;
    long long wins_team_1 = 0;
    long long draws = 0;

    std::size_t hands = 0;

    void add(const SimulationResults& other)
    {
        for (std::size_t team = 0; team < 2; ++team)
        {
            points[team] += other.points[team];
            tricks[team] += other.tricks[team];
            riscos[team] += other.riscos[team];
        }

        wins_team_0 += other.wins_team_0;
        wins_team_1 += other.wins_team_1;
        draws += other.draws;
        hands += other.hands;
    }
};

struct BenchmarkConfig
{
    std::size_t hands = 1000;
    std::size_t threads = 1;

    ParallelMode parallel_mode = ParallelMode::Games;

    std::vector<std::size_t> sample_counts{
        50, 250, 500
    };
};

[[nodiscard]] SimulationResults simulate_one_game(
    std::size_t game_index,
    bool monte_carlo_team_0,
    std::size_t sample_count,
    ParallelMode parallel_mode,
    std::size_t thread_count)
{
    SimulationResults results;

    constexpr int SIMULATION_MATCH_TARGET = 1'000'000;
    constexpr std::size_t HANDS_PER_GAME = 4;

    const auto game_seed =
        static_cast<std::uint32_t>(game_index + 1);

    Game game(game_seed, SIMULATION_MATCH_TARGET);
    game.start();

    // Each game owns its own strategies and RNG state.

    std::array<std::unique_ptr<PlayerStrategy>, 4> strategies;

    for (PlayerId player = 0; player < 4; ++player)
    {
        const bool is_monte_carlo_player =
            (player % 2 == 0) == monte_carlo_team_0;

        if (is_monte_carlo_player)
        {
            const auto strategy_seed =
                static_cast<std::uint32_t>(
                    0x9E3779B9u ^
                    (game_seed * 0x85EBCA6Bu) ^
                    (static_cast<std::uint32_t>(player) *
                     0xC2B2AE35u));

            if (parallel_mode == ParallelMode::Samples)
            {
                strategies[player] =
                    std::make_unique<MonteCarloStrategy>(
                        sample_count,
                        strategy_seed,
                        thread_count);
            }
            else
            {
                strategies[player] =
                    std::make_unique<MonteCarloStrategy>(
                        sample_count,
                        strategy_seed);
            }
        }
        else
        {
            strategies[player] =
                std::make_unique<GreedyStrategy>();
        }
    }

    for (std::size_t hand = 0;
         hand < HANDS_PER_GAME;
         ++hand)
    {
        const std::array<int, 2> riscos_before = {
            game.team_riscos(0),
            game.team_riscos(1)
        };

        while (!game.is_hand_over())
        {
            const PlayerId player_id =
                game.current_player_id();

            const PlayerDecisionContext context =
                game.decision_context();

            const std::size_t selection =
                strategies[player_id]->choose_card(context);

            (void)game.play_card(selection);
        }

        for (std::size_t team = 0; team < 2; ++team)
        {
            results.points[team] += game.team_points(team);
            results.tricks[team] += game.team_tricks(team);

            results.riscos[team] +=
                game.team_riscos(team) - riscos_before[team];
        }

        if (game.team_points(0) > game.team_points(1))
            ++results.wins_team_0;
        else if (game.team_points(1) > game.team_points(0))
            ++results.wins_team_1;
        else
            ++results.draws;

        ++results.hands;

        if (hand + 1 < HANDS_PER_GAME)
            game.start_next_hand();
    }

    return results;
}

[[nodiscard]] SimulationResults run_experiment(
    std::size_t number_of_hands,
    bool monte_carlo_team_0,
    std::size_t sample_count,
    ParallelMode parallel_mode,
    std::size_t thread_count)
{
    constexpr std::size_t HANDS_PER_GAME = 4;

    const std::size_t game_count =
        number_of_hands / HANDS_PER_GAME;

    std::vector<SimulationResults> per_game_results(
        game_count);

    const auto simulate_game =
        [&](std::size_t game_index)
    {
        per_game_results[game_index] =
            simulate_one_game(
                game_index,
                monte_carlo_team_0,
                sample_count,
                parallel_mode,
                thread_count);
    };

    if (parallel_mode == ParallelMode::Games)
    {
        // Parallelize independent games.

#ifdef _OPENMP
#pragma omp parallel for schedule(static) \
    num_threads(static_cast<int>(thread_count))
#endif
        for (long long game_index = 0;
             game_index < static_cast<long long>(game_count);
             ++game_index)
        {
            simulate_game(
                static_cast<std::size_t>(game_index));
        }
    }
    else
    {
        // Games run sequentially.
        // Monte Carlo samples are parallelized inside each decision.

        for (std::size_t game_index = 0;
             game_index < game_count;
             ++game_index)
        {
            simulate_game(game_index);
        }
    }

    SimulationResults total;

    for (const auto& game_results : per_game_results)
        total.add(game_results);

    return total;
}

void print_results(
    const SimulationResults& results,
    bool monte_carlo_team_0,
    std::size_t sample_count,
    ParallelMode parallel_mode,
    std::size_t thread_count,
    double elapsed_seconds)
{
    if (results.hands == 0)
    {
        std::cout
            << "No complete games simulated. "
            << "The requested hand count must be at least 4.\n";

        return;
    }

    const double hands =
        static_cast<double>(results.hands);

    const std::size_t monte_carlo_team =
        monte_carlo_team_0 ? 0 : 1;

    const std::size_t greedy_team =
        1 - monte_carlo_team;

    const long long monte_carlo_wins =
        monte_carlo_team_0
            ? results.wins_team_0
            : results.wins_team_1;

    const long long greedy_wins =
        monte_carlo_team_0
            ? results.wins_team_1
            : results.wins_team_0;

    std::cout << "\nHands simulated: "
              << results.hands << '\n';

    std::cout << "Monte Carlo samples per decision: "
              << sample_count << '\n';

    std::cout << "Parallelization mode: "
              << (parallel_mode == ParallelMode::Games
                      ? "games" : "samples")
              << '\n';

    std::cout << "Threads requested: "
              << thread_count << '\n';

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\nMonte Carlo team:\n";

    std::cout << "  Average points: "
              << results.points[monte_carlo_team] / hands << '\n';

    std::cout << "  Average tricks: "
              << results.tricks[monte_carlo_team] / hands << '\n';

    std::cout << "  Average riscos: "
              << results.riscos[monte_carlo_team] / hands << '\n';

    std::cout << "  Win rate: "
              << 100.0 * monte_carlo_wins / hands
              << "%\n";

    std::cout << "\nGreedy team:\n";

    std::cout << "  Average points: "
              << results.points[greedy_team] / hands << '\n';

    std::cout << "  Average tricks: "
              << results.tricks[greedy_team] / hands << '\n';

    std::cout << "  Average riscos: "
              << results.riscos[greedy_team] / hands << '\n';

    std::cout << "  Win rate: "
              << 100.0 * greedy_wins / hands
              << "%\n";

    std::cout << "\nDraw rate: "
              << 100.0 * results.draws / hands
              << "%\n";

    std::cout << "Elapsed time: "
              << elapsed_seconds << " seconds\n";

    if (elapsed_seconds > 0.0)
    {
        std::cout << "Hands per second: "
                  << hands / elapsed_seconds << '\n';
    }
}

void run_configuration(
    const BenchmarkConfig& config,
    std::size_t sample_count)
{
    std::cout << "\n========================================\n";

    std::cout << "Monte Carlo vs Greedy — "
              << sample_count << " samples\n";

    std::cout << "========================================\n";

    for (int orientation = 0;
         orientation < 2;
         ++orientation)
    {
        const bool monte_carlo_team_0 =
            (orientation == 0);

        std::cout << "\nExperiment: Monte Carlo team "
                  << (monte_carlo_team_0 ? 0 : 1)
                  << '\n';

        const auto start =
            std::chrono::steady_clock::now();

        const auto results = run_experiment(
            config.hands,
            monte_carlo_team_0,
            sample_count,
            config.parallel_mode,
            config.threads);

        const auto end =
            std::chrono::steady_clock::now();

        const double elapsed_seconds =
            std::chrono::duration<double>(
                end - start).count();

        print_results(
            results,
            monte_carlo_team_0,
            sample_count,
            config.parallel_mode,
            config.threads,
            elapsed_seconds);
    }
}

[[nodiscard]] std::size_t parse_positive_size(
    const std::string& value,
    const std::string& option_name)
{
    try
    {
        std::size_t parsed_characters = 0;

        const unsigned long long parsed =
            std::stoull(value, &parsed_characters);

        if (parsed_characters != value.size() ||
            parsed == 0 ||
            parsed > static_cast<unsigned long long>(SIZE_MAX))
        {
            throw std::invalid_argument(
                "invalid positive integer");
        }

        return static_cast<std::size_t>(parsed);
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument(
            "Invalid value for " + option_name + ": " + value);
    }
}

[[nodiscard]] std::vector<std::size_t> parse_sample_counts(
    const std::string& value)
{
    std::vector<std::size_t> samples;

    std::size_t start = 0;

    while (start < value.size())
    {
        const std::size_t comma =
            value.find(',', start);

        const std::string item = value.substr(
            start,
            comma == std::string::npos
                ? std::string::npos
                : comma - start);

        samples.push_back(
            parse_positive_size(item, "--samples"));

        if (comma == std::string::npos)
            break;

        start = comma + 1;
    }

    if (samples.empty())
    {
        throw std::invalid_argument(
            "--samples requires at least one value");
    }

    return samples;
}

void print_usage(const char* executable)
{
    std::cout
        << "Usage: " << executable << " [options]\n\n"
        << "Options:\n"
        << "  --hands N          Requested hands per orientation (default: 1000)\n"
        << "  --threads N        OpenMP worker threads (default: runtime maximum)\n"
        << "  --samples A,B,C    Monte Carlo samples per decision (default: 50,250,500)\n"
        << "  --parallel MODE    Parallelize 'games' or 'samples' (default: games)\n"
        << "  --help             Show this help\n\n"
        << "The hand count is rounded down to a multiple of four so each\n"
        << "independent game can rotate the dealer through all four seats.\n";
}

int main(int argc, char* argv[])
{
    BenchmarkConfig config;

#ifdef _OPENMP
    config.threads =
        static_cast<std::size_t>(omp_get_max_threads());
#else
    config.threads = 1;
#endif

    try
    {
        for (int i = 1; i < argc; ++i)
        {
            const std::string argument = argv[i];

            if (argument == "--help")
            {
                print_usage(argv[0]);
                return 0;
            }

            if (i + 1 >= argc)
            {
                throw std::invalid_argument(
                    "Missing value for " + argument);
            }

            const std::string value = argv[++i];

            if (argument == "--hands")
            {
                config.hands =
                    parse_positive_size(value, argument);
            }
            else if (argument == "--threads")
            {
                config.threads =
                    parse_positive_size(value, argument);
            }
            else if (argument == "--samples")
            {
                config.sample_counts =
                    parse_sample_counts(value);
            }
            else if (argument == "--parallel")
            {
                if (value == "games")
                    config.parallel_mode = ParallelMode::Games;
                else if (value == "samples")
                    config.parallel_mode = ParallelMode::Samples;
                else
                    throw std::invalid_argument(
                        "--parallel must be either 'games' or 'samples'");
            }
            else
            {
                throw std::invalid_argument(
                    "Unknown option: " + argument);
            }
        }

        if (config.threads >
            static_cast<std::size_t>(INT_MAX))
        {
            throw std::invalid_argument(
                "--threads is too large");
        }

#ifdef _OPENMP
        omp_set_dynamic(0);
#else
        if (config.threads > 1)
        {
            std::cerr
                << "Warning: this executable was built without OpenMP. "
                << "Running serially with one thread.\n";

            config.threads = 1;
        }
#endif

        const std::size_t actual_hands =
            (config.hands / 4) * 4;

        std::cout << "FreeSueca AI Benchmark\n";
        std::cout << "======================\n";

        std::cout << "Matchup: Monte Carlo vs Greedy\n";

        std::cout << "Hands per orientation: "
                  << actual_hands << '\n';

#ifdef _OPENMP
        std::cout << "OpenMP enabled; maximum available threads: "
                  << omp_get_max_threads() << '\n';
#else
        std::cout << "OpenMP disabled; running serially\n";
#endif

        std::cout << "Parallelization mode: "
                  << (config.parallel_mode == ParallelMode::Games
                          ? "games" : "samples")
                  << '\n';

        std::cout << "Threads requested: "
                  << config.threads << '\n';

        for (const std::size_t samples : config.sample_counts)
        {
            run_configuration(config, samples);
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: "
                  << exception.what() << '\n';

        print_usage(argv[0]);

        return 1;
    }

    return 0;
}