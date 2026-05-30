#pragma once

namespace corvus::cli {

class CliApp {
public:
    [[nodiscard]] int run(int argc, char** argv) const;
};

} // namespace corvus::cli