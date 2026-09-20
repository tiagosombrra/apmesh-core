# AP Mesh Core — Scientific Implementation Roadmap

Status: ACTIVE / AUTHORITATIVE
Last updated: 2026-09-20
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

Status: `IN INVESTIGATION / PRODUCTION IMPLEMENTATION COMPLETE /
FOCUSED CONTRACTS PASS / CUMULATIVE REGRESSION EXECUTED ONCE /
TERMINAL AUDIT COMPLETE / TMR0-TMR5 PASS / TMR6-TMR7 BLOCKED /
FORMAL ATTEMPT CONSUMED / STAGE UNQUALIFIED`

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

Status: `NOT STARTED`

Goal: certify continuous curve representation independent of meshing.

#### Cubic Bezier Evaluation

- Implement cubic Bezier evaluation from the mathematical definition.
- Verify endpoint, affine-invariance, reversal, and analytic fixture properties.
- Compare against independent high-precision/reference evaluation.

#### Curve Derivatives and Regularity

- Implement first and second derivatives.
- Define and detect regularity/zero-speed conditions.
- Verify line, near-line, inflection, localized-curvature, and degenerate cases.

#### Arc Length and Parameter Mapping

- Select an error-controlled integration strategy after literature review.
- Return value plus convergence/error diagnostics rather than a naked scalar.
- Verify against line and analytic arc references and under reparameterization stress.

#### Continuous Curve Geometry Regression

- Rerun all curve analytic/reference fixtures.
- Rerun all prerequisite regressions.
- Regenerate curve, derivative, speed, and arc-length error figures.
- Verify reversal and admitted reparameterization invariants.

Stage exit gate: continuous curve geometry is qualified before any adaptive sampling is introduced, and Continuous Curve Geometry Regression passes.

### Curve Differential Geometry — Curvature, Regularity, and Features

Status: `NOT STARTED`

Goal: certify intrinsic curve differential quantities used by boundary discretization.

Investigation problems will cover curvature definition/evaluation, regularity, feature classification, and scale robustness. Detailed executable work units will be defined only after Continuous Curve Geometry is qualified.

Mandatory stage regression: rerun all curve-differential fixtures plus every prerequisite regression and regenerate curvature/reference figures before qualification.

### Boundary Curve Discretization — Physical and Parameterization-Invariant Trace

Status: `NOT STARTED`

Goal: generate a canonical shared physical trace satisfying declared geometric and metric error criteria.

Investigation problems will cover physical approximation error, adaptive sampling/integration, metric-length control, gradation, shared trace identity, orientation, and parameterization invariance.

Mandatory stage regression: rerun line/arc/Bezier/adversarial parameterization cases, shared-orientation cases, and every prerequisite regression; regenerate trace and error figures before qualification.

### Surface Representation — Continuous Patch Geometry

Status: `NOT STARTED`

Goal: certify continuous patch/surface evaluation before differential geometry or meshing.

Investigation problems will cover Bezier/Coons representation, boundary consistency, derivatives, mapping, admissibility, and parameterization behavior.

Mandatory stage regression: rerun analytic surface fixtures, boundary consistency cases, parameterization cases, and every prerequisite regression; regenerate surface/boundary figures before qualification.

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

Cartesian Similarity Frames was squash-merged into `main` at
`12afa394af24c8f0627b12e3697010735826a9a1`. Its replacement qualification on
clean published candidate `8e6b587` passed CF0–CF7 in the declared WSL GCC 13
and Clang 18 libc++ envelope. The audited package comprises four build cells,
three repetitions per cell, 1,696 exact cases per certificate, exact
cross-cell equivalence, prerequisite preservation, and detached-verified
retention. The historical `e69e804` schema-defect package is retained as
negative tooling evidence and does not reduce this bounded qualification.

Foundation closed with FND0–FND7 `PASS` on clean published candidate
`b333755442b934c490abaecda886dd2a40e981ca`. The report-only qualification
recorded the required revision-bound build, CTest, dependency, comparison, and
retention evidence; the separate audit made the scientific decision. Historical
REC evidence at `85d215a` remained a comparison baseline and did not substitute
for current-candidate FND2, FND4, or FND6 evidence.

Current stage result:

**Geometry Primitives: QUALIFIED / GPR0—GPR7 PASS**

NQ-R1 on candidate `74fede5` remains retained as negative evidence: it lacked
explicit `min()` and `lowest()` classification evidence. NQ-R2 supplied only
that evidence without changing `numeric.cpp`, then passed the audited clean
four-cell regression on candidate `236d290`. N0–N7 are qualified. Foundation
is QUALIFIED at 100% within the declared WSL envelope. The Reproducible
Experiment Contract and its first two-replay/four-cell E0–E7 regression are
qualified on candidate `85d215a`; that historical evidence remains the accepted
comparison baseline. Foundation End-to-End passed its post-execution FND0–FND7
audit on `b333755`, with retained scope limitations. Point and Vector Semantics
are QUALIFIED on `ededf65` only within the declared WSL Ubuntu 24.04 GCC 13 /
Clang 18 libc++ envelope. That Point/Vector qualification alone authorized no
matrix, transform, predicate, topology, curve, surface, or meshing algorithm.
PR #3 is
integrated into `main` at `1ff6568`; its tree matches reviewed source head
`b170673`. The bounded Minimal Small Linear Algebra Contract was accepted after
Amendment 1 resolved its dependency, error, transpose, and scale ambiguities.
`Mat2`/`Mat3` are QUALIFIED on `3804e90` in the declared WSL GCC/Clang
Debug/Release envelope: LA0-LA7 passed following the canonical signed-zero
evidence correction, without production behavior change. The first `6fa00bd`
attempt remains immutable BLOCKED evidence. Cartesian Similarity Frames is QUALIFIED on `8e6b587` inside
the declared WSL GCC/Clang envelope after CF0–CF7 passed. Its historical
`e69e804` execution remains immutable blocked tooling evidence; it was not
used as a gate result. The single cumulative Geometry Primitives regression is
pre-registered in
`docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md`; its
report-only workflow and revision-bound execution runner passed focused
GCC/Clang development contracts, including retained synthetic success and
partial failure. Its first external manifest is immutable
`BLOCKED_BY_STALE_LIMITATION_CONTRACT` evidence and was never executed. Its
second external manifest is immutable `BLOCKED_BY_CTEST_DISCOVERY_PARSER_DEFECT`
evidence: build and CTest discovery passed, but no semantic CTest or certificate
was reached. Its third external manifest is immutable
`BLOCKED_BY_EVIDENCE_OUTPUT_DIRECTORY_DEFECT` evidence: GCC Debug configure,
build, discovery, and semantic CTest passed, but the certificate producer could
not open an output below the uncreated `certificates/` directory. Its fourth
external manifest, `f36a8fa1801eca6f992d7cb46f11b34c9d088abec8df769568cf3bb7f41dff5b`,
is immutable `BLOCKED_BY_SEMANTIC_CTEST_REGEX_DEFECT` evidence: the four CTest
commands used an unsupported non-capturing regex group, selected no tests, and
still returned zero. Its certificates remain diagnostic evidence only.

The fifth external manifest,
`a39069569625bbc16637d2078bddb2e91c654840782ad0c4026fc9b75ef42718`,
bound candidate `2f22ffd`, executed once and was independently audited
`PASS` for GPR0—GPR7: the sealed six-test allowlist passed in all four
cells, twelve certificates had equal semantic projections, twenty negative
checks and the dependency inventories passed, and detached retention was
verified. Geometry Primitives is QUALIFIED only in the declared WSL Ubuntu
24.04 GCC 13 / Clang 18 libc++ envelope.

Current active stage:

**Topological Model - IN INVESTIGATION / EDGE KERNEL IMPLEMENTED /
FACE-BOUNDARY IMPLEMENTED / FOCUSED CONTRACT PASS /
EDGE-USE INCIDENCE IMPLEMENTED / FOCUSED CONTRACT PASS /
STRUCTURAL INCIDENCE IMPLEMENTED / FOCUSED CONTRACT PASS /
IMMUTABLE SNAPSHOT IMPLEMENTED / FOCUSED CONTRACT PASS /
CUMULATIVE REGRESSION PRE-REGISTERED / REPORT-ONLY TOOLING IMPLEMENTED /
FOCUSED TOOLING CONTRACTS PASS / NOT PREPARED / NOT EXECUTED /
STAGE UNQUALIFIED**

The Identity and Oriented Edge Incidence Kernel defined in
`docs/decisions/TOPOLOGICAL_MODEL_ENTRY_DECISION.md` is implemented and its
focused GCC/Clang Debug contracts pass. The bounded Face Identity and Ordered
Boundary Cycles implementation is also complete and its focused GCC/Clang
Debug contract passes. Neither result authorizes `PatchId`, curves, surfaces,
outer/inner loop classification, manifold or non-manifold classification,
canonical topology serialization, qualification infrastructure, or meshing.
The third bounded work unit implements deterministic immutable edge-use
incidence enumeration. Its focused GCC 13 Debug and Clang 18/libc++ Debug
evidence passes; it does not qualify the stage or authorize boundary/manifold
interpretation, geometry, serialization, or a formal campaign.
Deterministic edge-incidence structural classification and immutable canonical
snapshot emission are implemented, with focused GCC/Clang Debug contracts
passing. Neither converts structural signatures into adjacency, pairing,
boundary, manifold, shell, or geometric claims. The cumulative TMR0–TMR7
protocol is pre-registered. Its smallest reusable report-only tooling layer is
implemented and passed focused GCC 13 Debug plus Clang 18/libc++ Debug
contracts in GitHub Actions run `35514834796`. The admitted GitHub-hosted
Ubuntu 24.04 cloud envelope is now explicitly bound for future formal TMR use by
`docs/decisions/TOPOLOGICAL_MODEL_CLOUD_QUALIFICATION_ENVIRONMENT_SUPPLEMENT.md`;
the scientific campaign remains unprepared and unexecuted.

The repository/public-cloud transition audit passed on 2026-09-20. FAST and the
reusable four-cell Major Semantic Regression passed on GitHub-hosted Ubuntu
24.04 without production C++ change; the audit record is
`docs/audits/2026-09-20-public-cloud-baseline.md`. Cloud INTEGRATION is now
accepted at 100%: GCC 13 Debug and Clang 18/libc++ Debug each passed the exact
seven-test semantic inventory, both checks are required by
`main-protection`, and the closure audit is
`docs/audits/2026-09-20-cloud-integration-closure.md`. The Major Semantic
Regression is manual and reserved for explicit major phase boundaries; final
functional-candidate run `35512093405` passed GCC/Clang Debug/Release after
the checkout runtime deprecation was removed by pinning the official
`actions/checkout` v7.0.1 commit.

The cloud QUALIFICATION-environment transition is accepted at 100% through
`docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md` and
`docs/audits/2026-09-20-cloud-qualification-environment-admission.md`.
CQE0–CQE7 passed in run `35513051098`; the final PR #13 candidate
`6a934de6e8f6fae35e6c38ec45b9b1f23b170acb` was revalidated by Qualification
Environment run `35513250315` and phase-boundary Major Semantic Regression run
`35513567930`, both PASS. PR #13 was squash-merged as
`0a7095d431e4bea3c9c73e75d22df2e713c7a8ab`; the reviewed candidate tree and
merged tree are identical
(`7144943abc7ffd861b92587217a112c0edf6f9b4`). Post-merge FAST run
`35513658207` and INTEGRATION run `35513658197` passed. The first admission
run `35512991310` remains retained as the single mechanical
`BLOCKED_BY_CMAKE_CACHE_TYPE_ASSERTION` attempt. The admitted cloud envelope
is explicitly distinct from the historical WSL qualification envelope and
fails closed on runner-image or package drift.

The report-only TMR0–TMR7 workflow and focused tooling contracts are implemented
without production C++ changes. The accepted cloud-environment supplement binds
the exact admitted GitHub-hosted Ubuntu 24.04 envelope, and the existing TMR
preparation/runner path fails closed on that identity. Focused GCC 13 Debug and
Clang 18/libc++ Debug validation passed in run `35516246789`; run
`35516204233` is retained as a mechanical protocol-guard failure before any
environment evaluation. The formal launch plan seals the admitted `/usr/bin`
CMake/CTest/Ninja and compiler paths, with focused GCC/Clang Debug PASS in run
`35516578411`.

A manual preparation-only GitHub Actions workflow is implemented. It is
restricted to explicit `workflow_dispatch` on canonical `main`, invokes only
`prepare` plus `validate-prepared`, writes to a new runner-temporary output
root, retains the sealed package with a pinned artifact action, rejects any
execution/terminal evidence, and contains no `execute` path. Runs
`35516864464` and `35516972035` exposed consecutive mechanical quoting
defects in the surrounding tooling workflow. The tooling stop was honored; run
`35517077819` then passed the complete focused/static contracts. PR #20
integrated the workflow as
`d9297ffad4f503b4ea11b056885749fff5872201`; post-merge FAST
`35519501704` and INTEGRATION `35519501663` passed.

Formal preparation run `35524700979` was dispatched once on canonical
`main` and produced retained artifact `10609500629` for candidate
`e5eda2663d6ff4b93ce1205660ff04d432acb9c0`. The package has GitHub/archive
SHA-256 `2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72`.
Independent audit of the package seal, lifecycle, candidate, cloud identity,
four-cell/two-repetition plan, 1534-file source inventory, 123 planned retained
artifacts, and all twelve critical input hashes passed. The package remains
unconsumed with `execution_requested=false`; TMR0-TMR7 remain
`NOT_EXECUTED`.

PR #23 integrated the preparation audit as
`b3d8130cdf75230ef7b71693d2325e5473091857`; post-merge FAST
`35525181361` and INTEGRATION `35525181462` passed.

A manual execution-only workflow is now implemented for the exact audited
artifact/candidate. It restores artifact ID `10609500629` directly to the
sealed output root, revalidates the full PREPARED binding, creates an immutable
manifest-hash execution-claim tag only after preflight, exposes exactly one
`execute` invocation, prunes reproducible build trees, verifies retention,
and uploads terminal evidence. A pre-existing claim blocks execution; once a
claim is created, failure consumes the attempt and there is no retry path.
Focused/static run `35525736120` passed in GCC 13 Debug and Clang 18/libc++
Debug. No claim or formal execution occurred.

PR #25 integrated the one-shot execution wrapper as
`d7019fbff97989a79fd27fcb1915073881a53564`; post-merge FAST
`35525932108` and INTEGRATION `35525932111` passed. No claim tag or formal
execution exists.

The execution authorization path is repository-resident
authorization-as-code. The exact manifest-bound `EXECUTE_ONCE` record is
introduced only by a separate pull request. Its merge to protected `main`
triggers a controller that validates a newly added immutable record, rejects an
existing execution claim, and calls the reusable one-shot executor. The executor
independently revalidates the authorization commit before consuming the exact
audited PREPARED package. Focused/static tooling runs `35527446051` and
`35527563934` passed in both GCC 13 Debug and Clang 18/libc++ Debug.

PR #27 integrated authorization-as-code as
`7bf2d409556c8318db72b86ef0d85253aa0583ec`. Required PR FAST
`35527616244` and INTEGRATION `35527616258` passed; post-merge FAST
`35527668634` and INTEGRATION `35527668624` passed. No
`EXECUTE_ONCE` record, execution claim, formal execution, or terminal package
exists.

The next bounded scientific action is one separate exact authorization-record
PR. Merging that record becomes the formal execution authorization event; the
resulting terminal package must then be independently audited before any
TMR0-TMR7 decision.
