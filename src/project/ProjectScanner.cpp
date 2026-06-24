#include "project/ProjectScanner.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace corvus::project {
    namespace {
        [[nodiscard]] bool isIgnoredDirectoryName(
            const std::string_view directoryName
        ) {
            constexpr std::array<std::string_view, 4> ignoredNames{
                ".git",
                "build",
                "node_modules",
                ".cache",
            };

            return std::find(
                ignoredNames.begin(),
                ignoredNames.end(),
                directoryName
            ) != ignoredNames.end();
        }

        [[nodiscard]] bool isSourceExtension(
            const std::filesystem::path& path
        ) {
            const std::string extension = path.extension().string();

            constexpr std::array<std::string_view, 5> sourceExtensions{
                ".cpp",
                ".hpp",
                ".h",
                ".cxx",
                ".cc",
            };

            return std::find(
                sourceExtensions.begin(),
                sourceExtensions.end(),
                extension
            ) != sourceExtensions.end();
        }

        [[nodiscard]] bool isImportantFileName(
            const std::string_view fileName
        ) {
            constexpr std::array<std::string_view, 6> importantFileNames{
                "README.md",
                "CMakeLists.txt",
                "CMakePresets.json",
                "ARCHITECTURE.md",
                "AI_CONTEXT.md",
                "tasks.md",
            };

            return std::find(
                importantFileNames.begin(),
                importantFileNames.end(),
                fileName
            ) != importantFileNames.end();
        }

        [[nodiscard]] bool pathContainsDirectoryNamed(
            const std::filesystem::path& path,
            const std::string_view directoryName
        ) {
            for (const std::filesystem::path& part : path) {
                if (part.string() == directoryName) {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] bool isTestFile(
            const std::filesystem::path& relativePath
        ) {
            const std::string fileName = relativePath.filename().string();

            return pathContainsDirectoryNamed(relativePath, "tests")
                || fileName.find("Test") != std::string::npos
                || fileName.find("Tests") != std::string::npos;
        }

        [[nodiscard]] std::filesystem::path makeRelativePath(
            const std::filesystem::path& path,
            const std::filesystem::path& rootPath
        ) {
            std::error_code error;
            const std::filesystem::path relativePath =
                std::filesystem::relative(path, rootPath, error);

            if (error) {
                return path;
            }

            return relativePath;
        }

        void sortPaths(std::vector<std::filesystem::path>& paths) {
            std::sort(paths.begin(), paths.end());
        }
    } // namespace

    ProjectScanResult ProjectScanner::scan(
        const std::filesystem::path& rootPath
    ) const {
        namespace fs = std::filesystem;

        ProjectScanResult result{
            .rootPath = rootPath,
        };

        std::error_code error;

        if (!fs::exists(rootPath, error) || error) {
            return result;
        }

        if (!fs::is_directory(rootPath, error) || error) {
            return result;
        }

        fs::recursive_directory_iterator iterator{
            rootPath,
            fs::directory_options::skip_permission_denied,
            error,
        };

        if (error) {
            return result;
        }

        const fs::recursive_directory_iterator end;

        while (iterator != end) {
            const fs::directory_entry& entry = *iterator;
            const fs::path currentPath = entry.path();
            const fs::path relativePath = makeRelativePath(
                currentPath,
                rootPath
            );

            std::error_code entryError;

            if (entry.is_directory(entryError)) {
                const std::string directoryName =
                    currentPath.filename().string();

                if (isIgnoredDirectoryName(directoryName)) {
                    result.ignoredDirectories.push_back(relativePath);
                    iterator.disable_recursion_pending();
                }
            } else if (entry.is_regular_file(entryError)) {
                if (isSourceExtension(currentPath)) {
                    result.sourceFiles.push_back(relativePath);
                }

                if (isTestFile(relativePath)) {
                    result.testFiles.push_back(relativePath);
                }

                if (isImportantFileName(currentPath.filename().string())) {
                    result.importantFiles.push_back(relativePath);
                }
            }

            iterator.increment(error);

            if (error) {
                return result;
            }
        }

        sortPaths(result.sourceFiles);
        sortPaths(result.testFiles);
        sortPaths(result.importantFiles);
        sortPaths(result.ignoredDirectories);

        return result;
    }

} // namespace corvus::project