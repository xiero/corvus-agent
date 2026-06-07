#pragma once

#include "tools/Tool.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace corvus::tools {
    class ToolRegistry {
        public:
            void registerTool(std::unique_ptr<Tool> tool);

            [[nodiscard]] const Tool* find(std::string_view name)const;
            [[nodiscard]] std::vector<ToolMetadata> listTools() const;

            [[nodiscard]] ToolResult run(
                std::string_view name,
                const ToolInput& input
            ) const;

        private:
            std::vector<std::unique_ptr<Tool>> tools_;
    };
}