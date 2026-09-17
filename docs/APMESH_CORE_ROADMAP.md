# AP Mesh Core — Scientific Implementation Roadmap

Status: ACTIVE / AUTHORITATIVE
Last updated: 2026-09-16
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

Status: `IN INVESTIGATION / POINT-VECTOR AND SMALL LINEAR ALGEBRA QUALIFIED / WSL Ubuntu 24.04`

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

- Define transformations required by later curve/surface verification.
- Verify invariance/equivariance properties under translation, rotation, and scale where mathematically appropriate.
- Produce transformation regression certificates.

#### Geometry Primitives Regression

- Rerun all primitive analytic tests across the declared scale envelope.
- Rerun Foundation regression.
- Regenerate numeric error summaries and transformation figures.
- Verify deterministic results and no change in accepted semantics.

Stage exit gate: primitive operations are analytically verified across the declared scale envelope, have no topology semantics, and Geometry Primitives Regression passes.

### Topological Model — Explicit Identity and Incidence

Status: `NOT STARTED`

Goal: represent the patch complex without inferring topology from geometry.

#### Vertex and Edge Identity

- Define strong `VertexId`, `EdgeId`, `CurveId`, `PatchId`, `SurfaceId` types.
- Establish that coincident coordinates do not imply shared identity.
- Add adversarial coincident-but-disconnected tests.

#### Edge Use and Orientation

- Define explicit edge uses/coedges and forward/reverse orientation.
- Verify manifold seams, reversed seams, and non-manifold fans.
- Reject contradictory or incomplete incidence before model construction.

#### Immutable Validated Model

- Implement mutable `ModelBuilder` followed by validation/finalization into immutable `Model`.
- Define complete side-coverage and incidence consistency rules.
- Produce canonical deterministic topology serialization/hash for experiments.

#### Topological Model Regression

- Rerun all topology fixtures including coincident-disconnected, reversed seam, manifold seam, and non-manifold fan.
- Rerun Foundation and Geometry Primitives regressions.
- Regenerate topology/incidence figures and canonical certificates.
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

### Parallel Equivalence — Optimize Only After Serial Certification

Status: `NOT STARTED`

Goal: introduce parallel execution without changing certified scientific semantics.

Mandatory stage regression: serial/parallel certificate equivalence, repeated-run determinism, race/sanitizer checks, and full prerequisite regression.

### Quad-Dominant Extension — Compatibility-Preserving Quadrilateral Research

Status: `NOT STARTED`

Goal: begin only after the triangular certified baseline is released. Detailed decomposition will be created from literature and experimental evidence at that time.

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

Current integration work: the isolated Minimal Small Linear Algebra candidate
`95258e9` has passed the clean four-cell regression and awaits controlled
integration review. No later Geometry work is included.

Foundation closed with FND0–FND7 `PASS` on clean published candidate
`b333755442b934c490abaecda886dd2a40e981ca`. The report-only qualification
recorded the required revision-bound build, CTest, dependency, comparison, and
retention evidence; the separate audit made the scientific decision. Historical
REC evidence at `85d215a` remained a comparison baseline and did not substitute
for current-candidate FND2, FND4, or FND6 evidence.

Current active investigation:

**Geometry Primitives — integrate qualified Minimal Small Linear Algebra**

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
Clang 18 libc++ envelope. Geometry Primitives remains IN INVESTIGATION: no
matrix, transform, predicate, topology, curve, surface, or meshing algorithm
is implemented or authorized by the Point/Vector qualification. PR #3 is
integrated into `main` at `1ff6568`; its tree matches reviewed source head
`b170673`. The bounded Minimal Small Linear Algebra Contract was accepted after
Amendment 1 resolved its dependency, error, transpose, and scale ambiguities.
`Mat2`/`Mat3` are QUALIFIED on `3804e90` in the declared WSL GCC/Clang
Debug/Release envelope: LA0-LA7 passed following the canonical signed-zero
evidence correction, without production behavior change. The first `6fa00bd`
attempt remains immutable BLOCKED evidence. Geometry Primitives remains IN
INVESTIGATION; Transformations and Coordinate Frames require a separate entry
decision and are not started by this closure.
