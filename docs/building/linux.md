# Building on Linux

Atlas builds on modern Linux distributions with a C++23-capable compiler.

## Requirements

You need either:

- GCC 14+; or
- Clang 17+ with a compatible standard library.

The repository uses C++23 language and library features such as `std::print`, `std::move_only_function`, and `std::views::as_const`. On Ubuntu 24.04, the default GCC 13 toolchain is too old for this project.

## Option 1: system packages

Install the toolchain and dependencies:

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake ninja-build pkg-config \
  libboost-dev libboost-iostreams-dev libboost-json-dev libboost-system-dev \
  liblua5.4-dev libmariadb-dev libpugixml-dev \
  libsimdutf-dev libspdlog-dev libssl-dev
```

If your distro does not ship `libsimdutf-dev`, build it from source first.

For Ubuntu 24.04, install GCC 14 explicitly:

```bash
sudo apt install -y g++-14
export CC=gcc-14 CXX=g++-14
```

Then configure and build:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

The resulting binary is `build/tfs`.

### Fedora / RHEL

```bash
sudo dnf install -y \
  gcc-c++ cmake ninja-build pkg-config \
  boost-devel lua-devel mariadb-connector-c-devel \
  pugixml-devel simdutf-devel spdlog-devel openssl-devel
```

### Arch / Manjaro

```bash
sudo pacman -S --needed \
  base-devel cmake ninja \
  boost lua mariadb-libs pugixml simdutf spdlog openssl
```

## Option 2: vcpkg manifest

This is the reproducible path used by CI.

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg
```

Then use the CMake presets:

```bash
cmake --preset linux-release
cmake --build --preset linux-release
```

The binary is placed under `build/linux-release/tfs`.

### Useful preset variants

```bash
# Debug build with compile_commands.json for editor integration
cmake --preset linux-debug && cmake --build --preset linux-debug

# Release build with unit tests
cmake --preset linux-release-tests && cmake --build --preset linux-release-tests

# Debug build with Address Sanitizer
cmake --preset linux-debug-asan && cmake --build --preset linux-debug-asan
```

## Running the server

After building, copy `config.lua.dist` to `config.lua`, update the database and network settings, and run the binary from the repository root:

```bash
./build/linux-release/tfs
```

## Troubleshooting

- `Boost not found` or outdated Boost versions: install a newer Boost package or use the vcpkg path.
- `fatal error: 'print' file not found`: switch to GCC 14+ or a compatible Clang/libstdc++ combination.
- First vcpkg builds are slow because dependencies are compiled from source. Later runs reuse the binary cache.
