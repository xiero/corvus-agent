#pragma once

#include "audit/AuditLog.hpp"
#include "core/AgentConfig.hpp"

#include <memory>

namespace corvus::audit {

[[nodiscard]] std::shared_ptr<AuditLog> createAuditLog(
    const corvus::core::AgentConfig& config
);

} // namespace corvus::audit