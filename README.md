# Corvus Agent

Corvus Agent is an engineering-grade AI TUI agent written in modern C++.

The goal is not to replace the engineer, but to amplify engineering control.

## Current milestone

Milestone 1: Hello Corvus

- C++20 project skeleton
- CMake build
- `corvus --help`
- `corvus --version`
- first unit test

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
./build/corvus --help
./build/corvus --version
```
