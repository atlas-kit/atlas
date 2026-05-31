# Compiling on Linux

Atlas builds on any modern Linux distribution with a C++23-capable compiler. You need either:

- **GCC 14+** (provides the C++23 `<print>` header in libstdc++), or
- **Clang 17+** paired with libstdc++ from GCC 14+ (install `g++-14` alongside) or with `libc++` (`-stdlib=libc++`, requires `libc++-18-dev` or newer).

A bare Clang 18 on Ubuntu 24.04 will fail with `fatal error: 'print' file not found` because the default libstdc++ ships with GCC 13.

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

> **Ubuntu 24.04 (noble)** does not ship `libsimdutf-dev`. Either upgrade to 24.10+/Debian 13+, or build simdutf from source:
>
> ```bash
> git clone --depth 1 --branch v5.7.2 https://github.com/simdutf/simdutf.git /tmp/simdutf
> cmake -G Ninja -S /tmp/simdutf -B /tmp/simdutf/build \
>   -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSIMDUTF_TOOLS=OFF -DSIMDUTF_BENCHMARKS=OFF
> cmake --build /tmp/simdutf/build && sudo cmake --install /tmp/simdutf/build
> ```

> **GCC 13 (Ubuntu 24.04 default)** is too old for `<print>`. Install GCC 14 and use it explicitly:
>
> ```bash
> sudo apt install -y g++-14
> export CC=gcc-14 CXX=g++-14
> ```

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
- **GCC version error / `'print' file not found`** — you need GCC 14+ for the C++23 features used by Atlas (`<print>`, `std::move_only_function`, `std::views::as_const`). On Ubuntu 24.04: `sudo apt install g++-14` and `export CC=gcc-14 CXX=g++-14`.
- **vcpkg builds slow on first run** — vcpkg compiles every dependency from source on the first invocation. Subsequent builds reuse the binary cache.
