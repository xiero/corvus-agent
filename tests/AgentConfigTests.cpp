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

void testSafeDefaults() {
    const corvus::core::AgentConfig config;

    require(config.name() == "Corvus Agent", "default name should be Corvus Agent");
    require(config.version() == "0.1.0", "default version should be 0.1.0");
    require(!config.deterministicMode(), "deterministic mode should be off by default");
    require(!config.mentorMode(), "mentor mode should be off by default");
}

void testModeFlagsCanBeEnabled() {
    corvus::core::AgentConfig config;

    config.setDeterministicMode(true);
    config.setMentorMode(true);

    require(config.deterministicMode(), "deterministic mode should be enabled");
    require(config.mentorMode(), "mentor mode should be enabled");
}

} // namespace

int main() {
    testSafeDefaults();
    testModeFlagsCanBeEnabled();

    std::cout << "AgentConfigTests passed\n";
    return 0;
}