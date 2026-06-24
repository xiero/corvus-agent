#pragma once

#include "audit/AuditEvent.hpp"

#include <filesystem>

namespace corvus::audit {

class AuditLog {
public:
    virtual ~AuditLog() = default;

    virtual void record(const AuditEvent& event) = 0;
};

class NullAuditLog final : public AuditLog {
public:
    void record(const AuditEvent& event) override;
};

class FileAuditLog final : public AuditLog {
public:
    explicit FileAuditLog(std::filesystem::path path);

    void record(const AuditEvent& event) override;

    [[nodiscard]] const std::filesystem::path& path() const;

private:
    std::filesystem::path path_;
    int sequence_{0};
};

} // namespace corvus::audit