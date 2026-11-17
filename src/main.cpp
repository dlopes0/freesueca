#include "freesueca/game.hpp"
#include "freesueca/application/strategy_factory.hpp"
#include "freesueca/interface/terminal_input.hpp"

#include <cstddef>
#include <cctype>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <array>
#include <memory>
#include <random>
#include <string>

namespace
{
    using namespace freesueca;
    using namespace freesueca::application;

    const char* suit_name(Suit suit)
    {
        switch (suit)
        {
            case Suit::Hearts:   return "Hearts";
            case Suit::Diamonds: return "Diamonds";
            case Suit::Clubs:    return "Clubs";
            case Suit::Spades:   return "Spades";
        }

        return "Unknown";
    }

    bool ask_yes_no(const char* question)
    {
        while (true)
        {
            std::cout << question << " (y/n): ";

            char answer{};

            if (!(std::cin >> answer))
            {
                return false;
            }

            answer = static_cast<char>(
                std::tolower(static_cast<unsigned char>(answer))
            );

            if (answer == 'y')
                return true;

            if (answer == 'n')
                return false;

            std::cout << "Please enter y or n.\n";
        }
    }

    const char* rank_symbol(freesueca::Rank rank)
    {
        using freesueca::Rank;

        switch (rank)
        {
            case Rank::Ace:   return "A";
            case Rank::Seven: return "7";
            case Rank::King:  return "K";
            case Rank::Jack:  return "J";
            case Rank::Queen: return "Q";
            case Rank::Six:   return "6";
            case Rank::Five:  return "5";
            case Rank::Four:  return "4";
            case Rank::Three: return "3";
            case Rank::Two:   return "2";
        }

        return "?";
    }

    const char* suit_symbol(freesueca::Suit suit)
    {
        using freesueca::Suit;

        switch (suit)
        {
            case Suit::Hearts:   return "♥";
            case Suit::Diamonds: return "♦";
            case Suit::Clubs:    return "♣";
            case Suit::Spades:   return "♠";
        }

        return "?";
    }

    void print_card(const freesueca::Card& card)
    {
        std::cout << rank_symbol(card.rank())
                  << suit_symbol(card.suit())
                  << " (" << card.points() << " pts)";
    }

    void print_hand(const freesueca::Player& player)
    {
        std::cout << "\nPlayer " << player.id() + 1 << ":\n";

        const auto& hand = player.hand();

        for (std::size_t i = 0; i < hand.size(); ++i)
        {
            std::cout << "  "
                      << i + 1 << ". ";

            print_card(hand[i]);

            std::cout << '\n';
        }
    }

    void print_hand_score(const Game& game)
    {
        std::cout << "\n--- Current hand ---\n";

        std::cout << "Team 1 (Players 1 and 3): "
                  << game.team_points(0) << " points, "
                  << game.team_tricks(0) << " tricks\n";

        std::cout << "Team 2 (Players 2 and 4): "
                  << game.team_points(1) << " points, "
                  << game.team_tricks(1) << " tricks\n";
    }

    void print_match_score(const Game& game)
    {
        std::cout << "\n--- Match standings ---\n";

        std::cout << "Team 1: "
                  << game.team_riscos(0) << " riscos\n";

        std::cout << "Team 2: "
                  << game.team_riscos(1) << " riscos\n";

        std::cout << "Target: "
                  << game.match_target() << " riscos\n";
    }

    void print_hand_header(const Game& game, const std::size_t hand_number)
    {
        std::cout << "\n\n";
        std::cout << "============================================================\n";
        std::cout << "HAND " << hand_number << '\n';
        std::cout << "============================================================\n";

        std::cout << "Dealer : Player " << game.dealer_id() + 1 << '\n';
        std::cout << "Trump  : " << suit_symbol(game.trump_suit())
                  << " " << suit_name(game.trump_suit()) << "  (";
        print_card(game.trump_card());
        std::cout << ")\n";

        std::cout << "------------------------------------------------------------\n";
        std::cout << "Teams: 1 & 3  vs  2 & 4\n";
        std::cout << "------------------------------------------------------------\n";
    }

    void print_trick_start(const std::size_t trick_number)
    {
        std::cout << '\n';
        std::cout << "---------------------- TRICK "
                  << trick_number
                  << " ----------------------\n";
    }

    void print_card_play(const PlayerId player_id, const bool is_human,
                         const Card& card)
    {
        std::cout << "Player " << player_id + 1
                  << (is_human ? " (Human) : " : " (CPU)   : ");
        print_card(card);
        std::cout << '\n';
    }

    void print_trick_result(const Game& game)
    {
        std::cout << "  Winner : Player "
                  << game.last_trick_winner() + 1 << '\n';

        std::cout << "  Points : "
                  << game.last_trick_points() << '\n';

        std::cout << "  Score  : Team 1 "
                  << game.team_points(0) << " pts / "
                  << game.team_tricks(0) << " tricks"
                  << "  |  Team 2 "
                  << game.team_points(1) << " pts / "
                  << game.team_tricks(1) << " tricks\n";

        std::cout << "------------------------------------------------------------\n";
    }

    void print_match_summary(const Game& game)
    {
        const MatchSummary summary = game.match_summary();

        std::cout << "\n========================\n";
        std::cout << "       MATCH SUMMARY\n";
        std::cout << "========================\n";

        for (std::size_t team = 0; team < 2; ++team)
        {
            std::cout << "\nTeam " << team + 1 << ":\n";
            std::cout << "  Riscos: "
                      << summary.riscos[team] << '\n';
            std::cout << "  Card points: "
                      << summary.card_points[team] << '\n';
            std::cout << "  Tricks: "
                      << summary.tricks[team] << '\n';
        }

        if (summary.winning_team.has_value())
        {
            std::cout << "\nTeam "
                      << *summary.winning_team + 1
                      << " wins the match!\n";
        }
        else
        {
            std::cout << "\nMatch ended without a winner.\n";
        }
    }

    void print_help()
    {

        std::cout << "\n--- Available commands ---\n"
                  << "  1-10  Play the corresponding card\n"
                  << "  o     Sort hand by suit and rank\n"
                  << "  h     Display your hand\n"
                  << "  s     Display current score\n"
                  << "  r     Display game rules\n"
                  << "  ?     Display this help\n"
                  << "  q     Quit the match\n";
    }

    int ask_match_target()
    {
        while (true)
        {
            std::cout << "\nHow many riscos are required to win the match? ";

            int target{};

            if (!(std::cin >> target))
                throw std::runtime_error("Input closed while configuring the match target.");

            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');

            if (target > 0)
                return target;

            std::cout << "The match target must be a positive number of riscos.\n";
        }
    }

    freesueca::application::CpuDifficulty ask_cpu_difficulty(
        const freesueca::PlayerId player_id)
    {
        using freesueca::application::CpuDifficulty;

        while (true)
        {
            std::cout << "CPU Player " << player_id + 1
                      << " difficulty:\n"
                      << "  1. Easy   (Random)\n"
                      << "  2. Medium (Greedy)\n"
                      << "  3. Hard   (Monte Carlo, 250 samples)\n"
                      << "Select difficulty [1-3]: ";

            std::size_t choice{};

            if (!(std::cin >> choice))
                throw std::runtime_error("Input closed while configuring CPU players.");

            if (choice >= 1 && choice <= 3)
            {
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n');

                switch (choice)
                {
                    case 1: return CpuDifficulty::Easy;
                    case 2: return CpuDifficulty::Medium;
                    case 3: return CpuDifficulty::Hard;
                    default: break;
                }
            }

            std::cout << "Please select 1, 2, or 3.\n\n";
        }
    }

    const char* difficulty_name(
        const freesueca::application::CpuDifficulty difficulty)
    {
        using freesueca::application::CpuDifficulty;

        switch (difficulty)
        {
            case CpuDifficulty::Easy:   return "Easy";
            case CpuDifficulty::Medium: return "Medium";
            case CpuDifficulty::Hard:   return "Hard";
        }

        return "Unknown";
    }

    void configure_players(
    std::array<bool, Game::PLAYER_COUNT>& human_players,
    std::array<std::unique_ptr<PlayerStrategy>, Game::PLAYER_COUNT>& strategies,
    std::array<CpuDifficulty, Game::PLAYER_COUNT>& difficulties)
    {
        std::size_t human_count = 0;

        // Select which players are controlled by humans.
        do
        {
            human_count = 0;

            std::cout << "\nPLAYER CONFIGURATION\n";
            std::cout << "--------------------\n";

            for (PlayerId player_id = 0;
                 player_id < Game::PLAYER_COUNT;
                 ++player_id)
            {
                const std::string question =
                    "Is Player " + std::to_string(player_id + 1)
                    + " human-controlled?";

                human_players[player_id] = ask_yes_no(question.c_str());

                if (human_players[player_id])
                    ++human_count;
            }

            if (human_count == 0)
                std::cout << "\nAt least one human player is required.\n";

        } while (human_count == 0);

        // Configure the AI opponents.
        std::cout << "\nCPU DIFFICULTY\n";
        std::cout << "--------------\n";

        for (PlayerId player_id = 0;
             player_id < Game::PLAYER_COUNT;
             ++player_id)
        {
            if (human_players[player_id])
            {
                strategies[player_id].reset();
                continue;
            }

            difficulties[player_id] = ask_cpu_difficulty(player_id);

            strategies[player_id] =
                make_cpu_strategy(difficulties[player_id]);
        }

        // Display the final configuration.
        std::cout << "\nPLAYERS\n";
        std::cout << "-------\n";

        for (PlayerId player_id = 0;
             player_id < Game::PLAYER_COUNT;
             ++player_id)
        {
            std::cout << "Player " << player_id + 1 << ": ";

            if (human_players[player_id])
                std::cout << "Human\n";
            else
                std::cout << "CPU (" << difficulty_name(difficulties[player_id])
                          << ")\n";
        }
    }

    void print_rules(const Game& game)
    {
        std::cout << "\n--- Basic Sueca rules ---\n"
                  << "  Players must follow the lead suit if possible.\n"
                  << "  A trick is won by the highest card of the lead suit,\n"
                  << "  unless a trump card is played.\n"
                  << "  Aces are worth 11 points.\n"
                  << "  Sevens are worth 10 points.\n"
                  << "  Kings are worth 4 points.\n"
                  << "  Jacks are worth 3 points.\n"
                  << "  Queens are worth 2 points.\n";

        std::cout << "\n  Trump: " << suit_symbol(game.trump_suit()) << " " << suit_name(game.trump_suit()) << "  (";
        print_card(game.trump_card());
        std::cout << "\n";
    }
}

int main()
{
    using namespace freesueca;
    using namespace freesueca::interface;
    using namespace freesueca::application;

    std::cout << "freesueca v1.0.0" << std::endl;
    std::cout << "  Copyright (c) 2026 David Lopes" << std::endl;

    const int match_target = ask_match_target();
    Game game(std::random_device{}(), match_target);

    std::array<bool, Game::PLAYER_COUNT> human_players{};

    std::array<std::unique_ptr<PlayerStrategy>, Game::PLAYER_COUNT> strategies;

    std::array<CpuDifficulty, Game::PLAYER_COUNT> cpu_difficulties{};

    configure_players(human_players, strategies, cpu_difficulties);

    bool new_match = true;

    while (new_match)
    {
        game.start();

        std::size_t hand_number = 1;

        std::cout << "\nMATCH STARTED\n";
        std::cout << "Target: " << game.match_target() << " riscos\n";

        while (true)
        {
            print_hand_header(game, hand_number);

            while (!game.is_hand_over())
            {
                const PlayerId player_id = game.current_player_id();
                const Player& player = game.player(player_id);

                const bool is_human = human_players[player_id];

                std::size_t selection{};

                if (is_human)
                {
                    std::cout << "\nPlayer " << player_id + 1
                              << " — YOUR TURN\n";
                    print_hand(player);

                    const PlayerAction action = read_player_action(
                        std::cin,
                        std::cout,
                        player.hand().size()
                    );

                    switch (action.type)
                    {
                        case PlayerActionType::PlayCard:
                            selection = action.card_index;
                            break;

                        case PlayerActionType::SortHand:
                            game.sort_current_player_hand();
                            std::cout << "\nHand sorted.\n";
                            print_hand(player);
                            continue;

                        case PlayerActionType::ShowHand:
                            print_hand(player);
                            continue;

                        case PlayerActionType::ShowScore:
                            print_hand_score(game);
                            print_match_score(game);
                            continue;

                        case PlayerActionType::ShowRules:
                            print_rules(game);
                            continue;

                        case PlayerActionType::Help:
                            print_help();
                            continue;

                        case PlayerActionType::Quit:
                            std::cout << "\nGame interrupted.\n";
                            return 0;

                        case PlayerActionType::Invalid:
                            std::cout << "Invalid input. Enter a card number or command.\n";
                            continue;
                    }
                }
                else
                {
                    const PlayerDecisionContext context = game.decision_context();
                    selection = strategies[player_id]->choose_card(context);
                }

                try
                {
                    const Card played_card = player.hand().at(selection);
                    const bool starts_new_trick =
                        game.current_trick().card_count() == 0;
                    const std::size_t trick_number =
                        game.tricks_played() + 1;

                    const bool trick_completed = game.play_card(selection);

                    if (starts_new_trick)
                        print_trick_start(trick_number);

                    print_card_play(player_id, is_human, played_card);

                    if (trick_completed)
                        print_trick_result(game);
                }
                catch (const std::invalid_argument& error)
                {
                    std::cout << error.what() << '\n';
                }
            }


            std::cout << "\n============================================================\n";
            std::cout << "HAND " << hand_number << " SUMMARY\n";
            std::cout << "============================================================\n";

            print_hand_score(game);
            print_match_score(game);
            std::cout << "============================================================\n";

            if (game.is_over())
            {
                std::cout << "\nMatch target reached!\n";
                print_match_summary(game);
                break;
            }

            if (!ask_yes_no("\nPlay the next hand?"))
            {
                std::cout << "\nMatch stopped by player.\n";
                print_match_summary(game);
                return 0;
            }

            game.start_next_hand();
            ++hand_number;
        }

        new_match = ask_yes_no("\nStart a new match?");
    }

    std::cout << "\nThanks for playing Free Sueca!\n";

    return 0;
}
