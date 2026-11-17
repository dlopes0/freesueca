#pragma once

#include <cstddef>

namespace freesueca::interface
{
    enum class PlayerActionType
    {
        PlayCard,
        SortHand,
        ShowHand,
        ShowScore,
        ShowRules,
        Help,
        Quit,
        Invalid
    };

    struct PlayerAction
    {
        PlayerActionType type = PlayerActionType::Invalid;
        std::size_t card_index = 0;
    };
}