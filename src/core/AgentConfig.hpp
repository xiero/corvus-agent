#pragma once

#include <string>

namespace corvus::core {

class AgentConfig {
public:
    AgentConfig();

    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] const std::string& version() const;

    [[nodiscard]] bool deterministicMode() const;
    [[nodiscard]] bool mentorMode() const;

    void setDeterministicMode(bool enabled);
    void setMentorMode(bool enabled);

private:
    std::string name_;
    std::string version_;
    bool deterministicMode_;
    bool mentorMode_;
};

} // namespace corvus::core