#include "core/AgentConfig.hpp"

namespace corvus::core {

AgentConfig::AgentConfig()
    : name_{"Corvus Agent"},
      version_{"0.1.0"},
      deterministicMode_{false},
      mentorMode_{false} {}

const std::string& AgentConfig::name() const {
    return name_;
}

const std::string& AgentConfig::version() const {
    return version_;
}

bool AgentConfig::deterministicMode() const {
    return deterministicMode_;
}

bool AgentConfig::mentorMode() const {
    return mentorMode_;
}

void AgentConfig::setDeterministicMode(bool enabled) {
    deterministicMode_ = enabled;
}

void AgentConfig::setMentorMode(bool enabled) {
    mentorMode_ = enabled;
}

} // namespace corvus::core