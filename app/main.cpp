#include "core/AgentConfig.hpp"

#include <iostream>
#include <string_view>

namespace {
    void printHelp(const corvus::core::AgentConfig& config) {
        std::cout << config.name() << "\n\n"
                  << "Usage:\n"
                  << " Corvus --help\n"
                  << " corvus --version\n\n"
                  << "Options:\n"
                  << " --help      Show this help message\n"
                  << " --version   Show version information\n";  
    }

    void printVersion(const corvus::core::AgentConfig& config) {
        std::cout << config.name() << " " << config.version() << "\n";
    }

} // namesapce

int main( int argc, char** argv) {
    const corvus::core::AgentConfig config;

    if(argc == 1) {
        printHelp(config);
        return 0;
    }

    const std::string_view argument{argv[1]};

    if(argument == "--help" || argument == "-h") {
        printHelp(config);
        return 0;
    }

    if(argument == "--version" || argument == "-v") {
        printVersion(config);
        return 0;
    }

    std::cerr << "Unknown argument: " << argument << "\n"
              << "Run `corvus --help` for usage.\n";

    return 1;

}