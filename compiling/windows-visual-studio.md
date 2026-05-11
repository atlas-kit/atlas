# Compiling on Windows with Visual Studio

Atlas ships a Visual Studio solution at [`vc18/atlas.slnx`](../vc18/atlas.slnx) for developers who prefer the IDE.

## Prerequisites

1. **Visual Studio 2022** (17.10+) with the *Desktop development with C++* workload.
   - The project pins `<PlatformToolset>v145</PlatformToolset>`. If you are on an older Visual Studio that ships only `v143`/`v144`, either install the latest VS 2022 Preview or right-click the project → **Retarget Solution** before building.
2. **vcpkg** integrated with MSBuild:

   ```cmd
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   C:\vcpkg\vcpkg integrate install
   ```

   The last command is **required** so MSBuild loads `vcpkg.targets` and installs the dependencies declared in `vcpkg.json` automatically.

## Building from the IDE

1. Open `vc18/atlas.slnx` in Visual Studio.
2. Pick the `Release|x64` configuration in the toolbar.
3. **Build → Build Solution** (`Ctrl+Shift+B`).

The first build takes 15–30 minutes because vcpkg compiles all dependencies. The output binary is `vc18/x64/Release/atlas-x64.exe` along with required DLLs.

## Building from the command line

From the *x64 Native Tools Command Prompt for VS 2022*:

```cmd
cd vc18
msbuild atlas.vcxproj /p:Configuration=Release /p:Platform=x64 /m
```

> The `.vcxproj` already sets `VcpkgEnableManifest=true`, so passing it on the command line is unnecessary.

## Running

Copy `config.lua.dist` to `config.lua` in the repo root, edit it, then run:

```cmd
vc18\x64\Release\atlas-x64.exe
```

## Troubleshooting

- **`error MSB8020: build tools for v145 cannot be found`** — your Visual Studio doesn't have the `v145` toolset. Either update VS or override the toolset on the command line:

  ```cmd
  msbuild atlas.vcxproj /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 ...
  ```

- **`Cannot open include file: 'boost/...'`** — vcpkg integration is missing or the manifest install was skipped. Re-run `C:\vcpkg\vcpkg integrate install` and rebuild.
- **`liblzma:x64-windows BUILD_FAILED`** — your local vcpkg is out of date. Run `git pull && bootstrap-vcpkg.bat` inside `C:\vcpkg`.

> If you'd rather not maintain the `.vcxproj`, the [CMake-based Windows build](windows-cmake.md) is what CI uses and produces the same artifact.
