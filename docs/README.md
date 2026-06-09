# Atlas documentation

This directory is the main documentation hub for the Atlas repository.

## Start here

- [Architecture and repository layout](architecture.md) — what lives in the C++ engine, data packs, and automation.
- [Build and run guides](building/README.md) — Linux, Windows, and Docker workflows.
- [Development and validation](development.md) — tests, benchmarks, Lua/XML checks, and contributor workflow.
- [Runtime configuration and operations](operations.md) — how the server starts, connects to MariaDB, and uses the generated config.

## What Atlas contains

Atlas is a server engine for Open Tibia-style worlds built from this repository's own C++23 codebase.

It includes:

- a C++ server runtime in [src/](../src)
- data-driven gameplay content in [data/](../data)
- Lua scripting support for actions, quests, systems, and item behavior
- MariaDB-backed persistence via [schema.sql](../schema.sql)
- CMake + vcpkg build automation, optional HTTP support, and Docker packaging
- CI workflows for tests, benchmarks, linting, and image publishing

Use the pages above to move from high-level understanding to practical setup and maintenance.
