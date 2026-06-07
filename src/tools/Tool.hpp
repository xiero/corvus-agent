#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>


namespace corvus::tools {
    
    struct ToolInput {
        std::map<std::string, std::string> arguments;
    };

    struct ToolResult {
        bool succeeded{false};
        std::vector<std::string> lines;
    };

    struct ToolMetadata
    {
        std::string name;
        std::string description;
    };

    class Tool {
        public:
            virtual ~Tool() = default;

            [[nodiscard]] virtual std::string_view name() const = 0;
            [[nodiscard]] virtual std::string_view description() const = 0;

            [[nodiscard]] virtual ToolResult execute( const ToolInput& input) const = 0;
    };
    
} // namespace corvus::tools