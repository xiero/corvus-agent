#pragma once

#include "safety/RiskLevel.hpp"

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace corvus::safety {

enum class SafetyDecision {
    Allowed,
    NeedsConfirmation,
    Denied
};

[[nodiscard]] std::string_view toString(SafetyDecision decision);

struct ToolCall {
    std::string toolName;
    std::map<std::string, std::string> arguments;
};

struct SafetyEvaluation {
    SafetyDecision decision{SafetyDecision::Denied};
    RiskLevel risk{RiskLevel::Critical};
    std::optional<std::string> reason;

    [[nodiscard]] bool allowed() const;
    [[nodiscard]] bool needsConfirmation() const;
    [[nodiscard]] bool denied() const;
};

struct SafetyPolicyConfig {
    bool readOnlyMode{true};
    bool allowAbsolutePaths{false};
    bool protectGitDirectory{true};
    bool denyHomeShortcut{true};
};

class SafetyPolicy {
    public:
        explicit SafetyPolicy(SafetyPolicyConfig config = {} );

        [[nodiscard]] bool readOnlyMode() const;
        [[nodiscard]] bool allowAbsolutePaths() const;
        [[nodiscard]] bool protectGitDirectory() const;
        [[nodiscard]] bool denyHomeShortcut() const;

    private:
        SafetyPolicyConfig config_;
};

} // namespace corvus::safety