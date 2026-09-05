# AP Mesh Core — Continuation State

Status: ACTIVE
Last updated: 2026-09-05
Authoritative roadmap: `docs/APMESH_CORE_ROADMAP.md`
Working branch: `foundation/numeric-contract`

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

**Numeric Contract**

Current executable work unit:

**Prepare a clean revision-bound N0–N7 manifest without starting the regression**

No greenfield meshing algorithm has been implemented yet.

The Architecture Contract is QUALIFIED for WSL Ubuntu 24.04 and is integrated
into `main` at `4927383`. All eight requirements passed the final audited
four-cell protocol.

The Numeric Contract is now IMPLEMENTED with qualification pending. It separates
topological identity, exact equality, numeric proximity, certified predicate
sign, and scientific acceptance. It fixes explicit physical scale, contextual
absolute/relative allowances, finite-value handling, and distinct degeneracy,
conditioning, and indeterminate outcomes. It does not select a robust predicate
implementation or claim geometry capability.

The pre-registered N0–N7 protocol's small implementation slice is complete:
floating classification, policy validation, and scalar scale-aware proximity.
Focused GCC 13 and Clang 18 Debug contracts pass. A report-only certificate
exporter, N1 environment probe, profile, semantic comparer, and formal runner
are now available. The runner writes a revision-bound `PREPARED` manifest by
default and cannot execute a changed candidate. The clean candidate manifest
has not been prepared or executed. Foundation remains at 25% until that gate is
executed and audited.

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

The same project bootstrap and bounded Architecture Contract regression are
qualified on Clang 18.1.3 with libc++ 18.1.3, including Debug and Release.
Numeric and later Foundation contracts remain open. Native Windows remains NOT
QUALIFIED; WSL execution does not qualify it.

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
3. `docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md`
4. `docs/decisions/FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md`
5. `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
6. `docs/research/REFERENCE_REGISTER.md`.

## Next admissible actions

1. On the committed clean candidate, prepare a deterministic N0–N7 manifest
   without launching it implicitly.
3. Do not implement geometry or robust predicates before the Numeric Contract
   is qualified.

Decision progress: 3/3 previously open questions resolved. Qualification progress:
8/8 Architecture Contract requirements qualified by the final protocol.
Foundation remains IN INVESTIGATION at 25% (Architecture Contract is the first
of four Foundation gates); Numeric, Reproducible Experiment, and Foundation
End-to-End work remain unqualified.

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
