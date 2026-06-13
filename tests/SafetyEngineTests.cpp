#include "safety/SafetyEngine.hpp"
#include "safety/SafetyPolicy.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Test failed: " << message << "\n";
        std::exit(1);
    }
}

corvus::safety::ToolCall makeToolCall(
    std::string toolName,
    std::map<std::string, std::string> arguments = {}
) {
    return corvus::safety::ToolCall{
        .toolName = std::move(toolName),
        .arguments = std::move(arguments),
    };
}

void requireDecision(
    const corvus::safety::SafetyEvaluation& evaluation,
    corvus::safety::SafetyDecision expectedDecision,
    const std::string& message
) {
    require(
        evaluation.decision == expectedDecision,
        message
    );
}

void testEchoIsAllowed() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(makeToolCall("echo"));

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Allowed,
        "echo should be allowed"
    );

    require(
        evaluation.risk == corvus::safety::RiskLevel::Low,
        "echo should be low risk"
    );
}

void testReadFileRelativePathIsAllowed() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "read_file",
                {
                    {"path", "README.md"},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Allowed,
        "relative read_file should be allowed"
    );
}

void testListFilesDefaultPathIsAllowed() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(makeToolCall("list_files"));

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Allowed,
        "list_files without path should be allowed"
    );
}

void testAbsolutePathIsDenied() {
    const corvus::safety::SafetyEngine engine;

    const std::string absolutePath =
        std::filesystem::current_path().string();

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "read_file",
                {
                    {"path", absolutePath},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Denied,
        "absolute paths should be denied"
    );

    require(
        evaluation.reason.has_value(),
        "denied absolute path should include a reason"
    );
}

void testPathTraversalIsDenied() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "read_file",
                {
                    {"path", "../secret.txt"},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Denied,
        "path traversal should be denied"
    );
}

void testGitDirectoryIsDenied() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "read_file",
                {
                    {"path", ".git/config"},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Denied,
        ".git paths should be denied"
    );
}

void testHomeShortcutIsDenied() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "read_file",
                {
                    {"path", "~/.ssh/id_rsa"},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Denied,
        "home shortcuts should be denied"
    );
}

void testWriteToolNeedsConfirmation() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "write_file",
                {
                    {"path", "README.md"},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::NeedsConfirmation,
        "write_file should need confirmation"
    );

    require(
        evaluation.risk == corvus::safety::RiskLevel::High,
        "write_file should be high risk"
    );
}

void testDangerousShellCommandIsDenied() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(
            makeToolCall(
                "shell",
                {
                    {"command", "rm -rf /"},
                }
            )
        );

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Denied,
        "dangerous shell command should be denied"
    );

    require(
        evaluation.risk == corvus::safety::RiskLevel::Critical,
        "dangerous shell command should be critical risk"
    );
}

void testUnknownToolIsDenied() {
    const corvus::safety::SafetyEngine engine;

    const corvus::safety::SafetyEvaluation evaluation =
        engine.evaluate(makeToolCall("mystery_tool"));

    requireDecision(
        evaluation,
        corvus::safety::SafetyDecision::Denied,
        "unknown tools should be denied"
    );
}

} // namespace

int main() {
    testEchoIsAllowed();
    testReadFileRelativePathIsAllowed();
    testListFilesDefaultPathIsAllowed();
    testAbsolutePathIsDenied();
    testPathTraversalIsDenied();
    testGitDirectoryIsDenied();
    testHomeShortcutIsDenied();
    testWriteToolNeedsConfirmation();
    testDangerousShellCommandIsDenied();
    testUnknownToolIsDenied();

    std::cout << "SafetyEngineTests passed\n";
    return 0;
}