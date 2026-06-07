#include "commands/CommandParser.hpp"

#include <cctype>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace corvus::commands {
namespace {

[[nodiscard]] bool isWhitespace(char character) {
    return std::isspace(static_cast<unsigned char>(character)) != 0;
}

[[nodiscard]] bool startsWith(
    std::string_view value,
    std::string_view prefix
) {
    return value.size() >= prefix.size()
        && value.substr(0, prefix.size()) == prefix;
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

[[nodiscard]] std::vector<std::string_view> splitWords(std::string_view value) {
    std::vector<std::string_view> words;

    value = trim(value);

    while (!value.empty()) {
        std::size_t end = 0;

        while (end < value.size() && !isWhitespace(value[end])) {
            ++end;
        }

        words.push_back(value.substr(0, end));
        value.remove_prefix(end);
        value = trim(value);
    }

    return words;
}

[[nodiscard]] std::map<std::string, std::string> parseKeyValueArguments(
    const std::vector<std::string_view>& words,
    std::size_t startIndex
) {
    std::map<std::string, std::string> arguments;

    for (std::size_t index = startIndex; index < words.size(); ++index) {
        const std::string_view word = words[index];
        const std::size_t separator = word.find('=');

        if (separator == std::string_view::npos || separator == 0) {
            continue;
        }

        const std::string key{word.substr(0, separator)};
        const std::string value{word.substr(separator + 1)};

        arguments[key] = value;
    }

    return arguments;
}

[[nodiscard]] Command parseToolCommand(std::string_view normalized) {
    const std::vector<std::string_view> words = splitWords(normalized);

    if (words.size() == 1) {
        return ToolCommand{};
    }

    if (words.size() == 2 && words[1] == "list") {
        return ToolListCommand{};
    }

    if (words.size() >= 3 && words[1] == "run") {
        return ToolRunCommand{
            .toolName = std::string{words[2]},
            .arguments = parseKeyValueArguments(words, 3),
        };
    }

    return UnknownCommand{std::string{normalized}};
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

    if (normalized == ":tool" || startsWith(normalized, ":tool ")) {
        return parseToolCommand(normalized);
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