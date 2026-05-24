#include "commands/Command.hpp"
#include "commands/CommandParser.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

namespace {

    void require(bool condition, const std::string& message) {
        if (!condition) {
            std::cerr << "Test failed: " << message << "\n";
            std::exit(1);
        }
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

    void testHelpCommand() {
        const std::optional<corvus::commands::Command> command =
            corvus::commands::parseCommand(":help");

        require(command.has_value(), ":help should produce a command");
        require(
            std::holds_alternative<corvus::commands::HelpCommand>(command.value()),
            ":help should produce HelpCommand"
        );
    }

    void testVersionCommandWithWhitespace() {
        const std::optional<corvus::commands::Command> command =
            corvus::commands::parseCommand("  :version  ");

        require(command.has_value(), ":version should produce a command");
        require(
            std::holds_alternative<corvus::commands::VersionCommand>(command.value()),
            ":version should produce VersionCommand"
        );
    }

    void testConfigCommand() {
        const std::optional<corvus::commands::Command> command =
            corvus::commands::parseCommand(":config");

        require(command.has_value(), ":config should produce a command");
        require(
            std::holds_alternative<corvus::commands::ConfigCommand>(command.value()),
            ":config should produce ConfigCommand"
        );
    }

    void testExitCommand() {
        const std::optional<corvus::commands::Command> command =
            corvus::commands::parseCommand(":exit");

        require(command.has_value(), ":exit should produce a command");
        require(
            std::holds_alternative<corvus::commands::ExitCommand>(command.value()),
            ":exit should produce ExitCommand"
        );
    }

    void testUnknownCommand() {
        const std::optional<corvus::commands::Command> command =
            corvus::commands::parseCommand(":dance");

        require(command.has_value(), "unknown input should still produce a command");
        require(
            std::holds_alternative<corvus::commands::UnknownCommand>(command.value()),
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
    testHelpCommand();
    testVersionCommandWithWhitespace();
    testConfigCommand();
    testExitCommand();
    testUnknownCommand();

    std::cout << "CommandParserTests passed\n";
    return 0;
}