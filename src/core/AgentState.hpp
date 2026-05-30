#pragma once

#include <string_view>

namespace corvus::core {
    enum class AgentState {
        Idle,
        Planning,
        ToolExecution,
        Validation,
        FinalResponse,
        Error
    };

    [[nodiscard]] std::string_view toString(AgentState state);
} // namespace corvus::core