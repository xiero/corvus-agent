#pragma once

#include "commands/Command.hpp"
#include "core/AgentRuntime.hpp"

#include <string>
#include <vector>

namespace corvus::commands {

struct CommandExecutionResult {
    std::vector<std::string> lines;
    bool shouldExit{false};
};

class CommandExecutor {
public:
    [[nodiscard]] CommandExecutionResult execute(
        const Command& command,
        corvus::core::AgentRuntime& runtime
    ) const;
};

} // namespace corvus::commands