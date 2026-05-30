#include "tui/TuiApp.hpp"

#include "commands/Command.hpp"
#include "commands/CommandParser.hpp"
#include "core/AgentEvent.hpp"
#include "core/AgentState.hpp"
#include "core/StateTransition.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace corvus::tui {
namespace {

[[nodiscard]] std::string boolToStatus(bool value) {
    return value ? "ON" : "OFF";
}

void pushLog(std::vector<std::string>& logLines, std::string line) {
    constexpr std::size_t maxLogLines = 200;

    logLines.push_back(std::move(line));

    if (logLines.size() > maxLogLines) {
        logLines.erase(logLines.begin());
    }
}

ftxui::Element renderStatePanel(const corvus::core::AgentRuntime& runtime) {
    using namespace ftxui;

    const corvus::core::AgentConfig& config = runtime.config();

    return vbox({
        text("Session / State") | bold,
        separator(),
        text("State: " + std::string{corvus::core::toString(runtime.state())}),
        text("Deterministic: " + boolToStatus(config.deterministicMode())),
        text("Mentor: " + boolToStatus(config.mentorMode())),
        separator(),
        text("Core: ready"),
        text("Input: command mode"),
    }) | border | size(WIDTH, EQUAL, 32);
}

ftxui::Element renderLogPanel(const std::vector<std::string>& logLines) {
    using namespace ftxui;

    Elements visibleLines;

    constexpr std::size_t maxVisibleLines = 14;
    const std::size_t startIndex =
        logLines.size() > maxVisibleLines ? logLines.size() - maxVisibleLines : 0;

    for (std::size_t index = startIndex; index < logLines.size(); ++index) {
        visibleLines.push_back(text(logLines[index]));
    }

    if (visibleLines.empty()) {
        visibleLines.push_back(text("No log entries yet."));
    }

    return vbox({
        text("Log") | bold,
        separator(),
        vbox(std::move(visibleLines)) | flex,
    }) | border | flex;
}

void printShellHelp(std::vector<std::string>& logLines) {
    pushLog(logLines, "Available commands:");
    pushLog(logLines, "  :help");
    pushLog(logLines, "  :version");
    pushLog(logLines, "  :config");
    pushLog(logLines, "  :state");
    pushLog(logLines, "  :plan");
    pushLog(logLines, "  :tool");
    pushLog(logLines, "  :validate");
    pushLog(logLines, "  :finish");
    pushLog(logLines, "  :reset");
    pushLog(logLines, "  :fail");
    pushLog(logLines, "  :exit");
}

void printConfig(
    std::vector<std::string>& logLines,
    const corvus::core::AgentRuntime& runtime
) {
    const corvus::core::AgentConfig& config = runtime.config();

    pushLog(logLines, "Config:");
    pushLog(logLines, "  name: " + config.name());
    pushLog(logLines, "  version: " + config.version());
    pushLog(logLines, "  deterministicMode: " + boolToStatus(config.deterministicMode()));
    pushLog(logLines, "  mentorMode: " + boolToStatus(config.mentorMode()));
    pushLog(
        logLines,
        "  state: " + std::string{corvus::core::toString(runtime.state())}
    );
}

[[nodiscard]] std::optional<corvus::core::AgentEvent> eventFromCommand(
    const corvus::commands::Command& command
) {
    using namespace corvus::commands;

    if (std::holds_alternative<PlanCommand>(command)) {
        return corvus::core::AgentEvent{corvus::core::StartPlanning{}};
    }

    if (std::holds_alternative<ToolCommand>(command)) {
        return corvus::core::AgentEvent{corvus::core::StartToolExecution{}};
    }

    if (std::holds_alternative<ValidateCommand>(command)) {
        return corvus::core::AgentEvent{corvus::core::StartValidation{}};
    }

    if (std::holds_alternative<FinishCommand>(command)) {
        return corvus::core::AgentEvent{corvus::core::Complete{}};
    }

    if (std::holds_alternative<ResetCommand>(command)) {
        return corvus::core::AgentEvent{corvus::core::Reset{}};
    }

    if (std::holds_alternative<FailCommand>(command)) {
        return corvus::core::AgentEvent{
            corvus::core::Fail{"manual failure command"}
        };
    }

    return std::nullopt;
}

bool handleStateCommand(
    const corvus::commands::Command& command,
    std::vector<std::string>& logLines,
    corvus::core::AgentRuntime& runtime
) {
    const std::optional<corvus::core::AgentEvent> event =
        eventFromCommand(command);

    if (!event.has_value()) {
        return false;
    }

    const std::string eventDescription =
        corvus::core::describeEvent(event.value());

    const corvus::core::TransitionResult result =
        runtime.dispatch(event.value());

    if (result.succeeded()) {
        pushLog(logLines, "Transition accepted: " + eventDescription);
        pushLog(
            logLines,
            "State: " + std::string{corvus::core::toString(runtime.state())}
        );
        return true;
    }

    pushLog(logLines, "Transition denied: " + result.error.value());
    pushLog(
        logLines,
        "State: " + std::string{corvus::core::toString(runtime.state())}
    );
    return true;
}

void handleCommand(
    std::string_view input,
    std::vector<std::string>& logLines,
    corvus::core::AgentRuntime& runtime,
    const std::function<void()>& quit
) {
    const std::optional<corvus::commands::Command> parsedCommand =
        corvus::commands::parseCommand(input);

    if (!parsedCommand.has_value()) {
        return;
    }

    pushLog(logLines, "> " + std::string{input});

    const corvus::commands::Command& command = parsedCommand.value();

    using namespace corvus::commands;

    if (std::holds_alternative<HelpCommand>(command)) {
        printShellHelp(logLines);
        return;
    }

    if (std::holds_alternative<VersionCommand>(command)) {
        pushLog(
            logLines,
            runtime.config().name() + " " + runtime.config().version()
        );
        return;
    }

    if (std::holds_alternative<ConfigCommand>(command)) {
        printConfig(logLines, runtime);
        return;
    }

    if (std::holds_alternative<StateCommand>(command)) {
        pushLog(
            logLines,
            "State: " + std::string{corvus::core::toString(runtime.state())}
        );
        return;
    }

    if (handleStateCommand(command, logLines, runtime)) {
        return;
    }

    if (std::holds_alternative<ExitCommand>(command)) {
        pushLog(logLines, "Goodbye from Corvus.");
        quit();
        return;
    }

    const auto& unknown = std::get<UnknownCommand>(command);
    pushLog(logLines, "Unknown command: " + unknown.input);
    pushLog(logLines, "Type :help for available commands.");
}

} // namespace

TuiApp::TuiApp(corvus::core::AgentConfig config)
    : runtime_{std::move(config)} {}

void TuiApp::run() {
    using namespace ftxui;

    auto screen = ScreenInteractive::TerminalOutput();

    std::string input;
    std::vector<std::string> logLines{
        "Corvus TUI started.",
        "Type :help for commands, :exit to quit.",
    };

    auto inputComponent = Input(&input, "type :help and press Enter");
    auto quit = screen.ExitLoopClosure();

    auto component = CatchEvent(inputComponent, [&](Event event) {
        if (event == Event::Return) {
            const std::string submittedInput = input;
            input.clear();

            handleCommand(submittedInput, logLines, runtime_, quit);
            return true;
        }

        return false;
    });

    auto renderer = Renderer(component, [&] {
        return vbox({
            hbox({
                renderStatePanel(runtime_),
                renderLogPanel(logLines),
            }) | flex,
            separator(),
            hbox({
                text("> "),
                inputComponent->Render() | flex,
            }) | border,
        });
    });

    screen.Loop(renderer);
}

} // namespace corvus::tui