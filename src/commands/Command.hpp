#pragma once

#include <map>
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

struct ToolListCommand {};

struct ToolRunCommand {
    std::string toolName;
    std::map<std::string, std::string> arguments;
};

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
    ToolListCommand,
    ToolRunCommand,
    UnknownCommand
>;

} // namespace corvus::commands