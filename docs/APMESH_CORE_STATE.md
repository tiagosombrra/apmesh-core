# AP Mesh Core — Continuation State

Status: ACTIVE
Last updated: 2026-09-04
Authoritative roadmap: `docs/APMESH_CORE_ROADMAP.md`
Working branch: `main`

## Purpose

This file is the compact continuation entry point for a new work session. Read this file first, then the authoritative roadmap, then the currently active contract/decision document. Historical discussion is not required to determine the next admissible action.

## Current strategic decision

The doctoral implementation is being reconstructed as a greenfield, modular, deterministic C++23 scientific core instead of progressively refactoring the legacy AP Mesh architecture.

The legacy implementation is preserved as:

- published/historical reference;
- source of fixtures and previous evidence;
- source of candidate algorithms to be re-derived and independently verified;
- comparison target, but never a mathematical oracle.

The greenfield implementation must eventually be usable as a library inside a larger application.

## Permanent rules already accepted

1. C++23.
2. Avoid third-party dependencies by default. Admit one only through an explicit technical/scientific justification showing material advantage.
3. Maximum three hierarchy levels: Scientific Stage -> Investigation Problem -> Executable Work Unit.
4. Use descriptive action names; short codes are secondary and never sufficient.
5. Keep `docs/APMESH_CORE_ROADMAP.md` synchronized with every meaningful advance.
6. Document every stage, decision, experiment, failure, retained limitation, and correction so another session can continue without conversation history.
7. Perform focused literature/best-practice research before changing or implementing scientific mechanisms whenever sources are available.
8. Every Scientific Stage ends with a mandatory cumulative regression campaign. A stage cannot become `QUALIFIED` with unresolved regressions.
9. If a later regression invalidates an earlier qualified result, reopen the earlier stage explicitly.
10. Correctness precedes optimization and parallelism.
11. Serial deterministic implementation precedes OpenMP/MPI or other parallel execution.
12. Topological identity is explicit and never inferred from geometric proximity.
13. Mutable global scientific state and universal global tolerances are forbidden.
14. Scientific/domain failures must be explicit and diagnosable; silent fallbacks are forbidden.
15. Generated results must be reproducible from declared inputs, revision, environment, and experiment manifest.

## Current active stage

**Foundation — Architecture, Numerics, and Reproducibility**

Current active investigation problem:

**Architecture Contract**

Current executable work unit:

**Review and commit the versioned Architecture Contract launcher before formal regression**

No greenfield meshing algorithm has been implemented yet.

Architecture decisions are recorded; qualification is REGRESSION PENDING.
The three open questions (header/package layout, certificate/report boundary,
ID allocation/canonical order) are resolved at specification level in
`docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`.

The executable specification is
`docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`, protocol 1:
GCC/Clang x Debug/Release, three certificate processes per cell, four separate
consumer validations, and all eight original architecture requirements. None
of this new regression has run; no architecture closure percentage is earned.

Current implementation gaps are active Release checks, strict language and
target library requirements, a separate consumer, and the small certificate/report
path. The next package also removes the unreachable bootstrap error API in favor
of a direct value and keeps `std::expected` verification in a local test specimen.
The bounded verification package is implemented and its focused development
checks pass for GCC 13/libstdc++ and Clang 18/libc++ in Debug: three bootstrap
CTest contracts per compiler and standalone GCC/Clang consumer builds. The
formal four-cell regression remains pending and has not been launched. The
versioned launcher prepares a clean-candidate manifest and plan by default;
its `--execute` mode consumes only a validated `PREPARED` manifest and remains
blocked pending focused contract validation and a separate authorization. Earlier
local Debug results remain limited toolchain evidence until that protocol runs
on a committed clean candidate.

## Bootstrap repository and toolchain

The dedicated local `apmesh-core` repository was initialized on 2026-09-04.
Its bootstrap documents were migrated from
`tiagosombrra/adaptive-patch-meshing`, branch
`research/apmesh-core-bootstrap`, commit
`01c4c1d614979350e77632b4c55fd497a7dc51f6`. No legacy implementation source
was copied.

The local language/standard-library probe is qualified on WSL Ubuntu 24.04:

- primary reference: GCC 13.3.0 with libstdc++;
- secondary qualification: Clang 18.1.3 with libc++ 18.1.3;
- CMake available locally: 3.28.3;
- Ninja available locally: 1.11.1.

Both compilers compiled and executed a C++23 `std::expected` probe with
`-Wall -Wextra -Wpedantic -Werror`.

The minimal `apmesh::core` library skeleton is qualified on the GCC reference:

- clean CMake configure with preset `gcc-debug`;
- Ninja build of `apmesh_core` and `apmesh_core_bootstrap_smoke`;
- CTest discovery of one `bootstrap`-labelled test;
- one passing `apmesh_core.bootstrap_smoke` execution.

The same clean project bootstrap is qualified on Clang 18.1.3 with libc++
18.1.3. These Debug smoke qualifications do not establish strict language mode
on the test target, Release checking, or repeated certificate evidence. The
remaining Architecture Contract gate remains pending. Native Windows remains
NOT QUALIFIED; WSL execution does not qualify it.

## Architecture direction already agreed

Intended dedicated library/repository name: `apmesh-core`.

Public namespace: `apmesh`.

Planned logical modules (not implemented by the current bootstrap):

- `base`
- `math`
- `geometry`
- `topology`
- `model`

Future modules are introduced only when scientifically required; candidate later modules include differential geometry, sizing, boundary discretization, meshing, adaptation, certification, and I/O adapters.

Initial build structure should prefer one library target with logical module boundaries. Separate linker targets are introduced only when independent reuse or dependency control justifies them.

Core algorithms must not perform file I/O, logging, plotting, or environment-dependent discovery.

## Required reading order for continuation

1. `docs/APMESH_CORE_STATE.md`
2. `docs/APMESH_CORE_ROADMAP.md`
3. `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
4. `docs/research/REFERENCE_REGISTER.md`
5. `docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`.

## Next admissible actions

1. Review and commit the versioned launcher; no geometry, ID classes, or formal
   regression launch.
2. Execute protocol 1 on the clean
   candidate and decide Architecture Contract qualification from all eight checks.
3. Only after Architecture Contract closure, start the Numeric Contract.

Decision progress: 3/3 previously open questions resolved. Qualification progress:
0/8 requirements qualified by the new protocol (NOT RUN). Foundation remains
IN INVESTIGATION; this documentation update closes no scientific stage.

## Stage closure protocol

Before closing any stage:

1. verify all investigation problems are resolved or explicitly blocked;
2. verify required literature/decision records exist;
3. run the stage-specific regression;
4. rerun all relevant prerequisite regressions;
5. regenerate declared figures/tables/certificates;
6. classify every difference;
7. write the stage decision/closure record;
8. update roadmap and this continuation state;
9. only then mark the stage `QUALIFIED` and authorize the next stage.

## Repository transition note

Bootstrap documentation was migrated into this dedicated greenfield repository
with immutable source revision provenance in `docs/BOOTSTRAP_PROVENANCE.md`.
The legacy implementation remains a reference repository and must not be mixed
into greenfield source.
