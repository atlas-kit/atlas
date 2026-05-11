# Compiling with Docker

The repository ships a multi-stage `Dockerfile` based on Debian Forky Slim. It produces a small runtime image containing only the `tfs` binary and the libraries it needs.

## Building the image

From the repo root:

```bash
docker build -t atlas:local .
```

The build:

1. Installs the toolchain and library development packages in a builder stage.
2. Compiles Atlas with CMake (`RelWithDebInfo`).
3. Copies the resulting binary into a slim runtime stage with only the runtime libraries.

The final image exposes ports `7171` and `7172` and uses `/srv` as the working directory.

## Running

Mount your data directory (containing `config.lua`, the map, the data scripts, etc.) at `/srv`:

```bash
docker run -d --rm \
  -p 7171:7171 -p 7172:7172 \
  -v /path/to/your/server-data:/srv \
  --name atlas atlas:local
```

## Pulling pre-built images

CI publishes images on every push to `dev` and on every version tag at `ghcr.io/atlas-kit/atlas`:

```bash
docker pull ghcr.io/atlas-kit/atlas:dev
```

See [`.github/workflows/docker-image.yml`](../.github/workflows/docker-image.yml) for the publishing logic.
