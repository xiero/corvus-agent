#include "tools/EchoTool.hpp"

namespace corvus::tools {
    namespace {

        [[nodiscard]] std::string findMessage(const ToolInput& input) {
            const auto message = input.arguments.find("message");

            if (message != input.arguments.end()) {
                return message->second;
            }

            const auto text = input.arguments.find("text");

            if (text != input.arguments.end()) {
                return text->second;
            }

            return {};
        }

        } // namespace

        std::string_view EchoTool::name() const {
            return "echo";
        }

        std::string_view EchoTool::description() const {
            return "Echoes a message back to the user.";
        }

        ToolResult EchoTool::execute(const ToolInput& input) const {
            const std::string message = findMessage(input);

            if (message.empty()) {
                return ToolResult{
                    .succeeded = false,
                    .lines = {
                        "Missing argument.",
                        "Usage: :tool run echo message=<text>",
                    },
                };
            }

            return ToolResult{
                .succeeded = true,
                .lines = {
                    message,
                },
            };
    }

} // namespace corvus::tools