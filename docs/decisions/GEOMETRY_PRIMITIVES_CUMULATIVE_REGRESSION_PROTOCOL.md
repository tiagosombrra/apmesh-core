# Geometry Primitives - Cumulative Regression Protocol

Status: PRE-REGISTERED / SECOND MANIFEST BLOCKED / GATES NOT EXECUTED
Date: 2026-09-18
Stage: Geometry Primitives - Exact Semantics Before Curves
Post-merge baseline: `ca20ad64cd0dd14b869225dae73401ad2b93464c`

## 1. Question and boundary

Can the qualified Point/Vector, Minimal Small Linear Algebra, and Cartesian
Similarity Frames capabilities operate together on one clean revision while
preserving the qualified Foundation contracts and all declared semantic
boundaries?

This is the single cumulative qualification gate for the Geometry Primitives
stage. It does not authorize another primitive, a production-code change, or a
separate formal campaign for each component.

## 2. Authority and fixed prerequisites

The protocol is bounded by the following accepted authorities and evidence:

- Foundation `QUALIFIED`, including FND0-FND7 on candidate `b333755`;
- Point and Vector Semantics PV0-PV7 `PASS` on candidate `ededf65`;
- Minimal Small Linear Algebra LA0-LA7 `PASS` on candidate `3804e90`, with the
  LA-only integration regression `PASS` on `95258e9`;
- Cartesian Similarity Frames CF0-CF7 `PASS` on candidate `8e6b587`; and
- the reviewed post-merge `main` baseline `ca20ad64`, whose tree contains all
  three qualified Geometry capabilities.

Historical blocked attempts remain negative evidence. They cannot be
reclassified as passing inputs and are not rerun by this protocol.

## 3. Fixed claim

Within WSL Ubuntu 24.04 using GCC 13 with libstdc++ and Clang 18 with libc++,
in Debug and Release, the integrated Geometry Primitives candidate may claim
only that:

1. `Point2`/`Point3`, `Vector2`/`Vector3`, `Mat2`/`Mat3`, and
   `CartesianFrame2`/`CartesianFrame3` preserve their qualified dimensional and
   semantic separation;
2. admitted matrix, vector, point, and frame compositions agree with
   independently stated component equations for the preregistered cases;
3. finite valid inputs either produce a finite result satisfying the declared
   postcondition or an explicit qualified error;
4. exact power-of-two scale relations and the canonical numeric treatment of
   signed zero are preserved where the operation is declared representable;
5. repeated executions and the four compiler/build cells have equivalent
   scientific claim fields; and
6. the current candidate preserves the qualified Foundation contracts and has
   no topology, predicate, curve, surface, mesh, I/O, logging, or parallel
   dependency in the production primitives.

This is empirical qualification over an enumerated finite envelope. It is not
an unconditional proof for every finite floating-point input.

## 4. Explicit non-claims

The regression does not qualify or authorize:

- geometric coincidence, orientation, incircle, intersection, or any robust
  topological predicate;
- vertex/edge identity, incidence, adjacency, merging, or topology;
- dynamic matrices, inverses, solves, decompositions, or eigensystems;
- arbitrary-angle rotations, general affine transforms, or approximate frame
  validation;
- segments, curves, surfaces, patches, CAD, discretization, or meshing;
- native Windows, OpenMP, MPI, SIMD, GPU, performance, or portability outside
  the declared compiler envelope; or
- comparison with the legacy AP Mesh implementation as an acceptance oracle.

## 5. Candidate and immutable preparation

One candidate must be a clean, committed, published descendant of
`ca20ad64`. Preparation must bind by SHA-256 at least:

- the full candidate commit and tracked-source inventory;
- this protocol and every authority named in Section 2;
- the fixed execution matrix and complete stable case list;
- the exact semantic CTest allowlist discovered from the candidate;
- the stage exporter, independent oracle, collector, comparer, and runner;
- invoked and resolved compiler/tool identities;
- planned command, certificate, comparison, failure, dependency, and retention
  inventories; and
- a new external output root that does not exist before preparation.

The immutable manifest is `PREPARED` only while
`execution_requested=false`. Execution must consume that exact manifest once.
Preparation and execution do not decide the scientific result.

No production C++ change after `ca20ad64` is admitted into the candidate unless
a separate bounded defect decision authorizes it before a new manifest is
created.

## 6. Reuse and execution design

The work unit must reuse `tools/experiment_runtime.py` and the established
manifest, command-record, failure, inventory, and retention conventions. It
must add at most one stage-level exporter, runner, collector/comparer, profile,
and focused tooling contract when an existing entry point cannot express the
cumulative evidence.

PV0-PV7, LA0-LA7, and CF0-CF7 are not launched as three historical campaigns.
Their accepted records are authority inputs; their current-candidate semantic
contracts are exercised through one sealed stage allowlist.

The exact matrix is:

| Cell | Compiler | Standard library | Build type |
| --- | --- | --- | --- |
| `gcc-debug` | GCC 13 C++ driver | libstdc++ | Debug |
| `gcc-release` | GCC 13 C++ driver | libstdc++ | Release |
| `clang-debug` | Clang 18 C++ driver | libc++ | Debug |
| `clang-release` | Clang 18 C++ driver | libc++ | Release |

Each cell produces three independent stage certificates. The runner is
fail-fast for command failure but must retain a structured terminal package for
both `PASS` and `BLOCKED` outcomes.

## 7. Fixed semantic allowlist

Every cell must build and execute exactly these current-candidate production
contracts:

- `apmesh_core.bootstrap_smoke`;
- `apmesh_core.numeric_contract`;
- `apmesh_core.geometry_primitives`;
- `apmesh_core.minimal_small_linear_algebra`;
- `apmesh_core.math_header_isolation`; and
- `apmesh_core.cartesian_frames`.

The preparation tooling must prove that these names exist exactly once. Tooling
self-tests, historical runners, and component retention tests are development
preflight, not scientific repetitions inside the four-cell matrix. The
stage-level runner must nevertheless exercise and record the qualified
reproducibility lifecycle directly: immutable preparation, exact command
records, repeated certificates, terminal failure retention, canonical
retention, and detached verification.

## 8. Preregistered integrated cases

The stable case list must cover both dimensions wherever the public API admits
them and must include:

- identity matrices and identity frames;
- nonzero finite origins, signed-permutation bases, and positive power-of-two
  scales;
- `F.point_to_world(p + v) - F.point_to_world(p) = F.vector_to_world(v)` for
  exactly representable cases;
- `F.point_to_local(F.point_to_world(p)) = p` and the corresponding vector
  round trip for preregistered representable cases;
- `apply(A, apply(B, v)) = apply(compose(A, B), v)` where all intermediates are
  finite and exactly representable;
- `transpose(compose(A, B)) = compose(transpose(B), transpose(A))`;
- identity-frame mappings agreeing with direct Point/Vector values;
- 2D/3D API separation and rejection of invalid cross-dimension expressions;
- canonical signed-zero comparison with raw sign retained as diagnostic
  provenance;
- safe ordinary, minimum-normal, subnormal-direction, and power-of-two boundary
  fixtures inherited from the qualified component envelopes;
- invalid construction, invalid frame, zero-length normalization, division by
  zero, scale-out-of-range, and deliberate non-finite-result paths; and
- cases demonstrating that determinant, cross product, and exact equality do
  not become predicate, orientation, coincidence, incidence, or topology
  claims.

Expected results must be generated from independent component equations or
closed-form exact fixtures, not copied from production outputs or the legacy
implementation. Rounded cases require an already qualified explicit proximity
policy and recorded residual; no default epsilon is admitted.

## 9. GPR0-GPR7 gate

| Gate | PASS condition |
| --- | --- |
| `GPR0` - Identity and scope | Clean published candidate, immutable manifest, complete authority hashes, exact matrix, and no unapproved production-code change from the post-merge baseline. |
| `GPR1` - Type and dependency boundaries | Dimension, point/vector, math/geometry, and production dependency boundaries pass exactly; no excluded subsystem is reachable from the primitive targets. |
| `GPR2` - Finite-state and error preservation | Every preregistered invalid or non-finite case produces the exact qualified error, with no clamp, retry, fallback, tolerance, or substituted value. |
| `GPR3` - Integrated analytic agreement | All affine, matrix/vector, frame, round-trip, composition, and transpose observations agree with the independent oracle for the declared exact cases. |
| `GPR4` - Scale and signed-zero semantics | Declared power-of-two relations, boundary fixtures, subnormal-direction handling, and canonical signed-zero rules remain consistent across the integrated operations. |
| `GPR5` - Repeat and cross-cell equivalence | Three certificates per cell and all four cells have identical scientific projections; provenance-only fields are explicitly excluded from that projection. |
| `GPR6` - Prerequisite preservation and isolation | The sealed semantic allowlist passes in every cell, Foundation remains consistent, and production primitives remain free of topology and later-stage dependencies. |
| `GPR7` - Evidence integrity and closure | Commands, certificates, comparisons, failures, inventories, authority hashes, limitations, canonical retention, and detached verification are complete and independently recomputable. |

Overall `PASS` requires GPR0-GPR7 to pass together. A gate cannot be inferred
from a historical component package or from another gate.

## 10. Stop and failure policy

- An unexpected production semantic difference makes the run `BLOCKED` and
  opens one separately authorized defect diagnosis. It is not corrected inside
  the campaign.
- A contradiction of Foundation, Point/Vector, Linear Algebra, or Cartesian
  Frames reopens that qualified prerequisite explicitly.
- A mechanical tooling defect receives one focused correction contract without
  changing the claim or acceptance rules. Two consecutive mechanical failures
  stop formal execution; there is no third attempt until the workflow is
  simplified or the defect is resolved outside the campaign.
- Partial execution, missing evidence, hash drift, unsealed input, or retention
  failure is `BLOCKED`, never partial `PASS`.
- Retry, rescue tuning, relaxed acceptance, and hidden fallback are forbidden.

## 11. Required retained outputs

The canonical package must retain only non-rebuildable evidence needed to
recompute the decision:

- prepared and terminal manifests;
- case index and stage certificates;
- exact semantic allowlist and command records;
- per-cell and cross-cell comparisons;
- GPR0-GPR7 JSON and concise Markdown summaries;
- source, input, dependency, and retained-artifact inventories;
- negative and partial-failure evidence;
- explicit limitations; and
- canonical retention manifest plus detached-worktree verification.

Build trees, caches, object files, executables, and other reproducible binaries
are excluded while their hashes, producing commands, and provenance remain in
the inventories. Numerical tables and certificates are required. No spatial
figure is required because the admitted capability has no spatial object or
topological configuration whose meaning is clearer in a figure.

## 12. Decision effect

- `PASS`: Geometry Primitives becomes `QUALIFIED` only in the declared WSL
  envelope. A separate scientific entry decision may then open Topological
  Model; no topology implementation starts automatically.
- `BLOCKED`: Geometry Primitives remains `IN INVESTIGATION`; accepted component
  qualifications remain intact unless the evidence specifically contradicts
  one of them.

At preregistration, this document created no profile, tooling, manifest,
execution, or qualification result. The bounded development package now adds
only `experiments/profiles/geometry_primitives_cumulative.json`, the
experimental cumulative exporter, the report-only
`tools/geometry_primitives_cumulative_evidence.py` validator/collector, and
`tools/run_geometry_primitives_cumulative.py` revision-bound runner. Focused
contracts pass in GCC 13 Debug and Clang 18 libc++ Debug, including immutable
preparation and retained synthetic success and partial failure. The runner consumes one
sealed manifest, records commands, comparisons, dependencies, terminal state,
and retention, but no formal GPR execution or gate result exists.

The first external manifest,
`a890b988559eb041c63104a45e0edbd66d6a9c07fdb912a45896bbe9ded81711`, is
retained as immutable `BLOCKED_BY_STALE_LIMITATION_CONTRACT` evidence. Its
sealed profile said that no prepared manifest existed, contradicting its own
`PREPARED` lifecycle state. It was not executed and does not affect any
component qualification or GPR gate.

The second external manifest,
`ad999a236cf68f537dd5d6bc159885c2e9404a50e824622f867ac24df59b9aa2`, is
retained as immutable `BLOCKED_BY_CTEST_DISCOVERY_PARSER_DEFECT` evidence.
Its configure, build, and CTest discovery commands succeeded; a spacing-sensitive
runner parser then rejected the valid CTest listing before semantic CTests or
certificates ran. It does not affect any component qualification or GPR gate.

## 13. Next bounded action

Publish the bounded CTest-discovery parser correction, then prepare and
independently review one new external `PREPARED` manifest bound to that
published candidate. The four-cell campaign remains unauthorized.
