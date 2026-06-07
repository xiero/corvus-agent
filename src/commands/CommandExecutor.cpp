#include "commands/CommandExecutor.hpp"

#include "core/AgentEvent.hpp"
#include "core/AgentState.hpp"
#include "core/StateTransition.hpp"
#include "tools/DefaultTools.hpp"
#include "tools/Tool.hpp"

#include <optional>
#include <string>
#include <variant>

namespace corvus::commands {
namespace {

[[nodiscard]] std::string boolToText(bool value) {
    return value ? "true" : "false";
}

void appendHelp(CommandExecutionResult& result) {
    result.lines.push_back("Available commands:");
    result.lines.push_back("  :help");
    result.lines.push_back("  :version");
    result.lines.push_back("  :config");
    result.lines.push_back("  :state");
    result.lines.push_back("  :plan");
    result.lines.push_back("  :tool");
    result.lines.push_back("  :validate");
    result.lines.push_back("  :finish");
    result.lines.push_back("  :reset");
    result.lines.push_back("  :fail");
    result.lines.push_back("  :tool list");
    result.lines.push_back("  :tool run echo message=hello");
    result.lines.push_back("  :tool run list_files path=.");
    result.lines.push_back("  :tool run read_file path=README.md");
    result.lines.push_back("  :exit");
}

void appendConfig(
    CommandExecutionResult& result,
    const corvus::core::AgentRuntime& runtime
) {
    const corvus::core::AgentConfig& config = runtime.config();

    result.lines.push_back("Config:");
    result.lines.push_back("  name: " + config.name());
    result.lines.push_back("  version: " + config.version());
    result.lines.push_back(
        "  deterministicMode: " + boolToText(config.deterministicMode())
    );
    result.lines.push_back(
        "  mentorMode: " + boolToText(config.mentorMode())
    );
    result.lines.push_back(
        "  state: " + std::string{corvus::core::toString(runtime.state())}
    );
}

void appendToolList(
    CommandExecutionResult& result,
    const corvus::tools::ToolRegistry& registry
) {
    const std::vector<corvus::tools::ToolMetadata> tools =
        registry.listTools();

    result.lines.push_back("Available tools:");

    if (tools.empty()) {
        result.lines.push_back("  (none)");
        return;
    }

    for (const corvus::tools::ToolMetadata& tool : tools) {
        result.lines.push_back(
            "  " + tool.name + " - " + tool.description
        );
    }
}

void appendToolRun(
    CommandExecutionResult& result,
    const ToolRunCommand& command,
    const corvus::tools::ToolRegistry& registry,
    const corvus::core::AgentRuntime& runtime
) {
    if (runtime.state() != corvus::core::AgentState::ToolExecution) {
        result.lines.push_back(
            "Tool run denied: current state is "
                + std::string{corvus::core::toString(runtime.state())}
        );
        result.lines.push_back(
            "Use :plan and :tool before running tools."
        );
        return;
    }

    corvus::tools::ToolInput input;
    input.arguments = command.arguments;

    const corvus::tools::ToolResult toolResult =
        registry.run(command.toolName, input);

    result.lines.push_back("Tool: " + command.toolName);
    result.lines.push_back(
        toolResult.succeeded ? "Status: succeeded" : "Status: failed"
    );

    for (const std::string& line : toolResult.lines) {
        result.lines.push_back(line);
    }
}

[[nodiscard]] std::optional<corvus::core::AgentEvent> eventFromCommand(
    const Command& command
) {
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

[[nodiscard]] bool appendStateTransitionResult(
    CommandExecutionResult& executionResult,
    const Command& command,
    corvus::core::AgentRuntime& runtime
) {
    const std::optional<corvus::core::AgentEvent> event =
        eventFromCommand(command);

    if (!event.has_value()) {
        return false;
    }

    const std::string eventDescription =
        corvus::core::describeEvent(event.value());

    const corvus::core::TransitionResult transitionResult =
        runtime.dispatch(event.value());

    if (transitionResult.succeeded()) {
        executionResult.lines.push_back(
            "Transition accepted: " + eventDescription
        );
    } else {
        executionResult.lines.push_back(
            "Transition denied: " + transitionResult.error.value()
        );
    }

    executionResult.lines.push_back(
        "State: " + std::string{corvus::core::toString(runtime.state())}
    );

    return true;
}

} // namespace

CommandExecutor::CommandExecutor()
    : toolRegistry_{corvus::tools::createDefaultToolRegistry()} {}

CommandExecutionResult CommandExecutor::execute(
    const Command& command,
    corvus::core::AgentRuntime& runtime
) const {
    CommandExecutionResult result;

    if (std::holds_alternative<HelpCommand>(command)) {
        appendHelp(result);
        return result;
    }

    if (std::holds_alternative<VersionCommand>(command)) {
        result.lines.push_back(
            runtime.config().name() + " " + runtime.config().version()
        );
        return result;
    }

    if (std::holds_alternative<ConfigCommand>(command)) {
        appendConfig(result, runtime);
        return result;
    }

    if (std::holds_alternative<StateCommand>(command)) {
        result.lines.push_back(
            "State: " + std::string{corvus::core::toString(runtime.state())}
        );
        return result;
    }

    if (std::holds_alternative<ToolListCommand>(command)) {
        appendToolList(result, toolRegistry_);
        return result;
    }

    if (std::holds_alternative<ToolRunCommand>(command)) {
        appendToolRun(
            result,
            std::get<ToolRunCommand>(command),
            toolRegistry_,
            runtime
        );
        return result;
    }

    if (appendStateTransitionResult(result, command, runtime)) {
        return result;
    }

    if (std::holds_alternative<ExitCommand>(command)) {
        result.lines.push_back("Goodbye from Corvus.");
        result.shouldExit = true;
        return result;
    }

    const auto& unknown = std::get<UnknownCommand>(command);
    result.lines.push_back("Unknown command: " + unknown.input);
    result.lines.push_back("Type :help for available commands.");
    return result;
}

} // namespace corvus::commands