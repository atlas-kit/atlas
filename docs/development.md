# Development and validation

This project ships with a complete validation stack for C++, Lua, XML, and packaging workflows.

## Build and test automation

The main validation workflow is implemented in [.github/workflows/](../.github/workflows):

- `build-vcpkg.yml` builds the server with vcpkg on Linux and Windows.
- `test.yml` runs unit tests against both the standard database backend and the Boost.MySQL backend.
- `benchmarks.yml` builds and executes benchmark targets.
- `docker-image.yml` publishes container images.
- `clang-format.yml` validates C++ formatting.
- `lua-check.yml` validates Lua syntax, linting, and formatting.
- `xml-syntax.yml` validates the XML data files.

## Tests and benchmarks

- [src/tests/](../src/tests) contains unit tests for core functionality such as base64, database access, file loading, RSA, SHA1, XTEA, and matrix area logic.
- [src/benchs/](../src/benchs) contains benchmark targets for performance-sensitive code.
- [src/http/tests/](../src/http/tests) contains HTTP-specific tests when the HTTP component is enabled.

Enable these with the CMake options:

- `BUILD_TESTING=ON`
- `BUILD_BENCHMARKING=ON`

## Lua and XML quality checks

The repository includes validation rules for Lua and XML content:

- [.luacheckrc](../.luacheckrc) controls Lua linting behavior.
- The GitHub workflow for Lua checks runs `luac -p`, `luacheck`, and LuaFormatter.
- The XML workflow validates all XML files under `data/` with `xmllint`.

## Contributor workflow

When contributing changes:

1. keep the C++ implementation and the data files in sync;
2. use the existing CMake presets when validating the build;
3. run the relevant tests or checks for any changed area; and
4. prefer small, focused changes that preserve the repository's structure.
