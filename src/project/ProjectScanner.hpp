#pragma once

#include <filesystem>
#include <vector>

namespace corvus::project {

struct ProjectScanResult {
    std::filesystem::path rootPath;
    std::vector<std::filesystem::path> sourceFiles;
    std::vector<std::filesystem::path> testFiles;
    std::vector<std::filesystem::path> importantFiles;
    std::vector<std::filesystem::path> ignoredDirectories;
};

class ProjectScanner {
public:
    [[nodiscard]] ProjectScanResult scan(
        const std::filesystem::path& rootPath
    ) const;
};

} // namespace corvus::project