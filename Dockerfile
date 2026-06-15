FROM debian:trixie-slim AS build

# simdutf is only packaged in Debian testing/unstable (forky/sid), not in trixie
# (stable), so build it from source and link it statically into the server binary.
# This keeps the runtime image free of any simdutf dependency.
ARG SIMDUTF_VERSION=8.2.0

RUN apt-get update -q && apt-get install -yq \
  build-essential \
  ca-certificates \
  cmake \
  curl \
  libboost-iostreams-dev \
  libboost-json-dev \
  libboost-system-dev \
  liblua5.4-dev \
  libmariadb-dev \
  libpugixml-dev \
  libspdlog-dev \
  libssl-dev \
  ninja-build

# Build and install simdutf as a static, position-independent library. The CMake
# package config it installs is what `find_package(simdutf CONFIG REQUIRED)` consumes.
RUN curl -fsSL "https://github.com/simdutf/simdutf/archive/refs/tags/v${SIMDUTF_VERSION}.tar.gz" \
    | tar -xz -C /tmp \
  && cmake -G Ninja -S "/tmp/simdutf-${SIMDUTF_VERSION}" -B /tmp/simdutf-build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DSIMDUTF_TESTS=OFF \
    -DSIMDUTF_BENCHMARKS=OFF \
    -DSIMDUTF_TOOLS=OFF \
  && cmake --build /tmp/simdutf-build \
  && cmake --install /tmp/simdutf-build \
  && rm -rf /tmp/simdutf-build "/tmp/simdutf-${SIMDUTF_VERSION}"

COPY cmake /usr/src/atlas/cmake/
COPY src /usr/src/atlas/src/
COPY CMakeLists.txt /usr/src/atlas/
WORKDIR /usr/src/atlas
RUN cmake -G Ninja -B build/docker-release -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  && cmake --build build/docker-release

FROM debian:trixie-slim
RUN apt-get update -q && apt-get install -yq \
  libboost-iostreams1.83.0 \
  libboost-json1.83.0 \
  liblua5.4-0 \
  libmariadb3 \
  libpugixml1v5 \
  libspdlog1.15 \
  libssl3t64 \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/src/atlas/build/docker-release/tfs /bin/tfs
COPY LICENSE key.pem /srv/

EXPOSE 7171 7172
WORKDIR /srv
VOLUME /srv
ENTRYPOINT ["/bin/tfs"]
