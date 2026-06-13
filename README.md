# Corvus Agent

Corvus Agent is an engineering-grade, safety-aware AI TUI agent written in modern C++.

The goal is not to replace the engineer, but to amplify engineering control: Corvus should help plan, inspect, execute, validate, and explain work while keeping the human firmly in charge.

## Current milestone

Milestone: Runtime foundation + controlled tool execution

Implemented so far:

- C++20 project skeleton with CMake
- Thin `main.cpp` entry point
- CLI adapter via `CliApp`
- Interactive shell mode
- FTXUI-based TUI cockpit
- Shared command parser and command executor
- Explicit agent runtime state machine
- Tool interface and tool registry
- Built-in read-only tools:
  - `echo`
  - `list_files`
  - `read_file`
- Safety Engine v1 for tool execution policy
- Unit tests for config, command parsing, state transitions, tools, and safety rules

## Architecture

Current high-level flow:

```text
app/main.cpp
  -> corvus::cli::CliApp
      -> shell mode
      -> corvus::tui::TuiApp
  -> corvus::commands::CommandParser
  -> corvus::commands::CommandExecutor
  -> corvus::core::AgentRuntime
  -> corvus::safety::SafetyEngine
  -> corvus::tools::ToolRegistry
  -> corvus::tools::Tool
```

The main executable is intentionally thin. Application behavior lives in explicit layers:

```text
main.cpp
  only process entry point

CliApp
  CLI argument handling, shell startup, TUI startup

TuiApp
  terminal UI rendering and input collection

CommandParser
  user text -> typed command

CommandExecutor
  command -> runtime action / tool request

AgentRuntime
  current agent state

StateTransition
  allowed and denied state transitions

SafetyEngine
  evaluates tool calls before execution

ToolRegistry
  stores and runs registered tools
```

## Agent states

Corvus currently uses an explicit runtime state machine:

```text
Idle
Planning
ToolExecution
Validation
FinalResponse
Error
```

Typical happy path:

```text
Idle
  -> Planning
  -> ToolExecution
  -> Validation
  -> FinalResponse
  -> Idle
```

State transitions are explicit and tested. Invalid transitions are rejected instead of silently mutating runtime state.

## Available commands

The same command execution layer is used by both the shell and the TUI.

```text
:help
:version
:config
:state

:plan
:tool
:validate
:finish
:reset
:fail

:tool list
:tool run echo message=hello
:tool run list_files path=.
:tool run read_file path=README.md

:exit
```

Important distinction:

```text
:tool
```

moves the runtime from `Planning` to `ToolExecution`.

```text
:tool list
:tool run ...
```

interacts with the tool registry.

## Tool safety

Tool calls are evaluated by the Safety Engine before execution.

Current policy:

```text
echo                     Allowed
list_files path=.        Allowed
read_file path=README.md Allowed

absolute paths           Denied
path traversal           Denied
.git paths               Denied
home shortcuts (~)       Denied

write_file               NeedsConfirmation
shell                    NeedsConfirmation / Denied for dangerous commands
unknown tools            Denied
```

Examples:

```text
:tool run read_file path=README.md
```

is allowed after entering `ToolExecution` state.

```text
:tool run read_file path=../secret.txt
```

is denied by the safety engine.

```text
:tool run read_file path=.git/config
```

is denied by the safety engine.

## Requirements

- CMake 3.20+
- C++20-capable compiler
- Git
- Internet access on first configure/build, because FTXUI is fetched through CMake `FetchContent`

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Run the CLI help:

```bash
./build/corvus --help
```

Run version output:

```bash
./build/corvus --version
```

Run shell mode:

```bash
./build/corvus --shell
```

Run TUI mode:

```bash
./build/corvus --tui
```

If CMake does not pick up newly added source files, recreate the build directory:

```bash
rm -rf build
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Shell example

```text
:state
:plan
:tool
:tool list
:tool run echo message=hello
:tool run list_files path=.
:tool run read_file path=README.md
:validate
:finish
:reset
:exit
```

## TUI example

```bash
./build/corvus --tui
```

Then try:

```text
:help
:state
:plan
:tool
:tool list
:tool run echo message=hello
:tool run read_file path=README.md
```

The left panel shows the current runtime state. The log panel shows command output, tool results, and safety decisions.

## Tests

Current test targets:

```text
AgentConfigTests
CommandParserTests
AgentStateMachineTests
ToolRegistryTests
SafetyEngineTests
```

Run all tests:

```bash
ctest --test-dir build --output-on-failure
```

## Project structure

```text
app/
  main.cpp

src/
  cli/
    CliApp.hpp
    CliApp.cpp

  commands/
    Command.hpp
    CommandParser.hpp
    CommandParser.cpp
    CommandExecutor.hpp
    CommandExecutor.cpp

  core/
    AgentConfig.hpp
    AgentConfig.cpp
    AgentEvent.hpp
    AgentEvent.cpp
    AgentRuntime.hpp
    AgentRuntime.cpp
    AgentState.hpp
    AgentState.cpp
    StateTransition.hpp
    StateTransition.cpp

  safety/
    RiskLevel.hpp
    RiskLevel.cpp
    SafetyPolicy.hpp
    SafetyPolicy.cpp
    SafetyEngine.hpp
    SafetyEngine.cpp

  tools/
    Tool.hpp
    ToolRegistry.hpp
    ToolRegistry.cpp
    EchoTool.hpp
    EchoTool.cpp
    ListFilesTool.hpp
    ListFilesTool.cpp
    ReadFileTool.hpp
    ReadFileTool.cpp
    DefaultTools.hpp
    DefaultTools.cpp

  tui/
    TuiApp.hpp
    TuiApp.cpp

tests/
  AgentConfigTests.cpp
  CommandParserTests.cpp
  AgentStateMachineTests.cpp
  ToolRegistryTests.cpp
  SafetyEngineTests.cpp
```

## Development principles

Corvus is being built around a few core rules:

- Keep `main.cpp` almost invisible.
- Keep core logic testable without the TUI.
- Keep UI adapters thin.
- Share command execution between shell and TUI.
- Make runtime state explicit.
- Run tools only through the registry.
- Evaluate tool calls through the safety engine before execution.
- Prefer read-only behavior first.
- Avoid hidden side effects.
- Follow the C++ Core Guidelines where practical.

## Roadmap

Near-term next steps:

- Confirmation flow for `NeedsConfirmation` safety decisions
- Better tool argument parsing, including quoted values
- Project root / workspace boundary awareness
- Structured command output objects
- Tool execution audit log
- First planning loop skeleton
- Validation stage integration
- LLM adapter boundary
- Diff-preview workflow for future write operations

Longer-term direction:

- TUI-first engineering cockpit
- CLI adapter
- Telegram / Discord adapters through a shared gateway
- Webhook/API adapter
- Policy-based remote access
- Deterministic audit and replay support
- Project-aware local engineering agent runtime
