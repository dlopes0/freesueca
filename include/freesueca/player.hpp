#pragma once

#include "freesueca/card.hpp"
#include "freesueca/types.hpp"

#include <cstddef>
#include <vector>

namespace freesueca
{
    class Player
    {
        public:
            explicit Player(PlayerId id);

            [[nodiscard]] PlayerId id() const noexcept;

            void add_card(Card card);
            [[nodiscard]] Card play_card(std::size_t index);
            void sort_hand();

            [[nodiscard]] bool has_suit(Suit suit) const noexcept;
            [[nodiscard]] bool has_card(const Card& card) const noexcept;

            [[nodiscard]] std::size_t card_count() const noexcept;

            [[nodiscard]] const std::vector<Card>& hand() const noexcept;

            void clear_hand() noexcept;

        private:
            PlayerId id_;
            std::vector<Card> hand_;
    };
}