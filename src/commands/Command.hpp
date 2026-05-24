#pragma once

#include <string>
#include <variant>

namespace corvus::commands {
    struct HelpCommand{};
    struct ExitCommand{};
    struct VersionCommand{};
    struct ConfigCommand{};

    struct UnknownCommand {
        std::string input;
    };

    using Command = std::variant <
        HelpCommand,
        ExitCommand,
        VersionCommand,
        ConfigCommand,
        UnknownCommand
    >;
} // namsespace corvus::commands