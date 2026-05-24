#pragma once

#include "commands/Command.hpp"

#include <optional>
#include <string_view>

namespace corvus::commands {
    [[nodiscard]] std::optional<Command> parseCommand(std::string_view input);
}//namesapce corvus::commands
