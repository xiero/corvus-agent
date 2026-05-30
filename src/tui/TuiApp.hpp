#pragma once

#include "core/AgentConfig.hpp"
#include "core/AgentRuntime.hpp"

namespace corvus::tui {

class TuiApp {
public:
    explicit TuiApp(corvus::core::AgentConfig config);

    void run();

private:
    corvus::core::AgentRuntime runtime_;
};

} // namespace corvus::tui