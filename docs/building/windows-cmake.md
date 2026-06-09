# Building on Windows with CMake

This is the supported Windows build path for Atlas.

## Prerequisites

1. Visual Studio 2022 with the Desktop development with C++ workload.
2. CMake 3.25 or newer.
3. vcpkg.

Install vcpkg:

```cmd
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
setx VCPKG_ROOT C:\vcpkg
```

Open a new terminal after setting `VCPKG_ROOT`.

## Build

Use the x64 Native Tools Command Prompt for Visual Studio 2022 so `cl.exe` is available:

```cmd
cmake --preset windows-release
cmake --build --preset windows-release
```

The first configure/build cycle can take 15 to 30 minutes because vcpkg compiles the dependency graph from source. Later runs reuse the cache.

The resulting binary is `build\windows-release\tfs.exe`.

## Other presets

```cmd
:: Debug build (dynamic CRT, unity build off)
cmake --preset windows-debug
cmake --build --preset windows-debug

:: Release build with unit tests
cmake --preset windows-release-tests
cmake --build --preset windows-release-tests

:: Release build with Address Sanitizer
cmake --preset windows-release-asan
cmake --build --preset windows-release-asan
```

## Running

Copy `config.lua.dist` to `config.lua`, adjust the configuration, and then run:

```cmd
build\windows-release\tfs.exe
```

## Troubleshooting

- `cl.exe not found`: use the x64 Native Tools Command Prompt, not a generic shell.
- vcpkg download failures: retry the configure step; transient GitHub or mirror outages are common.
- stale vcpkg cache or broken ports: update vcpkg with `git pull` and re-bootstrap.
- low disk space: remove old buildtrees or use `--clean-after-build` when possible.
