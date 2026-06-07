#include "commands/Command.hpp"
#include "commands/CommandParser.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Test failed: " << message << "\n";
        std::exit(1);
    }
}

template <typename ExpectedCommand>
void requireParsesAs(std::string_view input, const std::string& message) {
    const std::optional<corvus::commands::Command> command =
        corvus::commands::parseCommand(input);

    require(command.has_value(), message + " should produce a command");
    require(
        std::holds_alternative<ExpectedCommand>(command.value()),
        message + " should produce expected command type"
    );
}

void testEmptyInputProducesNoCommand() {
    const std::optional<corvus::commands::Command> command =
        corvus::commands::parseCommand("");

    require(!command.has_value(), "empty input should produce no command");
}

void testWhitespaceInputProducesNoCommand() {
    const std::optional<corvus::commands::Command> command =
        corvus::commands::parseCommand("   \t  ");

    require(!command.has_value(), "whitespace input should produce no command");
}

void testKnownCommands() {
    using namespace corvus::commands;

    requireParsesAs<HelpCommand>(":help", ":help");
    requireParsesAs<VersionCommand>("  :version  ", ":version");
    requireParsesAs<ConfigCommand>(":config", ":config");
    requireParsesAs<StateCommand>(":state", ":state");
    requireParsesAs<ExitCommand>(":exit", ":exit");

    requireParsesAs<PlanCommand>(":plan", ":plan");
    requireParsesAs<ToolCommand>(":tool", ":tool");
    requireParsesAs<ValidateCommand>(":validate", ":validate");
    requireParsesAs<FinishCommand>(":finish", ":finish");
    requireParsesAs<ResetCommand>(":reset", ":reset");
    requireParsesAs<FailCommand>(":fail", ":fail");
}

void testToolListCommand() {
    requireParsesAs<corvus::commands::ToolListCommand>(
        ":tool list",
        ":tool list"
    );
}

void testToolRunCommand() {
    const std::optional<corvus::commands::Command> command =
        corvus::commands::parseCommand(
            ":tool run echo message=hello mode=test"
        );

    require(command.has_value(), ":tool run should produce a command");
    require(
        std::holds_alternative<corvus::commands::ToolRunCommand>(
            command.value()
        ),
        ":tool run should produce ToolRunCommand"
    );

    const auto& toolRun =
        std::get<corvus::commands::ToolRunCommand>(command.value());

    require(toolRun.toolName == "echo", "tool name should be parsed");
    require(
        toolRun.arguments.count("message") == 1,
        "message argument should exist"
    );
    require(
        toolRun.arguments.at("message") == "hello",
        "message argument should be parsed"
    );
    require(
        toolRun.arguments.count("mode") == 1,
        "mode argument should exist"
    );
    require(
        toolRun.arguments.at("mode") == "test",
        "mode argument should be parsed"
    );
}

void testUnknownCommand() {
    const std::optional<corvus::commands::Command> command =
        corvus::commands::parseCommand(":dance");

    require(command.has_value(), "unknown input should still produce a command");
    require(
        std::holds_alternative<corvus::commands::UnknownCommand>(
            command.value()
        ),
        "unknown input should produce UnknownCommand"
    );

    const auto& unknown =
        std::get<corvus::commands::UnknownCommand>(command.value());

    require(unknown.input == ":dance", "unknown command should preserve input");
}

} // namespace

int main() {
    testEmptyInputProducesNoCommand();
    testWhitespaceInputProducesNoCommand();
    testKnownCommands();
    testToolListCommand();
    testToolRunCommand();
    testUnknownCommand();

    std::cout << "CommandParserTests passed\n";
    return 0;
}