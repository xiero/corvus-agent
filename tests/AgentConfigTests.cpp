#include "core/AgentConfig.hpp"

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

void testDefaultConfig() {
    const corvus::core::AgentConfig config;

    require(config.name() == "Corvus Agent", "default name should match");
    require(config.version() == "0.1.0", "default version should match");
    require(!config.deterministicMode(), "deterministic mode should be off by default");
    require(!config.mentorMode(), "mentor mode should be off by default");
}

void testDeterministicModeCanBeEnabled() {
    corvus::core::AgentConfig config;

    config.setDeterministicMode(true);

    require(
        config.deterministicMode(),
        "deterministic mode should be enabled"
    );
}

void testMentorModeCanBeEnabled() {
    corvus::core::AgentConfig config;

    config.setMentorMode(true);

    require(
        config.mentorMode(),
        "mentor mode should be enabled"
    );
}

} // namespace

int main() {
    testDefaultConfig();
    testDeterministicModeCanBeEnabled();
    testMentorModeCanBeEnabled();

    std::cout << "AgentConfigTests passed\n";
    return 0;
}