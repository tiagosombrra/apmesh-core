# Foundation Toolchain Baseline Qualification

Date: 2026-09-04

## Question

Can the local WSL Ubuntu 24.04 environment compile and execute the C++23
standard-library subset required before creating the first `apmesh::core`
target?

## Environment

| Role | Toolchain |
| --- | --- |
| primary reference | GCC 13.3.0 with libstdc++ |
| secondary qualification | Clang 18.1.3 with libc++ 18.1.3 |
| build system available | CMake 3.28.3, Ninja 1.11.1 |

The libc++ 18 development and ABI packages were installed in Ubuntu 24.04
before the Clang probe.

## Evidence

Each compiler compiled and executed the same program with:

```text
-std=c++23 -Wall -Wextra -Wpedantic -Werror
```

The program includes `<expected>`, requires `__cpp_lib_expected >= 202202L`,
constructs successful and error `std::expected<int, Error>` values, and returns
success only when both states behave as declared.

| Compiler | Compile | Execute |
| --- | --- | --- |
| GCC 13.3.0 | PASS | PASS |
| Clang 18.1.3 + libc++ 18.1.3 | PASS | PASS |

## Decision

The local language/standard-library baseline is `QUALIFIED` for the bootstrap
probe. GCC 13.3.0 is the primary reference and Clang 18.1.3 with libc++ is the
secondary qualification environment.

## GCC project-bootstrap evidence

The repository now contains one target-scoped C++23 static library,
`apmesh::core`, with one public bootstrap header and no geometry, topology, or
meshing implementation. On 2026-09-04 the WSL GCC reference completed:

```text
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset gcc-debug -N
ctest --preset gcc-debug --output-on-failure
```

Configuration, compilation, CTest discovery, and the one bootstrap test passed.

## Limitations and next action

The clean CMake/CTest path is now qualified on both listed compilers. The
Architecture Contract remains in review: no geometry, topology, numerical
policy, scientific algorithm, or end-to-end Architecture Contract regression is
closed by this decision.
