#include "cli/CliApp.hpp"

#include "commands/CommandExecutor.hpp"
#include "commands/CommandParser.hpp"
#include "core/AgentConfig.hpp"
#include "core/AgentRuntime.hpp"
#include "tui/TuiApp.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace corvus::cli {
namespace {

struct CliOptions {
    corvus::core::AgentConfig config;
    std::optional<std::string> mode;
};

void printHelp(const corvus::core::AgentConfig& config) {
    std::cout << config.name() << "\n\n"
              << "Usage:\n"
              << "  corvus --help\n"
              << "  corvus --version\n"
              << "  corvus --shell\n"
              << "  corvus --tui\n"
              << "  corvus --deterministic --shell\n"
              << "  corvus --deterministic --tui\n\n"
              << "Options:\n"
              << "  --help             Show this help message\n"
              << "  --version          Show version information\n"
              << "  --shell            Start the interactive Corvus shell\n"
              << "  --tui              Start the interactive Corvus TUI cockpit\n"
              << "  --deterministic    Enable audit logging for the session\n\n"
              << "Shell / TUI commands:\n"
              << "  :help                              Show commands\n"
              << "  :version                           Show version information\n"
              << "  :config                            Show runtime configuration\n"
              << "  :state                             Show current agent state\n"
              << "  :plan                              Move Idle -> Planning\n"
              << "  :tool                              Move Planning -> ToolExecution\n"
              << "  :validate                          Move ToolExecution -> Validation\n"
              << "  :finish                            Move Validation -> FinalResponse\n"
              << "  :reset                             Move FinalResponse/Error -> Idle\n"
              << "  :fail                              Move current state -> Error\n"
              << "  :tool list                         List registered tools\n"
              << "  :tool run echo message=hello       Run echo tool\n"
              << "  :tool run list_files path=.        List directory entries\n"
              << "  :tool run read_file path=README.md Read a text file\n"
              << "  :exit                              Exit the shell or TUI\n";
}

void printVersion(const corvus::core::AgentConfig& config) {
    std::cout << config.name() << " " << config.version() << "\n";
}

void printLines(const std::vector<std::string>& lines) {
    for (const std::string& line : lines) {
        std::cout << line << "\n";
    }
}

[[nodiscard]] CliOptions parseCliOptions(int argc, char** argv) {
    CliOptions options;

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument{argv[index]};

        if (argument == "--deterministic") {
            options.config.setDeterministicMode(true);
            continue;
        }

        if (!options.mode.has_value()) {
            options.mode = std::string{argument};
            continue;
        }

        options.mode = "__unknown__";
        return options;
    }

    return options;
}

void runShell(corvus::core::AgentRuntime runtime) {
    const corvus::commands::CommandExecutor executor{runtime.config()};
    executor.recordSessionStarted("shell");

    std::cout << "Corvus shell started.\n"
              << "Type :help for commands, :exit to quit.\n";

    if (runtime.config().deterministicMode()) {
        std::cout << "Deterministic mode: ON\n"
                  << "Audit log: .corvus/audit/latest.jsonl\n";
    }

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

        const corvus::commands::CommandExecutionResult result =
            executor.execute(parsedCommand.value(), runtime);

        printLines(result.lines);

        shouldContinue = !result.shouldExit;
    }

    executor.recordSessionEnded("shell");
}

} // namespace

int CliApp::run(int argc, char** argv) const {
    const CliOptions options = parseCliOptions(argc, argv);
    const corvus::core::AgentConfig& config = options.config;

    if (!options.mode.has_value()) {
        printHelp(config);
        return 0;
    }

    const std::string& mode = options.mode.value();

    if (mode == "--help" || mode == "-h") {
        printHelp(config);
        return 0;
    }

    if (mode == "--version" || mode == "-v") {
        printVersion(config);
        return 0;
    }

    if (mode == "--shell") {
        runShell(corvus::core::AgentRuntime{config});
        return 0;
    }

    if (mode == "--tui") {
        corvus::tui::TuiApp app{config};
        app.run();
        return 0;
    }

    std::cerr << "Unknown argument: " << mode << "\n"
              << "Run `corvus --help` for usage.\n";
    return 1;
}

} // namespace corvus::cli