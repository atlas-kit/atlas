# Compiling on Linux

Atlas builds on any modern Linux distribution with a C++23-capable compiler (GCC 14+ or Clang 17+).

You can pick between two paths:

1. **System packages** — fastest if you trust your distro's library versions.
2. **vcpkg manifest** — reproducible builds, matches what CI uses.

## 1. With system packages (Debian/Ubuntu)

Install the toolchain and dependencies:

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake ninja-build pkg-config \
  libboost-dev libboost-iostreams-dev libboost-json-dev libboost-system-dev \
  liblua5.4-dev libmariadb-dev libpugixml-dev \
  libsimdutf-dev libspdlog-dev libssl-dev
```

> `libboost-dev` is required when `ENABLE_HTTP=ON` (the default) because Boost.Beast is header-only and ships only with that package on Debian/Ubuntu.

Configure and build:

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

## 2. With vcpkg manifest (reproducible)

Clone vcpkg and bootstrap it once:

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg
```

Then configure with one of the Linux presets:

```bash
cmake --preset linux-release
cmake --build --preset linux-release
```

The binary is at `build/linux-release/tfs`.

### Other Linux presets

```bash
# Debug build with compile_commands.json for IDE/LSP support
cmake --preset linux-debug && cmake --build --preset linux-debug

# Release with unit tests
cmake --preset linux-release-tests && cmake --build --preset linux-release-tests

# Debug with Address Sanitizer
cmake --preset linux-debug-asan && cmake --build --preset linux-debug-asan
```

## Running

After building, copy `config.lua.dist` to `config.lua`, edit it, then run from the repo root:

```bash
./build/linux-release/tfs   # or wherever your binary lives
```

## Troubleshooting

- **"Boost not found" / version too old** — Atlas requires Boost 1.71+ (1.75+ when HTTP is enabled). Install a newer Boost from your distro's backports or use the vcpkg path.
- **GCC version error** — you need GCC 14+ for the C++23 features used by Atlas (`std::move_only_function`, `std::views::as_const`).
- **vcpkg builds slow on first run** — vcpkg compiles every dependency from source on the first invocation. Subsequent builds reuse the binary cache.
