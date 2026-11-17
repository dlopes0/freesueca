#pragma once

#include "freesueca/card.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>

namespace freesueca
{
    class Deck
    {
        public:
            static constexpr std::size_t SIZE=40;

            Deck();
            explicit Deck(std::uint32_t seed);

            void reset();
            void shuffle();

            [[nodiscard]] Card deal();

            [[nodiscard]] std::size_t remaining() const noexcept;

        private:
            std::array<Card,SIZE> cards_;

            std::size_t next_card_ = 0;

            std::mt19937 rng_;
    };
}