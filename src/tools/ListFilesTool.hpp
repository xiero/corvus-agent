#pragma once 

#include "tools/Tool.hpp"

namespace corvus::tools {
    class ListFilesTool final : public Tool {
        public:
            [[nodiscard]] std::string_view name() const override;
            [[nodiscard]] std::string_view description() const override;

            [[nodiscard]] ToolResult execute(const ToolInput& input) const override;
    };
} // namesapce corvus::tools