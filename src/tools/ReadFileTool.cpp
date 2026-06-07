#include "tools/ReadFileTool.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace corvus::tools {
namespace {

[[nodiscard]] std::string getPathArgument(const ToolInput& input) {
    const auto path = input.arguments.find("path");

    if (path != input.arguments.end()) {
        return path->second;
    }

    return {};
}

} // namespace

std::string_view ReadFileTool::name() const {
    return "read_file";
}

std::string_view ReadFileTool::description() const {
    return "Reads a text file and returns its first lines.";
}

ToolResult ReadFileTool::execute(const ToolInput& input) const {
    namespace fs = std::filesystem;

    const std::string pathArgument = getPathArgument(input);

    if (pathArgument.empty()) {
        return ToolResult{
            .succeeded = false,
            .lines = {
                "Missing argument.",
                "Usage: :tool run read_file path=<file>",
            },
        };
    }

    const fs::path filePath{pathArgument};

    std::error_code error;

    if (!fs::exists(filePath, error)) {
        return ToolResult{
            .succeeded = false,
            .lines = {
                "File does not exist: " + filePath.string(),
            },
        };
    }

    if (error) {
        return ToolResult{
            .succeeded = false,
            .lines = {
                "Could not inspect file: " + filePath.string(),
                "Error: " + error.message(),
            },
        };
    }

    if (!fs::is_regular_file(filePath, error)) {
        return ToolResult{
            .succeeded = false,
            .lines = {
                "Path is not a regular file: " + filePath.string(),
            },
        };
    }

    std::ifstream file{filePath};

    if (!file) {
        return ToolResult{
            .succeeded = false,
            .lines = {
                "Could not open file: " + filePath.string(),
            },
        };
    }

    constexpr std::size_t maxLines = 40;

    std::vector<std::string> lines;
    lines.push_back("Content of " + filePath.string() + ":");

    std::string line;
    std::size_t lineCount = 0;

    while (lineCount < maxLines && std::getline(file, line)) {
        lines.push_back("  " + line);
        ++lineCount;
    }

    if (lineCount == 0) {
        lines.push_back("  (empty)");
    }

    if (!file.eof()) {
        lines.push_back("  ... truncated");
    }

    return ToolResult{
        .succeeded = true,
        .lines = std::move(lines),
    };
}

} // namespace corvus::tools