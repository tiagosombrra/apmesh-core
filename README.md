# AP Mesh Core

Greenfield C++23 scientific core for AP Mesh. Foundation and Geometry
Primitives are qualified in the declared WSL toolchain envelope. The current
Topological Model stage has implemented explicit vertex, edge and face identity,
ordered boundary cycles, deterministic edge-use incidence, structural incidence
classification, consistency validation and canonical snapshots. These topology
capabilities have passed focused contracts but the Topological Model stage is
not yet qualified. Curves, surfaces, patches, meshing, sizing and adaptation
remain unimplemented.

Start with `docs/APMESH_CORE_STATE.md`, then read
`docs/APMESH_CORE_ROADMAP.md`.

## Local build environments

The Linux and native-Windows development builds use separate CMake presets and
never share a build directory or cache. Scientific qualification remains scoped
to its declared WSL/cloud environments; a successful native-Windows build is
ordinary development evidence only.

On WSL Ubuntu 24.04, use the qualified GCC 13 development preset:

```sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset fast
```

On native Windows, install Visual Studio 2022 Build Tools with the C++ desktop
workload, then use the MSVC x64 preset:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-fast
```

Linux presets are unavailable on Windows and Windows presets are unavailable
on Linux, preventing the two environments from reusing each other's CMake
caches.

### Native-Windows checkpoint

The four B-spline/NURBS evaluation paths now keep private intermediates as a
normalized significand and a separate binary exponent. Their range no longer
depends on `long double` having a wider exponent range than `double`. Public
inputs, outputs, knot policies and explicit unrepresentable-result failures
are unchanged.

The GCC 13, Clang 18/libc++ and MSVC x64 Debug/Release development runs pass all 41
ordinary contracts, including the unchanged extreme-finite regressions and a
new scaled-arithmetic/analytic-jet contract. Native Windows remains outside
the scientific qualification envelope. The six-cell validation, resolved
GCC Release sorting blocker and retained numerical limitations are recorded in
[`the portability audit`](docs/audits/2026-09-26-portable-spline-arithmetic.md).
