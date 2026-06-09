# Atlas

[![Build with vcpkg](https://github.com/atlas-kit/atlas/actions/workflows/build-vcpkg.yml/badge.svg)](https://github.com/atlas-kit/atlas/actions/workflows/build-vcpkg.yml)
[![Docker](https://github.com/atlas-kit/atlas/actions/workflows/docker-image.yml/badge.svg)](https://github.com/atlas-kit/atlas/actions/workflows/docker-image.yml)

Atlas is the server engine in this repository: a modern C++23 server runtime with Lua scripting, MariaDB-backed persistence, optional HTTP support, and data-driven gameplay content.

## What is in this repository

This repository contains the actual Atlas implementation rather than a copied set of legacy instructions.

- C++ server runtime in [src/](src)
- gameplay content and scripts in [data/](data)
- runtime configuration in [config.lua.dist](config.lua.dist)
- database schema in [schema.sql](schema.sql)
- build automation in [CMakeLists.txt](CMakeLists.txt), [CMakePresets.json](CMakePresets.json), and [vcpkg.json](vcpkg.json)
- CI and packaging workflows in [.github/workflows/](.github/workflows)

## Highlights

- C++23 server core with modular game, protocol, and scripting components
- Lua-based gameplay scripting under [data/scripts/](data/scripts)
- MariaDB/MySQL persistence with migration scripts in [data/migrations/](data/migrations)
- optional HTTP integration via [src/http/](src/http)
- unit tests and benchmark targets under [src/tests/](src/tests) and [src/benchs/](src/benchs)
- Docker packaging and GitHub Actions automation

## Quick start

1. Build the server with the guide that matches your platform in [docs/building/README.md](docs/building/README.md).
2. Copy [config.lua.dist](config.lua.dist) to `config.lua` and update the MySQL and network settings.
3. Import [schema.sql](schema.sql) into your MariaDB/MySQL database.
4. Start the binary from the repository root.

## Documentation

- [Documentation hub](docs/README.md)
- [Architecture and repository layout](docs/architecture.md)
- [Build and run guides](docs/building/README.md)
- [Development and validation](docs/development.md)
- [Runtime configuration and operations](docs/operations.md)

## Repository structure at a glance

- [src/](src) — server engine, protocols, database access, HTTP layer, tests, and benchmarks
- [data/](data) — items, monsters, scripts, migrations, world data, and XML definitions
- [.github/workflows/](.github/workflows) — CI for build, test, benchmark, Docker, Lua, and XML validation

## License

Atlas is distributed under the terms of the project license in [LICENSE](LICENSE).
