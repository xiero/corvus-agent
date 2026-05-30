#include "core/AgentEvent.hpp"
#include "core/AgentRuntime.hpp"
#include "core/AgentState.hpp"
#include "core/StateTransition.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Test failed: " << message << "\n";
        std::exit(1);
    }
}

void dispatchAndRequireSuccess(
    corvus::core::AgentRuntime& runtime,
    const corvus::core::AgentEvent& event,
    const std::string& message
) {
    const corvus::core::TransitionResult result = runtime.dispatch(event);

    require(result.succeeded(), message);
}

void testRuntimeStartsInIdle() {
    const corvus::core::AgentRuntime runtime{
        corvus::core::AgentConfig{}
    };

    require(
        runtime.state() == corvus::core::AgentState::Idle,
        "runtime should start in Idle"
    );
}

void testHappyPathTransitions() {
    corvus::core::AgentRuntime runtime{
        corvus::core::AgentConfig{}
    };

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::StartPlanning{},
        "Idle -> Planning should succeed"
    );

    require(
        runtime.state() == corvus::core::AgentState::Planning,
        "state should be Planning"
    );

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::StartToolExecution{},
        "Planning -> ToolExecution should succeed"
    );

    require(
        runtime.state() == corvus::core::AgentState::ToolExecution,
        "state should be ToolExecution"
    );

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::StartValidation{},
        "ToolExecution -> Validation should succeed"
    );

    require(
        runtime.state() == corvus::core::AgentState::Validation,
        "state should be Validation"
    );

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::Complete{},
        "Validation -> FinalResponse should succeed"
    );

    require(
        runtime.state() == corvus::core::AgentState::FinalResponse,
        "state should be FinalResponse"
    );
}

void testInvalidTransitionDoesNotChangeState() {
    corvus::core::AgentRuntime runtime{
        corvus::core::AgentConfig{}
    };

    const corvus::core::TransitionResult result =
        runtime.dispatch(corvus::core::StartToolExecution{});

    require(!result.succeeded(), "Idle -> ToolExecution should fail");
    require(result.error.has_value(), "failed transition should have an error");

    require(
        runtime.state() == corvus::core::AgentState::Idle,
        "failed transition should keep current state"
    );
}

void testFailMovesToError() {
    corvus::core::AgentRuntime runtime{
        corvus::core::AgentConfig{}
    };

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::Fail{"test failure"},
        "Fail should be accepted"
    );

    require(
        runtime.state() == corvus::core::AgentState::Error,
        "Fail should move runtime to Error"
    );
}

void testResetFromErrorMovesToIdle() {
    corvus::core::AgentRuntime runtime{
        corvus::core::AgentConfig{}
    };

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::Fail{"test failure"},
        "Fail should move runtime to Error before reset"
    );

    const corvus::core::TransitionResult result =
        runtime.dispatch(corvus::core::Reset{});

    require(result.succeeded(), "Reset from Error should succeed");

    require(
        runtime.state() == corvus::core::AgentState::Idle,
        "Reset from Error should move runtime to Idle"
    );
}

void testResetFromFinalResponseMovesToIdle() {
    corvus::core::AgentRuntime runtime{
        corvus::core::AgentConfig{}
    };

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::StartPlanning{},
        "Idle -> Planning setup should succeed"
    );

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::StartToolExecution{},
        "Planning -> ToolExecution setup should succeed"
    );

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::StartValidation{},
        "ToolExecution -> Validation setup should succeed"
    );

    dispatchAndRequireSuccess(
        runtime,
        corvus::core::Complete{},
        "Validation -> FinalResponse setup should succeed"
    );

    const corvus::core::TransitionResult result =
        runtime.dispatch(corvus::core::Reset{});

    require(result.succeeded(), "Reset from FinalResponse should succeed");

    require(
        runtime.state() == corvus::core::AgentState::Idle,
        "Reset from FinalResponse should move runtime to Idle"
    );
}

} // namespace

int main() {
    testRuntimeStartsInIdle();
    testHappyPathTransitions();
    testInvalidTransitionDoesNotChangeState();
    testFailMovesToError();
    testResetFromErrorMovesToIdle();
    testResetFromFinalResponseMovesToIdle();

    std::cout << "AgentStateMachineTests passed\n";
    return 0;
}