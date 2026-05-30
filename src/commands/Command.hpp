#pragma once

#include <string>
#include <variant>

namespace corvus::commands {

struct HelpCommand {};
struct ExitCommand {};
struct VersionCommand {};
struct ConfigCommand {};
struct StateCommand {};

struct PlanCommand {};
struct ToolCommand {};
struct ValidateCommand {};
struct FinishCommand {};
struct ResetCommand {};
struct FailCommand {};

struct UnknownCommand {
    std::string input;
};

using Command = std::variant<
    HelpCommand,
    ExitCommand,
    VersionCommand,
    ConfigCommand,
    StateCommand,
    PlanCommand,
    ToolCommand,
    ValidateCommand,
    FinishCommand,
    ResetCommand,
    FailCommand,
    UnknownCommand
>;

} // namespace corvus::commands