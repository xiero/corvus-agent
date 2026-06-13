#pragma once

#include <string_view>

namespace corvus::safety {
    enum class RiskLevel {
        Low,
        Medium,
        High,
        Critical
    };

    [[nodiscard]] std::string_view toString(RiskLevel riskLevel);
} // namesapce corvus::safety