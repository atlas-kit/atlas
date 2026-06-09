# Building Atlas

Choose the build path that matches your environment.

## Build options at a glance

| Path | Best for | Notes |
| --- | --- | --- |
| [linux.md](linux.md) | Linux development and production | Uses CMake and vcpkg or system packages. |
| [windows-cmake.md](windows-cmake.md) | Windows development | Recommended Microsoft Visual Studio + CMake + vcpkg path. |
| [docker.md](docker.md) | Containers and reproducible packaging | Builds the server into a slim runtime image. |
| [windows-visual-studio.md](windows-visual-studio.md) | Legacy reference only | Retired path; the CMake workflow is the supported one. |

## What the build system supports

The repository uses CMake and vcpkg with a few optional features:

- `ENABLE_HTTP=ON` enables the HTTP support layer.
- `USE_LIBMYSQL=OFF` uses libmariadb by default; `ON` switches to libmysql.
- `USE_BOOST_MYSQL=ON` switches the database backend to Boost.MySQL.
- `BUILD_TESTING=ON` adds unit tests.
- `BUILD_BENCHMARKING=ON` adds benchmark executables.
- `ENABLE_ASAN=ON`, `ENABLE_UNITY_BUILD`, `OPTIONS_ENABLE_CCACHE`, and `OPTIONS_ENABLE_SCCACHE` are available for development and CI workflows.

The root CMake presets in [CMakePresets.json](../../CMakePresets.json) cover Linux and Windows release/debug/test/benchmark configurations.

## Recommended first step

If you are new to the project:

1. Review the runtime requirements in [../operations.md](../operations.md).
2. Pick the build guide for your platform.
3. Copy [config.lua.dist](../../config.lua.dist) to `config.lua` before starting the server.
