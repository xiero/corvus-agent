#pragma once

#include "core/AgentEvent.hpp"
#include "core/AgentState.hpp"

#include <optional>
#include <string>

namespace corvus::core {
    struct TransitionResult {
        AgentState state;
        std::optional<std::string> error;

        [[nodiscard]] bool succeeded() const;
    };

    [[nodiscard]] TransitionResult transition(
        AgentState currentState,
        const AgentEvent& event
    );
} // namespace corvus::core