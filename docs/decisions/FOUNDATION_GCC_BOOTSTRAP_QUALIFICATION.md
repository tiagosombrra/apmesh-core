# Foundation GCC Bootstrap Qualification

Date: 2026-09-04

## Question

Can the minimal target-scoped C++23 `apmesh::core` skeleton configure, build,
and execute its smoke test from a clean GCC reference build tree?

## Scope

The implementation consists only of:

- the `apmesh::core` static-library target;
- `apmesh::core::inspect_bootstrap()` returning a typed bootstrap certificate;
- one CTest smoke executable.

It deliberately excludes geometry, topology, numerical computation, I/O, and
meshing behavior.

## Environment and command

| Item | Value |
| --- | --- |
| OS | WSL Ubuntu 24.04 |
| compiler | GCC 13.3.0 with libstdc++ |
| build system | CMake 3.28.3, Ninja 1.11.1 |
| preset | `gcc-debug` |

```text
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug -N
ctest --preset gcc-debug --output-on-failure
```

## Result

| Check | Result |
| --- | --- |
| configure | PASS |
| build | PASS |
| CTest discovery | PASS: 1 test |
| `apmesh_core.bootstrap_smoke` | PASS |

The smoke test creates no filesystem artifacts and verifies the target's typed
bootstrap certificate.

## Decision

The clean GCC project bootstrap is `QUALIFIED`. This is not a closure of the
Architecture Contract and does not qualify Clang/libc++, scientific behavior,
or any meshing capability.

## Follow-up

The corresponding Clang/libc++ qualification passed and is recorded in
`docs/decisions/FOUNDATION_CLANG_BOOTSTRAP_QUALIFICATION.md`.
