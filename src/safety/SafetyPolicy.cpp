#include "safety/SafetyPolicy.hpp"

#include <utility>

namespace corvus::safety {

std::string_view toString(SafetyDecision decision) {
    switch (decision) {
        case SafetyDecision::Allowed:
            return "Allowed";

        case SafetyDecision::NeedsConfirmation:
            return "NeedsConfirmation";

        case SafetyDecision::Denied:
            return "Denied";
    }

    return "Unknown";
}

bool SafetyEvaluation::allowed() const {
    return decision == SafetyDecision::Allowed;
}

bool SafetyEvaluation::needsConfirmation() const {
    return decision == SafetyDecision::NeedsConfirmation;
}

bool SafetyEvaluation::denied() const {
    return decision == SafetyDecision::Denied;
}

SafetyPolicy::SafetyPolicy(SafetyPolicyConfig config)
    : config_{std::move(config)} {}

bool SafetyPolicy::readOnlyMode() const {
    return config_.readOnlyMode;
}

bool SafetyPolicy::allowAbsolutePaths() const {
    return config_.allowAbsolutePaths;
}

bool SafetyPolicy::protectGitDirectory() const {
    return config_.protectGitDirectory;
}

bool SafetyPolicy::denyHomeShortcut() const {
    return config_.denyHomeShortcut;
}

} // namespace corvus::safety