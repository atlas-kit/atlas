# Architecture and repository layout

Atlas is organized around a small set of responsibilities: the core C++ server runtime, data-driven content, and automation for build, test, and packaging.

## Core runtime

The main engine lives in [src/](../src).

- [src/otserv.cpp](../src/otserv.cpp) starts the server, loads the configuration, opens the database, and initializes the game subsystems.
- [src/game.cpp](../src/game.cpp) and related files implement the game loop, world simulation, combat, items, monsters, houses, and player state.
- [src/protocol*.cpp](../src) and [src/networkmessage.cpp](../src/networkmessage.cpp) handle the client protocol and network transport.
- [src/http/](../src/http) provides the optional HTTP surface used for status or integration endpoints when `ENABLE_HTTP=ON`.
- [src/lua/](../src/lua) and [src/events/](../src/events) connect the C++ engine to Lua-powered scripting and event handling.

## Data and content

The gameplay content lives in [data/](../data).

- [data/actions/](../data/actions), [data/spells/](../data/spells), [data/weapons/](../data/weapons), and [data/talkactions/](../data/talkactions) hold gameplay definitions.
- [data/monster/](../data/monster), [data/items/](../data/items), [data/world/](../data/world), and [data/XML/](../data/XML) define monsters, items, world data, and XML configuration.
- [data/scripts/](../data/scripts) contains the Lua scripts used by the server runtime, including systems, quests, movements, and action handlers.
- [data/migrations/](../data/migrations) contains database migration scripts that update the schema over time.

## Configuration and persistence

- [config.lua.dist](../config.lua.dist) is the shipped runtime configuration template. It contains network ports, world rules, rates, MySQL settings, and startup flags.
- [schema.sql](../schema.sql) defines the MariaDB/MySQL schema used by the server.
- The runtime expects a working MariaDB instance and will copy `config.lua.dist` to `config.lua` on startup if needed.

## Build, testing, and packaging

- [CMakeLists.txt](../CMakeLists.txt) and [CMakePresets.json](../CMakePresets.json) define the main build pipeline.
- [vcpkg.json](../vcpkg.json) declares dependencies and optional features such as HTTP, Boost.MySQL, tests, and benchmarks.
- [src/tests/](../src/tests) contains unit tests, and [src/benchs/](../src/benchs) contains benchmark targets.
- [.github/workflows/](../.github/workflows) runs the CI jobs for builds, tests, benchmarks, Docker image publishing, and Lua/XML validation.

## Summary

The repository is intentionally split into:

1. a compiled C++ runtime for the server engine;
2. data-driven content and Lua scripts for gameplay behavior; and
3. automation to build, test, package, and validate the project.
