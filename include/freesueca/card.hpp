#pragma once

#include <string>

namespace freesueca
{
    enum class Suit
    {
        Hearts,
        Diamonds,
        Clubs,
        Spades
    };

    enum class Rank {
        Two,
        Three,
        Four,
        Five,
        Six,
        Queen,
        Jack,
        King,
        Seven,
        Ace
    };

    class Card
    {
    public:
        Card(Suit suit, Rank rank);

        [[nodiscard]] Suit suit() const noexcept;
        [[nodiscard]] Rank rank() const noexcept;

        [[nodiscard]] int points() const noexcept;
        [[nodiscard]] std::string name() const;

        bool operator==(const Card& other) const = default;

    private:
        Suit suit_;
        Rank rank_;
    };
}