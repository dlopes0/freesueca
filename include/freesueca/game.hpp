#pragma once

#include "freesueca/deck.hpp"
#include "freesueca/player.hpp"
#include "freesueca/trick.hpp"
#include "freesueca/player_decision_context.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>

namespace freesueca
{
    enum class GameState {
        NotStarted,
        InProgress,
        HandFinished,
        Finished
    };

    struct MatchSummary {
        std::array<int, 2> riscos{};
        std::array<int, 2> card_points{};
        std::array<int, 2> tricks{};
        std::optional<std::size_t> winning_team;
    };

    class Game
    {
    public:
        static constexpr std::size_t PLAYER_COUNT = 4;
        static constexpr std::size_t CARDS_PER_PLAYER = 10;
        static constexpr int DEFAULT_MATCH_TARGET = 10;

        Game();
        explicit Game(std::uint32_t seed);
        Game(std::uint32_t seed, int match_target);

        void start();

        // Plays a card from the current player's hand
        // Returns true if this play completes the trick
        bool play_card(std::size_t hand_index);

        [[nodiscard]] const Player& player(PlayerId index) const;

        [[nodiscard]] PlayerId current_player_id() const noexcept;
        [[nodiscard]] Suit trump_suit() const noexcept;

        [[nodiscard]] PlayerId dealer_id() const noexcept;
        [[nodiscard]] const Card& trump_card() const;
        void start_next_hand();
        void sort_current_player_hand();

        [[nodiscard]] const Trick& current_trick() const noexcept;
        [[nodiscard]] const std::vector<Trick>& trick_history() const noexcept;
        [[nodiscard]] const Trick& last_trick() const;
        [[nodiscard]] const Deck& deck() const noexcept;

        [[nodiscard]] int team_points(std::size_t team) const;
        [[nodiscard]] PlayerId last_trick_winner() const noexcept;
        [[nodiscard]] int last_trick_points() const noexcept;

        [[nodiscard]] std::size_t tricks_played() const noexcept;
        [[nodiscard]] bool is_hand_over() const noexcept;
        [[nodiscard]] bool is_over() const noexcept;
        [[nodiscard]] int match_target() const noexcept;

        [[nodiscard]] GameState state() const noexcept;

        [[nodiscard]] int team_tricks(std::size_t team) const;
        [[nodiscard]] int team_riscos(std::size_t team) const;

        [[nodiscard]] MatchSummary match_summary() const noexcept;

        [[nodiscard]] std::vector<std::size_t> legal_card_indices() const;

        [[nodiscard]] PlayerDecisionContext decision_context() const;

    private:
        Deck deck_;

        std::array<Player, PLAYER_COUNT> players_ {
            Player(0),
            Player(1),
            Player(2),
            Player(3)
        };

        Suit trump_suit_ = Suit::Hearts;
        Trick trick_{Suit::Hearts};
        std::vector<Trick> trick_history_;

        PlayerId dealer_id_ = 0;
        std::optional<Card> trump_card_;

        std::array<int, 2> team_points_{};

        PlayerId current_player_ = 0;
        std::size_t tricks_played_ = 0;

        PlayerId last_trick_winner_ = 0;
        int last_trick_points_ = 0;

        GameState state_ = GameState::NotStarted;

        std::array<int, 2> team_tricks_{};
        std::array<int, 2> team_riscos_{};

        std::array<int, 2> match_points_{};
        std::array<int, 2> match_tricks_{};

        void deal_hand();
        void finish_hand();

        int match_target_ = DEFAULT_MATCH_TARGET;
    };
}