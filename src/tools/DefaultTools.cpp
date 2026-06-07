#include "tools/DefaultTools.hpp"

#include "tools/EchoTool.hpp"
#include "tools/ListFilesTool.hpp"
#include "tools/ReadFileTool.hpp"

#include <memory>

namespace corvus::tools {

ToolRegistry createDefaultToolRegistry() {
    ToolRegistry registry;

    registry.registerTool(std::make_unique<EchoTool>());
    registry.registerTool(std::make_unique<ListFilesTool>());
    registry.registerTool(std::make_unique<ReadFileTool>());

    return registry;
}

} // namespace corvus::tools