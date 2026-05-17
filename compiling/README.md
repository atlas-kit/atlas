# Compiling Atlas

Atlas can be compiled in multiple ways. Pick the guide that matches your environment:

| Platform | Guide | Notes |
|---|---|---|
| Linux | [linux.md](linux.md) | Recommended for production. CMake + vcpkg or system packages. |
| Windows | [windows-cmake.md](windows-cmake.md) | Uses vcpkg manifest and Ninja via CMake presets. |
| Docker | [docker.md](docker.md) | Self-contained image build, no host toolchain required. |

## Build options

The CMake build understands a few options that can be passed with `-D<NAME>=ON|OFF` at configure time:

| Option | Default | Description |
|---|---|---|
| `ENABLE_HTTP` | `ON` | Build the HTTP server module (Boost Beast + JSON). |
| `USE_LIBMYSQL` | `OFF` | Use `libmysql` instead of `libmariadb` for the SQL client. |
| `BUILD_TESTING` | `OFF` | Build unit tests (requires Boost.Test). |
| `ENABLE_ASAN` | `OFF` | Enable Address Sanitizer. |
| `ENABLE_UNITY_BUILD` | `ON` | Combine source files into unity translation units for faster builds. |
| `OPTIONS_ENABLE_IPO` | `ON` | Enable interprocedural optimization / link-time optimization (LTO). |
| `OPTIONS_ENABLE_CCACHE` | `OFF` | Use `ccache` as compiler launcher (Linux/macOS). |
| `OPTIONS_ENABLE_SCCACHE` | `OFF` | Use `sccache` as compiler launcher (cross-platform). |

## CMake presets

The repository ships with `CMakePresets.json` that exposes ready-to-use configurations:

| Preset | Platform | Build type | Tests | Notes |
|---|---|---|---|---|
| `linux-release` | Linux | RelWithDebInfo | no | |
| `linux-debug` | Linux | Debug | no | Disables unity build, exports `compile_commands.json`. |
| `linux-release-tests` | Linux | RelWithDebInfo | yes | |
| `linux-debug-asan` | Linux | Debug | yes | Address Sanitizer enabled. |
| `windows-release` | Windows | RelWithDebInfo | no | Static linking via `x64-windows-static-release` triplet. |
| `windows-debug` | Windows | Debug | no | Dynamic CRT, unity build off. |
| `windows-release-tests` | Windows | RelWithDebInfo | yes | |
| `windows-release-asan` | Windows | RelWithDebInfo | yes | Address Sanitizer enabled. |
| `vcpkg` | Any | Multi-config | configurable | Legacy preset using `Ninja Multi-Config`. |
