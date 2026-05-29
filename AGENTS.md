# AGENTS.md

## Project Overview

**Atlas** — open-source MMORPG server emulator (C++23). Fork of The Forgotten Server. Clients connect via [OTClient](https://github.com/mehah/otclient).

Toolchain: CMake 3.25+, vcpkg, Ninja. Core C++ library links Lua, spdlog, pugixml, OpenSSL, and Boost (iostreams). All game logic lives in `src/`; data and scripts live in `data/`.

## Setup Commands

### Prerequisites

- CMake 3.25+
- [vcpkg](https://github.com/microsoft/vcpkg) installed with `VCPKG_ROOT` env set
- **Windows:** Run `vcvarsall.bat x64` before building (Ninja uses `cl.exe` from PATH)

### CMake Presets

All presets use Ninja as the generator. `binaryDir` is `${sourceDir}/build/<presetName>`.

| Preset                  | Config                                                    |
| ----------------------- | --------------------------------------------------------- |
| `linux-release`         | RelWithDebInfo, vcpkg x64-linux                           |
| `linux-debug`           | Debug, export compile_commands.json, unity build disabled |
| `linux-release-tests`   | + `BUILD_TESTING=ON`                                      |
| `linux-debug-asan`      | Debug + ASan, + `BUILD_TESTING=ON`                        |
| `windows-release`       | RelWithDebInfo, vcpkg x64-windows-static-release          |
| `windows-debug`         | Debug, vcpkg x64-windows, unity build disabled            |
| `windows-release-tests` | + `BUILD_TESTING=ON`                                      |
| `windows-release-asan`  | + ASan, x64-windows, + `BUILD_TESTING=ON`                 |

### Optional Feature Flags

Cache variables passed via `-D` or `configurePresetAdditionalArgs`:

- `BUILD_TESTING=ON` — build Boost.Test unit tests
- `ENABLE_ASAN=ON` — enable AddressSanitizer for supported toolchains/builds; use for diagnostics/sanitizer-specific runs due to runtime overhead (set `ASAN_OPTIONS=exitcode=0` in CI)
- `OPTIONS_ENABLE_CCACHE=ON` / `OPTIONS_ENABLE_SCCACHE=ON` — compiler cache
- `OPTIONS_ENABLE_IPO=ON/OFF` — interprocedural optimization / LTO (default: auto-detected ON)

## Development Workflow

### Build

```bash
# Linux
cmake --preset=linux-release
cmake --build --preset=linux-release

# Windows (run `vcvarsall.bat x64` first)
cmake --preset=windows-release
cmake --build --preset=windows-release
```

### Run (local)

Copy `config.lua.dist` to `config.lua` and edit DB connection and game settings. Import schema first:

```bash
mysql -u atlas -p atlas < schema.sql
```

Run from project root:

```bash
./build/linux-release/tfs
```

Ports (defaults in `config.lua`): 7171 (status), 7172 (game protocol), 8080 (HTTP).

## Code Formatting

### C++

```bash
# Linux
cmake --build --preset=linux-release --target format

# Windows
cmake --build --preset=windows-release --target format
```

Style: `.clang-format` (Google, 120ch, tabs for indentation with width 4, `BreakBeforeBraces: Custom`).

### Lua

```bash
# Requires: luarocks install --server=https://luarocks.org/dev luaformatter
find . -name "*.lua" -not -path "./build/*" -exec luaformatter -i {} +
```

Linting: `.luacheckrc` (ignores 111, 631 file-wide; 113 in `data/`).

## Testing Instructions

```bash
cmake --preset=linux-release-tests
cmake --build --preset=linux-release-tests
ctest --preset=linux-release-tests
```

Tests are per-feature files (`test_<feature>.cpp`) under `src/tests/`. Each produces a standalone binary. Run a specific test directly by path:

```bash
./build/linux-release-tests/src/tests/test_base64
```

## Architecture

```
src/
  (flat C++ files) — all game logic compiles into `tfslib` CMake target
  lua/           — Lua C API bindings, module registry (api, env, error, meta)
  events/        — Lua event handlers (creature, monster, party, player)
  http/          — optional HTTP REST server (Boost.Beast, 8 modules)
  tests/         — Boost.Test unit tests (test_*.cpp)
  otserv.cpp     — main entry point
  otpch.h        — precompiled header (all sources `#include <otpch.h>`)
cmake/
  modules/       — custom CMake tooling (FindMySQL, LoggingHelper, MessageColors)
data/
  actions/       — item actions (.xml)
  chatchannels/  — channel routing config (.xml)
  items/         — client item definitions (items.xml, items.otb)
  lib/           — Lua-side game wrappers (core, compat, debugging: 30+ .lua files)
  logs/          — server logging configuration
  migrations/    — database versioning / schema updates
  monster/       — monster definitions (monsters.xml + monster/lua/ Lua event scripts)
  movements/     — movement event triggers (.xml + lib)
  npc/           — NPC configs (.xml) + npcsystem lib (.lua)
  scripts/       — NPC/talkaction/spell Lua scripts (attack, healing, conjuring, support, etc.)
  talkactions/   — NPC chat command definitions
  weapons/       — weapon behavior configs (.xml + .lua)
  world/         — map data (.otbm)
  XML/           — game data XML (vocational, groups, etc.)
```

Server config: `config.lua` (runtime settings). Env vars: `.env.example` (DB host/user/pass). MySQL backend uses libmariadb/libmysql via `src/database.cpp`.

## Build & Deploy

### Docker

Caches system packages (no vcpkg). Installs Boost.MariaDB from Ubuntu repos.

```bash
docker build -t atlas-server .
docker run -d -p 7171:7171 -p 7172:7172 -p 8080:8080 -v ./config.lua:/srv/config.lua atlas-server
```

### CI/CD

Workflows in `.github/workflows/` (run on `dev` branch pushes / PRs):

- `build-vcpkg.yml` — Linux + Windows release builds
- `test.yml` — unit tests (MariaDB service)
- `clang-format.yml` — C++ style check
- `lua-check.yml` — Lua linting
- `xml-syntax.yml` — XML validation
- `docker-image.yml` — Docker image build

## Pull Request Guidelines

- Follow the [coding style guide](https://github.com/otland/forgottenserver/wiki/TFS-Coding-Style-Guide)
- Use the PR template (`.github/PULL_REQUEST_TEMPLATE.md`)
- CI must pass: `build-vcpkg`, `test`, `clang-format`, `lua-check`, `xml-syntax`
- Title format: `[component] Brief description`

## Debugging & Troubleshooting

- Run `linux-debug-asan` / `windows-release-asan` to catch memory issues.
- Linux compile errors? Verify `CXX=g++-14` or newer. CI runs on g++-14.
- vcpkg dependency drift? Update `builtin-baseline` in `vcpkg.json` to pull newer port versions.
- DB connection failures? Verify `config.lua` mysql settings match your DB; `schema.sql` must be imported.
