#include <catch2/catch_test_macros.hpp>

#include "freesueca/game.hpp"

#include <stdexcept>

using namespace freesueca;

namespace
{
    void play_legal_card(Game& game)
    {
        const std::size_t player_id = game.current_player_id();
        const Player& player = game.player(player_id);

        std::size_t selected_index = 0;

        // The first player in a trick can play any card
        if (game.current_trick().card_count() > 0)
        {
            const Suit lead = game.current_trick().lead_suit();

            // If the player has the lead suit, find a card of that suit
            if (player.has_suit(lead))
            {
                for (std::size_t i = 0; i < player.card_count(); ++i)
                {
                    if (player.hand()[i].suit() == lead)
                    {
                        selected_index = i;
                        break;
                    }
                }
            }
        }

        game.play_card(selected_index);
    }

    void finish_hand(Game& game)
    {
        while (!game.is_hand_over())
        {
            play_legal_card(game);
        }
    }
}

TEST_CASE("Game starts with four players", "[game]")
{
    Game game(12345);

    game.start();

    for (std::size_t i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        CHECK(game.player(i).id() == i);
    }
}

TEST_CASE("Game deals ten cards to each player", "[game]")
{
    Game game(12345);

    game.start();

    for (std::size_t i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        CHECK(game.player(i).card_count() == 10);
    }

    CHECK(game.deck().remaining() == 0);
}

TEST_CASE("Dealer receives the trump card", "[game]")
{
    Game game(12345);
    game.start();

    const PlayerId dealer = game.dealer_id();

    CHECK(game.trump_card() == game.player(dealer).hand().back());
    CHECK(game.trump_suit() == game.trump_card().suit());
}

TEST_CASE("Players take turns in anti-clockwise order", "[game]")
{
    Game game(12345);
    game.start();

    CHECK(game.dealer_id() == 0);
    CHECK(game.current_player_id() == 1);

    game.play_card(0);
    CHECK(game.current_player_id() == 2);

    game.play_card(0);
    CHECK(game.current_player_id() == 3);

    game.play_card(0);
    CHECK(game.current_player_id() == 0);
}

TEST_CASE("Player must follow the lead suit when possible", "[game]")
{
    Game game(12345);
    game.start();

    // First player leads.
    game.play_card(0);

    const Suit lead = game.current_trick().lead_suit();

    // Identify the player whose turn it is.
    const PlayerId follower_id = game.current_player_id();
    const Player& follower = game.player(follower_id);

    REQUIRE(follower.has_suit(lead));

    // Find an off-suit card.
    std::size_t off_suit_index = follower.card_count();

    for (std::size_t i = 0; i < follower.card_count(); ++i)
    {
        if (follower.hand()[i].suit() != lead)
        {
            off_suit_index = i;
            break;
        }
    }

    REQUIRE(off_suit_index < follower.card_count());

    const auto original_count = follower.card_count();

    CHECK_THROWS_AS(
        game.play_card(off_suit_index),
        std::invalid_argument
    );

    CHECK(game.player(follower_id).card_count() == original_count);
    CHECK(game.current_player_id() == follower_id);
}

TEST_CASE("Player can play off-suit when they cannot follow suit", "[game]")
{
    Game game(12345);
    game.start();

    game.play_card(0);

    const Suit lead = game.current_trick().lead_suit();
    const Player& second_player = game.player(1);

    if (!second_player.has_suit(lead))
    {
        CHECK_NOTHROW(game.play_card(0));
        CHECK(game.current_player_id() == 2);
    }
}

TEST_CASE("Winner of trick leads the next trick", "[game]")
{
    Game game(12345);
    game.start();

    for (std::size_t i = 0; i < Game::PLAYER_COUNT - 1; ++i)
    {
        play_legal_card(game);
        CHECK(game.current_trick().card_count() == i + 1);
    }

    play_legal_card(game);

    CHECK(game.tricks_played() == 1);
    CHECK(game.current_trick().card_count() == 0);
    CHECK(game.current_player_id() == game.last_trick_winner());
}

TEST_CASE("Trick points are awarded to the winning team", "[game]")
{
    Game game(12345);
    game.start();

    // Play four legal cards
    for (std::size_t i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        play_legal_card(game);
    }

    const auto winner = game.last_trick_winner();
    const auto points = game.last_trick_points();

    CHECK(game.team_points(winner % 2) == points);
    CHECK(game.team_points(1 - winner % 2) == 0);
}

TEST_CASE("Completing ten tricks finishes the hand", "[game]")
{
    Game game(12345);
    game.start();

    while (!game.is_hand_over())
    {
        play_legal_card(game);
    }

    CHECK(game.tricks_played() == Game::CARDS_PER_PLAYER);
    CHECK(game.is_hand_over());
    CHECK(game.state() == GameState::HandFinished);
    CHECK_FALSE(game.is_over());
}

TEST_CASE("Cannot play before starting a game", "[game]")
{
    Game game(12345);

    CHECK_THROWS_AS(game.play_card(0), std::logic_error);
}

TEST_CASE("Game can be restarted", "[game]")
{
    Game game(12345);
    game.start();

    const PlayerId current = game.current_player_id();

    game.play_card(0);

    REQUIRE(game.player(current).card_count() == 9);

    game.start();

    for (std::size_t i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        CHECK(game.player(i).card_count() == 10);
    }

    CHECK(game.deck().remaining() == 0);
    CHECK(game.trick_history().empty());
    CHECK(game.tricks_played() == 0);
}

TEST_CASE("Game rejects invalid player indexes", "[game]")
{
    Game game(12345);

    REQUIRE_THROWS_AS(
        game.player(Game::PLAYER_COUNT),
        std::out_of_range
    );
}

TEST_CASE("Game preserves the last completed trick", "[game]")
{
    Game game(12345);
    game.start();

    for (std::size_t i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        play_legal_card(game);
    }

    CHECK(game.last_trick().is_complete());
    CHECK(game.last_trick().card_count() == 4);

    CHECK(game.current_trick().card_count() == 0);

    CHECK(game.last_trick().points() == game.last_trick_points());
}

TEST_CASE("Game preserves all completed tricks", "[game]")
{
    Game game(12345);
    game.start();

    for (int i = 0; i < 3 * Game::PLAYER_COUNT; ++i)
    {
        play_legal_card(game);
    }

    CHECK(game.tricks_played() == 3);
    CHECK(game.trick_history().size() == 3);

    for (const auto& trick : game.trick_history())
    {
        CHECK(trick.is_complete());
        CHECK(trick.card_count() == 4);
    }
}

TEST_CASE("Last trick corresponds to the end of game history", "[game]")
{
    Game game(12345);
    game.start();

    for (int i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        play_legal_card(game);
    }

    REQUIRE(game.trick_history().size() == 1);

    CHECK(game.last_trick().played_cards() == game.trick_history().back().played_cards());

    CHECK(game.last_trick().points() == game.last_trick_points());
}

TEST_CASE("Game history is cleared when restarting", "[game]")
{
    Game game(12345);
    game.start();

    for (int i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        play_legal_card(game);
    }

    REQUIRE(game.trick_history().size() == 1);

    game.start();

    CHECK(game.trick_history().empty());
    CHECK(game.tricks_played() == 0);
    CHECK(game.current_trick().card_count() == 0);
}

TEST_CASE("Cannot access last trick before one is completed", "[game]")
{
    Game game(12345);
    game.start();

    CHECK_THROWS_AS(game.last_trick(), std::logic_error);
}

TEST_CASE("Game starts in the NotStarted state", "[game]")
{
    Game game(12345);

    CHECK(game.state() == GameState::NotStarted);
    CHECK_FALSE(game.is_over());
}

TEST_CASE("Starting a game changes its state", "[game]")
{
    Game game(12345);

    game.start();

    CHECK(game.state() == GameState::InProgress);
}

TEST_CASE("Restarting a finished game resets its state")
{
    Game game(12345, 1);
    game.start();

    while (!game.is_hand_over())
    {
        play_legal_card(game);
    }

    REQUIRE(game.is_over());

    game.start();

    CHECK(game.state() == GameState::InProgress);
    CHECK_FALSE(game.is_over());
    CHECK(game.dealer_id() == 0);
    CHECK(game.team_riscos(0) == 0);
    CHECK(game.team_riscos(1) == 0);
}

TEST_CASE("Player after dealer leads the first trick", "[game]")
{
    Game game(12345);
    game.start();

    CHECK(
        game.current_player_id()
        == (game.dealer_id() + 1) % Game::PLAYER_COUNT
    );
}

TEST_CASE("Dealer rotates after each completed hand", "[game]")
{
    Game game(12345);
    game.start();

    while (!game.is_hand_over())
    {
        play_legal_card(game);
    }

    CHECK(game.dealer_id() == 0);

    game.start_next_hand();

    CHECK(game.dealer_id() == 1);
    CHECK(game.current_player_id() == 2);
    CHECK(game.state() == GameState::InProgress);

    for (std::size_t i = 0; i < Game::PLAYER_COUNT; ++i)
    {
        CHECK(game.player(i).card_count() == 10);
    }
}

TEST_CASE("Starting a new match resets match scores")
{
    Game game(12345);
    game.start();

    CHECK(game.team_riscos(0) == 0);
    CHECK(game.team_riscos(1) == 0);
    CHECK(game.dealer_id() == 0);

    // Finish the hand.
    while (!game.is_hand_over()) {
        play_legal_card(game);
    }

    game.start();

    CHECK(game.team_riscos(0) == 0);
    CHECK(game.team_riscos(1) == 0);
    CHECK(game.dealer_id() == 0);
    CHECK(game.state() == GameState::InProgress);
}

TEST_CASE("Next hand preserves accumulated riscos")
{
    Game game(12345);
    game.start();

    while (!game.is_hand_over())
    {
        play_legal_card(game);
    }

    const int previous_team0 = game.team_riscos(0);
    const int previous_team1 = game.team_riscos(1);

    CHECK(previous_team0 + previous_team1 > 0);

    game.start_next_hand();

    CHECK(game.team_riscos(0) == previous_team0);
    CHECK(game.team_riscos(1) == previous_team1);

    CHECK(game.team_points(0) == 0);
    CHECK(game.team_points(1) == 0);

    CHECK(game.team_tricks(0) == 0);
    CHECK(game.team_tricks(1) == 0);

    CHECK(game.tricks_played() == 0);
    CHECK(game.dealer_id() == 1);
    CHECK(game.current_player_id() == 2);
}

TEST_CASE("Cannot start next hand while playing")
{
    Game game(12345);

    CHECK_THROWS_AS(
        game.start_next_hand(),
        std::logic_error
    );

    game.start();

    CHECK_THROWS_AS(
        game.start_next_hand(),
        std::logic_error
    );
}

TEST_CASE("Default match target is 10 riscos")
{
    Game game;

    CHECK(game.match_target() == 10);
}

TEST_CASE("Match target can be configured")
{
    Game game(12345, 4);

    CHECK(game.match_target() == 4);
}

TEST_CASE("Match target must be positive")
{
    CHECK_THROWS_AS(Game(12345, 0), std::invalid_argument);
    CHECK_THROWS_AS(Game(12345, -1), std::invalid_argument);
}

TEST_CASE("A hand can finish while the match continues")
{
    Game game(12345, 10);
    game.start();

    while (!game.is_hand_over())
    {
        play_legal_card(game);
    }

    CHECK(game.state() == GameState::HandFinished);
    CHECK_FALSE(game.is_over());

    CHECK_NOTHROW(game.start_next_hand());
}

TEST_CASE("Match finishes when a team reaches its target")
{
    Game game(12345, 1);
    game.start();

    while (!game.is_hand_over())
    {
        play_legal_card(game);
    }

    CHECK(game.is_over());
    CHECK(game.state() == GameState::Finished);
}

TEST_CASE("Match summary starts with zero statistics")
{
    Game game(12345);
    game.start();

    const MatchSummary summary = game.match_summary();

    CHECK(summary.riscos[0] == 0);
    CHECK(summary.riscos[1] == 0);

    CHECK(summary.card_points[0] == 0);
    CHECK(summary.card_points[1] == 0);

    CHECK(summary.tricks[0] == 0);
    CHECK(summary.tricks[1] == 0);

    CHECK_FALSE(summary.winning_team.has_value());
}

TEST_CASE("Match summary accumulates statistics across hands")
{
    Game game(12345, 10);
    game.start();

    finish_hand(game);

    const MatchSummary first = game.match_summary();

    const int first_points =
        first.card_points[0] + first.card_points[1];

    const int first_tricks =
        first.tricks[0] + first.tricks[1];

    CHECK(first_points == 120);
    CHECK(first_tricks == 10);

    game.start_next_hand();
    finish_hand(game);

    const MatchSummary second = game.match_summary();

    CHECK(second.card_points[0] + second.card_points[1] == 240);
    CHECK(second.tricks[0] + second.tricks[1] == 20);

    CHECK(
        second.card_points[0] >= first.card_points[0]
    );

    CHECK(
        second.card_points[1] >= first.card_points[1]
    );

    CHECK_FALSE(second.winning_team.has_value());
}

TEST_CASE("Match summary identifies the winning team")
{
    Game game(12345, 1);
    game.start();

    finish_hand(game);

    const MatchSummary summary = game.match_summary();

    REQUIRE(summary.winning_team.has_value());

    const std::size_t winner = *summary.winning_team;

    CHECK(winner < 2);
    CHECK(summary.riscos[winner] >= 1);
    CHECK(summary.riscos[1 - winner] == 0);
}

TEST_CASE("Decision context contains current team scores")
{
    Game game(1234);
    game.start();

    // Play one complete trick.
    while (game.tricks_played() == 0)
    {
        const auto legal = game.legal_card_indices();
        game.play_card(legal.front());
    }

    const PlayerDecisionContext context =
        game.decision_context();

    const std::size_t our_team = context.player_id % 2;
    const std::size_t opposing_team = 1 - our_team;

    CHECK(context.our_team_points == game.team_points(our_team));
    CHECK(context.opposing_team_points == game.team_points(opposing_team));
}

