cmake_minimum_required(VERSION 3.20)

project(corvus-agent
  VERSION 0.1.0
  DESCRIPTION "Engineering-grade AI TUI agent"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(FetchContent)

set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v6.1.9
)

FetchContent_MakeAvailable(ftxui)

add_library(corvus_core
  src/core/AgentConfig.cpp
  src/core/AgentState.cpp
  src/core/AgentEvent.cpp
  src/core/StateTransition.cpp
  src/core/AgentRuntime.cpp
  src/commands/CommandParser.cpp
  src/commands/CommandExecutor.cpp
)

target_include_directories(corvus_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(corvus_core
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_library(corvus_tui
  src/tui/TuiApp.cpp
)

target_include_directories(corvus_tui
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(corvus_tui
  PUBLIC
    corvus_core
  PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

target_compile_options(corvus_tui
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_library(corvus_cli
  src/cli/CliApp.cpp
)

target_include_directories(corvus_cli
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(corvus_cli
  PUBLIC
    corvus_core
  PRIVATE
    corvus_tui
)

target_compile_options(corvus_cli
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(corvus
  app/main.cpp
)

target_link_libraries(corvus
  PRIVATE
    corvus_cli
)

target_compile_options(corvus
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(BUILD_TESTING)
  add_executable(corvus_agent_config_tests
    tests/AgentConfigTests.cpp
  )

  target_link_libraries(corvus_agent_config_tests
    PRIVATE
      corvus_core
  )

  target_compile_options(corvus_agent_config_tests
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )

  add_test(NAME AgentConfigTests COMMAND corvus_agent_config_tests)

  add_executable(corvus_command_parser_tests
    tests/CommandParserTests.cpp
  )

  target_link_libraries(corvus_command_parser_tests
    PRIVATE
      corvus_core
  )

  target_compile_options(corvus_command_parser_tests
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )

  add_test(NAME CommandParserTests COMMAND corvus_command_parser_tests)

  add_executable(corvus_agent_state_machine_tests
    tests/AgentStateMachineTests.cpp
  )

  target_link_libraries(corvus_agent_state_machine_tests
    PRIVATE
      corvus_core
  )

  target_compile_options(corvus_agent_state_machine_tests
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )

  add_test(NAME AgentStateMachineTests COMMAND corvus_agent_state_machine_tests)
endif()