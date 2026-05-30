#include "core/AgentState.hpp"

namespace corvus::core {
    std::string_view toString(AgentState state) {
        switch (state){
            case AgentState::Idle:
                return "Idle";

            case AgentState::Planning:
                return "Planning";

            case AgentState::ToolExecution:
                return "ToolExecution";
            
            case AgentState::Validation:
                return "Validation";
            
            case AgentState::FinalResponse:
                return "FinalResponse";

            case AgentState::Error:
                return "Error";
            default:
                break;
        }

        return "Unknown";
    }
}//namespace corvus::core