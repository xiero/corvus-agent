#include "core/AgentEvent.hpp"

namespace corvus::core {
namespace {

    template <typename... Visitors>
    struct Overloaded : Visitors... {
        using Visitors::operator()...;
    };

    template <typename... Visitors>
    Overloaded(Visitors...) -> Overloaded<Visitors...>;


} //namsespace

std::string describeEvent(const AgentEvent& event) {
    return std::visit(
         Overloaded{
            [](const StartPlanning&) {
                return std::string{"StartPlanning"};
            },
            [](const StartToolExecution&) {
                return std::string{"StartToolExecution"};
            },
            [](const StartValidation&) {
                return std::string{"StartValidation"};
            },
            [](const Complete&) {
                return std::string{"Complete"};
            },
            [](const Reset&) {
                return std::string{"Reset"};
            },
            [](const Fail& fail) {
                return std::string{"Fail: "} + fail.reason;
            },
        },
        event
    );
}


} // namespace corvus::core