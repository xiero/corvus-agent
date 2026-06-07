#include "tools/ToolRegistry.hpp"

#include <string>
#include <utility>

namespace corvus::tools {

void ToolRegistry::registerTool(std::unique_ptr<Tool> tool) {
    if (tool == nullptr) {
        return;
    }

    if (find(tool->name()) != nullptr) {
        return;
    }

    tools_.push_back(std::move(tool));
}

const Tool* ToolRegistry::find(std::string_view name) const {
    for (const std::unique_ptr<Tool>& tool : tools_) {
        if (tool->name() == name) {
            return tool.get();
        }
    }

    return nullptr;
}

std::vector<ToolMetadata> ToolRegistry::listTools() const {
    std::vector<ToolMetadata> result;

    for (const std::unique_ptr<Tool>& tool : tools_) {
        result.push_back(
            ToolMetadata{
                .name = std::string{tool->name()},
                .description = std::string{tool->description()},
            }
        );
    }

    return result;
}

ToolResult ToolRegistry::run(
    std::string_view name,
    const ToolInput& input
) const {
    const Tool* tool = find(name);

    if (tool == nullptr) {
        return ToolResult{
            .succeeded = false,
            .lines = {
                "Tool not found: " + std::string{name},
            },
        };
    }

    return tool->execute(input);
}

} // namespace corvus::tools