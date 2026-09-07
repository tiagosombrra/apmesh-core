# AP Mesh Core — Continuation State

Status: ACTIVE
Last updated: 2026-09-07
Authoritative roadmap: `docs/APMESH_CORE_ROADMAP.md`
Working branch: `foundation/reproducible-experiment-contract`

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

**Foundation End-to-End Regression**

Current executable work unit:

**Independently audit the report-only Foundation qualifier and its focused contract against FND0–FND7. Decide only whether preparation may be authorized. Do not prepare or execute the formal regression.**

No greenfield meshing algorithm has been implemented yet.

The Architecture Contract is QUALIFIED for WSL Ubuntu 24.04 and is integrated
into `main` at `4927383`. All eight requirements passed the final audited
four-cell protocol.

NQ-R1 preserved `numeric.cpp` and collected complete passing evidence for N0,
N1, and N3–N7 on clean candidate
`74fede5ae5999580f2ef76e944cf61e334f44064`. N2 remains blocked because the
pre-registered minimum/maximum finite fixture class does not explicitly classify
`std::numeric_limits<double>::min()` or `lowest()`. No numeric implementation
defect has been demonstrated. The contract still separates topological
identity, exact equality, numeric proximity, certified predicate sign, and
scientific acceptance.

NQ-R2 added explicit `min()` and `lowest()` classification evidence only in the
focused test, report-only certificate, and independent oracle; `numeric.cpp`
and the public API remained unchanged. Its revision-bound four-cell regression
on candidate `236d290a20227f0abd646073499c0d3e20a19f8e` passed the formal audit:
N0--N7 all pass. The Numeric Contract is `QUALIFIED` inside the declared WSL
Ubuntu 24.04 GCC 13/Clang 18 envelope.

Evidence status: the revision-bound NQ-R1 manifest at
`C:\Users\tiago\AppData\Local\Temp\apmesh-core-nq-r1-531d0795e1a94e1e9f43a99a578f63ce\manifest.json`
with SHA-256
`bcc39af9b75a7bd6fe1a0b02607f617372dd9bd62d8014ea69d31a097eed2a9f`
was audited as N0/N1/N3–N7 `PASS` and N2 `BLOCKED`. The earlier N0/N1 `PASS`,
N2–N7 `BLOCKED` decision remains preserved as negative historical evidence.

Scientific qualification: the Architecture, Numeric, and Reproducible Experiment
Contracts are `QUALIFIED` in the declared WSL Ubuntu 24.04 envelope. Foundation
remains `IN INVESTIGATION` at 75%; only Foundation End-to-End Regression remains
unqualified.

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
The Reproducible Experiment Contract is qualified. Foundation End-to-End and later contracts remain open. Native
Windows remains NOT QUALIFIED; WSL execution does not qualify it.

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
3. `docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md`
4. `docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md`
5. `docs/decisions/FOUNDATION_REPRODUCIBLE_EXPERIMENT_AMENDMENT.md`
6. `docs/decisions/FOUNDATION_REPRODUCIBLE_EXPERIMENT_QUALIFICATION.md`
7. `docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md`
8. `docs/decisions/FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md`
9. `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
10. `docs/research/REFERENCE_REGISTER.md`.

## Next admissible actions

1. Independently audit the Foundation profile, report-only qualifier, and
   focused contract against FND0–FND7. Decide only whether preparation may be
   authorized; do not prepare or execute the formal regression.

Decision progress: 3/3 Numeric Contract questions resolved. Qualification
progress: Architecture, Numeric, and Reproducible Experiment Contracts are
qualified. Foundation remains IN INVESTIGATION at 75% (three of four Foundation
gates); Foundation End-to-End remains unqualified.

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
