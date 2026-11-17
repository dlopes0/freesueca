#pragma once

#include "freesueca/trick.hpp"
#include "freesueca/types.hpp"

#include <array>
#include <vector>

namespace freesueca
{
    class VoidTracker
    {
    public:
        void record(const Trick& trick);
        void record(const std::vector<Trick>& tricks);

        [[nodiscard]] bool is_void(
            PlayerId player_id,
            Suit suit) const;

    private:
        [[nodiscard]] static std::size_t suit_index(Suit suit);

        static constexpr std::size_t PLAYER_COUNT = 4;
        static constexpr std::size_t SUIT_COUNT = 4;

        std::array<std::array<bool, SUIT_COUNT>, PLAYER_COUNT> void_suits_{};
    };
}