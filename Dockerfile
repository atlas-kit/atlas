FROM debian:forky-slim AS build
RUN apt-get update -q && apt-get install -yq \
  build-essential \
  cmake \
  libboost-iostreams1.83-dev \
  libboost-json1.83-dev \
  libboost-system1.83-dev \
  liblua5.4-dev \
  libmariadb-dev \
  libpugixml-dev \
  libsimdutf-dev \
  libspdlog-dev \
  libssl-dev \
  ninja-build

COPY cmake /usr/src/atlas/cmake/
COPY src /usr/src/atlas/src/
COPY CMakeLists.txt /usr/src/atlas/
WORKDIR /usr/src/atlas
RUN cmake -G Ninja -B build/docker-release -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  && cmake --build build/docker-release

FROM debian:forky-slim
RUN apt-get update -q && apt-get install -yq \
  libboost-iostreams1.83.0 \
  libboost-json1.83.0 \
  liblua5.4-0 \
  libmariadb3 \
  libpugixml1v5 \
  libsimdutf33 \
  libspdlog1.15 \
  libssl3t64 \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/src/atlas/build/docker-release/tfs /bin/tfs
COPY LICENSE key.pem /srv/

EXPOSE 7171 7172
WORKDIR /srv
VOLUME /srv
ENTRYPOINT ["/bin/tfs"]
