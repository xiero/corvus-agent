#include "project/ProjectScanner.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
    void require(bool condition, const std::string& message) {
        if (!condition) {
            std::cerr << "Test failed: " << message << "\n";
            std::exit(1);
        }
    }

    struct TemporaryDirectory {
        TemporaryDirectory()
            : path{
                std::filesystem::temp_directory_path()
                / (
                    "corvus_project_scanner_test_"
                    + std::to_string(
                        std::chrono::steady_clock::now()
                            .time_since_epoch()
                            .count()
                    )
                )
            } {
            std::filesystem::create_directories(path);
        }

        ~TemporaryDirectory() {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }

        std::filesystem::path path;
    };

    void writeFile(const std::filesystem::path& path) {
        std::filesystem::create_directories(path.parent_path());

        std::ofstream file{path};
        file << "test\n";
    }

    [[nodiscard]] bool containsPath(
        const std::vector<std::filesystem::path>& paths,
        const std::filesystem::path& expected
    ) {
        for (const std::filesystem::path& path : paths) {
            if (path == expected) {
                return true;
            }
        }

        return false;
    }

    void testNonExistingRootReturnsEmptyResult() {
        const corvus::project::ProjectScanner scanner;

        const std::filesystem::path missingPath =
            std::filesystem::temp_directory_path()
            / "corvus_missing_project_scanner_root";

        const corvus::project::ProjectScanResult result =
            scanner.scan(missingPath);

        require(result.sourceFiles.empty(), "source files should be empty");
        require(result.testFiles.empty(), "test files should be empty");
        require(
            result.importantFiles.empty(),
            "important files should be empty"
        );
        require(
            result.ignoredDirectories.empty(),
            "ignored directories should be empty"
        );
    }

    void testEmptyDirectoryReturnsEmptyResult() {
        TemporaryDirectory temporaryDirectory;
        const corvus::project::ProjectScanner scanner;

        const corvus::project::ProjectScanResult result =
            scanner.scan(temporaryDirectory.path);

        require(result.sourceFiles.empty(), "source files should be empty");
        require(result.testFiles.empty(), "test files should be empty");
        require(
            result.importantFiles.empty(),
            "important files should be empty"
        );
        require(
            result.ignoredDirectories.empty(),
            "ignored directories should be empty"
        );
    }

    void testSourceFilesAreDetected() {
        TemporaryDirectory temporaryDirectory;

        writeFile(temporaryDirectory.path / "src/main.cpp");
        writeFile(temporaryDirectory.path / "src/core/Agent.hpp");
        writeFile(temporaryDirectory.path / "src/core/Agent.cpp");

        const corvus::project::ProjectScanner scanner;
        const corvus::project::ProjectScanResult result =
            scanner.scan(temporaryDirectory.path);

        require(
            containsPath(result.sourceFiles, "src/main.cpp"),
            "main.cpp should be detected as source"
        );
        require(
            containsPath(result.sourceFiles, "src/core/Agent.hpp"),
            "Agent.hpp should be detected as source"
        );
        require(
            containsPath(result.sourceFiles, "src/core/Agent.cpp"),
            "Agent.cpp should be detected as source"
        );
    }

    void testTestFilesAreDetected() {
        TemporaryDirectory temporaryDirectory;

        writeFile(temporaryDirectory.path / "tests/AgentConfigTests.cpp");
        writeFile(temporaryDirectory.path / "src/core/StateMachineTest.cpp");

        const corvus::project::ProjectScanner scanner;
        const corvus::project::ProjectScanResult result =
            scanner.scan(temporaryDirectory.path);

        require(
            containsPath(result.testFiles, "tests/AgentConfigTests.cpp"),
            "tests directory file should be detected as test file"
        );
        require(
            containsPath(result.testFiles, "src/core/StateMachineTest.cpp"),
            "filename containing Test should be detected as test file"
        );
    }

    void testImportantFilesAreDetected() {
        TemporaryDirectory temporaryDirectory;

        writeFile(temporaryDirectory.path / "README.md");
        writeFile(temporaryDirectory.path / "CMakeLists.txt");
        writeFile(temporaryDirectory.path / "docs/ARCHITECTURE.md");
        writeFile(temporaryDirectory.path / "docs/AI_CONTEXT.md");
        writeFile(temporaryDirectory.path / "tasks.md");

        const corvus::project::ProjectScanner scanner;
        const corvus::project::ProjectScanResult result =
            scanner.scan(temporaryDirectory.path);

        require(
            containsPath(result.importantFiles, "README.md"),
            "README.md should be important"
        );
        require(
            containsPath(result.importantFiles, "CMakeLists.txt"),
            "CMakeLists.txt should be important"
        );
        require(
            containsPath(result.importantFiles, "docs/ARCHITECTURE.md"),
            "ARCHITECTURE.md should be important"
        );
        require(
            containsPath(result.importantFiles, "docs/AI_CONTEXT.md"),
            "AI_CONTEXT.md should be important"
        );
        require(
            containsPath(result.importantFiles, "tasks.md"),
            "tasks.md should be important"
        );
    }

    void testIgnoredDirectoriesAreSkipped() {
        TemporaryDirectory temporaryDirectory;

        writeFile(temporaryDirectory.path / ".git/config");
        writeFile(temporaryDirectory.path / "build/generated.cpp");
        writeFile(temporaryDirectory.path / "node_modules/package/index.js");
        writeFile(temporaryDirectory.path / ".cache/tmp.txt");
        writeFile(temporaryDirectory.path / "src/RealFile.cpp");

        const corvus::project::ProjectScanner scanner;
        const corvus::project::ProjectScanResult result =
            scanner.scan(temporaryDirectory.path);

        require(
            containsPath(result.ignoredDirectories, ".git"),
            ".git should be listed as ignored"
        );
        require(
            containsPath(result.ignoredDirectories, "build"),
            "build should be listed as ignored"
        );
        require(
            containsPath(result.ignoredDirectories, "node_modules"),
            "node_modules should be listed as ignored"
        );
        require(
            containsPath(result.ignoredDirectories, ".cache"),
            ".cache should be listed as ignored"
        );

        require(
            !containsPath(result.sourceFiles, "build/generated.cpp"),
            "source file inside ignored build directory should be skipped"
        );
        require(
            containsPath(result.sourceFiles, "src/RealFile.cpp"),
            "source file outside ignored directories should be detected"
        );
    }

} // namespace

int main() {
    testNonExistingRootReturnsEmptyResult();
    testEmptyDirectoryReturnsEmptyResult();
    testSourceFilesAreDetected();
    testTestFilesAreDetected();
    testImportantFilesAreDetected();
    testIgnoredDirectoriesAreSkipped();

    std::cout << "ProjectScannerTests passed\n";
    return 0;
}