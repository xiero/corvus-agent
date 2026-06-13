#include "safety/RiskLevel.hpp"

namespace corvus::safety {

std::string_view toString(RiskLevel riskLevel) {
    switch (riskLevel)
    {
    case RiskLevel::Low:
        return "Low";

    case RiskLevel::Medium:
        return "Medium";

    case RiskLevel::High:
        return "High";

    case RiskLevel::Critical:
        return "Critical";
    
    }

    return "UnKnown";

}
    
} // namesapce corvus::safety