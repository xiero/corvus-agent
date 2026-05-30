#include "core/StateTransition.hpp"

#include<string>
#include<string_view>
#include<utility>

namespace corvus::core {
namespace {

    template <typename... Visitors>
    struct Overloaded : Visitors... {
        using Visitors::operator()...;
    };

    template <typename... Visitors>
    Overloaded(Visitors...) -> Overloaded<Visitors...>;

    [[nodiscard]] TransitionResult allowed(AgentState nextState) {
        return TransitionResult{
            .state = nextState,
            .error = std::nullopt,
        };
    }

    [[nodiscard]] TransitionResult denied(
        AgentState currentState,
        std::string reason
    ) {
        return TransitionResult {
            .state = currentState,
            .error = std::move(reason),
        };
    }

    [[nodiscard]] TransitionResult requireState(
        AgentState currentState,
        AgentState expectedState,
        AgentState nextState,
        std::string_view eventName
    ) {
        if(currentState == expectedState){
            return allowed(nextState);
        }

        return denied(
            currentState,
            std::string{"Cannot handle"}
                + std::string{eventName}
                + " from state "
                + std::string{toString(currentState)}
        );
    }

} // namespace

bool TransitionResult::succeeded() const {
    return !error.has_value();
}

TransitionResult transition(
    AgentState currentState,
    const AgentEvent& event
) {
    return std::visit(
        Overloaded{
            [currentState](const StartPlanning&) {
                return requireState(
                    currentState,
                    AgentState::Idle,
                    AgentState::Planning,
                    "StartPlanning"
                );
            },
            [currentState](const StartToolExecution&){
                return requireState(
                    currentState,
                    AgentState::Planning,
                    AgentState::ToolExecution,
                    "StartToolExecution"
                );
            },
            [currentState](const StartValidation&) {
                return requireState(
                    currentState,
                    AgentState::ToolExecution,
                    AgentState::Validation,
                    "StartValidation"
                );
            },
            [currentState](const Complete&) {
                return requireState(
                    currentState,
                    AgentState::Validation,
                    AgentState::FinalResponse,
                    "Complete"
                );
            },
            [currentState](const Reset&) {
                if (
                    currentState == AgentState::FinalResponse
                    || currentState == AgentState::Error
                    || currentState == AgentState::Idle
                ) {
                    return allowed(AgentState::Idle);
                }

                return denied(
                    currentState,
                    std::string{"Cannot reset from state "}
                        + std::string{toString(currentState)}
                );
            },
            [](const Fail& fail) {
                return TransitionResult{
                    .state = AgentState::Error,
                    .error = std::nullopt,
                };
            },

        },
        event   
    );
}

} // namespace corvus::core