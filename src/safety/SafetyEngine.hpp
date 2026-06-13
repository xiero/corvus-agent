#pragma once

#include "safety/SafetyPolicy.hpp"

namespace corvus::safety {

class SafetyEngine {
public:
    explicit SafetyEngine(SafetyPolicy policy = SafetyPolicy{});

    [[nodiscard]] SafetyEvaluation evaluate(const ToolCall& toolCall) const;

private:
    SafetyPolicy policy_;
};

} // namespace corvus::safety