#include "audit/AuditEvent.hpp"
#include "audit/AuditLog.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Test failed: " << message << "\n";
        std::exit(1);
    }
}

struct TemporaryDirectory {
    TemporaryDirectory()
        : path{
            std::filesystem::temp_directory_path()
            / (
                "corvus_audit_log_test_"
                + std::to_string(
                    std::chrono::steady_clock::now()
                        .time_since_epoch()
                        .count()
                )
            )
        } {
        std::filesystem::create_directories(path);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    std::filesystem::path path;
};

[[nodiscard]] std::string readWholeFile(const std::filesystem::path& path) {
    std::ifstream input{path};

    std::string content;
    std::string line;

    while (std::getline(input, line)) {
        content += line;
        content += '\n';
    }

    return content;
}

void testEventTypeToString() {
    require(
        corvus::audit::toString(corvus::audit::AuditEventType::SessionStarted)
            == "SessionStarted",
        "SessionStarted should stringify"
    );

    require(
        corvus::audit::toString(corvus::audit::AuditEventType::ToolExecuted)
            == "ToolExecuted",
        "ToolExecuted should stringify"
    );
}

void testNullAuditLogAcceptsEvents() {
    corvus::audit::NullAuditLog auditLog;

    auditLog.record(
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::CommandReceived,
            .fields = {
                {"command", "HelpCommand"},
            },
        }
    );
}

void testFileAuditLogWritesJsonLines() {
    TemporaryDirectory temporaryDirectory;

    const std::filesystem::path auditPath =
        temporaryDirectory.path / "audit.jsonl";

    corvus::audit::FileAuditLog auditLog{auditPath};

    auditLog.record(
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::SessionStarted,
            .fields = {
                {"interface", "test"},
            },
        }
    );

    auditLog.record(
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::CommandReceived,
            .fields = {
                {"command", "PlanCommand"},
                {"state", "Idle"},
            },
        }
    );

    const std::string content = readWholeFile(auditPath);

    require(
        content.find("\"seq\":1") != std::string::npos,
        "audit log should contain first sequence number"
    );

    require(
        content.find("\"type\":\"SessionStarted\"") != std::string::npos,
        "audit log should contain SessionStarted"
    );

    require(
        content.find("\"interface\":\"test\"") != std::string::npos,
        "audit log should contain interface field"
    );

    require(
        content.find("\"seq\":2") != std::string::npos,
        "audit log should contain second sequence number"
    );

    require(
        content.find("\"type\":\"CommandReceived\"") != std::string::npos,
        "audit log should contain CommandReceived"
    );

    require(
        content.find("\"command\":\"PlanCommand\"") != std::string::npos,
        "audit log should contain command field"
    );
}

void testFileAuditLogEscapesJsonStrings() {
    TemporaryDirectory temporaryDirectory;

    const std::filesystem::path auditPath =
        temporaryDirectory.path / "audit.jsonl";

    corvus::audit::FileAuditLog auditLog{auditPath};

    auditLog.record(
        corvus::audit::AuditEvent{
            .type = corvus::audit::AuditEventType::CommandReceived,
            .fields = {
                {"command", "UnknownCommand(\"hello\")"},
            },
        }
    );

    const std::string content = readWholeFile(auditPath);

    require(
        content.find("\\\"hello\\\"") != std::string::npos,
        "audit log should escape quotes"
    );
}

} // namespace

int main() {
    testEventTypeToString();
    testNullAuditLogAcceptsEvents();
    testFileAuditLogWritesJsonLines();
    testFileAuditLogEscapesJsonStrings();

    std::cout << "AuditLogTests passed\n";
    return 0;
}