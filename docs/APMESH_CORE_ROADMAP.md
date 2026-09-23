# AP Mesh Core — Scientific Implementation Roadmap

Status: ACTIVE / AUTHORITATIVE
Last updated: 2026-09-22
Scope: greenfield scientific core that will replace, module by module, the legacy implementation as the doctoral reference implementation.

> This file is the single authoritative roadmap for the greenfield AP Mesh Core effort. Every implementation, experiment, correction, stage closure, regression, or scope change MUST update this document in the same change set.

## 1. Purpose

The goal is not to refactor the legacy AP Mesh incrementally. The goal is to reconstruct the scientifically defensible technique as a clean, modern, deterministic C++23 library, with explicit contracts, reproducible experiments, numerical evidence, generated figures, and a posteriori certification.

The legacy repository remains a historical/reference implementation and a source of fixtures, published behavior, previous experiments, and candidate algorithms. It is never an oracle: a disagreement with legacy behavior must be resolved against mathematics, literature, controlled experiments, and declared contracts.

The implementation strategy is incremental. A later scientific stage MUST NOT be treated as qualified while a prerequisite stage remains unqualified.

## 2. Permanent project rules

### 2.1 Three-level decomposition rule

Work is decomposed into at most three levels:

1. **Scientific Stage** — a coherent scientific capability.
2. **Investigation Problem** — one precise question inside that capability.
3. **Executable Work Unit** — one reviewable implementation/verification action.

If an executable work unit is still too large, the parent investigation problem must be split. A fourth hierarchy level is forbidden.

### 2.2 Descriptive naming rule

Identifiers such as `F1`, `AV2`, or `P3` are never sufficient names. A short code may exist only for ordering, but every stage, problem, artifact, branch, issue, experiment, and commit must use a descriptive action-oriented name.

Examples:

- `Foundation — Architecture and Scientific Reproducibility`
- `Geometry Primitives — Certify Point and Vector Semantics`
- `Curve Geometry — Verify Cubic Bezier Derivatives`
- `Boundary Discretization — Enforce Parameterization-Invariant Metric Length`

### 2.3 Evidence-before-closure rule

A work unit may close only when its declared evidence exists. Depending on the unit, evidence may include:

- mathematical reference or derivation;
- unit tests;
- analytic verification case;
- numerical error table;
- deterministic experiment manifest;
- generated figures;
- comparison against an independent implementation or literature result;
- negative/adversarial test;
- machine-readable certificate;
- human-readable decision record.

`build succeeds`, `program exits with zero`, and visual inspection alone are never sufficient scientific closure criteria.

### 2.4 Mandatory end-of-stage regression rule

Every Scientific Stage MUST end with a dedicated regression campaign before it can become `QUALIFIED`.

The regression campaign must:

- rerun all qualified evidence from the current stage;
- rerun all relevant qualified evidence from every prerequisite stage;
- include declared adversarial/negative fixtures;
- compare machine-readable certificates against accepted expectations;
- regenerate required plots/tables/images from clean inputs;
- verify deterministic repeated execution;
- record compiler/toolchain/platform and input hashes;
- classify every difference as `expected`, `regression`, or `investigation_required`;
- produce a regression report and immutable evidence manifest.

A stage cannot close if its end-of-stage regression contains an unresolved regression. If a newly discovered issue invalidates a previously qualified prerequisite, that prerequisite stage is reopened and the roadmap status must reflect this.

### 2.5 Literature-first rule

Before implementing or changing a scientific mechanism, perform a focused literature and best-practice review. The review must identify, where applicable:

- mathematical definition and assumptions;
- accepted algorithms in geometric computing / mesh generation;
- known failure modes and robustness requirements;
- established error or quality metrics;
- verification/validation methodology;
- modern C++/software engineering guidance relevant to the implementation.

References used to justify design decisions are recorded in `docs/research/REFERENCE_REGISTER.md` and linked from the corresponding stage decision document.

### 2.6 Roadmap synchronization rule

Every meaningful change must update at least one status entry in this roadmap. A stage-closing change must also update:

- the roadmap status;
- the stage decision/closure document;
- the experiment/evidence index;
- the end-of-stage regression report;
- retained limitations/blockers.

This is intended to make the repository self-sufficient for continuation in a new work session without relying on historical discussion.

### 2.7 Implementation-first qualification rule

Scientific rigor is applied at the level of the claim being made, not by giving
every small value type its own formal campaign. Evidence has three levels:

1. **Component development** — implementation plus focused unit, analytic,
   negative, and header/dependency contracts. Passing evidence permits review
   and integration with status `IMPLEMENTED / FOCUSED CONTRACTS PASS`; it does
   not permit a `QUALIFIED` claim.
2. **Investigation integration** — the coherent group of components is tested
   together against its declared scientific invariants and prerequisite
   contracts. This may use a second compiler or configuration when the
   investigation has a real portability or numeric-semantic risk, but it does
   not require a new manifest framework per class.
3. **Scientific-stage qualification** — one revision-bound cumulative campaign,
   retained evidence package, and scientific audit close the stage. Formal
   manifests, four-cell matrices, detached retention, and publication-grade
   reports belong here unless an explicit scientific decision explains why an
   earlier standalone claim cannot safely wait for stage closure.

During ordinary implementation work, the target active-effort distribution is
60--65% scientific/C++ implementation, 25--30% focused validation and tooling,
and about 10% documentation/governance. This is an anti-overengineering signal,
not rigid accounting. Stage-closing campaigns are the declared exception
because validation is their primary output. If support work dominates an
ordinary work unit, its evidence design is simplified before more tooling is
added.

The executable policy has three profiles: FAST for ordinary GCC Debug semantic
and focused feedback; INTEGRATION for the relevant cumulative semantic tests on
GCC Debug and Clang/libc++ Debug, with Release only for a declared risk; and
QUALIFICATION for explicit scientific-stage closure. `BUILD_TESTING=ON`
registers FAST/INTEGRATION tests. Historical evidence, runners, retention, and
qualification tooling require `APMESH_ENABLE_QUALIFICATION_TESTS=ON`. Changing
this execution policy does not reopen any qualified stage or weaken its claim.

One reusable experiment runtime and stage-level runner are preferred over
component-specific launchers, collectors, comparers, and schemas. A mechanical
tooling defect that does not change implementation, hypotheses, expectations,
or acceptance receives one focused regression contract and resumes without a
new scientific decision cycle. Two consecutive mechanical failures in the same
formal campaign trigger a tooling stop: no third execution is allowed until the
workflow is simplified or deferred to the cumulative stage regression.

Documentation records current authority, terminal evidence, retained
limitations, and stage decisions. It does not create a new protocol, decision
record, or chronological narrative for every implementation step. Small
components may be integrated before formal stage qualification when their
focused contracts pass and their status remains explicitly unqualified.

### 2.8 Repository continuity and progress-reporting rule

The repository must remain sufficient to continue the project without chat
history. Every major status change records a compact checkpoint in
`docs/APMESH_CORE_STATE.md` with the accepted revision, active stage, terminal
CI/regression state, retained blockers, progress lanes, and exact next
admissible action.

Each execution reports work-class percentages that sum to 100% across
implementation, tests/validation, evidence/experiments, and
documentation/governance. These percentages describe effort distribution, not
scientific completion. Stage completion is reported separately as independent
0--100% lanes for production implementation, focused validation,
stage-regression/qualification tooling, formal evidence execution, and closure
audit/documentation.

At every major phase boundary, run an explicit repository audit and cumulative
semantic regression before authorizing further scientific scope. Engineering
CI regressions do not substitute for a stage's pre-registered scientific
qualification protocol.

## 3. Technology baseline

The greenfield core is defined as follows unless changed by a reviewed architecture decision:

- Language: **C++23**.
- Build system: modern target-based **CMake**.
- Initial runtime dependencies: **C++ standard library only**.
- Third-party libraries: avoided by default; admitted only through an explicit decision demonstrating material scientific or engineering advantage over the added dependency and coupling.
- Initial execution model: deterministic single-threaded reference implementation.
- Parallelism: introduced only after serial behavior is certified, followed by equivalence testing.
- Mutable global scientific state: forbidden.
- Scientific failures: explicit typed results, preferentially `std::expected`.
- Geometry/topology identity: explicit typed IDs; never inferred from coordinates.
- Scientific model after validation: immutable.
- Core algorithms: no file I/O, logging side effects, GUI, or implicit environment dependence.
- Numeric tolerances: explicit and contextual; no universal global epsilon.
- Every stage ends with a mandatory regression gate.

### Local bootstrap toolchain qualification

Status: `QUALIFIED` for the language/standard-library probe, clean GCC/Clang
project bootstraps, and the bounded Architecture Contract gate.

- Primary local reference: WSL Ubuntu 24.04, GCC 13.3.0, libstdc++.
- Secondary local qualification: WSL Ubuntu 24.04, Clang 18.1.3, libc++
  18.1.3.
- Available build tools: CMake 3.28.3 and Ninja 1.11.1.
- Both compilers compile and execute the declared C++23 `std::expected` probe
  under `-Wall -Wextra -Wpedantic -Werror`.
- GCC 13.3.0 cleanly configures and builds the initial `apmesh::core` target
  and passes the CTest `apmesh_core.bootstrap_smoke`.
- Clang 18.1.3 with libc++ 18.1.3 cleanly configures and builds the same target
  and passes the same CTest smoke test.

The bounded Architecture Contract regression passed in GCC 13/libstdc++ and
Clang 18/libc++, Debug and Release. Its final audit qualified all eight declared
requirements, including clean scratch-directory behavior and unambiguous
cell/repetition evidence identity.

The evidence and limitations are recorded in
`docs/decisions/FOUNDATION_TOOLCHAIN_BASELINE_QUALIFICATION.md`.
The qualification includes Release checks, target isolation, external
consumption, and repeated structured evidence. Native Windows remains NOT
QUALIFIED.

## 4. Repository strategy

The greenfield core now has a dedicated local `apmesh-core` repository/library.
Bootstrap contracts were migrated from the
`research/apmesh-core-bootstrap` branch of `adaptive-patch-meshing` with source
commit provenance.

No production legacy algorithm is to be modified as part of the greenfield bootstrap unless a separate explicitly authorized legacy-maintenance task requires it.

The migrated documents retain immutable source references in
`docs/BOOTSTRAP_PROVENANCE.md`. The dedicated private remote is configured, and
formal candidates still require committed clean revisions.

## 5. Status vocabulary

- `NOT STARTED`
- `IN INVESTIGATION`
- `IMPLEMENTED / UNQUALIFIED`
- `REGRESSION PENDING`
- `QUALIFIED`
- `BLOCKED`
- `REOPENED`
- `SUPERSEDED`

Scientific closure and algorithmic qualification are distinct. A negative investigation result may scientifically close a question while leaving the implementation `BLOCKED` or `UNQUALIFIED`.

## 6. Scientific implementation stages

### Foundation — Architecture, Numerics, and Reproducibility

Status: `QUALIFIED / WSL Ubuntu 24.04`

Goal: establish the project contracts required to trust subsequent scientific work.

#### Architecture Contract

Status: `QUALIFIED` on WSL Ubuntu 24.04

- **Define project/module boundaries** — SPECIFIED. Public/private header layout,
  one target, package identity, consumer scope, and dependency policy are fixed
  in the Architecture Contract. Installed distribution remains deferred.
- **Qualify compiler and build-system baseline** — language/standard-library
  probe and clean CMake/CTest bootstraps QUALIFIED on GCC 13.3 and Clang
  18.1/libc++.
- **Define data model and ownership** — SPECIFIED at architectural level: builder-owned
  typed IDs, immutable model, explicit incidence, value/RAII lifetime, deterministic
  allocation and serialization. Scientific types and their tests remain in their
  later stages; no topology implementation is implied.
- **Define API and error semantics** — SPECIFIED. Infallible operations return values;
  real domain failures use explicit results. Bootstrap error vocabulary moves to
  the test specimen. Numeric classification remains with Numeric Contract.
- **Verify the bounded architecture bootstrap** — QUALIFIED. The corrected
  four-cell regression passed all eight requirements with three repetitions per
  cell; the audited evidence is retained in
  `docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`.

#### Numeric Contract

Status: `QUALIFIED` on the declared WSL Ubuntu 24.04 compiler envelope

- **Define physical scale and units policy** — SPECIFIED. Every dimensional
  decision requires an explicit positive finite scale and operation-owned
  absolute/relative allowances; no universal epsilon is admitted.
- **Define floating-point comparison and geometric predicates policy** —
  SPECIFIED. Identity, equality, proximity, predicate sign, and scientific
  acceptance are separate relations. Robust predicate implementation remains a
  later bounded work unit.
- **Define degeneracy and conditioning policy** — SPECIFIED. Invalid numeric,
  invalid policy, degenerate, ill-conditioned, and indeterminate outcomes have
  distinct semantics.
- **Initial qualification attempt** — BLOCKED / RETAINED EVIDENCE. The
  revision-bound audit for candidate
  `7827a9633e97aadcc2b2777648b02ffa7a308b8e` classified N0/N1 `PASS` and
  N2–N7 `BLOCKED`; the formal decision is retained in
  `docs/decisions/FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md`. The audited
  evidence SHA-256 is
  `ec37ca0e2f93011782f5de42535ccf95783e46c374b691a9e1748506c3d4f9e5`.
  This blocks scientific qualification without changing the implementation or
  claiming a numeric capability.
- **NQ-R1 evidence-completeness recovery** — FORMAL REGRESSION BLOCKED.
  Candidate `74fede5` preserved `numeric.cpp`; N0/N1/N3–N7 passed. N2 remains
  blocked because the pre-registered finite-extrema fixture class does not
  explicitly classify `std::numeric_limits<double>::min()` or `lowest()`. The
  audited manifest SHA-256 is
  `bcc39af9b75a7bd6fe1a0b02607f617372dd9bd62d8014ea69d31a097eed2a9f`.
  Foundation remains at 25%; no implementation defect is currently shown.
- **NQ-R2 finite-extrema evidence recovery** — QUALIFIED. Candidate
  `236d290a20227f0abd646073499c0d3e20a19f8e` adds explicit `min()` and
  `lowest()` classifications only to the focused contract, report-only
  certificate, and independent oracle; `numeric.cpp` remains unchanged. Its
  clean four-cell regression passed all twelve CTest processes, produced twelve
  byte-identical certificates, and preserved the Architecture Contract across
  all four cells with five required negative checks rejected. The audited
  manifest SHA-256 is
  `8ab37917a8e8de90cbbebe7ef5d393ef76ce877acc0e64275fd5d31cc15bdbd6`.
  N0–N7 are `PASS`; the Numeric Contract is qualified only within the declared
  WSL Ubuntu 24.04 GCC 13/Clang 18 envelope.

#### Reproducible Experiment Contract

Status: `QUALIFIED` on WSL Ubuntu 24.04

- **Define experiment manifest** — SPECIFIED. A profile declares claims,
  execution matrix, inputs, artifacts, equivalence, gates, and limitations; a
  `PREPARED` manifest binds them to one clean revision and empty output root.
- **Define evidence and failure semantics** — SPECIFIED. Command records,
  artifact inventory, derivation, explicit terminal states, and prohibition of
  hidden retry/fallback are fixed in the contract.
- **Define replay equivalence** — SPECIFIED. Byte, canonical-JSON, semantic,
  and numeric equivalence are separate; volatile provenance fields must be
  exhaustively declared and cannot be claim fields.
- **First bounded regression** — PASS. Two independent replays completed in
  each qualified GCC/Clang Debug/Release cell on candidate `85d215a`.
  Two independent replays of the frozen Numeric Contract specimen will run in
  each of the four qualified GCC/Clang Debug/Release cells. E0–E7 require
  complete provenance, repeatability, cross-cell reproducibility, eight
  negative-fixture rejections, and preservation of the Architecture and Numeric
  contracts. The audit accepted E0–E7; the canonical retained package is
  `evidence/foundation/reproducible-experiment-contract/rec-e0-e7-85d215a/`.
- **Completeness and durable-retention amendment** — QUALIFIED. The amendment
  fixed the E0–E7 implementation obligations, control/evidence separation, and
  durable retention; retained evidence verifies against a clean worktree bound
  to the candidate revision.

#### Foundation End-to-End Regression

Status: `QUALIFIED / FND0–FND7 PASS / candidate b333755 / WSL Ubuntu 24.04`

- Reuse the qualified REC execution path; do not introduce a parallel campaign
  framework.
- Execute two replays in each declared GCC/Clang Debug/Release cell from one
  clean candidate, including same-revision Architecture and Numeric
  prerequisites.
- Compare claim fields with the accepted REC baseline at `85d215a` and classify
  every difference as `NO_CHANGE`, `EXPECTED_CHANGE`, `REGRESSION`, or
  `INVESTIGATION_REQUIRED`.
- Regenerate and retain the stage certificate, compact table, status figure,
  dependency inventory, manifests, hashes, and limitations.
- Require FND0–FND7 from
  `docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md` to pass jointly.
- Use the report-only Foundation qualifier for source-scope, accepted-baseline,
  dependency, and closure evidence; it may emit collected evidence or
  `BLOCKED`, but never scientific `PASS`.
- Evaluate FPR0–FPR6 before preparation. Their joint success may authorize one
  `PREPARED` manifest only; it does not satisfy, waive, or alter FND0–FND7 and
  does not authorize execution.
- Treat retained REC evidence at `85d215a` only as qualified historical evidence
  and the accepted comparison baseline. It cannot substitute for current-candidate
  execution, deterministic claims, or retention required by FND2, FND4, and FND6.

The formal post-execution audit accepted FND0–FND7 jointly on candidate
`b333755442b934c490abaecda886dd2a40e981ca`. The Foundation evidence package
retains four clean compiler/build cells, two replays per cell, 586 sealed files,
and no dependency or contract-test finding. The only retained limitations are
the declared WSL Ubuntu 24.04 envelope and the exclusion of geometry, topology,
meshing, convergence, performance, parallel equivalence, and native Windows.
The canonical retained package is
`evidence/foundation/foundation-end-to-end/fnd0-fnd7-b333755/`; all 586 declared
entries were recovered from the original package and verified by size and
SHA-256 against retention manifest
`bbd316c8084a6bb4fd862112288ee6d826870ca63589977e77d0453916a7b937`.

Stage exit gate: all three contracts reviewed; minimal C++23 library builds from a clean checkout; one deterministic smoke experiment is fully reproducible from manifest to certificate and figure; Foundation End-to-End Regression passes.

### Geometry Primitives — Exact Semantics Before Curves

Status: `QUALIFIED / GPR0-GPR7 PASS / candidate 2f22ffd / WSL Ubuntu 24.04`

Goal: establish independently verifiable spatial primitives without mesh-generation dependencies.

Entry authority:
`docs/decisions/GEOMETRY_PRIMITIVES_ENTRY_DECISION.md`. The first bounded work
unit is restricted to distinct 2D/3D point and vector value semantics, finite
construction, affine/Euclidean operations, explicit failures, and focused
analytic evidence. It does not authorize topology, predicates, matrices,
transforms, curves, surfaces, or meshing.

The bounded point/vector implementation and its focused `geometry;contract`
CTest passed on GCC/Clang Debug/Release in the qualified WSL envelope. Focused
CTest evidence alone did not qualify cross-compiler claim equivalence, native
Windows, or the Geometry Primitives stage regression.

The revision-bound work-unit protocol is recorded in
`docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md`. Its `PV0`–`PV7`
gates qualify only Point and Vector Semantics, require one fixed four-cell run
with three independent focused processes per cell, and reuse the current
Architecture/Numeric/Reproducible Experiment preservation contracts without
repeating the full historical REC campaign.
The formal execution on clean published candidate `ededf65` passed PV0–PV7:
four GCC/Clang Debug/Release cells, three independent focused processes per
cell, twelve semantically equivalent certificates, the exact
Foundation-preservation CTest set in every cell, and detached retention
verification. The separate audit qualified only Point and Vector Semantics in
the declared WSL envelope. The decision and retained limitations are recorded
in `docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION.md`.

PR #3 integrated the reviewed Point/Vector package into `main` at
`1ff6568c908ea144b903a70a2497c000a89e35eb`. The resulting tree is identical to
the reviewed source head `b17067312b523e934c81d56b6cde7948f30ff93f`;
both resolve to tree `bea6ffaa71877811daa98d1f69a299446b12f401`.
The source branch was removed after this identity check.

A first formal attempt on `b7f8fe9` remains retained as
`BLOCKED_BY_CONTRACT_SELECTION_DEFECT`: its PV6 selector admitted a historical
Foundation preflight self-test whose scope intentionally excludes later
Geometry changes. That incident is historical negative evidence; it was not
reused or reclassified by the successful `ededf65` execution.

#### Point and Vector Semantics

- Implement and verify `Point2`, `Point3`, `Vector2`, `Vector3` semantic separation.
- Verify arithmetic, dot product, cross product, norm, normalization, finite-value handling.
- Add analytic tests and adversarial scale tests.

#### Small Linear Algebra

Status: `QUALIFIED / LA0-LA7 PASS / WSL Ubuntu 24.04`

- Authority: `docs/contracts/APMESH_CORE_MINIMAL_SMALL_LINEAR_ALGEBRA_CONTRACT.md`.
- Candidate hypothesis: concrete fixed-size `Mat2` and `Mat3` value semantics,
  finite construction, checked access, identity, transpose, matching
  matrix-vector application, same-dimension composition, and an algebraic-only
  determinant are sufficient for the next dependency.
- Explicitly exclude dynamic matrices, inverses, solves, decompositions,
  eigensystems, determinant predicates, transformations, topology, curves,
  surfaces, meshing, and new dependencies.
- Require independent analytic cases, explicit non-finite failure, preservation
  of Foundation and Point/Vector semantics, and a later revision-bound
  qualification before this investigation problem can close.
- Amendment 1 fixes the one-way `math -> geometry` dependency, exact error
  vocabulary, transpose identity `(A B)^T = B^T A^T`, and operation-specific
  power-of-two scale laws without expanding the admitted capability.
- The bounded implementation provides only `Mat2`, `Mat3`, their admitted
  matrix-only operations, and one-way Geometry-side `Vector2`/`Vector3`
  adapters. Focused `apmesh_core.minimal_small_linear_algebra` and
  `apmesh_core.math_header_isolation` CTests passed on GCC 13 Debug and Clang
  18 libc++ Debug in the declared WSL Ubuntu 24.04 envelope. This is focused
  implementation evidence, not formal LA0-LA7 qualification.
- The formal LA0-LA7 protocol is preregistered in
  `docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md`.
  It fixes one four-cell GCC/Clang Debug/Release matrix, three independent
  semantic certificates per cell, exact prerequisite preservation, immutable
  evidence, and a separate scientific audit. Its report-only profile, exporter,
  runner, comparer, negative contracts, and retention verifier pass focused
  GCC/Clang checks. At preregistration time, no manifest had been prepared and
  no formal execution had started.
- Admission correction package (2026-09-12): explicit permutation outputs and
  bidirectional dimension rejection; named output fields including both scale
  composition laws; planned source/runtime/artifact inventories; transitive and
  compiler-observed dependencies; preparation seal and exclusive consumption;
  real disposable-repository failure and retention negatives. Five focused
  contracts pass on GCC 13 Debug and on Clang 18 libc++ Debug. Terminal
  retention is also verified after relocation. The next step is admission
  audit; this work adds no mathematical capability or qualification.
- LA2/LA7 correction (2026-09-13): retain all matrix entries for the 39
  non-finite constructor cases; enumerate eight certificate mutations with
  baseline/validator hashes, exact rejection reasons, retained inputs, and
  independent recomputation. The planned CLI and its retained command binding
  are tested. Common terminal artifacts are required in both outcomes;
  failure-only and sealing-failure artifacts have explicit conditions.
  Detached-check, inventory-write, and verification failures retain the prior
  terminal evidence and produce verifiable `BLOCKED` diagnostic archives,
  without retrying success sealing or claiming detached success. Five focused
  contracts pass in each GCC/Clang Debug build; the expanded runner contract
  passes in both as well. Production math/geometry is unchanged. At that point,
  manifest preparation and execution remained blocked pending admission audit.
- First formal attempt (2026-09-13): the four-cell campaign on clean published
  candidate `6fa00bd` completed, but scientific audit classified the attempt
  `BLOCKED`. LA0, LA1, LA2, LA4, LA5, and LA6 are supported; LA3 and LA7 are
  blocked because `mat2_quarter_turn_square` contains raw `0x0p+0` versus
  `-0x0p+0` while the validator asserted `exact_match=true` without a declared
  signed-zero canonicalization rule. The evidence remains immutable.
- Signed-zero correction (2026-09-13): LA exact comparison is canonical numeric
  hexadecimal comparison; both signed zeros normalize to `0x0p+0`, while raw
  encodings remain diagnostic provenance. The evidence oracle, validator,
  comparer, retention checks, schemas, and focused GCC/Clang contracts pass;
  production code is unchanged. At the time of this correction, a new
  revision-bound campaign still required an independent admission audit.
- Qualification closure (2026-09-13): the one authorized revision-bound
  four-cell execution on clean published candidate `3804e90` passed LA0-LA7.
  It retained 12 semantically equivalent certificates, 40 expected negative
  rejections, 56 successful command records, source/runtime inventories,
  retention, and detached verification. The formal decision is recorded in
  `docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION.md`.
  The blocked `6fa00bd` attempt remains negative evidence; neither production
  math nor Geometry behavior changed.
- Durable-retention audit (2026-09-16): the original package was located and
  copied byte-for-byte to
  `evidence/geometry-primitives/minimal-small-linear-algebra/la0-la7-3804e90-prepared-20260913-02/`.
  All 201 manifest-declared entries passed size and SHA-256 verification against
  retention manifest
  `24894a2cf9875254862a27ac80582d2652a2543c891a59ed7920478d7c6aeb65`;
  reproducible build/cache extras were not imported.
- LA-only integration regression (2026-09-16): clean published candidate
  `95258e9` passed LA0-LA7 in the same four-cell envelope. All 56 commands
  succeeded; 12 certificates, 40 expected negative rejections, identical
  cross-cell projection, detached verification, and a 201-entry hash-verified
  external retention package were audited. Only the compact canonical summary
  is tracked at
  `evidence/geometry-primitives/minimal-small-linear-algebra/la0-la7-95258e9-integration-summary.json`.
  No production math, Geometry adapter, or later Geometry capability changed.

#### Transformations and Coordinate Frames

Status: `QUALIFIED / CF0–CF7 PASS / candidate 8e6b587 / WSL Ubuntu 24.04`

- Authority:
  `docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md`.
- The first bounded capability is `CartesianFrame2`/`CartesianFrame3`: finite
  origin, exact signed-permutation `Mat2`/`Mat3` basis, positive power-of-two
  scale, and local/world maps that keep points distinct from vectors.
- Exclude general affine transforms, arbitrary-angle rotation, general inverse,
  approximate validation, predicates, topology, curves, surfaces, meshes, and
  native-Windows qualification.
- The focused analytic contract passed on GCC 13 and Clang 18 libc++, in Debug
  and Release. The revision-bound CF0–CF7 protocol and admission
  infrastructure, including an immutable PREPARED consumer, exact CTest
  discovery/execution binding, terminal partial-failure records, tool executable
  identities, explicit per-cell/cross-cell and JSON/Markdown derived-evidence
  recomputation, and detached-worktree retention,
  are recorded at
  `docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`.
  The replacement manifest for clean published candidate `8e6b587` was
  executed once at a distinct external root and audited `PASS` for CF0–CF7:
  twelve identical semantic certificates, exact prerequisite preservation,
  60 zero-exit command records, and detached-verified canonical retention.
  The earlier `e69e804` package remains immutable
  `BLOCKED_BY_COMPILE_COMMANDS_SCHEMA_DEFECT` tooling evidence. Cartesian
  Similarity Frames alone was qualified at that point; the later cumulative
  Geometry Primitives regression is recorded below as the stage-closing result.

#### Geometry Primitives Regression

- Authority:
  `docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md`.
- Run one revision-bound cumulative campaign over Point/Vector, Minimal Small
  Linear Algebra, and Cartesian Similarity Frames on the same clean candidate.
- Exercise the sealed semantic CTest allowlist and integrated analytic cases
  across the declared four-cell WSL compiler/build matrix.
- Verify finite-state and error preservation, exact admitted compositions,
  scale and signed-zero semantics, repeatability, cross-cell equivalence,
  Foundation preservation, isolation from topology, and retained-evidence
  integrity through GPR0-GPR7.
- Produce compact numerical tables and certificates. A spatial figure is not
  required for these algebraic value types and coordinate maps.

Stage exit gate: primitive operations are analytically verified across the declared scale envelope, have no topology semantics, and Geometry Primitives Regression passes.

### Topological Model — Explicit Identity and Incidence

Status: `QUALIFIED / PRODUCTION IMPLEMENTATION COMPLETE /
FOCUSED CONTRACTS PASS / SECOND CUMULATIVE REGRESSION COMPLETE /
TMR0-TMR7 PASS / candidate 37f9af77 / GitHub Ubuntu 24.04 x86_64`

Goal: represent the patch complex without inferring topology from geometry.

Entry authority:
`docs/decisions/TOPOLOGICAL_MODEL_ENTRY_DECISION.md`. The first bounded work
unit is limited to strong `VertexId`/`EdgeId` semantics, explicit edge
endpoints, oriented edge uses, deterministic mutable construction, and atomic
finalization into an immutable topology model. It authorizes no curve, patch,
surface, face-cycle, manifold, welding, serialization, or meshing behavior.
The complete stage requirements below remain mandatory later work and are not
reduced by this entry decision.

The bounded candidate implements only
`include/apmesh/topology/topology.hpp`, `src/topology/topology.cpp`, and the
focused `apmesh_core.topological_model` CTest. Its GCC 13 Debug and Clang 18
libc++ Debug focused contracts pass in the declared WSL Ubuntu 24.04 envelope.
This is implementation evidence only: no stage qualification, face/patch
incidence, manifold behavior, canonical serialization, or formal campaign is
authorized by this result.

The second bounded contract in the same decision document implements only
strong topological `FaceId`, ordered non-empty boundary loops of existing
`EdgeUse` values, arbitrary positive valence, atomic builder insertion, and
immutable lookup. It separates `FaceId` from future `PatchId`, permits multiple
loops without outer/inner semantics, and preserves repeated-edge and arbitrary
face-incidence cases for later manifold classification. Its focused CTest passed
in GCC 13 Debug and Clang 18/libc++ Debug for the declared risk cases. It does
not qualify the stage or authorize any excluded topology/geometry behavior.

The third bounded contract implements only deterministic immutable enumeration
of the reverse relation from each edge to every stored face-boundary
`EdgeUse` occurrence. Records preserve face identity, loop/use ordinals, and
declared orientation without introducing loop/use identity, adjacency,
pairing, manifold classification, geometry, serialization, or qualification.
Its focused GCC 13 Debug and Clang 18/libc++ Debug contract passes.

The fourth bounded contract authorizes only a deterministic structural
classification derived from those records. It records cardinality, orientation
balance, distinct owners, and repetition without interpreting them as
adjacency, pairing, boundary status, manifoldness, fan order, or geometry.
Its focused GCC 13 Debug and Clang 18/libc++ Debug contract passes. The fifth
bounded contract below defines the authorized continuation.

The fifth bounded contract consolidates the immutable model before cumulative
regression. It now performs full internal-consistency revalidation, exposes a
read-only count summary, and emits a byte-exact canonical `apmesh-topology-v1`
snapshot of the authoritative identity/incidence relation. Its focused GCC 13
Debug and Clang 18/libc++ Debug contracts pass. It does not authorize repair,
deserialization, adjacency, pairing, manifold policy, geometry, or a formal
campaign.

The single stage-exit regression is pre-registered in
`docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md`. TMR0–TMR7
bind the completed topology model, current-candidate Foundation/Geometry
preservation, exact forward/reverse incidence, structural recomputation,
canonical snapshot bytes, repeated/cross-cell determinism, and retained
evidence integrity.

The first formal cloud campaign was authorized by PR #29 and executed exactly
once in run `35528077223` against candidate
`e5eda2663d6ff4b93ce1205660ff04d432acb9c0`. The immutable execution claim
exists and the formal attempt is consumed. Terminal artifact `10610497080`
was independently audited. Package integrity, detached verification, eight
topology certificates, cross-cell equivalence, negative outcomes, and all
recorded commands are internally consistent.

The terminal scientific audit is
`docs/audits/2026-09-20-topological-model-tmr-terminal-audit.md`. It records
TMR0–TMR5 `PASS`, TMR6–TMR7 `BLOCKED`, and overall `BLOCKED`. The blocker
is evidence cardinality: the protocol requires the exact seven-test semantic
allowlist once in each of two repetitions per cell, but the sealed runner
executed semantic CTest once per cell while repeating only certificate
production. Existing semantic CTests are 7/7 PASS in all four cells; there is
no observed production-semantic contradiction. A new campaign is forbidden
until a separately authorized diagnosis resolves the protocol/runner
repetition-cardinality mismatch.

PR #30 integrated this terminal audit as
`83a135127302ca328bf49e3e71fbb8ac2e16da2b`. Post-merge FAST
`35529230596` and INTEGRATION `35529230618` passed. The next bounded work
is therefore the separately authorized repetition-cardinality diagnosis; no
curve work, runner correction, preparation, or new formal execution is yet
authorized.

The repetition-cardinality diagnosis is now complete in
`docs/decisions/TOPOLOGICAL_MODEL_TMR_REPETITION_CARDINALITY_DIAGNOSIS.md`.
It confirms a mechanical runner defect, not a production-topology regression.
For every cell, future repetitions must each invoke build, rediscover the exact
allowlist, run semantic CTest, produce a certificate, and validate it. Configure
remains cell-scoped. The required future command cardinality is 56 records over
the fixed four-cell/two-repetition matrix.

The diagnosis also records a fragile protocol-title guard exposed by TMR
tooling run `35529611062`. The next bounded work after diagnosis integration
is one focused mechanical correction of the repetition scope, inventories,
focused contracts, and stable protocol guard. No formal preparation or
execution is authorized by that correction.

PR #33 integrated the diagnosis as
`da47c01da9cfcabafca4638c02006be8f1372aea`. Required PR FAST
`35529950706` and INTEGRATION `35529950569` passed; post-merge FAST
`35529991911` and INTEGRATION `35529991909` passed. The next bounded work
is now the focused mechanical runner/protocol-guard correction only.

That correction was implemented on
`topology/tmr-repetition-cardinality-correction`. The runner now configures
each cell once and executes build/discovery/semantic CTest/certificate/
validation in each of the two repetitions, yielding the diagnosed 56-command
shape. The stable protocol guard no longer depends on a transient section
title. Final branch-head TMR Tooling run `35530579354` passed in GCC 13 Debug
and Clang 18/libc++ Debug.

PR #35 integrated the correction as
`6df723b68d265e2e3081a774d7312aa227fdcef6`. Required PR FAST
`35530643533` and INTEGRATION `35530643582` passed; post-merge
INTEGRATION `35530685060` and FAST `35530685096` also passed. No
production topology or scientific acceptance criterion changed.

The corrected formal PREPARED package was created by run `35531261000`
against canonical `main` candidate
`37f9af77f38e12af0a92d3c0f57f1ad31a218144`.

Independent preparation audit records **PASS / PREPARED / NOT EXECUTED**:

- artifact `10611054028`;
- ZIP SHA-256
  `96a47fcecc524e0a4baccee899bd88be8443dbc9778a55271a8376ebe2f6a1ab`;
- manifest SHA-256
  `f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`;
- seal SHA-256
  `366c782c571f6e63e320ac65e51dc464b0e3b3c8de498cfaf49ef50672dba9c2`;
- exact 1546-file candidate inventory agreement;
- 12/12 critical input hashes matched;
- all cloud observations PASS;
- corrected 56-command / 112-log planned campaign;
- no execution claim or terminal evidence.

Audit authority:
`docs/audits/2026-09-20-topological-model-tmr-corrected-preparation-audit.md`.

The next bounded work after audit integration is **not execution yet**. The
existing repository-resident authorization machinery remains hard-bound to the
consumed first PREPARED package. One separate authorization-binding/
generalization work item must safely bind the audited second package before a
new `EXECUTE_ONCE` record becomes admissible.

PR #37 integrated the corrected preparation audit as
`1f004a06aa9c6c72e4053b23c67ce514e322369d`. Post-merge INTEGRATION
`35531732329` and FAST `35531732270` passed. The next bounded work is now
the authorization-binding/generalization change only.

The generic authorization-binding implementation is now complete on
`topology/tmr-generic-authorization-binding`. It preserves the one-file
repository authorization event while removing campaign-specific constants from
the controller/executor, structurally binds machine-readable PREPARED audits,
checks GitHub artifact digest/provenance, and derives claim/concurrency from the
manifest hash. TMR Tooling run `35532220165` passed in GCC 13 Debug and
Clang 18/libc++ Debug. No `EXECUTE_ONCE` record has been added by this work
item.

PR #39 integrated the generic binding as
`3b5febcbda49977e834708344f561e6cba074fbf`. Final branch-head TMR Tooling
`35532329855`, PR FAST `35532379951`, PR INTEGRATION `35532379945`,
post-merge FAST `35532410672`, and post-merge INTEGRATION
`35532410659` all passed. The sole next bounded work is now the exact
one-file `EXECUTE_ONCE` authorization for the audited second PREPARED
manifest.

A final pre-authorization audit found that the controller's commit diff was
restricted to `experiments/authorizations`, so unrelated changes in the same
commit would not have been visible to the isolation check. The focused
`topology/tmr-authorization-whole-commit-guard` correction removes that
pathspec and strengthens the static contract. TMR Tooling run `35532624980`
passed in both declared tooling cells. Authorization remains deferred until
this guard correction is separately integrated and closed.

PR #41 integrated the correction as
`d4a3a2da64c84ec922e881e899a153287593b79c`. Final TMR Tooling
`35532708479`, PR FAST `35533302056`, PR INTEGRATION `35533302046`,
post-merge FAST `35533347184`, and post-merge INTEGRATION
`35533347164` all passed. The correction checkpoint is closed. The sole next
bounded work is the exact one-file `EXECUTE_ONCE` authorization for the
audited second PREPARED package.

PR #43 then merged the exact one-file second `EXECUTE_ONCE` authorization as
`cddd959574ed6a677ac755a5b329d53a9cfe32ec`. Protected-main run
`35533702004` validated the authorization, audited PREPARED binding, exact
artifact provenance and admitted cloud environment; created the immutable
manifest-hash execution claim; executed the corrected sealed campaign exactly
once; verified retention; and retained terminal artifact `10612032787`.

Independent terminal audit is retained in
`docs/audits/2026-09-20-topological-model-tmr-corrected-terminal-audit.md`.
It recomputes the complete terminal package and records TMR0–TMR7 `PASS`,
overall `PASS`. The package contains the required 56 command records,
eight exact semantic CTest repetitions (56 individual semantic tests, all
passing), eight byte-identical topology certificates, complete negative and
dependency evidence, detached verification, and exact retention.

Therefore the Topological Model stage is **QUALIFIED in the exact formally
admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud envelope**. This does not
establish WSL/cloud equivalence and does not expand any excluded topology or
geometry claim.

Per the stage-exit protocol, curve implementation does not start automatically.
After terminal-audit integration and closure, the next bounded transition is a
separate scientific entry decision for **Curve Representation — Continuous
Geometry Before Discretization**.

PR #44 integrated the qualifying terminal audit as
`bc9c82275fa91d8a756f831ea4af506ab3bbcfa8`. PR FAST
`35534295054`, PR INTEGRATION `35534295078`, post-merge FAST
`35534347597`, and post-merge INTEGRATION `35534347623` passed. The
Topological Model stage-exit checkpoint is therefore closed. The next bounded
work is the separate Curve Representation scientific entry decision only.

#### Vertex and Edge Identity

- Define strong `VertexId`, `EdgeId`, and `FaceId` topological types. Curve,
  patch, and surface identity remain separate future layers.
- Establish that coincident coordinates do not imply shared identity.
- Add adversarial coincident-but-disconnected tests.

#### Edge Use and Orientation

- Define explicit edge uses/coedges and forward/reverse orientation.
- Verify manifold seams, reversed seams, and non-manifold fans.
- Reject contradictory or incomplete incidence before model construction.

#### Face Identity and Boundary Cycles

- Represent a face with one or more ordered, closed `EdgeUse` cycles of
  arbitrary positive valence.
- Validate cycle closure using resolved vertex identity only; never coordinates
  or tolerance.
- Preserve multiple loops without premature outer/inner, trimming, winding, or
  nesting semantics.
- Keep `FaceId` strongly distinct from future `PatchId`; do not assume a
  universal four-sided patch.
- Admit repeated-edge and arbitrary face-incidence cases without prematurely
  classifying manifoldness.

#### Immutable Validated Model

- Implement mutable `ModelBuilder` followed by validation/finalization into immutable `Model`.
- Define complete side-coverage and incidence consistency rules.
- Produce canonical deterministic topology serialization/hash for experiments.

#### Topological Model Regression

- Execute the single pre-registered TMR0–TMR7 workflow on the fixed four-cell,
  two-repetition matrix.
- Exercise exact identity, boundary-cycle, incidence, signature, consistency,
  transactionality, and canonical-snapshot cases without semantic
  boundary/manifold classification.
- Preserve Foundation and Geometry through the exact current-candidate
  semantic allowlist; do not relaunch their historical campaigns.
- Retain canonical certificates and compact decision evidence. No figure is
  required before a geometric embedding exists.
- Verify that no coordinate proximity changes topological identity.

Stage exit gate: canonical synthetic models reproduce declared topology exactly without coordinate-based welding, and Topological Model Regression passes.

### Curve Representation — Continuous Geometry Before Discretization

Status: `QUALIFIED / CGR0–CGR7 PASS / candidate f7dc8d82 /
GitHub Ubuntu 24.04 x86_64 / ALL ADMITTED CONTINUOUS-CURVE WORK UNITS
INTEGRATED`

Goal: certify continuous curve representation independent of meshing.

Entry authority:
`docs/decisions/CURVE_REPRESENTATION_ENTRY_DECISION.md`.

The first bounded work unit is **Polynomial Cubic Bézier Value Representation
and Evaluation**. It admits only immutable 2D/3D polynomial cubic Bézier value
types, four ordered finite control points, normalized parameter domain
`[0,1]`, recursive de Casteljau evaluation using component-wise
`std::lerp`, exact endpoint semantics, geometric reversal, and focused
analytic/adversarial evidence.

It explicitly excludes derivatives, regularity, curvature, arc length,
subdivision APIs, rational/arbitrary-degree splines, topology ownership,
discretization, quadrilateral meshing, and parallel execution. Passing the
focused contract will establish only `IMPLEMENTED / FOCUSED CONTRACTS PASS /
NOT QUALIFIED`; stage qualification remains a later cumulative regression.

PR #46 integrated the entry authority as
`c76e2946c8c9ffec658e4c8aa146f1abdca62f33`. PR FAST
`35536256853`, PR INTEGRATION `35536256854`, post-merge FAST
`35536324878`, and post-merge INTEGRATION `35536324875` passed. The entry
checkpoint is closed. The sole next work item is the bounded cubic
representation/evaluation implementation.

The implementation branch is
`curve/cubic-bezier-value-evaluation`. It remains limited to the fixed cubic
value/evaluation contract. FAST `35541613233` and INTEGRATION
`35541613216` passed, with both new curve contracts and all selected
prerequisite semantic tests passing in GCC 13 Debug and Clang 18/libc++ Debug.

The work-unit result is **IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT
QUALIFIED**. Stage-level qualification remains deferred. After integration and
closure, the next bounded transition is a separate Curve Derivatives and
Regularity decision; derivative implementation does not begin automatically.

PR #48 integrated the first curve work unit as
`bde874311d9960c5fab7ce03b26b6cd5fbd61b34`. Final PR FAST
`35541914952`, PR INTEGRATION `35541914958`, post-merge FAST
`35541963486`, and post-merge INTEGRATION `35541963489` passed. The cubic
value/evaluation checkpoint is closed. The sole next bounded transition is the
separate Curve Derivatives and Regularity decision.

#### Cubic Bezier Evaluation

- Implement `CubicBezier2`/`CubicBezier3` value semantics from four
  ordered qualified points.
- Evaluate only on `t∈[0,1]` by recursive de Casteljau interpolation in
  Bernstein form; do not convert to the power basis.
- Verify exact endpoints, reversal, translation/admitted-frame covariance,
  degenerate control polygons, finite extreme-value fixtures, explicit
  parameter failures, and deterministic repeatability.
- Compare selected fixtures against independent exact/high-precision
  expectations under explicit `ProximityPolicy`; no global epsilon.

#### Curve Derivatives and Regularity

Decision authority:
`docs/decisions/CURVE_DERIVATIVES_REGULARITY_DECISION.md`.

PR #50 integrated the decision as
`9c3caa35b580402fa0d7ce71412f3def7bbd8aa4`. PR FAST
`35542378763`, PR INTEGRATION `35542378797`, post-merge FAST
`35542416703`, and post-merge INTEGRATION `35542416695` passed. The
decision checkpoint is closed; the next bounded work is differential
evaluation/pointwise speed implementation only.

The bounded first differential work unit is **Cubic Bézier Differential
Evaluation and Pointwise Speed**:

- evaluate first derivative as the quadratic Bézier hodograph;
- evaluate second derivative as the linear derivative of that hodograph;
- evaluate pointwise speed with the qualified stable norm;
- verify endpoint/hodograph formulas, reversal signs, translation invariance,
  admitted-frame covariance, constant/linear/stationary fixtures, finite
  extremes and deterministic repeatability;
- treat exact zero derivative as valid pointwise data.

Global interval regularity is deliberately separated from this work unit.
Sampling does not prove `B'(t) != 0` for every `t∈[0,1]`; therefore no
`is_regular()` or equivalent interval-wide claim is authorized here. A later
Global Cubic Regularity Certification decision may investigate complete root /
interval evidence after differential evaluation is integrated.

Implementation branch `curve/cubic-bezier-differential-evaluation` now
provides first derivative, second derivative and pointwise speed for
`CubicBezier2`/`CubicBezier3` within that exact boundary. PR #52 FAST
`35544242913` passed; PR #52 INTEGRATION `35544242911` passed in GCC 13
Debug and Clang 18/libc++ Debug. The work unit was integrated by PR #52 as
`1b1da2893168e5f3da3fb98595809d7210f49f6b`. Final PR FAST
`35544348814`, PR INTEGRATION `35544348767`, post-merge FAST
`35544417439`, and post-merge INTEGRATION `35544417474` all passed.

The result is **IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT
QUALIFIED**.

The next bounded transition is the separate **Global Cubic Regularity
Certification** decision. No global regularity claim follows from pointwise
speed sampling.


#### Global Cubic Regularity Certification

Status: `IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED`

Authority:
`docs/decisions/CURVE_GLOBAL_REGULARITY_CERTIFICATION_DECISION.md`.

The bounded method certifies regularity through conservative Bernstein
enclosures of the quartic squared-speed polynomial
`s(t)=B'(t)·B'(t)`. Recursive midpoint subdivision covers the complete
`[0,1]` interval. A curve is reported regular only when every retained leaf
has a strictly positive lower Bernstein coefficient bound.

The result vocabulary explicitly separates `regular`, exact witnessed
`degenerate`, and `indeterminate`. Resource exhaustion or insufficient
floating enclosure may only produce `indeterminate`.

Sampling, speed epsilon, general root solving, public interval arithmetic,
arc-length integration, curvature, discretization and parallel execution remain
excluded.


PR #55 integrated the bounded decision as
`224c8ab530f88475c2d8281cb60682a7c0db851a`. PR FAST
`35549162022`, PR INTEGRATION `35549161971`, post-merge FAST
`35549240099`, and post-merge INTEGRATION `35549240093` passed. The
decision checkpoint is closed; implementation of the bounded certifier is now
the sole next work item.


The bounded implementation candidate now provides the explicit certification
policy/result vocabulary, private conservative interval enclosure operations,
quartic squared-speed Bernstein construction, complete midpoint subdivision and
focused 2D/3D regularity evidence. PR #57 FAST `35549795835` and INTEGRATION `35549795743` passed in the
declared GCC/Clang cells. No excluded downstream capability is included.


PR #57 integrated the bounded certifier as
`e0830b19e760b0e08162bb08c93b0462b60c1191`. Final PR FAST
`35549866348`, PR INTEGRATION `35549866244`, post-merge FAST
`35549946614`, and post-merge INTEGRATION `35549946583` passed. The
regularity work-unit checkpoint is closed.

#### Arc Length and Parameter Mapping

Decision authority:
`docs/decisions/CURVE_ARC_LENGTH_PARAMETER_MAPPING_DECISION.md`.

The investigation is intentionally split into two bounded work units.

**Work unit 1 — Certified Cubic Bézier Total Arc-Length Enclosure**

- use deterministic dyadic de Casteljau subdivision;
- use conservative chord lower bounds and control-polygon upper bounds;
- retain a finite enclosure `[lower, upper]`;
- report `converged` only when the explicit global width policy is proven;
- return `indeterminate` with the best valid enclosure when resources are
  exhausted;
- require no global-regularity precondition for total length;
- verify straight, constant, degree-elevated parabola, reversal, translation,
  power-of-two scale, 2D/3D parity, stationary, extreme and resource-limited
  cases;
- keep conservative arithmetic private to the curve module.

Certified total-length implementation is complete on
`curve/cubic-bezier-total-arc-length-enclosure`. It uses deterministic
edge-vector dyadic subdivision, conservative chord/control-polygon enclosure,
private outward arithmetic, explicit `converged/indeterminate` evidence, and
a scaled interval norm using correctly-rounded `sqrt`. Final FAST
`35552642188` and INTEGRATION `35552642196` pass in GCC/Clang.

PR #61 integrated the bounded implementation as
`5d89edfd391dc5548245f35ccedc2ac4c6c6951a`. Final PR FAST
`35552739398`, PR INTEGRATION `35552739395`, post-merge FAST
`35580244685`, and post-merge INTEGRATION `35580244722` all passed.
Work unit 1 is therefore **IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
NOT QUALIFIED**. The next bounded transition is the separate cumulative/inverse
mapping decision.

**Work unit 2 — Cumulative Arc-Length Mapping and Certified Inverse Bracketing**

Decision authority under integration:
`docs/decisions/CURVE_CUMULATIVE_ARC_LENGTH_INVERSE_BRACKETING_DECISION.md`.

This investigation is further split to avoid coupling two new guarantees.

**Work Unit 2A — Certified Cumulative Arc-Length Enclosure**

- certify `S(t)=length(B|[0,t])` for `t in [0,1]`;
- retain a conservative prefix-length enclosure, never only a scalar estimate;
- use private outward prefix construction and the integrated certified
  total-length semantics;
- require no global regularity precondition for forward cumulative length;
- preserve endpoint, reversal, embedding, translation, scale and deterministic
  evidence;
- remain serial and independent of physical discretization.

**Work Unit 2B — Certified Inverse Arc-Length Bracketing**

- remains blocked until Work Unit 2A is implemented, integrated and closed;
- requires same-curve global regularity before claiming a unique inverse;
- must retain a proven parameter bracket rather than one unqualified estimate;
- must treat normalized-fraction targets through the retained total-length
  enclosure;
- uses deterministic bracket refinement as the correctness mechanism;
- Newton/secant acceleration, lookup tables or fitted approximate
  parameterizations cannot be correctness authorities.

The total-length Work Unit 1 checkpoint was closed by PR #62 merged as
`689cd9b44d97eb53e6d9aa3f88d96a846ecaf8c3`; post-merge FAST
`35580547608` and INTEGRATION `35580547591` passed.

PR #63 integrated the cumulative/inverse decision as
`32428d29407949f058d44bf2dfdcab59600714c1`. PR FAST `35581198802`,
PR INTEGRATION `35581198681`, post-merge FAST `35581291836`, and
post-merge INTEGRATION `35581291965` passed. The decision checkpoint is
closed.

Work Unit 2A — Certified Cumulative Arc-Length Enclosure is
**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED**.
PR #66 merged it as
`3cfb580cae2e2d26e87e9dfcfeab0aade0a3a3be`. Final PR FAST
`35591661827`, final PR INTEGRATION `35591661836`, post-merge FAST
`35591774007`, and post-merge INTEGRATION `35591773982` passed. The
implementation builds certified prefix edge enclosures directly from original
represented control data with outward de Casteljau algebra and reuses the
integrated total-length enclosure engine.

Work Unit 2A is closed.

Work Unit 2B — Certified Inverse Arc-Length Bracketing has an
**INTEGRATED / CLOSED IMPLEMENTATION DECISION**.

Decision authority:
`docs/decisions/CURVE_INVERSE_ARC_LENGTH_BRACKETING_IMPLEMENTATION_DECISION.md`.

PR #68 integrated the documentation-only decision as
`f0faaf53e5b898e0270fc0e406cf7337e8d95bb1`. PR FAST
`35592555795`, PR INTEGRATION `35592555778`, post-merge FAST
`35592839727`, and post-merge INTEGRATION `35592839775` passed.

The bounded implementation is now authorized. It must retain same-curve
regularity authority, total and cumulative length uncertainty, certified
parameter brackets, deterministic midpoint bisection and explicit
indeterminate/resource semantics. No lookup/sampling approximation, physical
discretization, surface, quadrilateral or parallel capability is admitted.

The bounded implementation is complete on
`curve/certified-inverse-arc-length-bracketing` and is
**IMPLEMENTED / FOCUSED CONTRACTS PASS / VALIDATED_UNMERGED / NOT QUALIFIED**.

The public result is an inspectable certified parameter bracket retaining
same-curve regularity, total-length, lower-cumulative and upper-cumulative
evidence. Absolute and normalized-fraction modes preserve target uncertainty;
deterministic midpoint bisection is the only refinement authority. Ambiguous
midpoint and iteration-resource paths retain the last valid bracket and return
`indeterminate`.

PR #70 validation:
- FAST `35593878035`: PASS;
- INTEGRATION `35593878082`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

No physical sampling, boundary discretization, surface, quadrilateral,
parallel or qualification capability is introduced. After integration and
closure, the next bounded transition is pre-registration of the Continuous
Curve Geometry Regression.

PR #70 integrated Work Unit 2B as
`a82fa1c96fc6665e586753d5e4eb698012a79be3`. Final PR FAST
`35594075619`, final PR INTEGRATION `35594075595`, post-merge FAST
`35594160853`, and post-merge INTEGRATION `35594160817` passed.

Work Unit 2B is therefore **IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSED / NOT QUALIFIED**. The next bounded transition is now only
pre-registration of the **Continuous Curve Geometry Regression**. Physical
boundary discretization remains blocked until the Curve Representation stage is
qualified.

Gauss–Kronrod remains diagnostic/reference-only at this stage: its nested-rule
difference is an error estimate, whereas the first work unit requires an
explicit conservative enclosure.

No physical discretization, curvature, surface, quadrilateral or parallel
capability is admitted by this decision.

PR #59 integrated the original Arc Length and Parameter Mapping decision as
`65cd93818fa54eac00c6f63ebefa3615074a82cd`; Work Unit 1 was subsequently
implemented by PR #61 and closed by PR #62. The next transition is governed by
the separate cumulative/inverse decision above.

#### Continuous Curve Geometry Regression

Status: **PRE-REGISTERED / DOCUMENTATION-ONLY / NO FORMAL EXECUTION
AUTHORIZED**.

Protocol authority:
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`.

The pre-registered stage-exit regression fixes:

- rerun the exact fourteen-test prerequisite + curve semantic allowlist;
- execute four admitted cloud cells × two repetitions;
- retain eight independently validated curve scientific certificates;
- require same-cell deterministic certificate projections;
- independently validate cross-cell analytic/metamorphic relations;
- regenerate deterministic curve/differential/arc-length/inverse regression
  data and SVG evidence;
- retain negative/adversarial and dependency-isolation evidence;
- decide CGR0–CGR7 independently of workflow success.

Planned successful command cardinality is 56 command records and 112 command
logs. The eight semantic CTest repetitions execute 14 tests each, therefore
112 individual semantic test executions are required.

Stage exit gate: Curve Representation is qualified only if CGR0–CGR7 all PASS
after one separately prepared and authorized formal campaign. Physical
boundary discretization remains blocked until that qualification is integrated
and closed.

PR #73 integrated the protocol as
`af580a9358428e1607c77f7557355595a0a7c45c`. PR FAST
`35598147692`, PR INTEGRATION `35598147572`, post-merge FAST
`35598248171`, and post-merge INTEGRATION `35598248036` passed. The
protocol checkpoint is closed. The sole next bounded work is report-only CGR
tooling; formal preparation/execution remains deferred.

The report-only CGR tooling is implemented on
`curve/continuous-geometry-regression-tooling` and independently audited in
`docs/audits/2026-09-21-continuous-curve-geometry-regression-tooling-audit.md`.

The tooling fixes the exact four-cell × two-repetition campaign plan without
executing it: 56 command records, 112 command logs, eight discoveries, eight
semantic CTest records, 112 individual semantic test executions and eight
certificates. It adds independent analytic/metamorphic certificate validation,
real fail-closed adversarial forgery checks, and deterministic CSV/JSON/SVG
regression evidence. All eleven protocol-frozen semantic files remain
byte-identical to baseline `438620efa1f93d29b442e9ba199882a09d2359d9`.

Focused CGR Tooling run `35601303879` passed in GCC 13 Debug and Clang
18/libc++ Debug. This remains report-only: CGR0–CGR7 are `NOT_EXECUTED`,
Curve Representation is not qualified, and formal preparation/execution remain
deferred until this tooling is integrated and its checkpoint is closed.

PR #75 integrated the report-only tooling as
`6e3952f2bdee5ca9bdfe076d5af2932b359d130f`. Final branch-head tooling
`35602067270`, PR FAST `35602217696`, PR INTEGRATION `35602217726`,
post-merge FAST `35603677515`, and post-merge INTEGRATION
`35603677377` all passed. The report-only tooling checkpoint is closed.

The sole next bounded work is one separate formal PREPARED-package design
decision. It must freeze preparation/lifecycle/revision-binding requirements
before any formal preparation implementation. Formal execution remains
unauthorized.

The design is now specified in
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md`.
It keeps the report-only runner immutable, requires a separate formal runner,
seals the future executor/authorization path before preparation, defines the
exact seven-file PREPARED package and independent audit boundary, and preserves
the 56-command / 112-log / eight-certificate / 112-semantic-test campaign
shape. No formal package or execution is authorized by the decision itself.

After decision integration and checkpoint closure, the next bounded work is
formal campaign infrastructure implementation only.

PR #77 integrated the PREPARED lifecycle design as
`7ff5c6e5f52f9f5bd8153d00856239649aac0eff`. PR FAST
`35604864218`, PR INTEGRATION `35604864050`, post-merge FAST
`35604986634`, and post-merge INTEGRATION `35604986654` passed. The
design checkpoint is closed. The next bounded work is formal CGR campaign
infrastructure implementation only; no formal package or execution is yet
authorized.

Formal campaign infrastructure is now implemented on
`curve/cgr-formal-campaign-infrastructure` under
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md`.

The implementation adds the separate formal runner, preparation-only workflow,
reusable one-shot executor, protected-main authorization-as-code, complete-
commit isolation, manifest-hash claim semantics, synthetic success/BLOCKED
retention contracts, and dedicated formal-tooling validation without changing
any frozen semantic file.

Formal tooling runs `35613409036` and final synchronized-head run
`35613745665` passed in GCC 13 Debug and Clang 18/libc++ Debug. The exact
formal plan remains 56 commands, 112 command logs, eight semantic repetitions,
112 individual semantic tests and eight certificate slots.

Infrastructure audit authorities:

- `docs/audits/2026-09-21-continuous-curve-geometry-regression-formal-infrastructure-audit.md`;
- `docs/audits/2026-09-21-continuous-curve-geometry-regression-formal-infrastructure-audit.json`.

No real PREPARED package, authorization, claim, terminal package or CGR gate
decision exists. After infrastructure integration and closure, the next
bounded step is exactly one formal PREPARED dispatch from clean canonical
`main`, followed by independent PREPARED audit. Formal execution remains
unauthorized.

PR #79 integrated the formal campaign infrastructure as
`a0232e0c00aae1338b55ba0b45997db1a3c00464`. The validated branch head and
merge commit have identical Git tree
`2ece9368e9b48a0c1db7fc1494a8713a833a9b43`. Final formal tooling
`35614078272`, PR FAST `35614279053`, PR INTEGRATION `35614278985`,
post-merge FAST `35614426119`, and post-merge INTEGRATION
`35614426153` all passed. The infrastructure checkpoint is closed.

The sole next bounded action is one formal PREPARED dispatch from canonical
clean `main`, followed by independent PREPARED audit; no formal execution is
yet authorized.

The formal PREPARED dispatch was executed as run `35620525792` on canonical
`main` candidate `f7dc8d82d881858b6481d6d2d1383d8a561684c5`.

Independent preparation audit records **PASS / PREPARED / NOT EXECUTED**:

- artifact `10649325906`;
- ZIP SHA-256
  `952cadc3d5cc761105d5100319cf24077cd9b0ac5d0a42000a8ae1e3eac91063`;
- prepared-manifest SHA-256
  `201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`;
- preparation-seal SHA-256
  `686c5192f5373c42e54339fdd38519e62ebef009dc74ae927c34fe97919b5353`;
- exact 1595-path candidate inventory agreement;
- 18/18 critical inputs and 11/11 frozen semantics independently matched;
- all four admitted cloud observations PASS;
- exact 56-command / 112-log / eight-semantic-repetition /
  112-individual-test / eight-certificate planned campaign;
- CGR0–CGR7 remain `NOT_EXECUTED`.

Audit authority:
`docs/audits/2026-09-21-continuous-curve-geometry-regression-preparation-audit.md`.

After audit integration and closure, the sole next bounded action is one exact
manifest-bound `EXECUTE_ONCE` authorization PR. Formal execution remains
unauthorized until that later one-file merge.

PR #81 integrated the PREPARED audit as
`988d0877d78ccd0c1ed4d368a802a8a4cad28d7b`. PR FAST
`35629544372`, PR INTEGRATION `35629544344`, post-merge FAST
`35629633009`, and post-merge INTEGRATION `35629632897` all passed.
The PREPARED-audit checkpoint is closed. The sole next bounded action is the
exact one-file `EXECUTE_ONCE` authorization for manifest
`201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`.

#### Continuous Curve Geometry Regression — stage qualification

Formal PREPARED package run `35620525792` bound clean candidate
`f7dc8d82d881858b6481d6d2d1383d8a561684c5`, exact frozen semantic
baseline `438620efa1f93d29b442e9ba199882a09d2359d9`, 1595 tracked source
paths, the admitted four-cell cloud matrix, and the pre-registered
56-command / 112-log / eight-certificate plan.

The PREPARED audit was integrated by PR #81 as
`988d0877d78ccd0c1ed4d368a802a8a4cad28d7b` and closed before formal
authorization.

PR #83 then merged the exact one-file `EXECUTE_ONCE` record as
`e8b17256924e907d0859b8ac7061600ffc404b9e`. Protected-main run
`35630423134` validated the authorization and artifact provenance, created
the immutable manifest-hash claim, executed the sealed campaign exactly once,
verified retention, and retained artifact `10654358199` with independently
recomputed archive SHA-256
`a4b59453dcc9f9cacb4265ba540e1a3a443f3b6c80509411e0d6d4b18eff7aa6`.

Independent terminal audit:
`docs/audits/2026-09-21-continuous-curve-geometry-regression-terminal-audit.md`.

Audit result:

- CGR0 identity and scope: `PASS`;
- CGR1 representation and value: `PASS`;
- CGR2 differential evaluation and speed: `PASS`;
- CGR3 global regularity: `PASS`;
- CGR4 arc length and inverse mapping: `PASS`;
- CGR5 repeat and cross-cell equivalence: `PASS`;
- CGR6 prerequisite preservation and isolation: `PASS`;
- CGR7 evidence integrity and closure: `PASS`;
- overall: `PASS`.

The terminal package retains exactly 56 successful command records, 112
command logs, eight exact 14-test semantic repetitions (112 individual tests,
all passing), eight validated curve certificates, complete negative/dependency
evidence, deterministic derived CSV/SVG evidence, detached verification and
exact retention.

**Curve Representation — Continuous Geometry Before Discretization is
QUALIFIED only in the exact formally admitted GitHub-hosted Ubuntu 24.04
x86_64 cloud environment.**

No WSL/cloud equivalence, physical boundary discretization, surface, meshing,
Quad-Dominant, parallel, GPU/SIMD or anisotropy qualification is implied.

Per the stage-exit protocol, the next bounded scientific transition after
terminal-audit integration and closure is one separate entry decision for
**Curve Differential Geometry — Curvature, Regularity, and Features**.
Boundary Curve Discretization does not start automatically.

PR #84 integrated the qualifying CGR terminal audit as
`d0045767d5a4c7fb910fd3e8aaccea73673fb558`. PR FAST
`35634165038`, PR INTEGRATION `35634165026`, post-merge FAST
`35634295936`, and post-merge INTEGRATION `35634295812` all passed.
The Curve Representation qualification checkpoint is closed. The next bounded
work is the separate Curve Differential Geometry entry decision only.

### Curve Differential Geometry — Curvature, Regularity, and Features

Status: `IN INVESTIGATION / POINTWISE CURVATURE INTEGRATED /
SIGNED PLANAR CURVATURE INTEGRATED /
SIMPLE-INFLECTION INTEGRATED / FOCUSED CONTRACTS PASS / NOT QUALIFIED /
PAUSED FOR REPRESENTATION-BREADTH SEAM`

Goal: certify intrinsic curve differential quantities used by later boundary
discretization without conflating local differential evaluation with global
feature classification.

Entry authority:
`docs/decisions/CURVE_DIFFERENTIAL_GEOMETRY_ENTRY_DECISION.md`.

The first bounded work unit is **Pointwise Curvature Magnitude on Regular Cubic
Bézier Curves**.

Authorized first-unit semantics:

- reuse qualified `B'`, `B''`, speed and vector geometry;
- 2D/3D nonnegative pointwise curvature magnitude;
- exact parameter domain `[0,1]`;
- explicit singular-parameter failure when `B'(t)==0`;
- no epsilon-based singularity threshold;
- regular inflections may return exactly zero curvature;
- reversal/translation/orthogonal-frame invariance;
- reciprocal uniform-scale covariance;
- 2D/3D planar embedding parity;
- scale-aware finite evaluation with explicit non-finite-result failure.

Explicitly deferred:

- signed planar curvature;
- Frenet frames and torsion;
- global curvature bounds/extrema/monotonicity;
- inflection isolation and feature classification;
- discretization and curvature-driven sizing;
- surfaces, meshing, Quad-Dominant and parallel execution.

Mandatory stage regression remains later: all admitted curve-differential
fixtures plus every qualified prerequisite regression and regenerated
curvature/reference evidence before stage qualification.

PR #86 integrated the entry decision as
`e728e89f08b23cd0720e502e3efe0c565198d376`. PR FAST
`35671674354`, PR INTEGRATION `35671674360`, post-merge FAST
`35671824327`, and post-merge INTEGRATION `35671824323` passed. The entry
checkpoint is closed. The sole next bounded work item is implementation of
**Pointwise Curvature Magnitude on Regular Cubic Bézier Curves**.

That first work unit is now implemented on
`curve/pointwise-curvature-magnitude`. The public curve API exposes
2D/3D nonnegative pointwise curvature magnitude with exact singularity
semantics and scale-aware evaluation. Focused FAST `35672497018` and
INTEGRATION `35672497040` pass in the declared GCC/Clang cells.

PR #88 integrated the first work unit as
`b1a279fbe2592cd4fe7f5a688318ac50f0e60d35`. PR FAST
`35672497018`, PR INTEGRATION `35672497040`, post-merge FAST
`35673041777`, and post-merge INTEGRATION `35673041859` passed.

Pointwise Curvature Magnitude is therefore **IMPLEMENTED / FOCUSED CONTRACTS
PASS / INTEGRATED / NOT QUALIFIED**. The next Curve Differential Geometry
investigation requires a separate literature-backed decision; no later work
unit is implied by this closure.

The next bounded decision is now
`docs/decisions/CURVE_SIGNED_PLANAR_CURVATURE_DECISION.md`.

It admits only **Pointwise Signed Curvature on Regular Planar Cubic Bézier
Curves**: a 2D orientation-sensitive local value with reversal/reflection sign
laws, exact singularity semantics, scale-aware evaluation and explicit
magnitude parity. Certified inflection isolation, global curvature bounds,
extrema/monotonicity and feature classification remain separate later
investigations.

PR #90 integrated the signed-curvature decision as
`8da6ad656871c23f26f74f148298283970338583`. PR FAST
`35674524237`, PR INTEGRATION `35674524211`, post-merge FAST
`35674581493`, and post-merge INTEGRATION `35674581550` passed. The
decision checkpoint is closed. The sole next bounded work item is the 2D
pointwise signed-curvature implementation.

That bounded work unit is now implemented on
`curve/signed-planar-curvature`. The API remains 2D-only and shares the
scale-aware planar curvature core with magnitude evaluation. Focused FAST
`35675167119` and INTEGRATION `35675167196` pass in the declared
GCC/Clang cells.

PR #92 integrated the signed-curvature work unit as
`170c8c8a8db8676933e8107a1eb8abb2dedd6204`. Final PR FAST
`35675261376`, final PR INTEGRATION `35675261409`, post-merge FAST
`35675349460`, and post-merge INTEGRATION `35675349468` passed.

Pointwise Signed Curvature is therefore **IMPLEMENTED / FOCUSED CONTRACTS
PASS / INTEGRATED / NOT QUALIFIED**. The next Curve Differential Geometry
investigation again requires a separate literature-backed decision; no later
work unit is implied by this closure.

The next bounded decision is now
`docs/decisions/CURVE_CERTIFIED_SIMPLE_INFLECTION_ISOLATION_DECISION.md`.

It selects **Certified Simple Planar Inflection Isolation on Globally Regular
Cubic Bézier Curves** as the next investigation. The decision reduces
`det(B',B'')` for a planar cubic to its exact quadratic Bernstein form,
requires global regularity certification first, and uses certified Bernstein
sign variation/subdivision to isolate only simple interior roots. Internal
subdivision-boundary root obligations are tracked explicitly so open-interval
sign counting cannot silently lose a root. Multiple, tangential or unresolved
roots remain `indeterminate`; sampled pointwise signed curvature is not a
proof mechanism.

Repository mapping is explicit in the decision: existing public curve API,
production source, private interval enclosure machinery, prerequisite contracts,
future focused test path and documentation authorities are all identified.

No implementation, global curvature bound, extrema, feature classification,
Boundary Curve Discretization, sizing, surfaces, meshing, Quad-Dominant or
parallel execution is authorized until this decision is separately integrated
and closed.

PR #94 integrated the decision as
`44e04205c604abec8dc92f31930a371dc0c56cd1`. Final PR FAST
`35677428464`, PR INTEGRATION `35677428431`, post-merge FAST
`35677498659`, and post-merge INTEGRATION `35677498682` passed.

The decision checkpoint is closed. The sole next bounded work item is the
mapped **Certified Simple Planar Inflection Isolation on Globally Regular Cubic
Bézier Curves** implementation. No later Curve Differential Geometry or
downstream capability is implied.

The mapped implementation is now complete on
`curve/certified-simple-inflection-isolation`:

- public 2D-only isolation policy/result/evidence and
  `CubicBezier2::isolate_simple_inflections`;
- private conservative quadratic Bernstein root evidence;
- explicit internal subdivision-boundary zero protection;
- analytic zero/one/two-root fixtures;
- multiple/near-multiple adversarial evidence;
- reversal/frame/translation/scale covariance;
- deterministic and extreme-coordinate contracts;
- header and dependency isolation through the existing curve regression
  boundary.

Final focused validation passes in FAST `35678475990` and INTEGRATION
`35678475955` for GCC 13 Debug and Clang 18/libc++ Debug.

The work unit was integrated by PR #96 as
`c4905589c2ee8700c58560ef1a99a49a3821af4e`.

Final PR FAST `35678624215`, PR INTEGRATION `35678624192`,
post-merge FAST `35678808956`, and post-merge INTEGRATION
`35678808941` all passed.

The work unit is therefore **IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / NOT QUALIFIED**. The implementation checkpoint is closed.

No later Curve Differential Geometry capability is implied. The next bounded
work is one new literature-backed decision that compares the remaining
candidates before authorizing any production implementation.


### Curve Representation Breadth Gate — Analytic, Rational, and Spline Families

Status: `IN INVESTIGATION / PARAMETRIC CONTRACT INTEGRATED /
LINE SEGMENT INTEGRATED / RATIONAL QUADRATIC BÉZIER INTEGRATED /
ORIENTED TRIM INTEGRATED / TWO-SPAN CUBIC B-SPLINE INTEGRATED /
TWO-SPAN CUBIC NURBS IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSURE PENDING /
NOT QUALIFIED / CUBIC BASELINE QUALIFICATION PRESERVED`

The existing Curve Representation qualification remains valid only for the
frozen polynomial cubic Bézier scope implemented by `CubicBezier2` and
`CubicBezier3`. It does not imply support or qualification for other curve
families.

A separate literature-backed scope-extension decision is mandatory before the
project claims a general boundary-curve representation envelope. That decision
must explicitly evaluate, admit or defer each of the following:

- dedicated line/segment semantics;
- circular and general conic arcs, with exact rather than polynomial
  approximation semantics where admitted;
- arbitrary-degree polynomial Bézier curves;
- rational Bézier curves;
- B-spline curves;
- NURBS curves;
- composite/piecewise and trimmed parameter-interval semantics;
- a reusable C++23 curve abstraction/concept that prevents differential,
  length and later discretization algorithms from being duplicated per concrete
  curve family.

At minimum, every curve family required by the declared doctoral admissible
input class and by the later `line/arc/Bezier` boundary-discretization
regression must be implemented and scientifically admitted before that later
stage can be qualified.

This gate is a scope extension, not a reinterpretation of CGR0–CGR7. The
qualified cubic-Bézier baseline remains frozen and must be preserved by every
extension regression.

Active decision authority:
`docs/decisions/CURVE_PARAMETRIC_FAMILY_ABSTRACTION_DECISION.md`.

The decision compares continued Cubic-Bézier-only differential work, direct
addition of a second concrete curve family, and a minimal static parametric
curve semantic seam. It selects the seam first.

PR #98 integrated the decision as
`12ecbf584751dadb0dd142c485b1cd4f220736d8`.

Validation:

- final PR FAST `35711481469`: PASS;
- final PR INTEGRATION `35711481473`: PASS;
- post-merge FAST `35711563476`: PASS;
- post-merge INTEGRATION `35711563585`: PASS.

After this separate closure checkpoint is integrated and post-merge validation
passes, the sole first implementation work unit is **Bounded Parametric Curve
Contract and Cubic Bézier Conformance**. That work unit may add only a finite
closed parameter-domain vocabulary, static 2D/3D C++23 curve concepts,
reversal-parameter semantics, and unchanged Cubic-Bézier conformance.

Line/segment, circle/conic arc, arbitrary-degree/rational Bézier, B-spline,
NURBS, composite/trimmed curves and all surface representations remain
separate later work units.

The bounded parametric contract implementation was integrated by PR #101 as
`0674cd8531c3033a30282ba6bf95078b54d8c331`. Final PR FAST
`35713249842` and INTEGRATION `35713249775` passed with 18/18 tests in
each required cell; post-merge FAST `35713409188` and INTEGRATION
`35713409166` also passed.

Closure PR #102 merged as
`63479c8a7414a61be6f8ac1629e934f506d4f3de`. Closure post-merge FAST
`35713788156` and INTEGRATION `35713788216` passed.

The active first-concrete-family decision is
`docs/decisions/CURVE_BOUNDED_LINE_SEGMENT_DECISION.md`.

It compares line/segment, circle/conic arc, rational/arbitrary-degree Bézier,
B-spline and NURBS and selects **Bounded Directed Line Segment Representation
in 2D and 3D**.

PR #103 integrated the decision as
`2b42c78a2dbf0ede225144339dbf100900bef672`. Final PR FAST
`35719338493`, PR INTEGRATION `35719338492`, post-merge FAST
`35719435059`, and post-merge INTEGRATION `35719434961` all passed.

Closure PR #104 merged as
`326ffdf724912e8841a74c3c0b69756ca23e14c2`. Closure post-merge FAST
`35719744251` and INTEGRATION `35719744291` passed.

The bounded line-segment implementation was integrated by PR #105 as
`87ced22d033e5478c134aa66c2eef4b6017a4596`.

Validation history is retained:

- initial candidate FAST `35720284585` and INTEGRATION `35720284182`
  failed mechanically in the focused test because `Vector2`/`Vector3`
  aliases were missing;
- corrected candidate FAST `35720421004` and INTEGRATION `35720420984`
  passed 19/19 in every required cell;
- final documentation-synchronized PR-head FAST `35721616589` and
  INTEGRATION `35721616596` passed 19/19;
- post-merge FAST `35721779942` and INTEGRATION `35721779739` passed
  19/19 in every required cell.

Production now contains `LineSegment2` and `LineSegment3`. The common
parametric contract was not changed. The family work unit is integrated but
not scientifically qualified as a broadened representation stage.

Implementation closure PR #106 merged as
`b435ddbbf93f94741014b26d081dbf5bdbb7c9e6`. Closure post-merge FAST
`35722144861` and INTEGRATION `35722144806` passed.

The active second-family decision is
`docs/decisions/CURVE_RATIONAL_QUADRATIC_BEZIER_DECISION.md`.

It compares dedicated analytic conics, rational/arbitrary-degree Bézier,
B-spline, NURBS and composition/trimming and selects **Positive-Weight Rational
Quadratic Bézier Representation in 2D and 3D** as the next candidate.

PR #107 integrated that decision as
`4ae5a81cec0c3f6512f81a47b7a4d1a6f97fd6ad`. Decision-head FAST
`35722805362`, INTEGRATION `35722805446`, post-merge FAST
`35722894744` and post-merge INTEGRATION `35722894725` all passed.

Closure PR #108 merged as
`30f32997dec0aa7937c9730eb5ce24e2f80bb964`. Closure post-merge FAST
`35723209087` and INTEGRATION `35723209143` passed.

The rational-quadratic implementation was integrated by PR #109 as
`6600875dfbb33d1a37603e32bcf452625373c462`.

Validation:

- candidate FAST `35726985299`: PASS, 20/20;
- candidate INTEGRATION `35726985356`: PASS, 20/20 in GCC and Clang;
- final PR-head FAST `35727147220`: PASS;
- final PR-head INTEGRATION `35727147303`: PASS;
- post-merge FAST `35727296946`: PASS;
- post-merge INTEGRATION `35727296931`: PASS.

Production now contains `RationalQuadraticBezier2/3`. The common
bounded-parametric concept remains unchanged. This focused integration does not
widen the original CGR qualification claim.

Implementation closure PR #110 merged as
`93b082ce660fd8d2c012b96ef7319b240de6d9d2`. Closure PR FAST
`35727517602`, PR INTEGRATION `35727517607`, post-merge FAST
`35727653016`, and post-merge INTEGRATION `35727652961` all passed.

The rational-quadratic work unit is therefore closed. No later family is
authorized by that closure.

Terminal reconciliation PR #111 merged as
`7579254ebd0d6843fdc3761376132a2b7d9fa43c`. Its post-merge FAST
`35728104539` and INTEGRATION `35728104607` passed.

The active third breadth decision is
`docs/decisions/CURVE_TRIMMED_PARAMETRIC_SUBCURVE_DECISION.md`.

It compares analytic conics, arbitrary-degree Bézier, B-spline, NURBS,
heterogeneous composition and trimming. It selects **Oriented Trimmed
Parametric Subcurve Semantics in 2D/3D** before another mathematical family.

The decision does not authorize polycurve/type-erasure storage, periodic
trimming or any new concrete curve family.

Decision closure PR #113 merged as
`74cafc0f7e64abe159303fe7116dcbaac4d8fad7`; closure post-merge FAST
`35729923695` and INTEGRATION `35729923468` passed.

Trim implementation PR #114 merged as
`133a98ea056b12d86049e36abc0370208776106b`; candidate/final/post-merge
FAST and INTEGRATION all passed with 21/21 ordinary tests.

Trim implementation closure PR #115 merged as
`e29a08b07c175a91410123f99867eef4190b983b`; closure post-merge FAST
`35731529518` and INTEGRATION `35731529546` passed.

Production therefore includes oriented static trimming semantics over admitted
bounded curve bases, without heterogeneous runtime composition.

The bounded first B-spline decision is
`docs/decisions/CURVE_TWO_SPAN_CUBIC_BSPLINE_DECISION.md`.

It selects a fixed non-rational, non-periodic, clamped cubic B-spline with
exactly two spans and one simple interior knot, isolating knot/local-support
semantics before general B-spline/NURBS breadth.

Repository-specific sequencing matters: the qualified Cartesian-frame claim
does not include arbitrary-angle rotations, so a general analytic 3D circle
would require a separate arbitrary supporting-plane/orientation decision.
Rational control geometry supplies a smaller 2D/3D conic-capable step without
silently weakening that prerequisite.

### Boundary Curve Discretization — Physical and Parameterization-Invariant Trace

Status: `NOT STARTED`

Goal: generate a canonical shared physical trace satisfying declared geometric and metric error criteria.

Investigation problems will cover physical approximation error, adaptive sampling/integration, metric-length control, gradation, shared trace identity, orientation, and parameterization invariance.

Mandatory stage regression: rerun line/arc/Bezier/adversarial parameterization cases, shared-orientation cases, and every prerequisite regression; regenerate trace and error figures before qualification.

### Surface Representation — Continuous Patch Geometry

Status: `BICUBIC NURBS DOUBLE-KNOT C1 DECISION ACTIVE / NOT QUALIFIED`

Goal: certify continuous patch/surface evaluation before differential geometry
or meshing.

The surface-entry decision must not assume that Bézier/Coons alone constitutes
the final admissible geometry envelope. It must explicitly define production
coverage and qualification boundaries for:

- tensor-product polynomial Bézier patches;
- Coons/transfinite patches where used by the AP Mesh construction;
- rational Bézier patches;
- B-spline and NURBS surfaces;
- analytic surfaces required by the admissible model class and independent
  validation set, including at least plane, cylinder, cone, sphere and torus
  when those entities are admitted;
- ruled, extrusion and revolution surfaces when required by the chosen input
  class;
- trimmed-surface semantics and the separation between continuous supporting
  surface geometry, trimming curves and explicit topology identity;
- deterministic parameter-domain/orientation semantics and boundary
  consistency.

An entry decision may justify deferring a family that is outside the doctoral
admissible input class, but no family is implicitly covered. In particular,
the future plane/cylinder/sphere/paraboloid/saddle differential-geometry
regression must state whether each fixture is a production representation or
an independent analytic oracle.

Mandatory stage regression: rerun analytic surface fixtures, boundary
consistency cases, parameterization cases, every admitted concrete surface
family, and every prerequisite regression; regenerate surface/boundary figures
before qualification.

### Surface Differential Geometry — Metric, Normals, and Curvatures

Status: `NOT STARTED`

Goal: independently verify first/second fundamental forms, normals, principal curvatures, Gaussian/mean curvature, regularity, and conditioning.

Mandatory stage regression: rerun plane/cylinder/sphere/paraboloid/saddle and near-degenerate admissibility fixtures, together with every prerequisite regression; regenerate field visualizations before qualification.

### Physical Sizing Field — Error-Driven Isotropic Baseline

Status: `NOT STARTED`

Goal: derive and verify scalar physical sizing from a declared approximation-error objective rather than an unqualified curvature heuristic.

Mandatory stage regression: rerun analytic error-vs-size fixtures, scale tests, localized-curvature cases, parameterization cases, and every prerequisite regression; regenerate sizing/error fields before qualification.

### Shared Boundary Certification — Patch Compatibility

Status: `NOT STARTED`

Goal: certify topology, orientation, physical realization, trace identity, global mesh identity, and boundary-side quality across every shared edge/fan.

Mandatory stage regression: rerun manifold, reversed, periodic, crease/smooth, and non-manifold fan fixtures plus every prerequisite regression; regenerate boundary compatibility figures before qualification.

### Patch Interior Meshing — Constrained Triangular Baseline

Status: `NOT STARTED`

Goal: generate a robust deterministic triangular interior that exactly preserves certified boundaries and satisfies declared physical sizing and quality criteria.

Mandatory stage regression: rerun analytic patches and all declared mesh-quality/adversarial fixtures plus every prerequisite regression; regenerate mesh, quality, and error figures before qualification.

### Mesh Optimization and Projection — Preserve Certified Invariants

Status: `NOT STARTED`

Goal: improve mesh quality without breaking geometry, topology, boundary, sizing, or deterministic invariants.

Mandatory stage regression: compare pre/post optimization certificates on all qualified fixtures and rerun every prerequisite regression before qualification.

### Adaptive Meshing Loop — Error, Decision, Refinement, Acceptance

Status: `NOT STARTED`

Goal: connect certified geometry, sizing, meshing, error estimation, and refinement into a monotone, deterministic, diagnosable adaptive process.

Mandatory stage regression: rerun convergence, non-convergence, localized-error, transition, and stop-policy fixtures plus every prerequisite regression; regenerate convergence histories before qualification.

### Global Certification — Admissible Input to Certified Mesh

Status: `NOT STARTED`

Goal: define the end-to-end contract: for every input in the declared admissible class, return either a mesh satisfying the certificate or an explicit classified failure.

Mandatory stage regression: execute the full certified fixture hierarchy, selected literature/benchmark models, deterministic repeats, scale/reparameterization variants, and all prior regression gates. This is the doctoral triangular-baseline release gate.

### Quad-Dominant Extension — Compatibility-Preserving Quadrilateral Research

Status: `NOT STARTED`

Goal: begin only after the triangular certified baseline is released. Detailed decomposition will be created from literature and experimental evidence at that time.

The quad-dominant extension remains serial and deterministic while its
scientific semantics, compatibility constraints, quality criteria, and
regression evidence are established. Parallel execution is intentionally
deferred until this extension is qualified.

### Parallel Equivalence — Optimize Only After Serial and Quad Certification

Status: `NOT STARTED`

Goal: introduce parallel execution only after the certified serial triangular
baseline and the serial Quad-Dominant Extension are scientifically closed,
without changing any qualified scientific semantics.

Mandatory stage regression: serial/parallel certificate equivalence,
repeated-run determinism, race/sanitizer checks, and full prerequisite
regression across every qualified serial capability, including the
quad-dominant extension.

### Tensor/Anisotropic Extension — Compatibility-Aware Metric Meshing

Status: `NOT STARTED`

Goal: begin only after the required isotropic and compatibility foundations are certified. Detailed decomposition will be created from literature and experimental evidence at that time.

## 7. Evidence and documentation structure

The greenfield repository should converge to this structure:

```text
docs/
  APMESH_CORE_ROADMAP.md
  contracts/
  decisions/
  research/
    REFERENCE_REGISTER.md
  stages/
experiments/
  manifests/
  expected/
  scripts/
results/                 # generated / normally ignored
  <stage>/<experiment>/
    certificate.json
    metrics.csv
    figures/
    report.md
```

Each qualified stage must have a human-readable decision document recording:

- question;
- scope and assumptions;
- literature basis;
- implementation revision;
- fixtures;
- expected and measured quantities;
- figures;
- regression result;
- retained limitations;
- decision and next admissible action.

## 8. Current action

Current scientific work focus:

**Surface Representation — Bounded Cubic Bézier Surface of Revolution —
IMPLEMENTATION ACTIVE / NOT QUALIFIED**

Decision authority:
`docs/decisions/SURFACE_CUBIC_BEZIER_REVOLUTION_DECISION.md`.

Closed decision checkpoint:

- decision PR #179 merge:
  `112f3b7ae3c439d729380fec07d065997bf11e56`;
- decision post-merge FAST `35910017412`: PASS;
- decision post-merge INTEGRATION `35910017524`: PASS;
- decision closure PR #180 merge:
  `715ad5dc0ef068bec5f68b260df0dd3abd0fcf52`;
- closure post-merge FAST `35910422052`: PASS;
- closure post-merge INTEGRATION `35910422072`: PASS.

Active implementation branch:
`surface/cubic-bezier-revolution`.

Authorized scope:

- one `CubicBezier3` generatrix;
- one `AxisPlacement3`;
- signed finite `0 < abs(sweep) < 2*pi`;
- normalized `[0,1]^2` domain;
- Rodrigues-style arbitrary-axis rotation;
- analytic Su/Sv/Suu/Suv/Svv;
- U/V reversal;
- cylinder and annular-sector references;
- expected inventory: 34 ordinary tests.

Candidate implementation is now mapped to the authorized new header/source,
one focused test and CMake registration, with no common contract changes.
FAST/INTEGRATION validation is pending.

Complete periodic revolution, analytic elementary surfaces, general trimming,
broader Coons/transfinite boundaries and remaining NURBS breadth remain later
decisions.

Surface Differential Geometry and Boundary Curve Discretization remain blocked.

The long-term ordering remains:

**Global Certification → Quad-Dominant Extension → Parallel Equivalence →
Tensor/Anisotropic Extension.**

Parallel execution must not precede serial Quad-Dominant qualification.
