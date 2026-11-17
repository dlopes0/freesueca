
#pragma once

#include "freesueca/card.hpp"
#include "freesueca/trick.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace freesueca
{
    class CardTracker
    {
    public:
        CardTracker() = default;

        void record(const Card& card) noexcept;
        void record(const Trick& trick) noexcept;
        void record(const std::vector<Trick>& tricks) noexcept;

        [[nodiscard]]
        bool has_been_played(const Card& card) const noexcept;

        [[nodiscard]]
        std::size_t cards_played() const noexcept;

        [[nodiscard]]
        std::vector<Card> unplayed_cards() const;

        [[nodiscard]]
        std::vector<Card> unknown_cards(
            const std::vector<Card>& own_hand
        ) const;

    private:
        [[nodiscard]]
        static std::size_t card_index(const Card& card) noexcept;

        std::array<bool, 40> played_{};
        std::size_t cards_played_ = 0;
    };
}
