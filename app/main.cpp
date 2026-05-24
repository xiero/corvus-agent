#include "commands/Command.hpp"
#include "commands/CommandParser.hpp"
#include "core/AgentConfig.hpp"
#include "tui/TuiApp.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

void printHelp(const corvus::core::AgentConfig& config) {
    std::cout << config.name() << "\n\n"
              << "Usage:\n"
              << "  corvus --help\n"
              << "  corvus --version\n"
              << "  corvus --shell\n"
              << "  corvus --tui\n\n"
              << "Options:\n"
              << "  --help       Show this help message\n"
              << "  --version    Show version information\n"
              << "  --shell      Start the interactive Corvus shell\n"
              << "  --tui        Start the interactive Corvus TUI cockpit\n\n"
              << "Shell / TUI commands:\n"
              << "  :help        Show commands\n"
              << "  :version     Show version information\n"
              << "  :config      Show runtime configuration\n"
              << "  :exit        Exit the shell or TUI\n";
}

void printVersion(const corvus::core::AgentConfig& config) {
    std::cout << config.name() << " " << config.version() << "\n";
}

void printShellHelp() {
    std::cout << "Available commands:\n"
              << "  :help\n"
              << "  :version\n"
              << "  :config\n"
              << "  :exit\n";
}

void printConfig(const corvus::core::AgentConfig& config) {
    std::cout << "Config:\n"
              << "  name: " << config.name() << "\n"
              << "  version: " << config.version() << "\n"
              << "  deterministicMode: "
              << (config.deterministicMode() ? "true" : "false") << "\n"
              << "  mentorMode: "
              << (config.mentorMode() ? "true" : "false") << "\n";
}

bool handleShellCommand(
    const corvus::commands::Command& command,
    const corvus::core::AgentConfig& config
) {
    using namespace corvus::commands;

    if (std::holds_alternative<HelpCommand>(command)) {
        printShellHelp();
        return true;
    }

    if (std::holds_alternative<VersionCommand>(command)) {
        printVersion(config);
        return true;
    }

    if (std::holds_alternative<ConfigCommand>(command)) {
        printConfig(config);
        return true;
    }

    if (std::holds_alternative<ExitCommand>(command)) {
        std::cout << "Goodbye from Corvus.\n";
        return false;
    }

    const auto& unknown = std::get<UnknownCommand>(command);
    std::cout << "Unknown command: " << unknown.input << "\n"
              << "Type :help for available commands.\n";
    return true;
}

void runShell(const corvus::core::AgentConfig& config) {
    std::cout << "Corvus shell started.\n"
              << "Type :help for commands, :exit to quit.\n";

    std::string line;
    bool shouldContinue = true;

    while (shouldContinue) {
        std::cout << "> ";

        if (!std::getline(std::cin, line)) {
            std::cout << "\nInput closed. Goodbye from Corvus.\n";
            break;
        }

        const std::optional<corvus::commands::Command> parsedCommand =
            corvus::commands::parseCommand(line);

        if (!parsedCommand.has_value()) {
            continue;
        }

        shouldContinue = handleShellCommand(parsedCommand.value(), config);
    }
}

} // namespace

int main(int argc, char** argv) {
    const corvus::core::AgentConfig config;

    if (argc == 1) {
        printHelp(config);
        return 0;
    }

    const std::string_view argument{argv[1]};

    if (argument == "--help" || argument == "-h") {
        printHelp(config);
        return 0;
    }

    if (argument == "--version" || argument == "-v") {
        printVersion(config);
        return 0;
    }

    if (argument == "--shell") {
        runShell(config);
        return 0;
    }

    if (argument == "--tui") {
        corvus::tui::TuiApp app{config};
        app.run();
        return 0;
    }

    std::cerr << "Unknown argument: " << argument << "\n"
              << "Run `corvus --help` for usage.\n";
    return 1;
}