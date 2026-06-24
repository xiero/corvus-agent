#pragma once

#include <map>
#include <string>
#include <string_view>

namespace corvus::audit {

enum class AuditEventType {
    SessionStarted,
    SessionEnded,
    CommandReceived,
    StateTransition,
    SafetyEvaluated,
    ToolExecuted
};

[[nodiscard]] std::string_view toString(AuditEventType type);

struct AuditEvent {
    AuditEventType type;
    std::map<std::string, std::string> fields;
};



} // namespace corvus::audit
