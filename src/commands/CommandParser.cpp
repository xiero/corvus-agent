#include "commands/CommandParser.hpp"

#include <cctype>

namespace corvus::commands {
namespace {

[[nodiscard]] bool isWhitespace(char character) {
    return std::isspace(static_cast<unsigned char>(character)) != 0;
}

[[nodiscard]] std::string_view trim(std::string_view value) {
    while (!value.empty() && isWhitespace(value.front())) {
        value.remove_prefix(1);
    }

    while (!value.empty() && isWhitespace(value.back())) {
        value.remove_suffix(1);
    }

    return value;
}

} // namespace

std::optional<Command> parseCommand(std::string_view input) {
    const std::string_view normalized = trim(input);

    if (normalized.empty()) {
        return std::nullopt;
    }

    if (normalized == ":help") {
        return HelpCommand{};
    }

    if (normalized == ":exit") {
        return ExitCommand{};
    }

    if (normalized == ":version") {
        return VersionCommand{};
    }

    if (normalized == ":config") {
        return ConfigCommand{};
    }

    if (normalized == ":state") {
        return StateCommand{};
    }

    if (normalized == ":plan") {
        return PlanCommand{};
    }

    if (normalized == ":tool") {
        return ToolCommand{};
    }

    if (normalized == ":validate") {
        return ValidateCommand{};
    }

    if (normalized == ":finish") {
        return FinishCommand{};
    }

    if (normalized == ":reset") {
        return ResetCommand{};
    }

    if (normalized == ":fail") {
        return FailCommand{};
    }

    return UnknownCommand{std::string{normalized}};
}

} // namespace corvus::commands