#include "cli/CliApp.hpp"

int main(int argc, char** argv) {
    const corvus::cli::CliApp app;
    return app.run(argc, argv);
}