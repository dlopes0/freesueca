#include "freesueca/interface/terminal_input.hpp"

#include <charconv>
#include <string>
#include <system_error>
#include <iostream>

namespace freesueca::interface
{
    PlayerAction read_player_action(
        std::istream& input,
        std::ostream& output,
        const std::size_t card_count
    )
    {
        output << "\nSelection (1-" << card_count
               << ", o=sort, h=hand, s=score, "
               << "r=rules, ?=help, q=quit): ";

        std::string command;

        if (!(input >> command))
        {
            return {PlayerActionType::Quit, 0};
        }

        if (command == "o" || command == "O" || command == "sort")
            return {PlayerActionType::SortHand, 0};

        if (command == "h" || command == "H" || command == "hand")
            return {PlayerActionType::ShowHand, 0};

        if (command == "s" || command == "S" || command == "score")
            return {PlayerActionType::ShowScore, 0};

        if (command == "r" || command == "R" || command == "rules")
            return {PlayerActionType::ShowRules, 0};

        if (command == "?")
            return {PlayerActionType::Help, 0};

        if (command == "q" || command == "Q" || command == "quit")
            return {PlayerActionType::Quit, 0};

        std::size_t selection{};

        const auto [end, error] = std::from_chars(
            command.data(),
            command.data() + command.size(),
            selection
        );

        if (error == std::errc{}
            && end == command.data() + command.size()
            && selection >= 1
            && selection <= card_count)
        {
            return {
                PlayerActionType::PlayCard,
                selection - 1
            };
        }

        return {PlayerActionType::Invalid, 0};
    }
}