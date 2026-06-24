#pragma once

#include "audit/AuditLog.hpp"
#include "commands/Command.hpp"
#include "core/AgentRuntime.hpp"
#include "safety/SafetyEngine.hpp"
#include "tools/ToolRegistry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace corvus::commands {

struct CommandExecutionResult {
    std::vector<std::string> lines;
    bool shouldExit{false};
};

class CommandExecutor {
public:
    explicit CommandExecutor(const corvus::core::AgentConfig& config);

    void recordSessionStarted(std::string_view interfaceName) const;
    void recordSessionEnded(std::string_view interfaceName) const;

    [[nodiscard]] CommandExecutionResult execute(
        const Command& command,
        corvus::core::AgentRuntime& runtime
    ) const;

private:
    corvus::tools::ToolRegistry toolRegistry_;
    corvus::safety::SafetyEngine safetyEngine_;
    std::shared_ptr<corvus::audit::AuditLog> auditLog_;
};

} // namespace corvus::commands