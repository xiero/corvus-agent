#include "audit/AuditEvent.hpp"

namespace corvus::audit {

std::string_view toString(AuditEventType type) {
    switch (type) {
        case AuditEventType::SessionStarted:
            return "SessionStarted";

        case AuditEventType::SessionEnded:
            return "SessionEnded";

        case AuditEventType::CommandReceived:
            return "CommandReceived";

        case AuditEventType::StateTransition:
            return "StateTransition";

        case AuditEventType::SafetyEvaluated:
            return "SafetyEvaluated";

        case AuditEventType::ToolExecuted:
            return "ToolExecuted";
    }

    return "Unknown";
}

} // namespace corvus::audit