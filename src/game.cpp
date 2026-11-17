#include "freesueca/game.hpp"
#include "freesueca/scoring.hpp"

#include <optional>
#include <stdexcept>
#include <algorithm>

namespace freesueca
{
    Game::Game() = default;

    Game::Game(std::uint32_t seed) : Game(seed, DEFAULT_MATCH_TARGET)
    {

    }

    Game::Game(std::uint32_t seed, int match_target)
    : deck_(seed),
      match_target_(match_target)
    {
        if (match_target_ <= 0)
        {
            throw std::invalid_argument("Match target must be positive");
        }
    }

    void Game::start()
    {
        dealer_id_ = 0;

        team_riscos_.fill(0);
        match_points_.fill(0);
        match_tricks_.fill(0);

        deal_hand();
    }

    bool Game::play_card(std::size_t hand_index)
    {
        if (state_ == GameState::NotStarted)
        {
            throw std::logic_error("Game has not started");
        }

        if (state_ == GameState::Finished)
        {
            throw std::logic_error("Game is already over");
        }

        const auto& hand = players_[current_player_].hand();

        if (hand_index >= hand.size())
        {
            throw std::out_of_range("Invalid card index");
        }

        const Card selected_card = hand[hand_index];

        // Players must follow the lead suit if possivle
        if (trick_.card_count() > 0)
        {
            const Suit lead = trick_.lead_suit();

            if (players_[current_player_].has_suit(lead)
                && selected_card.suit() != lead)
            {
                throw std::invalid_argument(
                    "You must follow the lead suit"
                    );
            }
        }

        const Card played_card =
            players_[current_player_].play_card(hand_index);

        trick_.play_card(current_player_, played_card);

        if (trick_.is_complete())
        {
            last_trick_winner_ = trick_.winning_player();
            last_trick_points_ = trick_.points();

            // Preserve the completed trick
            trick_history_.push_back(trick_);

            // Players 0 and 2 form team 0
            // Players 1 and 3 form team 1
            const std::size_t winning_team = last_trick_winner_ % 2;

            team_points_[winning_team] += last_trick_points_;
            team_tricks_[winning_team]++;

            match_points_[winning_team] += last_trick_points_;
            match_tricks_[winning_team]++;

            ++tricks_played_;

            // Check whether the game has finished
            if (tricks_played_ == CARDS_PER_PLAYER)
            {
                finish_hand();

                const int highest_score = std::max(
                    team_riscos_[0],
                    team_riscos_[1]
                );

                state_ = highest_score >= match_target_
                    ? GameState::Finished
                    : GameState::HandFinished;
            }

            // The winner leads the next trick
            current_player_ = last_trick_winner_;

            trick_ = Trick(trump_suit_);

            return true;
        }

        // Otherwise, the next player takes their turn
        current_player_ = (current_player_ + 1) % PLAYER_COUNT;

        return false;
    }

    const Player& Game::player(PlayerId index) const
    {
        if (index >= PLAYER_COUNT)
        {
            throw std::out_of_range("Invalid player index");
        }

        return players_[index];
    }

    PlayerId Game::current_player_id() const noexcept
    {
        return current_player_;
    }

    Suit Game::trump_suit() const noexcept
    {
        return trump_suit_;
    }

    void Game::sort_current_player_hand()
    {
        if (state_ != GameState::InProgress)
        {
            throw std::logic_error(
                "Cannot sort a hand when the game is not in progress"
            );
        }

        players_[current_player_].sort_hand();
    }

    const Trick& Game::current_trick() const noexcept
    {
        return trick_;
    }

    const std::vector<Trick>& Game::trick_history() const noexcept
    {
        return trick_history_;
    }

    const Trick& Game::last_trick() const
    {
        if (trick_history_.empty())
        {
            throw std::logic_error("No completed tricks");
        }

        return trick_history_.back();
    }

    PlayerId Game::dealer_id() const noexcept
    {
        return dealer_id_;
    }

    const Card& Game::trump_card() const
    {
        if (!trump_card_)
        {
            throw std::logic_error("Trump card has not been dealt");
        }

        return *trump_card_;
    }

    void Game::start_next_hand()
    {
        if (state_ != GameState::HandFinished)
        {
            throw std::logic_error(
                "Cannot start another hand"
            );
        }

        dealer_id_ = (dealer_id_ + 1) % PLAYER_COUNT;

        deal_hand();
    }

    const Deck& Game::deck() const noexcept
    {
        return deck_;
    }

    int Game::team_points(std::size_t team) const
    {
        if (team >= team_points_.size())
        {
            throw std::out_of_range("Invalid team index");
        }

        return team_points_[team];
    }

    PlayerId Game::last_trick_winner() const noexcept
    {
        return last_trick_winner_;
    }

    int Game::last_trick_points() const noexcept
    {
        return last_trick_points_;
    }

    std::size_t Game::tricks_played() const noexcept
    {
        return tricks_played_;
    }

    bool Game::is_hand_over() const noexcept
    {
        return state_ == GameState::HandFinished ||
               state_ == GameState::Finished;
    }

    bool Game::is_over() const noexcept
    {
        return state_ == GameState::Finished;
    }

    int Game::match_target() const noexcept
    {
        return match_target_;
    }

    GameState Game::state() const noexcept
    {
        return state_;
    }

    int Game::team_tricks(std::size_t team) const
    {
        if (team >= 2) {
            throw std::out_of_range("Invalid team ID");
        }

        return team_tricks_[team];
    }

    int Game::team_riscos(std::size_t team) const
    {
        if (team >= 2) {
            throw std::out_of_range("Invalid team ID");
        }

        return team_riscos_[team];
    }

    void Game::finish_hand()
    {
        for (std::size_t team = 0; team < 2; ++team) {
            const std::size_t opposing_team = 1 - team;

            team_riscos_[team] += calculate_riscos(
                team_points_[team],
                team_points_[opposing_team],
                team_tricks_[team]
            );
        }
    }

    void Game::deal_hand()
    {
        deck_.reset();
        deck_.shuffle();

        for (Player& player : players_) {
            player.clear_hand();
        }

        team_points_.fill(0);
        team_tricks_.fill(0);

        trick_history_.clear();

        tricks_played_ = 0;
        last_trick_winner_ = 0;
        last_trick_points_ = 0;

        trump_card_.reset();

        for (std::size_t round = 0; round < CARDS_PER_PLAYER; ++round) {
            for (std::size_t offset = 1; offset <= PLAYER_COUNT; ++offset) {
                const PlayerId player_id =
                    (dealer_id_ + offset) % PLAYER_COUNT;

                players_[player_id].add_card(deck_.deal());
            }
        }

        trump_card_ = players_[dealer_id_].hand().back();
        trump_suit_ = trump_card_->suit();

        trick_ = Trick(trump_suit_);

        current_player_ = (dealer_id_ + 1) % PLAYER_COUNT;

        state_ = GameState::InProgress;
    }

    MatchSummary Game::match_summary() const noexcept
    {
        MatchSummary summary;

        summary.riscos = team_riscos_;
        summary.card_points = match_points_;
        summary.tricks = match_tricks_;

        if (state_ == GameState::Finished) {
            summary.winning_team =
                team_riscos_[0] > team_riscos_[1] ? 0 : 1;
        }

        return summary;
    }

    std::vector<std::size_t> Game::legal_card_indices() const
    {
        if (state_ != GameState::InProgress)
        {
            throw std::logic_error("Game is not in progress");
        }

        const Player& current = player(current_player_);

        std::vector<std::size_t> legal;

        if (current_trick().card_count() == 0)
        {
            for (std::size_t i = 0; i < current.card_count(); ++i)
            {
                legal.push_back(i);
            }

            return legal;
        }

        const Suit lead = current_trick().lead_suit();

        for (std::size_t i = 0; i < current.card_count(); ++i)
        {
            if (!current.has_suit(lead) ||
                current.hand()[i].suit() == lead)
            {
                legal.push_back(i);
            }
        }

        return legal;
    }

    PlayerDecisionContext Game::decision_context() const
    {
        const Player& current_player = player(current_player_id());

        const std::size_t our_team = current_player_id() % 2;
        const std::size_t opposing_team = 1 - our_team;

        return {
            current_player_id(),
            current_player.hand(),
            legal_card_indices(),
            trump_suit_,
            trick_,
            trick_history_,
            team_points(our_team),
            team_points(opposing_team)
        };
    }
}