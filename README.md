# FreeSueca

FreeSueca is a C++20 implementation of the Portuguese trick-taking card game **Sueca**. The project is built around a separation between the game engine, player strategies, application configuration, and terminal interface.

The goal is to provide a clear, testable foundation for playing Sueca against computer-controlled opponents, while also making it possible to experiment with and evaluate different decision-making strategies.

## Features

- Four-player Sueca with two opposing teams:
  - Team 1: Players 1 and 3
  - Team 2: Players 2 and 4
- Standard 40-card deck and ten tricks per hand.
- Configurable match target, expressed in **riscos**.
- Human and CPU players can be mixed in the same match.
- Three selectable CPU difficulty levels:
  - **Easy** — random legal-card selection.
  - **Medium** — greedy, heuristic-based card selection.
  - **Hard** — Monte Carlo decision strategy (250 samples per decision in the CLI).
- Terminal commands for inspecting and sorting a hand, viewing scores, and consulting the rules.
- A separate simulation executable for comparing Monte Carlo and Greedy strategies.
- Unit tests built with Catch2.

## Rules implemented

FreeSueca uses a 40-card deck with the following ranks:

`A, 7, K, J, Q, 6, 5, 4, 3, 2`

Card points are:

| Card | Points |
|---|---:|
| Ace | 11 |
| Seven | 10 |
| King | 4 |
| Jack | 3 |
| Queen | 2 |
| Six, Five, Four, Three, Two | 0 |

Each hand consists of four players receiving ten cards each. Players must follow the lead suit when they can. A trick is won by the highest card in the lead suit, unless a trump card is played. The winner of a trick leads the next one.

At the end of a hand, the teams' card points and number of tricks determine the riscos awarded. The match ends when a team reaches the configured target.

## Requirements

- A C++20-compatible compiler.
- CMake 3.20 or newer.
- OpenMP when `FREESUECA_ENABLE_OPENMP` is enabled (the default).
- Git and an internet connection when configuring tests for the first time, because CMake FetchContent downloads Catch2.

The project has been developed with a CMake-based workflow and is suitable for IDEs such as CLion, as well as command-line builds.

## Build

Clone the repository and enter its directory:

```bash
git clone <repository-url>
cd freesueca
```

Configure and build the project:

```bash
cmake -S . -B build
cmake --build build --parallel
```

This builds the following executables:

- `freesueca` — interactive terminal game.
- `freesueca_simulation` — strategy simulation and comparison tool.
- `freesueca_tests` — test executable, when `BUILD_TESTING` is enabled.

### Build without OpenMP

OpenMP support can be disabled if it is not available in your environment:

```bash
cmake -S . -B build -DFREESUECA_ENABLE_OPENMP=OFF
cmake --build build --parallel
```

The simulation remains available, but its parallel execution support is disabled.

## Run the game

Start the interactive game from the build directory:

```bash
./build/freesueca
```

The setup sequence is:

1. Enter the number of riscos required to win the match.
2. Choose whether each of the four players is human-controlled.
3. Choose a difficulty for each CPU player.
4. Play the match.

At least one player must be human-controlled.

### In-game commands

During a human player's turn, enter a card number or one of the following commands:

| Input | Action |
|---|---|
| `1`–`10` | Play the corresponding card in your hand |
| `o` or `sort` | Sort your hand by suit and rank |
| `h` or `hand` | Display your hand |
| `s` or `score` | Display the current hand score and match standings |
| `r` or `rules` | Display the basic rules |
| `?` | Display the command help |
| `q` or `quit` | Quit the match |

The terminal output is presented as a chronological transcript. Each hand has a header with its dealer and trump information, followed by consistently formatted trick records and a hand summary.

## CPU strategies

The application currently exposes three difficulty levels through a strategy factory:

| Difficulty | Strategy | Behaviour |
|---|---|---|
| Easy | `RandomStrategy` | Selects randomly from the legal cards. |
| Medium | `GreedyStrategy` | Uses local heuristics to evaluate candidate plays. |
| Hard | `MonteCarloStrategy` | Evaluates candidate plays using sampled possible hidden hands and simulated outcomes. |

The strategy interface is independent of the terminal interface. A strategy receives a `PlayerDecisionContext` containing the acting player's hand, legal card indices, trump suit, current trick, trick history, and current team points. It returns the index of the card it chooses to play.

This design allows strategies to be developed and tested without coupling them to user input or terminal output.

## Strategy simulation

The `freesueca_simulation` executable runs experiments comparing a Monte Carlo team against a Greedy team. It tests both team orientations and reports aggregate points, tricks, riscos, win rates, elapsed time, and hands per second.

Run it with the default configuration:

```bash
./build/freesueca_simulation
```

Available options:

| Option | Description | Default |
|---|---|---|
| `--hands N` | Requested number of hands per team orientation | `1000` |
| `--threads N` | Number of OpenMP worker threads | Runtime maximum |
| `--samples A,B,C` | Monte Carlo samples per decision for each experiment | `50,250,500` |
| `--parallel MODE` | Parallelize independent `games` or Monte Carlo `samples` | `games` |
| `--help` | Display usage information | — |

Example:

```bash
./build/freesueca_simulation --hands 4000 --threads 8 --samples 50,250,500 --parallel games
```

The requested hand count is rounded down to a multiple of four. Each simulated game contains four hands, allowing the dealer to rotate through all four player positions. The simulation uses a large match target so that each experiment can complete its configured set of hands.

## Tests

Build the test target as part of the normal CMake build, then run:

```bash
ctest --test-dir build --output-on-failure
```

The test suite covers the card and deck model, dealing, players, tricks, scoring, game state, card ordering, decision context, strategy implementations, hidden-hand sampling, hand simulation, terminal input, and strategy factory.

Catch2 is retrieved through CMake FetchContent. The first configure may therefore require network access.

## Project structure

```text
.
├── CMakeLists.txt
├── include/
│   └── freesueca/
│       ├── application/       # Application-level configuration interfaces
│       ├── interface/         # Terminal input and player actions
│       ├── card.hpp
│       ├── deck.hpp
│       ├── game.hpp
│       ├── player.hpp
│       ├── player_strategy.hpp
│       ├── ...                # Strategies, tracking and simulation support
├── src/
│   ├── application/           # CPU strategy factory
│   ├── interface/             # Terminal input implementation
│   ├── main.cpp               # Interactive game entry point
│   ├── simulation.cpp         # Strategy experiment executable
│   └── ...                    # Game engine and strategy implementations
└── tests/                     # Catch2 unit tests
```

### Main components

- **Game engine**: cards, deck, players, tricks, dealing, legal plays, hand progression, and match scoring.
- **Player strategies**: common strategy interface and implementations such as Random, Greedy, Counting, and Monte Carlo.
- **Decision support**: card tracking, void tracking, risk assessment, hidden-hand sampling, and hand simulation.
- **Application layer**: maps user-selected CPU difficulty levels to concrete strategy implementations.
- **Terminal interface**: parses human actions independently of the game engine.
- **Simulation**: runs repeatable seeded experiments to compare strategy performance.
- **Tests**: validates the core rules and supporting components independently.

## Design principles

The project follows a few practical design goals:

- Keep the game engine independent from terminal input and output.
- Use a common strategy interface so different AI approaches can be substituted.
- Keep application configuration outside the core game rules.
- Make important game behaviour testable with automated tests.
- Support experiments with seeded games and configurable simulation parameters.

## Current scope

The repository contains a playable terminal application and a strategy simulation tool. It does not currently provide a graphical interface, online multiplayer, or network play.

## License

No license is specified in the project files. Add a `LICENSE` file and update this section before distributing the project under an open-source license.
