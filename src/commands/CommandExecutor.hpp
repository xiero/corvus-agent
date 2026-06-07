#pragma once

#include "commands/Command.hpp"
#include "core/AgentRuntime.hpp"
#include "tools/ToolRegistry.hpp"

#include <string>
#include <vector>

namespace corvus::commands {

struct CommandExecutionResult {
    std::vector<std::string> lines;
    bool shouldExit{false};
};

class CommandExecutor {
public:
    CommandExecutor();

    [[nodiscard]] CommandExecutionResult execute(
        const Command& command,
        corvus::core::AgentRuntime& runtime
    ) const;

private:
    corvus::tools::ToolRegistry toolRegistry_;
};

} // namespace corvus::commands