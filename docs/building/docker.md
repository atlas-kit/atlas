# Building with Docker

Atlas provides a multi-stage Docker build that packages the server binary and its runtime dependencies into a slim image.

## Build the image

From the repository root:

```bash
docker build -t atlas:local .
```

The build process:

1. installs the compiler and development libraries in the builder stage;
2. compiles Atlas with CMake using `RelWithDebInfo`; and
3. copies only the runtime artifacts into the final image.

The final image exposes ports `7171` and `7172` and uses `/srv` as the working directory.

## Run the container

Mount your server data directory at `/srv`:

```bash
docker run -d --rm \
  -p 7171:7171 -p 7172:7172 \
  -v /path/to/your/server-data:/srv \
  --name atlas atlas:local
```

The mounted directory should contain the runtime files expected by the server, including `config.lua`, the map data, and the Lua scripts.

## Pre-built images

CI publishes images for the `dev` branch and release tags to GitHub Container Registry.

```bash
docker pull ghcr.io/atlas-kit/atlas:dev
```

The publishing workflow is defined in [.github/workflows/docker-image.yml](../../.github/workflows/docker-image.yml).
