#pragma once

#include "core/AgentConfig.hpp"

namespace corvus::tui {
    class TuiApp {
        public:
            explicit TuiApp(corvus::core::AgentConfig config);
            void run();
        private:
            corvus::core::AgentConfig config_;

    };
} //namespace corvus::tui