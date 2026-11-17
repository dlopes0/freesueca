#pragma once

#include "freesueca/card.hpp"
#include "freesueca/types.hpp"

#include <cstddef>
#include <vector>

namespace freesueca
{
    struct PlayedCard
    {
        PlayerId player_id;
        Card card;

        bool operator==(const PlayedCard&) const = default;
    };

    class Trick
    {
    public:
        static constexpr std::size_t PLAYER_COUNT = 4;

        explicit Trick(Suit trump_suit);

        void play_card(PlayerId player_id, Card card);

        [[nodiscard]] bool is_complete() const noexcept;
        [[nodiscard]] std::size_t card_count() const noexcept;

        [[nodiscard]] PlayerId winning_player() const;
        [[nodiscard]] int points() const noexcept;

        [[nodiscard]] Suit lead_suit() const;
        [[nodiscard]] Suit trump_suit() const noexcept;

        [[nodiscard]] const std::vector<PlayedCard>& played_cards() const noexcept;

    private:
        Suit trump_suit_;
        std::vector<PlayedCard> played_cards_;
    };
}