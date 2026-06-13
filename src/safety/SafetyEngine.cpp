#include "safety/SafetyEngine.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace corvus::safety {
namespace {

[[nodiscard]] SafetyEvaluation allowed(RiskLevel riskLevel) {
    return SafetyEvaluation{
        .decision = SafetyDecision::Allowed,
        .risk = riskLevel,
        .reason = std::nullopt,
    };
}

[[nodiscard]] SafetyEvaluation needsConfirmation(
    RiskLevel riskLevel,
    std::string reason
) {
    return SafetyEvaluation{
        .decision = SafetyDecision::NeedsConfirmation,
        .risk = riskLevel,
        .reason = std::move(reason),
    };
}

[[nodiscard]] SafetyEvaluation denied(
    RiskLevel riskLevel,
    std::string reason
) {
    return SafetyEvaluation{
        .decision = SafetyDecision::Denied,
        .risk = riskLevel,
        .reason = std::move(reason),
    };
}

[[nodiscard]] std::optional<std::string> findArgument(
    const ToolCall& toolCall,
    std::string_view key
) {
    const auto argument = toolCall.arguments.find(std::string{key});

    if (argument == toolCall.arguments.end()) {
        return std::nullopt;
    }

    return argument->second;
}

[[nodiscard]] bool containsPathTraversal(const std::filesystem::path& path) {
    for (const std::filesystem::path& part : path) {
        if (part == "..") {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool containsGitDirectory(const std::filesystem::path& path) {
    for (const std::filesystem::path& part : path) {
        if (part == ".git") {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool startsWithHomeShortcut(std::string_view path) {
    return !path.empty() && path.front() == '~';
}

[[nodiscard]] std::optional<std::string> validatePathArgument(
    const SafetyPolicy& policy,
    std::string_view pathText
) {
    if (pathText.empty()) {
        return std::nullopt;
    }

    if (policy.denyHomeShortcut() && startsWithHomeShortcut(pathText)) {
        return "Home directory shortcuts are denied: " + std::string{pathText};
    }

    const std::filesystem::path path{pathText};

    if (!policy.allowAbsolutePaths() && path.is_absolute()) {
        return "Absolute paths are denied: " + path.string();
    }

    if (containsPathTraversal(path)) {
        return "Path traversal is denied: " + path.string();
    }

    if (policy.protectGitDirectory() && containsGitDirectory(path)) {
        return ".git paths are denied: " + path.string();
    }

    return std::nullopt;
}

[[nodiscard]] SafetyEvaluation evaluateReadOnlyPathTool(
    const SafetyPolicy& policy,
    const ToolCall& toolCall
) {
    const std::optional<std::string> pathArgument =
        findArgument(toolCall, "path");

    if (!pathArgument.has_value()) {
        return allowed(RiskLevel::Low);
    }

    const std::optional<std::string> pathError =
        validatePathArgument(policy, pathArgument.value());

    if (pathError.has_value()) {
        return denied(RiskLevel::High, pathError.value());
    }

    return allowed(RiskLevel::Low);
}

[[nodiscard]] bool containsDangerousShellFragment(std::string_view command) {
    return command.find("rm -rf") != std::string_view::npos
        || command.find("mkfs") != std::string_view::npos
        || command.find("dd ") != std::string_view::npos
        || command.find(":(){") != std::string_view::npos
        || command.find("sudo ") != std::string_view::npos
        || command.find("chmod -R 777") != std::string_view::npos;
}

[[nodiscard]] SafetyEvaluation evaluateShellTool(const ToolCall& toolCall) {
    const std::optional<std::string> command =
        findArgument(toolCall, "command");

    if (!command.has_value()) {
        return needsConfirmation(
            RiskLevel::High,
            "Shell tool requires explicit confirmation."
        );
    }

    if (containsDangerousShellFragment(command.value())) {
        return denied(
            RiskLevel::Critical,
            "Dangerous shell command denied: " + command.value()
        );
    }

    return needsConfirmation(
        RiskLevel::High,
        "Shell command requires explicit confirmation: " + command.value()
    );
}

} // namespace

SafetyEngine::SafetyEngine(SafetyPolicy policy)
    : policy_{std::move(policy)} {}

SafetyEvaluation SafetyEngine::evaluate(const ToolCall& toolCall) const {
    if (toolCall.toolName == "echo") {
        return allowed(RiskLevel::Low);
    }

    if (toolCall.toolName == "list_files") {
        return evaluateReadOnlyPathTool(policy_, toolCall);
    }

    if (toolCall.toolName == "read_file") {
        return evaluateReadOnlyPathTool(policy_, toolCall);
    }

    if (
        toolCall.toolName == "write_file"
        || toolCall.toolName == "write_file_preview"
    ) {
        return needsConfirmation(
            RiskLevel::High,
            "Write tools require confirmation."
        );
    }

    if (toolCall.toolName == "shell") {
        return evaluateShellTool(toolCall);
    }

    return denied(
        RiskLevel::Medium,
        "Tool is not covered by safety policy: " + toolCall.toolName
    );
}

} // namespace corvus::safety