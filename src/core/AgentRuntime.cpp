#include "core/AgentRuntime.hpp"

#include <utility>

namespace corvus::core {

    AgentRuntime::AgentRuntime(AgentConfig config)
        : config_{std::move(config)},
          state_{AgentState::Idle}{}
    
    const AgentConfig& AgentRuntime::config() const {
        return config_;
    }

    AgentState AgentRuntime::state() const {
        return state_;
    }

    TransitionResult AgentRuntime::dispatch(const AgentEvent& event) {
        const TransitionResult result = transition(state_, event);

        if(result.succeeded()){
            state_ = result.state;
        }

        return result;
    }

}