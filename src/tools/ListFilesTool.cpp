#include "tools/ListFilesTool.hpp"

#include <algorithm>
#include <filesystem>
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

            return ".";
        }
    } // namespace

    std::string_view ListFilesTool::name() const {
        return "list_files";
    }

    std::string_view ListFilesTool::description() const {
        return "Lists direct children of a directory.";
    }

    ToolResult ListFilesTool::execute(const ToolInput& input) const {
        namespace fs = std::filesystem;

        const fs::path directoryPath{getPathArgument(input)};

        std::error_code error;

        if (!fs::exists(directoryPath, error)) {
            return ToolResult{
                .succeeded = false,
                .lines = {
                    "Directory does not exist: " + directoryPath.string(),
                },
            };
        }

        if (error) {
            return ToolResult{
                .succeeded = false,
                .lines = {
                    "Could not inspect path: " + directoryPath.string(),
                    "Error: " + error.message(),
                },
            };
        }

        if (!fs::is_directory(directoryPath, error)) {
            return ToolResult{
                .succeeded = false,
                .lines = {
                    "Path is not a directory: " + directoryPath.string(),
                },
            };
        }

        fs::directory_iterator iterator{directoryPath, error};

        if (error) {
            return ToolResult{
                .succeeded = false,
                .lines = {
                    "Could not list directory: " + directoryPath.string(),
                    "Error: " + error.message(),
                },
            };
        }

        const fs::directory_iterator end;
        std::vector<std::string> entries;

        while (iterator != end) {
            const fs::directory_entry& entry = *iterator;

            std::string name = entry.path().filename().string();

            std::error_code typeError;
            if (entry.is_directory(typeError)) {
                name += "/";
            }

            entries.push_back(std::move(name));

            iterator.increment(error);

            if (error) {
                return ToolResult{
                    .succeeded = false,
                    .lines = {
                        "Could not continue listing directory: "
                            + directoryPath.string(),
                        "Error: " + error.message(),
                    },
                };
            }
        }

        std::sort(entries.begin(), entries.end());

        std::vector<std::string> lines;
        lines.push_back("Entries in " + directoryPath.string() + ":");

        if (entries.empty()) {
            lines.push_back("  (empty)");
        }

        for (const std::string& entry : entries) {
            lines.push_back("  " + entry);
        }

        return ToolResult{
            .succeeded = true,
            .lines = std::move(lines),
        };
    }

} // namespace corvus::tools