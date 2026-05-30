#include "tui/TuiApp.hpp"

#include "commands/CommandExecutor.hpp"
#include "commands/CommandParser.hpp"
#include "core/AgentState.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <functional>

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

void pushLines(
    std::vector<std::string>& logLines,
    const std::vector<std::string>& lines
) {
    for (const std::string& line : lines) {
        pushLog(logLines, line);
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
        logLines.size() > maxVisibleLines
            ? logLines.size() - maxVisibleLines
            : 0;

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

void handleCommand(
    std::string_view input,
    std::vector<std::string>& logLines,
    corvus::core::AgentRuntime& runtime,
    const corvus::commands::CommandExecutor& executor,
    const std::function<void()>& quit
) {
    const std::optional<corvus::commands::Command> parsedCommand =
        corvus::commands::parseCommand(input);

    if (!parsedCommand.has_value()) {
        return;
    }

    pushLog(logLines, "> " + std::string{input});

    const corvus::commands::CommandExecutionResult result =
        executor.execute(parsedCommand.value(), runtime);

    pushLines(logLines, result.lines);

    if (result.shouldExit) {
        quit();
    }
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

    const corvus::commands::CommandExecutor executor;

    auto inputComponent = Input(&input, "type :help and press Enter");
    auto quit = screen.ExitLoopClosure();

    auto component = CatchEvent(inputComponent, [&](Event event) {
        if (event == Event::Return) {
            const std::string submittedInput = input;
            input.clear();

            handleCommand(
                submittedInput,
                logLines,
                runtime_,
                executor,
                quit
            );

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