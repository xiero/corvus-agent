#include "audit/AuditLog.hpp"

#include <fstream>
#include <string>
#include <system_error>
#include <utility>

namespace corvus::audit {
namespace {

[[nodiscard]] std::string escapeJson(std::string_view value) {
    std::string escaped;

    for (const char character : value) {
        switch (character) {
            case '\\':
                escaped += "\\\\";
                break;

            case '"':
                escaped += "\\\"";
                break;

            case '\n':
                escaped += "\\n";
                break;

            case '\r':
                escaped += "\\r";
                break;

            case '\t':
                escaped += "\\t";
                break;

            default:
                escaped += character;
                break;
        }
    }

    return escaped;
}

void writeJsonString(
    std::ostream& output,
    std::string_view key,
    std::string_view value
) {
    output << '"' << escapeJson(key) << "\":\"" << escapeJson(value) << '"';
}

} // namespace

void NullAuditLog::record(const AuditEvent& event) {
    static_cast<void>(event);
}

FileAuditLog::FileAuditLog(std::filesystem::path path)
    : path_{std::move(path)} {
    std::error_code error;

    if (path_.has_parent_path()) {
        std::filesystem::create_directories(path_.parent_path(), error);
    }

    std::ofstream output{path_, std::ios::out | std::ios::trunc};
    output << "";
}

void FileAuditLog::record(const AuditEvent& event) {
    ++sequence_;

    std::ofstream output{path_, std::ios::out | std::ios::app};

    if (!output) {
        return;
    }

    output << "{\"seq\":" << sequence_ << ',';
    writeJsonString(output, "type", toString(event.type));
    output << ",\"fields\":{";

    bool first = true;

    for (const auto& [key, value] : event.fields) {
        if (!first) {
            output << ',';
        }

        writeJsonString(output, key, value);
        first = false;
    }

    output << "}}\n";
}

const std::filesystem::path& FileAuditLog::path() const {
    return path_;
}

} // namespace corvus::audit
