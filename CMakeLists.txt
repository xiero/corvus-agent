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

add_library(corvus_core
  src/core/AgentConfig.cpp
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

add_executable(corvus
  app/main.cpp
)

target_link_libraries(corvus
  PRIVATE
    corvus_core
)

target_compile_options(corvus
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(BUILD_TESTING)
  add_executable(corvus_tests
    tests/AgentConfigTests.cpp
  )

  target_link_libraries(corvus_tests
    PRIVATE
      corvus_core
  )

  target_compile_options(corvus_tests
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )

  add_test(NAME AgentConfigTests COMMAND corvus_tests)
endif()