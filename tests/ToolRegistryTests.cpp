#include "tools/DefaultTools.hpp"
#include "tools/Tool.hpp"
#include "tools/ToolRegistry.hpp"

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
                "corvus_tool_registry_test_"
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

bool containsSubstring(
    const std::vector<std::string>& lines,
    const std::string& expected
) {
    for (const std::string& line : lines) {
        if (line.find(expected) != std::string::npos) {
            return true;
        }
    }

    return false;
}

void testDefaultRegistryContainsExpectedTools() {
    const corvus::tools::ToolRegistry registry =
        corvus::tools::createDefaultToolRegistry();

    const std::vector<corvus::tools::ToolMetadata> tools =
        registry.listTools();

    require(
        containsSubstring(
            {
                tools.at(0).name,
                tools.at(1).name,
                tools.at(2).name,
            },
            "echo"
        ),
        "default registry should contain echo"
    );

    require(registry.find("echo") != nullptr, "echo tool should exist");
    require(
        registry.find("list_files") != nullptr,
        "list_files tool should exist"
    );
    require(
        registry.find("read_file") != nullptr,
        "read_file tool should exist"
    );
}

void testEchoToolRuns() {
    const corvus::tools::ToolRegistry registry =
        corvus::tools::createDefaultToolRegistry();

    corvus::tools::ToolInput input;
    input.arguments["message"] = "hello-corvus";

    const corvus::tools::ToolResult result =
        registry.run("echo", input);

    require(result.succeeded, "echo should succeed");
    require(
        containsSubstring(result.lines, "hello-corvus"),
        "echo should return the message"
    );
}

void testMissingToolFails() {
    const corvus::tools::ToolRegistry registry =
        corvus::tools::createDefaultToolRegistry();

    const corvus::tools::ToolResult result =
        registry.run("missing_tool", corvus::tools::ToolInput{});

    require(!result.succeeded, "missing tool should fail");
    require(
        containsSubstring(result.lines, "Tool not found"),
        "missing tool should explain failure"
    );
}

void testListFilesToolRuns() {
    TemporaryDirectory temporaryDirectory;

    {
        std::ofstream file{temporaryDirectory.path / "alpha.txt"};
        file << "alpha\n";
    }

    std::filesystem::create_directory(temporaryDirectory.path / "nested");

    const corvus::tools::ToolRegistry registry =
        corvus::tools::createDefaultToolRegistry();

    corvus::tools::ToolInput input;
    input.arguments["path"] = temporaryDirectory.path.string();

    const corvus::tools::ToolResult result =
        registry.run("list_files", input);

    require(result.succeeded, "list_files should succeed");
    require(
        containsSubstring(result.lines, "alpha.txt"),
        "list_files should include regular files"
    );
    require(
        containsSubstring(result.lines, "nested/"),
        "list_files should mark directories"
    );
}

void testReadFileToolRuns() {
    TemporaryDirectory temporaryDirectory;

    const std::filesystem::path filePath =
        temporaryDirectory.path / "note.txt";

    {
        std::ofstream file{filePath};
        file << "line-one\n";
        file << "line-two\n";
    }

    const corvus::tools::ToolRegistry registry =
        corvus::tools::createDefaultToolRegistry();

    corvus::tools::ToolInput input;
    input.arguments["path"] = filePath.string();

    const corvus::tools::ToolResult result =
        registry.run("read_file", input);

    require(result.succeeded, "read_file should succeed");
    require(
        containsSubstring(result.lines, "line-one"),
        "read_file should include first line"
    );
    require(
        containsSubstring(result.lines, "line-two"),
        "read_file should include second line"
    );
}

} // namespace

int main() {
    testDefaultRegistryContainsExpectedTools();
    testEchoToolRuns();
    testMissingToolFails();
    testListFilesToolRuns();
    testReadFileToolRuns();

    std::cout << "ToolRegistryTests passed\n";
    return 0;
}