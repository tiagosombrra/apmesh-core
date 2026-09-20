# AP Mesh Core — Continuation State

Status: ACTIVE
Last updated: 2026-09-19
Authoritative roadmap: `docs/APMESH_CORE_ROADMAP.md`
Working branch: verify with Git; Minimal Small Linear Algebra is integrated into
`main` at `ca51c333b5fcae33f05b5e25f6b0780ec195ad77`.

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
16. Small components close with focused analytic contracts and may integrate as
    implemented but unqualified; formal qualification occurs cumulatively at
    scientific-stage exit.
17. Formal manifests and four-cell campaigns are not created per value type
    unless a separate scientific decision proves that the claim cannot wait for
    stage closure.
18. Ordinary work targets at least 50% scientific/C++ implementation, at most
    35% focused validation/tooling, and at most 15% documentation/governance.
19. One reusable stage-level evidence workflow is preferred over new runners,
    collectors, comparers, and schemas for each component.
20. A mechanical tooling defect gets one focused regression contract without a
    new scientific decision cycle; two consecutive mechanical failures stop the
    standalone campaign before any third execution.
21. Documentation records current authority and terminal evidence without
    creating a chronological micro-record for each implementation step.

## Current active stage

**Topological Model — Explicit Identity and Incidence**

Current stage decision:

**Identity and Oriented Edge Incidence Kernel — IMPLEMENTED / FOCUSED CONTRACT
PASS / NOT QUALIFIED.** The bounded candidate implements only strong
`VertexId`/`EdgeId` semantics, explicit edge endpoints, oriented edge uses,
deterministic mutable construction, and atomic finalization into an immutable
topology model. Its focused CTest passed in WSL Ubuntu 24.04 GCC 13 Debug and
Clang 18/libc++ Debug. It authorizes no face/patch cycle, manifold or
non-manifold classification, coordinate welding, curve/surface association,
canonical serialization, qualification infrastructure, or campaign.

Current prerequisite closure evidence:

**Geometry Primitives: QUALIFIED / GPR0-GPR7 PASS.** The bounded GPR0-GPR7
protocol qualifies only the joint
Point/Vector, Minimal Small Linear Algebra, and Cartesian Similarity Frames
capability already integrated in post-merge `main` baseline
`ca20ad64cd0dd14b869225dae73401ad2b93464c`. It authorizes no new production
type or behavior. The reusable profile, experimental integrated exporter, and
report-only validator/collector pass focused GCC 13 Debug and Clang 18 libc++
Debug contracts. The revision-bound runner seals preparation, consumes one
manifest exactly once, retains either terminal outcome, and leaves every gate
pending scientific audit. Its focused contracts cover immutable preparation and
retained synthetic success and partial failure. The first external manifest,
`a890b988559eb041c63104a45e0edbd66d6a9c07fdb912a45896bbe9ded81711`, is
immutable `BLOCKED_BY_STALE_LIMITATION_CONTRACT`: it was never executed because
its sealed limitations contradicted the `PREPARED` state. The correction is
limited to the profile contract and remains immutable. The second external
manifest, `ad999a236cf68f537dd5d6bc159885c2e9404a50e824622f867ac24df59b9aa2`,
is immutable `BLOCKED_BY_CTEST_DISCOVERY_PARSER_DEFECT`: configure, build, and
CTest discovery succeeded, but a spacing-sensitive parser rejected the valid
CTest listing before semantic tests or certificates ran. The correction is
limited to runner tooling; a new manifest is required and execution remains
unauthorized.

The third external manifest,
`16bac9f16868ccd98c3a437cb524098fa0671b86a60d532010a837ebcb14e9bf`, is
immutable `BLOCKED_BY_EVIDENCE_OUTPUT_DIRECTORY_DEFECT` evidence. Its GCC
Debug configure, build, CTest discovery, and semantic CTest commands succeeded,
but the first certificate producer could not open its output because the runner
had not created `certificates/`. The correction is limited to runner output
directory initialization; no certificate, gate, or component qualification was
reached. A new manifest is required and execution remains unauthorized.

Current correction status: a fourth external manifest,
`f36a8fa1801eca6f992d7cb46f11b34c9d088abec8df769568cf3bb7f41dff5b`, bound
to clean candidate `8811a00`, is immutable
`BLOCKED_BY_SEMANTIC_CTEST_REGEX_DEFECT` evidence. It retained twelve
cross-cell-equivalent certificates, negative evidence, and detached retention,
but all four semantic CTest commands used an unsupported `(?:...)` group. CTest
selected no tests while returning zero; consequently GPR6 prerequisite
preservation and GPR7 closure are not evidenced. This is a runner defect, not
a production or component-qualification regression.

The fifth external manifest,
`a39069569625bbc16637d2078bddb2e91c654840782ad0c4026fc9b75ef42718`, bound
to clean published candidate `2f22ffd`, executed once and passed the separate
GPR0-GPR7 audit. All four GCC/Clang Debug/Release cells ran the exact sealed
six-test allowlist and produced three certificates each. The 52 command
records, 12 certificate hashes, 20 negative rejections, dependency/runtime
inventories, and detached retention verification all passed. Geometry Primitives
is QUALIFIED only in the declared WSL Ubuntu 24.04 envelope.

Cartesian Similarity Frames is **QUALIFIED / CF0–CF7 PASS / candidate
`8e6b5872072bb8077731b5879d1c5b751e375087` / WSL Ubuntu 24.04 GCC 13 and
Clang 18 libc++.** `CartesianFrame2` and `CartesianFrame3` remain bounded to
an exact signed-permutation basis, finite origin, and positive power-of-two
scale. The replacement revision-bound execution produced twelve identical
semantic certificates across four compiler/build cells and three repetitions,
with 1,696 unique exact cases per certificate, exact prerequisite preservation,
and a detached-verified 168-file canonical retention package. The earlier
`e69e804` package remains immutable
`BLOCKED_BY_COMPILE_COMMANDS_SCHEMA_DEFECT` evidence; it is not a defect in
Cartesian Frames behavior and was not overwritten. This qualification covers
only matching point/vector local-world mappings within the declared envelope;
it does not authorize general transformations, tolerance-based validation,
inversion, predicates, topology, curves, surfaces, or meshing. The entry
decision and fixed qualification record are in
`docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md`
and `docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`.

PR #5 was squash-merged into `main` at
`12afa394af24c8f0627b12e3697010735826a9a1`. Its tree is identical to reviewed
source head `5c96593780b88f9e297253e600ccf5b5f2ba1c95`; both resolve to tree
`8597575861c4573c850fb6e30e193e610674ce5c`. The source branch was removed only
after that identity was verified.

PR #6 was squash-merged into `main` at
`ca20ad64cd0dd14b869225dae73401ad2b93464c`. Local `main`, `origin/main`, and
`HEAD` were aligned at that commit with clean tree
`01154f434aaffe8e2b5b57710f21a38efbe95d8d`. The merge added the bounded
Cartesian Frames qualification record and evidence infrastructure; it did not
change the already integrated production implementation.

Minimal Small Linear Algebra remains `QUALIFIED` on clean published candidate
`3804e903f56a226d38319fd44255b5832639815d` within the declared WSL Ubuntu
24.04 GCC 13 / Clang 18 libc++ envelope. Its LA-only integration regression on
`95258e9faaeaae89e801874fe1ce600e80285427` passed before its reviewed squash
merge into `main` as `ca51c333b5fcae33f05b5e25f6b0780ec195ad77`.

Foundation is `QUALIFIED` at 100% within the declared WSL Ubuntu 24.04
envelope. Bounded point/vector geometry is implemented and has passed its
focused four-cell contract; meshing remains unimplemented.

PR #3 was squash-merged into `main` at
`1ff6568c908ea144b903a70a2497c000a89e35eb`. Its tree is identical to the
reviewed source head `b17067312b523e934c81d56b6cde7948f30ff93f`; both
resolve to tree `bea6ffaa71877811daa98d1f69a299446b12f401`. The source
branch was removed only after that identity was verified. Point and Vector
Semantics therefore remains qualified and is now integrated into the canonical
`main` history.

Geometry Primitives entry is approved by
`docs/decisions/GEOMETRY_PRIMITIVES_ENTRY_DECISION.md`. The authorized first
implementation is limited to distinct `Point2`, `Point3`, `Vector2`, and
`Vector3` value semantics, finite construction, affine/Euclidean operations,
explicit failures, and focused analytic evidence. Matrices, transforms,
predicates, topology, curves, surfaces, and meshing remain blocked. The focused
contract passed on GCC/Clang Debug/Release. The revision-bound Point/Vector
qualification protocol is recorded. Its first formal attempt on
`b7f8fe9` stopped in `gcc-debug` because the PV6 selector included a historical
Foundation preflight self-test rather than only the required Architecture,
Numeric, and Reproducible Experiment contracts. The terminal evidence is
retained as `BLOCKED_BY_CONTRACT_SELECTION_DEFECT`; the complete stage remains
unqualified.

The corrected exact preservation selector was executed once on clean published
candidate `ededf6584941de9dfe6a47633ffd67caf17d4f51`. The formal matrix
completed four GCC/Clang Debug/Release cells with three independent Point/Vector
processes per cell. All twelve certificates were semantically identical across
the matrix; every Foundation-preservation CTest set passed; negative evidence
rejected duplicate cases, unsafe compiler flags, and forged proximity values;
and detached retention verification passed. The separate audit accepted
PV0–PV7. **Point and Vector Semantics is QUALIFIED** only in the declared WSL
Ubuntu 24.04 GCC 13 / Clang 18 libc++ envelope. The full Geometry Primitives
stage remains `IN INVESTIGATION`; no matrix, transform, predicate, topology,
curve, surface, or meshing capability is authorized.
The canonical retained evidence package is
`evidence/geometry-primitives/point-vector/pv0-pv7-ededf65/`; it excludes
scratch builds, caches, and reproducible binaries while preserving hash-bound
manifests, certificates, inventories, reports, negative evidence, and focused
logs.

Foundation End-to-End closed with FND0–FND7 `PASS` on clean published candidate
`b333755442b934c490abaecda886dd2a40e981ca`. The report-only qualifier produced
revision-bound evidence; the separate audit made the scientific closure
decision. Historical REC evidence at `85d215a` remained a qualified comparison
baseline and did not substitute for current-candidate FND2, FND4, or FND6.
The original 586-file source manifest is preserved in the canonical Foundation
package at `evidence/foundation/foundation-end-to-end/fnd0-fnd7-b333755/`.
The canonical package retains 560 non-rebuildable files; its manifest SHA-256
is `ce873c59e663a570773469639a96e1f75587954adda0064d1a9d775773064040`.
The hash-bound exclusion inventory records the 28 reproducible blobs
(1,252,808 bytes) with their path, size, SHA-256, command, and provenance.

The retained limitations are unchanged: qualification is restricted to WSL
Ubuntu 24.04 and makes no native-Windows, geometry, topology, meshing,
convergence, performance, parallel-equivalence, or universal-portability claim.

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
Contracts are `QUALIFIED` in the declared WSL Ubuntu 24.04 envelope. The
Foundation End-to-End Regression passed FND0–FND7 jointly on clean published
candidate `b333755442b934c490abaecda886dd2a40e981ca`; Foundation is
`QUALIFIED` at 100% with retained scope limitations.

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
The Reproducible Experiment Contract and Foundation End-to-End Regression are
qualified. Later scientific stages remain open. Native Windows remains NOT
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
3. `docs/contracts/APMESH_CORE_MINIMAL_SMALL_LINEAR_ALGEBRA_CONTRACT.md`
4. `docs/decisions/GEOMETRY_PRIMITIVES_ENTRY_DECISION.md`
5. `docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION.md`
6. `docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md`
7. `docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md`
8. `docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_SIGNED_ZERO_DECISION.md`
9. `docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md`
10. `docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`
11. `docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md`
12. `docs/decisions/TOPOLOGICAL_MODEL_ENTRY_DECISION.md`
13. `docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md`
14. `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
15. `docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md`
16. `docs/research/REFERENCE_REGISTER.md`.

## Next admissible actions

Geometry Primitives is QUALIFIED on candidate `2f22ffd` after the audited
GPR0-GPR7 campaign. Preserve the immutable historical blocked attempts and the
fifth manifest/retention package as the stage evidence. Foundation and Geometry
Primitives remain qualified only in their declared WSL envelope.

Review the bounded Identity and Oriented Edge Incidence Kernel defined by
`docs/decisions/TOPOLOGICAL_MODEL_ENTRY_DECISION.md` before publication. Its
focused GCC 13 Debug and Clang 18/libc++ Debug contract passes, but Topological
Model remains unqualified. Do not introduce faces, patches, curves, manifold
classification, canonical serialization, formal qualification infrastructure,
or a campaign.

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
