#pragma once

#include "core/AgentConfig.hpp"
#include "core/AgentEvent.hpp"
#include "core/AgentState.hpp"
#include "core/StateTransition.hpp"

namespace corvus::core {
    class AgentRuntime {
        public:
            explicit AgentRuntime(AgentConfig config);

            [[nodiscard]] const AgentConfig& config() const;
            [[nodiscard]] AgentState state() const;

            [[nodiscard]] TransitionResult dispatch(const AgentEvent& event);
        
        private:
            AgentConfig config_;
            AgentState state_;
    };
} // namespace corvus::core