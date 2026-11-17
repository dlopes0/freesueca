cmake_minimum_required(VERSION 3.20)

project(freesueca
        VERSION 1.0.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

option(FREESUECA_ENABLE_OPENMP "Enable OpenMP parallelization" ON)

# Game library
add_library(freesueca_lib
        src/card.cpp
        src/deck.cpp
        src/player.cpp
        src/game.cpp
        src/trick.cpp
        src/scoring.cpp
        src/random_strategy.cpp
        src/greedy_strategy.cpp
        src/card_tracker.cpp
        src/counting_strategy.cpp
        src/void_tracker.cpp
        src/risk_assessment.cpp
        src/hidden_hand_sampler.cpp
        src/hand_simulator.cpp
        src/monte_carlo_strategy.cpp
)

target_include_directories(freesueca_lib
        PUBLIC include
)

# Interface library
add_library(freesueca_interface
        src/interface/terminal_input.cpp
)

target_include_directories(freesueca_interface
        PUBLIC include
)

# Application layer: application-level configuration and strategy creation.
add_library(freesueca_application
        src/application/strategy_factory.cpp
)

target_include_directories(freesueca_application
        PUBLIC include
)

target_link_libraries(freesueca_application
        PUBLIC freesueca_lib
)

# CLI executable
add_executable(freesueca
        src/main.cpp
)

target_link_libraries(freesueca
        PRIVATE
        freesueca_application
        freesueca_interface
)

# Simulation
add_executable(freesueca_simulation
        src/simulation.cpp
)

target_link_libraries(freesueca_simulation
        PRIVATE
        freesueca_lib
)

# Optional OpenMP support
if(FREESUECA_ENABLE_OPENMP)
    find_package(OpenMP REQUIRED)

    # MonteCarloStrategy uses OpenMP internally.
    target_link_libraries(freesueca_lib
            PRIVATE OpenMP::OpenMP_CXX
    )

    # The simulation executable also uses OpenMP.
    target_link_libraries(freesueca_simulation
            PRIVATE OpenMP::OpenMP_CXX
    )
endif()

# Testing
if(BUILD_TESTING)

    include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.8.1
    )

    FetchContent_MakeAvailable(Catch2)

    add_executable(freesueca_tests
            tests/test_card.cpp
            tests/test_deck.cpp
            tests/test_player.cpp
            tests/test_dealing.cpp
            tests/test_game.cpp
            tests/test_trick.cpp
            tests/test_scoring.cpp
            tests/test_strategy.cpp
            tests/test_card_order.cpp
            tests/test_player_decision_context.cpp
            tests/test_card_tracker.cpp
            tests/test_counting_strategy.cpp
            tests/test_void_tracker.cpp
            tests/test_risk_assessment.cpp
            tests/test_hidden_hand_sampler.cpp
            tests/test_hand_simulator.cpp
            tests/test_monte_carlo_strategy.cpp
            tests/test_terminal_input.cpp
            tests/test_strategy_factory.cpp
    )

    target_link_libraries(freesueca_tests
            PRIVATE
            freesueca_lib
            freesueca_interface
            freesueca_application
            Catch2::Catch2WithMain
    )

    include(Catch)

    catch_discover_tests(freesueca_tests)

endif()
