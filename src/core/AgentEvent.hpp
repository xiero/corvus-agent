#pragma once

#include <string>
#include <variant>

namespace corvus::core {
    struct StartPlanning {};
    struct StartToolExecution{};
    struct StartValidation{};
    struct Complete{};
    struct Reset{};

    struct Fail {
        std::string reason;
    };

    using AgentEvent = std::variant<
        StartPlanning,
        StartToolExecution,
        StartValidation,
        Complete,
        Reset,
        Fail
    >;

    [[nodiscard]] std::string describeEvent(const AgentEvent& event);

} // namespace corvus::core