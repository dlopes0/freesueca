#include "freesueca/hidden_hand_sampler.hpp"

#include "freesueca/card_tracker.hpp"
#include "freesueca/void_tracker.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace freesueca
{
    namespace
    {
        constexpr std::size_t PLAYER_COUNT = 4;
        constexpr std::size_t SUIT_COUNT = 4;
        constexpr int CARDS_PER_PLAYER = 10;
        constexpr std::size_t OPPONENT_COUNT = 3;
        constexpr int MAX_CAPACITY = CARDS_PER_PLAYER;

        using Capacities = std::array<int, OPPONENT_COUNT>;
        using SuitCounts = std::array<int, SUIT_COUNT>;
        using VoidMatrix = std::array<
            std::array<bool, OPPONENT_COUNT>, SUIT_COUNT>;

        constexpr std::array<Suit, SUIT_COUNT> SUITS{
            Suit::Hearts,
            Suit::Diamonds,
            Suit::Clubs,
            Suit::Spades
        };

        std::size_t suit_index(Suit suit)
        {
            switch (suit) {
                case Suit::Hearts:   return 0;
                case Suit::Diamonds: return 1;
                case Suit::Clubs:    return 2;
                case Suit::Spades:   return 3;
            }

            throw std::invalid_argument("Invalid suit");
        }

        std::uint64_t binomial(int n, int k)
        {
            if (k < 0 || k > n)
                return 0;

            k = std::min(k, n - k);

            std::uint64_t result = 1;

            for (int i = 1; i <= k; ++i)
                result = result * (n - k + i) / i;

            return result;
        }

        std::uint64_t multinomial(int n, int a, int b, int c)
        {
            return binomial(n, a) * binomial(n - a, b);
        }

        struct AllocationOption {
            Capacities allocated{};
            std::uint64_t weight = 0;
        };

        class AllocationCounter {
        public:
            AllocationCounter(
                const SuitCounts& suit_counts,
                const VoidMatrix& voids)
                : suit_counts_(suit_counts),
                  voids_(voids)
            {
            }

            [[nodiscard]] std::uint64_t count(
                std::size_t suit,
                const Capacities& capacities)
            {
                if (suit == SUIT_COUNT)
                    return capacities == Capacities{} ? 1 : 0;

                const int a = capacities[0];
                const int b = capacities[1];
                const int c = capacities[2];

                if (a < 0 || a > MAX_CAPACITY ||
                    b < 0 || b > MAX_CAPACITY ||
                    c < 0 || c > MAX_CAPACITY)
                {
                    return 0;
                }

                if (computed_[suit][a][b][c])
                    return memo_[suit][a][b][c];

                computed_[suit][a][b][c] = true;

                std::uint64_t total = 0;

                for (const AllocationOption& option :
                     options(suit, capacities))
                {
                    total += option.weight;
                }

                memo_[suit][a][b][c] = total;

                return total;
            }

            [[nodiscard]] std::vector<AllocationOption> options(
                std::size_t suit,
                const Capacities& capacities)
            {
                std::vector<AllocationOption> result;

                if (suit >= SUIT_COUNT)
                    return result;

                const int cards = suit_counts_[suit];

                for (int a = 0; a <= std::min(cards, capacities[0]); ++a)
                {
                    if (a > 0 && voids_[suit][0])
                        continue;

                    for (int b = 0;
                         b <= std::min(cards - a, capacities[1]);
                         ++b)
                    {
                        if (b > 0 && voids_[suit][1])
                            continue;

                        const int c = cards - a - b;

                        if (c > capacities[2])
                            continue;

                        if (c > 0 && voids_[suit][2])
                            continue;

                        Capacities remaining{
                            capacities[0] - a,
                            capacities[1] - b,
                            capacities[2] - c
                        };

                        const std::uint64_t future_ways =
                            count(suit + 1, remaining);

                        if (future_ways == 0)
                            continue;

                        const std::uint64_t current_ways =
                            multinomial(cards, a, b, c);

                        result.push_back({
                            {a, b, c},
                            current_ways * future_ways
                        });
                    }
                }

                return result;
            }

        private:
            SuitCounts suit_counts_;
            VoidMatrix voids_;

            // At most 10 cards can be assigned to each opponent.
            using Memo = std::array<
                std::array<
                    std::array<
                        std::array<std::uint64_t, 11>, 11>, 11>, 4>;

            using Computed = std::array<
                std::array<
                    std::array<
                        std::array<bool, 11>, 11>, 11>, 4>;

            Memo memo_{};
            Computed computed_{};
        };

    }

    HiddenHandSampler::HiddenHandSampler()
        : rng_(std::random_device{}())
    {
    }

    HiddenHandSampler::HiddenHandSampler(std::uint32_t seed)
        : rng_(seed)
    {
    }

    SampledHands HiddenHandSampler::sample(
        const PlayerDecisionContext& context)
    {
        if (context.player_id >= PLAYER_COUNT)
            throw std::out_of_range("Invalid player ID");

        // Count the cards already played by each player.
        std::array<int, PLAYER_COUNT> played_counts{};

        const auto count_played_cards =
            [&](const Trick& trick)
        {
            for (const PlayedCard& played : trick.played_cards())
            {
                if (played.player_id >= PLAYER_COUNT)
                    throw std::logic_error("Invalid player in trick");

                ++played_counts[played.player_id];
            }
        };

        for (const Trick& trick : context.trick_history)
            count_played_cards(trick);

        count_played_cards(context.current_trick);

        for (const int count : played_counts)
        {
            if (count > CARDS_PER_PLAYER)
                throw std::logic_error("Invalid played-card count");
        }

        // Identify the three opponents and their remaining capacities.
        std::array<PlayerId, OPPONENT_COUNT> opponents{};
        Capacities capacities{};

        std::size_t opponent_index = 0;

        for (PlayerId player = 0; player < PLAYER_COUNT; ++player)
        {
            if (player == context.player_id)
                continue;

            opponents[opponent_index] = player;

            capacities[opponent_index] =
                CARDS_PER_PLAYER - played_counts[player];

            ++opponent_index;
        }

        if (context.hand.size() !=
            static_cast<std::size_t>(
                CARDS_PER_PLAYER - played_counts[context.player_id]))
        {
            throw std::logic_error(
                "Own hand size is inconsistent with played cards");
        }

        // Reconstruct the cards that are still unknown.
        CardTracker card_tracker;

        card_tracker.record(context.trick_history);
        card_tracker.record(context.current_trick);

        const std::vector<Card> unknown_cards =
            card_tracker.unknown_cards(context.hand);

        const int required_cards =
            capacities[0] + capacities[1] + capacities[2];

        if (unknown_cards.size() !=
            static_cast<std::size_t>(required_cards))
        {
            throw std::logic_error(
                "Unknown card count does not match opponent capacities");
        }

        // Group unknown cards by suit.
        std::array<std::vector<Card>, SUIT_COUNT> cards_by_suit;
        SuitCounts suit_counts{};

        for (const Card& card : unknown_cards)
        {
            const std::size_t index = suit_index(card.suit());

            cards_by_suit[index].push_back(card);
            ++suit_counts[index];
        }

        // Reconstruct known void information.
        VoidTracker void_tracker;

        void_tracker.record(context.trick_history);
        void_tracker.record(context.current_trick);

        VoidMatrix voids{};

        for (std::size_t suit = 0; suit < SUIT_COUNT; ++suit)
        {
            for (std::size_t opponent = 0;
                 opponent < OPPONENT_COUNT;
                 ++opponent)
            {
                voids[suit][opponent] =
                    void_tracker.is_void(
                        opponents[opponent],
                        SUITS[suit]);
            }
        }

        // Count all valid distributions.
        AllocationCounter counter(suit_counts, voids);

        const std::uint64_t total_ways =
            counter.count(0, capacities);

        if (total_ways == 0)
        {
            throw std::logic_error(
                "No valid hidden-hand distribution exists");
        }

        // Preserve our own hand.
        SampledHands result;
        result[context.player_id] = context.hand;

        // Distribute each suit according to its valid allocations.
        for (std::size_t suit = 0; suit < SUIT_COUNT; ++suit)
        {
            const auto allocations =
                counter.options(suit, capacities);

            std::uint64_t total_weight = 0;

            for (const AllocationOption& option : allocations)
                total_weight += option.weight;

            if (total_weight == 0)
                throw std::logic_error("No valid suit allocation");

            std::uniform_int_distribution<std::uint64_t> distribution(
                0, total_weight - 1);

            std::uint64_t selection = distribution(rng_);

            const AllocationOption* selected = nullptr;

            for (const AllocationOption& option : allocations)
            {
                if (selection < option.weight)
                {
                    selected = &option;
                    break;
                }

                selection -= option.weight;
            }

            if (selected == nullptr)
                throw std::logic_error("Allocation selection failed");

            auto& suit_cards = cards_by_suit[suit];

            std::shuffle(
                suit_cards.begin(),
                suit_cards.end(),
                rng_);

            std::size_t offset = 0;

            for (std::size_t opponent = 0;
                 opponent < OPPONENT_COUNT;
                 ++opponent)
            {
                const std::size_t count =
                    static_cast<std::size_t>(
                        selected->allocated[opponent]);

                result[opponents[opponent]].insert(
                    result[opponents[opponent]].end(),
                    suit_cards.begin() + offset,
                    suit_cards.begin() + offset + count);

                offset += count;

                capacities[opponent] -=
                    static_cast<int>(count);
            }
        }

        if (capacities != Capacities{})
            throw std::logic_error("Incomplete hidden-hand distribution");

        return result;
    }

}