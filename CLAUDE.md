# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Build: `qmake && make`
- For Windows: Use Qt Creator with MSVC compiler
- For Linux: Use `CONFIG+=astralinux` for Astra Linux compatibility

## Code Style
- C++17 standard with Qt 5+ framework
- Use 4-space indentation, no tabs
- Prefix member variables with `m_` (e.g., `m_parameterModel`)
- Use PascalCase for class names, enums, and constants
- Use camelCase for methods and variables
- Classes in `ParamControl` namespace
- Use `#pragma once` for header files
- Document with Doxygen-style comments (`@brief`, `@param`)
- Use Qt types (QString, QVector, QMap) over STL equivalents where appropriate
- Prefer smart pointers (std::shared_ptr, std::unique_ptr) over raw pointers
- Use Qt's signal/slot mechanism for event handling
- Follow Model-View separation pattern
- Return bool for operations that can fail, check explicitly for errors