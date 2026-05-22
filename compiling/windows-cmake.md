# Compiling on Windows with CMake

This is the recommended way to build Atlas on Windows. It uses the same toolchain as CI: vcpkg manifest mode + Ninja via CMake presets.

## Prerequisites

1. **Visual Studio 2022** with the *Desktop development with C++* workload (only the MSVC compiler and Windows SDK are needed — you do not need to open the IDE).
2. **CMake 3.25+** — included with VS 2022; otherwise install from [cmake.org](https://cmake.org/download/).
3. **vcpkg**:

   ```cmd
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   setx VCPKG_ROOT C:\vcpkg
   ```

   Open a new terminal so `VCPKG_ROOT` is picked up.

## Building

Open the **x64 Native Tools Command Prompt for VS 2022** (so `cl.exe` is on `PATH`) and from the repo root run:

```cmd
cmake --preset windows-release
cmake --build --preset windows-release
```

The first run takes 15–30 minutes because vcpkg compiles all dependencies from source. Subsequent runs reuse the binary cache.

The resulting binary is at `build\windows-release\tfs.exe`.

## Other Windows presets

```cmd
:: Debug build (dynamic CRT, unity build off)
cmake --preset windows-debug
cmake --build --preset windows-debug

:: Release with unit tests
cmake --preset windows-release-tests
cmake --build --preset windows-release-tests

:: Release with Address Sanitizer
cmake --preset windows-release-asan
cmake --build --preset windows-release-asan
```

## Running

Copy `config.lua.dist` to `config.lua` in the repo root and edit it. Then:

```cmd
build\windows-release\tfs.exe
```

## Troubleshooting

- **`cl.exe not found`** — open *x64 Native Tools Command Prompt for VS 2022*, not a regular `cmd`/PowerShell.
- **vcpkg fails to download a port (HTTP 502)** — transient GitHub outage; re-run the configure step.
- **`liblzma:x64-windows` build failure on a stale vcpkg** — update vcpkg:

  ```cmd
  cd C:\vcpkg
  git pull
  .\bootstrap-vcpkg.bat
  ```
- **Out of disk space** — vcpkg buildtrees can grow large. Add `--clean-after-build` or delete `C:\vcpkg\buildtrees` periodically.
