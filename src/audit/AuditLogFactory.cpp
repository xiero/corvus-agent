#include "audit/AuditLogFactory.hpp"

#include <memory>

namespace corvus::audit {

std::shared_ptr<AuditLog> createAuditLog(
    const corvus::core::AgentConfig& config
) {
    if (!config.deterministicMode()) {
        return std::make_shared<NullAuditLog>();
    }

    return std::make_shared<FileAuditLog>(
        std::filesystem::path{".corvus"} / "audit" / "latest.jsonl"
    );
}

} // namespace corvus::audit