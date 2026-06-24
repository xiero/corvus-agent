#include "commands/CommandExecutor.hpp"

#include "audit/AuditEvent.hpp"
#include "audit/AuditLogFactory.hpp"
#include "core/AgentEvent.hpp"
#include "core/AgentState.hpp"
#include "core/StateTransition.hpp"
#include "safety/SafetyPolicy.hpp"
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

[[nodiscard]] std::string describeCommand(const Command& command) {
    if (std::holds_alternative<HelpCommand>(command)) {
        return "HelpCommand";
    }

    if (std::holds_alternative<ExitCommand>(command)) {
        return "ExitCommand";
    }

    if (std::holds_alternative<VersionCommand>(command)) {
        return "VersionCommand";
    }

    if (std::holds_alternative<ConfigCommand>(command)) {
        return "ConfigCommand";
    }

    if (std::holds_alternative<StateCommand>(command)) {
        return "StateCommand";
    }

    if (std::holds_alternative<PlanCommand>(command)) {
        return "PlanCommand";
    }

    if (std::holds_alternative<ToolCommand>(command)) {
        return "ToolCommand";
    }

    if (std::holds_alternative<ValidateCommand>(command)) {
        return "ValidateCommand";
    }

    if (std::holds_alternative<FinishCommand>(command)) {
        return "FinishCommand";
    }

    if (std::holds_alternative<ResetCommand>(command)) {
        return "ResetCommand";
    }

    if (std::holds_alternative<FailCommand>(command)) {
        return "FailCommand";
    }

    if (std::holds_alternative<ToolListCommand>(command)) {
        return "ToolListCommand";
    }

    if (std::holds_alternative<ToolRunCommand>(command)) {
        const ToolRunCommand& toolRun = std::get<ToolRunCommand>(command);
        return "ToolRunCommand(" + toolRun.toolName + ")";
    }

    if (std::holds_alternative<UnknownCommand>(command)) {
        const UnknownCommand& unknown = std::get<UnknownCommand>(command);
        return "UnknownCommand(" + unknown.input + ")";
    }

    return "Unknown";
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

[[nodiscard]] corvus::safety::ToolCall toolCallFromCommand(
    const ToolRunCommand& command
) {
    return corvus::safety::ToolCall{
        .toolName = command.toolName,
        .arguments = command.arguments,
    };
}

void appendSafetyEvaluation(
    CommandExecutionResult& result,
    const corvus::safety::SafetyEvaluation& evaluation
) {
    result.lines.push_back(
        "Safety: "
            + std::string{corvus::safety::toString(evaluation.decision)}
            + " (risk: "
            + std::string{corvus::safety::toString(evaluation.risk)}
            + ")"
    );

    if (evaluation.reason.has_value()) {
        result.lines.push_back("Reason: " + evaluation.reason.value());
    }
}

void recordAuditEvent(
    const std::shared_ptr<corvus::audit::AuditLog>& auditLog,
    corvus::audit::AuditEvent event
) {
    if (auditLog != nullptr) {
        auditLog->record(event);
    }
}

void appendToolRun(
    CommandExecutionResult& result,
    const ToolRunCommand& command,
    const corvus::tools::ToolRegistry& registry,
    const corvus::safety::SafetyEngine& safetyEngine,
    const corvus::core::AgentRuntime& runtime,
    const std::shared_ptr<corvus::audit::AuditLog>& auditLog
) {
    if (runtime.state() != corvus::core::AgentState::ToolExecution) {
        result.lines.push_back(
            "Tool run denied: current state is "
                + std::string{corvus::core::toString(runtime.state())}
        );
        result.lines.push_back(
            "Use :plan and :tool before running tools."
        );

        recordAuditEvent(
            auditLog,
            corvus::audit::AuditEvent{
                .type = corvus::audit::AuditEventType::ToolExecuted,
                .fields = {
                    {"tool", command.toolName},
                    {"status", "blocked_by_state"},
                    {"state", std::string{corvus::core::toString(runtime.state())}},
                },
            }
        );

        return;
    }

    const corvus::safety::ToolCall toolCall =
        toolCallFromCommand(command);

    const corvus::safety::SafetyEvaluation safetyEvaluation =
        safetyEngine.evaluate(toolCall);

    appendSafetyEvaluation(result, safetyEvaluation);

    recordAuditEvent(
        auditLog,
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::SafetyEvaluated,
            .fields = {
                {"tool", command.toolName},
                {"decision", std::string{corvus::safety::toString(safetyEvaluation.decision)}},
                {"risk", std::string{corvus::safety::toString(safetyEvaluation.risk)}},
                {"reason", safetyEvaluation.reason.value_or("")},
            },
        }
    );

    if (safetyEvaluation.denied()) {
        result.lines.push_back("Tool execution blocked by safety engine.");

        recordAuditEvent(
            auditLog,
            corvus::audit::AuditEvent{
                .type = corvus::audit::AuditEventType::ToolExecuted,
                .fields = {
                    {"tool", command.toolName},
                    {"status", "blocked_by_safety"},
                },
            }
        );

        return;
    }

    if (safetyEvaluation.needsConfirmation()) {
        result.lines.push_back(
            "Tool execution paused: confirmation flow is not implemented yet."
        );

        recordAuditEvent(
            auditLog,
            corvus::audit::AuditEvent{
                .type = corvus::audit::AuditEventType::ToolExecuted,
                .fields = {
                    {"tool", command.toolName},
                    {"status", "paused_for_confirmation"},
                },
            }
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

    recordAuditEvent(
        auditLog,
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::ToolExecuted,
            .fields = {
                {"tool", command.toolName},
                {"status", toolResult.succeeded ? "succeeded" : "failed"},
            },
        }
    );
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
    corvus::core::AgentRuntime& runtime,
    const std::shared_ptr<corvus::audit::AuditLog>& auditLog
) {
    const std::optional<corvus::core::AgentEvent> event =
        eventFromCommand(command);

    if (!event.has_value()) {
        return false;
    }

    const corvus::core::AgentState previousState = runtime.state();

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

    recordAuditEvent(
        auditLog,
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::StateTransition,
            .fields = {
                {"event", eventDescription},
                {"from", std::string{corvus::core::toString(previousState)}},
                {"to", std::string{corvus::core::toString(runtime.state())}},
                {"status", transitionResult.succeeded() ? "accepted" : "denied"},
                {"reason", transitionResult.error.value_or("")},
            },
        }
    );

    return true;
}

} // namespace

CommandExecutor::CommandExecutor(const corvus::core::AgentConfig& config)
    : toolRegistry_{corvus::tools::createDefaultToolRegistry()},
      safetyEngine_{corvus::safety::SafetyEngine{}},
      auditLog_{corvus::audit::createAuditLog(config)} {}

void CommandExecutor::recordSessionStarted(std::string_view interfaceName) const {
    recordAuditEvent(
        auditLog_,
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::SessionStarted,
            .fields = {
                {"interface", std::string{interfaceName}},
            },
        }
    );
}

void CommandExecutor::recordSessionEnded(std::string_view interfaceName) const {
    recordAuditEvent(
        auditLog_,
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::SessionEnded,
            .fields = {
                {"interface", std::string{interfaceName}},
            },
        }
    );
}

CommandExecutionResult CommandExecutor::execute(
    const Command& command,
    corvus::core::AgentRuntime& runtime
) const {
    CommandExecutionResult result;

    recordAuditEvent(
        auditLog_,
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::CommandReceived,
            .fields = {
                {"command", describeCommand(command)},
                {"state", std::string{corvus::core::toString(runtime.state())}},
            },
        }
    );

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
            safetyEngine_,
            runtime,
            auditLog_
        );
        return result;
    }

    if (appendStateTransitionResult(result, command, runtime, auditLog_)) {
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