# Geometry Primitives — Cartesian Frames Qualification Protocol

Status: PRE-REGISTERED / REPORT-ONLY INFRASTRUCTURE IMPLEMENTED / CF0–CF7 NOT EXECUTED
Date: 2026-09-17
Stage: Geometry Primitives — Exact Semantics Before Curves
Work unit: Qualify Cartesian Similarity Frames
Entry authority:
`docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md`

## 1. Question and decision boundary

Can the implemented `CartesianFrame2` and `CartesianFrame3` capability be
qualified, on one clean revision, as a deterministic local/world mapping layer
for the exact signed-permutation and power-of-two-scale domain admitted by the
entry authority?

This protocol qualifies only that bounded capability. It cannot qualify general
transformations, approximate frames, arbitrary-angle rotations, affine or
projective maps, inverses or solves, predicates, topology, curves, surfaces,
meshes, native Windows, parallel execution, or the complete Geometry Primitives
stage. Focused CTest success is implementation evidence, not CF0–CF7 closure.

## 2. Fixed claim

The candidate may claim only that, within the declared WSL Ubuntu 24.04 GCC 13
and Clang 18 libc++ envelope:

1. `CartesianFrame2` and `CartesianFrame3` accept a matching finite origin, an
   exact signed-permutation basis, and a scale exponent only when both `2^k`
   and `2^-k` are finite and nonzero;
2. point and vector APIs remain statically separated by dimension and
   translation is applied only to points;
3. local/world maps agree with independently evaluated component formulas for
   the enumerated representable cases;
4. invalid frames, invalid scales, and non-finite mapped results produce their
   declared explicit `GeometryError` without clamp, retry, or fallback;
5. repeated observations are semantically equivalent within a cell and across
   the four qualified compiler/build cells; and
6. the qualified Foundation, Point/Vector, and Minimal Small Linear Algebra
   prerequisites remain passing on the same candidate.

An exact round trip is claimed only for the preregistered cases whose declared
intermediate and final values are exactly representable. The protocol makes no
claim of bitwise invertibility for every finite `double` input: subtraction of
an origin, addition of a translated coordinate, underflow, overflow, and normal
rounding may lose information outside that bounded set.

## 3. Candidate and preparation binding

Preparation is admissible only from one committed, clean, published candidate
whose upstream resolves to the same full commit. One immutable `PREPARED`
manifest must bind by SHA-256:

- the candidate commit and complete tracked-source inventory;
- this protocol, its entry authority, current roadmap/state, profile, runner,
  collector, comparer, focused contract, public headers, implementation
  sources, CMake input, and prerequisite authorities;
- exact toolchains, presets, commands, working directory, relevant environment
  identity, external output roots, fixtures, repetitions, fields, equivalence
  rules, gates, and retained limitations; and
- planned source, compile-command, runtime-dependency, certificate, comparison,
  failure, and retention inventories.

The manifest records `execution_requested=false`. A separate authorization may
execute that unchanged manifest once. A failed attempt is terminal evidence;
no automatic retry, expectation change, or in-place repair is allowed.

Preparation and execution do not decide qualification. Report-only tooling may
emit only `EVIDENCE_COLLECTED_PENDING_AUDIT`. A separate scientific audit alone
may record `QUALIFIED` or `BLOCKED`.

The fixed profile, independent certificate exporter/oracle, comparer, split
certificate/retention negatives, exact-allowlist planner and executor,
immutable `PREPARED` manifest creator/consumer, terminal partial-failure path,
and detached-worktree retention verifier are implemented. The creator binds a
clean, published candidate, source/input inventories, a sealed plan, the
invoked and resolved toolchain paths with executable SHA-256, and the four-cell
outputs. Retention recomputes the sealed command plan, exact CTest discovery
and execution bindings, negatives, inventories, runtime-dependency observations,
gate summary, and either successful detached verification or a sealed failure
record. Focused evidence
neither creates a manifest nor authorizes or claims a formal CF0–CF7 execution
or scientific closure.

## 4. Fixed execution matrix

Exactly four clean cells are required:

| Cell | Compiler/library | Build |
| --- | --- | --- |
| `gcc-debug` | GCC 13 / libstdc++ | Debug |
| `gcc-release` | GCC 13 / libstdc++ | Release |
| `clang-debug` | Clang 18 / libc++ | Debug |
| `clang-release` | Clang 18 / libc++ | Release |

Each cell builds the admitted library, focused Cartesian Frames contract, and
evidence exporter from the same candidate. Each cell executes three independent
semantic-certificate processes. The formal runner is launched once and fails
fast while retaining partial evidence.

Each cell also runs once an exact, versioned prerequisite allowlist containing
only the qualified Architecture, Numeric, Reproducible Experiment,
Point/Vector, and Minimal Small Linear Algebra preservation contracts. A broad
CTest label or global test count is not an admissible substitute. The profile
must enumerate the expected test names and reject missing or additional tests.

## 5. Fixed analytic and adversarial cases

Before preparation, the evidence profile must seal an exact stable case list
covering every family below. The list may not be expanded or adapted after an
execution is observed:

- identity frames in 2D and 3D;
- every exact signed-permutation basis: all 8 in 2D and all 48 in 3D, including
  proper and reflecting bases without attaching orientation semantics;
- scale exponents `{-8, -1, 0, 1, 8}` for representative exactly
  representable point and vector inputs;
- reciprocal-safety boundary exponents `{-1075, -1024, -1023, 1023, 1024}`
  and `std::numeric_limits<int>::min()`, with acceptance or rejection fixed by
  the entry authority rather than by a fixture-specific rule;
- nonzero finite origins and cases that distinguish point translation from
  vector mapping;
- exact local-to-world-to-local and world-to-local-to-world round trips in both
  dimensions;
- affine compatibility and point-difference/vector compatibility;
- compile-time point/vector separation and rejection of cross-dimensional map
  calls;
- duplicate, missing, non-unit, and otherwise invalid signed-permutation bases;
- non-finite proposed origin and basis components rejected by the qualified
  Point/Vector or Mat constructor before a frame can be formed, preserving the
  layered error classification rather than inventing a frame-level substitute;
- scale exponents whose scale or reciprocal is zero or non-finite;
- mapped point and vector cases that deliberately produce non-finite results;
  and
- signed-zero inputs and results under the inherited canonical comparison
  policy.

Expected values must be derived independently from the component equations in
the entry authority, not by calling production frame methods. Exact finite
nonzero values use hexadecimal floating representation. `+0.0` and `-0.0`
canonicalize to `0x0p+0` for semantic comparison while their raw encodings
remain diagnostic provenance. This is not a tolerance.

Rounded cases are outside this protocol. If a required fixture cannot be
expressed with exactly representable declared intermediates and outputs, the
attempt is `BLOCKED` pending a separate proximity-policy decision; the expected
value must not be adapted after observation.

## 6. Certificate and equivalence

Every case record contains at least:

- schema version, stable case ID, dimension, operation, and claim category;
- origin, basis, scale exponent, and point/vector inputs in canonical
  hexadecimal representation;
- expected and observed result kind and exact `GeometryError` classification;
- expected and observed point/vector components when successful;
- exact-match status and declared signed-zero policy; and
- explicit non-claims for orientation, handedness, rank, incidence, topology,
  conditioning, and general numerical invertibility.

Within one cell, all three semantic projections must be identical. Across
cells, case sets, result kinds, errors, canonical exact fields, and non-claims
must match exactly. Only declared compiler, library, path, process, timing,
dependency, and artifact-hash provenance may differ.

Missing, duplicate, additional, schema-invalid, non-finite where finite was
expected, or unclassified claim data is `BLOCKED`, not equivalent.

## 7. CF0–CF7 gates

| Gate | PASS condition |
| --- | --- |
| `CF0` — Scope and revision identity | Candidate is clean, published, revision-bound, and contains only the admitted Cartesian Frames capability plus evidence infrastructure; excluded transformations and later Geometry capabilities are absent. |
| `CF1` — Construction domain | Every preregistered admissible finite origin, signed-permutation basis, and reciprocal-safe scale is accepted; every declared invalid layered input, invalid basis, and unsafe scale is rejected with the exact declared classification. |
| `CF2` — Point/vector and dimension separation | Matching point/vector APIs are admitted, cross-dimensional calls are rejected at compile time, and translation affects points but never vectors. |
| `CF3` — Analytic map agreement | Local/world point and vector maps, affine compatibility, and difference compatibility agree with independent component equations for every enumerated exact case. |
| `CF4` — Bounded round trips and failure semantics | Both round-trip directions are exact for the preregistered representable subset; deliberate non-finite mapped results fail explicitly; no universal round-trip claim, tolerance, clamp, retry, or fallback is introduced. |
| `CF5` — Repeatability and cross-cell equivalence | Three independent processes per cell agree semantically, and all four GCC/Clang Debug/Release cells satisfy the declared exact equivalence and signed-zero rules. |
| `CF6` — Prerequisite preservation | The exact Architecture, Numeric, Reproducible Experiment, Point/Vector, and Minimal Small Linear Algebra allowlist passes in every cell on the same candidate with no semantic contradiction. |
| `CF7` — Evidence integrity and bounded closure | Prepared/terminal manifests, commands, certificates, comparisons, failures, inventories, hashes, limitations, retention seal, and detached-candidate verification are complete and independently verifiable. |

Overall `PASS` requires CF0–CF7 to pass together. No gate may be inferred from
another gate, a global test count, successful preparation, or collector status.

## 8. Stop and failure policy

The qualification is `BLOCKED` if any required command fails, evidence is
absent, repetitions disagree, a cross-cell difference remains unexplained, or
a prerequisite contract fails. It is also `BLOCKED` if passing requires:

- a tolerance or rounded fixture not separately authorized;
- a general inverse, solve, decomposition, or approximate orthonormality test;
- determinant sign, handedness, orientation, incidence, or topology semantics;
- arbitrary-angle, affine, projective, nonuniform-scale, curve, surface, or
  mesh capability;
- a hidden retry, fallback, clamp, relaxed expectation, or fixture-specific
  rescue; or
- adaptation of expected results after observing the formal run.

The first failed gate opens one bounded diagnosis. Failed evidence remains
immutable, and neither production behavior nor expectations are corrected
inside the scientific audit. A contradiction of a qualified prerequisite
reopens that prerequisite explicitly.

## 9. Required retained outputs

The terminal package must contain or hash-link:

- prepared and terminal manifests, launch plan, state history, and command
  records;
- candidate source/input inventories and authority hashes;
- twelve complete Cartesian Frames semantic certificates;
- per-cell and cross-cell comparisons;
- four prerequisite-preservation records and exact CTest discovery inventories;
- negative-fixture and partial-failure outcomes;
- compile-command and runtime-dependency inventories;
- compact JSON and Markdown gate summaries;
- explicit retained limitations; and
- a canonical retention manifest with detached-worktree verification.

Reproducible build trees, caches, object files, libraries, and executables are
excluded from canonical retention. No spatial figure is required because this
capability has no curve, surface, topology, or mesh semantics.

## 10. Decision effect and retained limitations

- `PASS`: only **Cartesian Similarity Frames** becomes `QUALIFIED` inside the
  declared WSL compiler envelope. Geometry Primitives remains `IN
  INVESTIGATION` until its cumulative stage regression passes.
- `BLOCKED`: Cartesian Frames remains implemented but unqualified, and one
  bounded diagnosis may be authorized.
- A prerequisite contradiction reopens that prerequisite instead of being
  absorbed as a Cartesian Frames limitation.

Native Windows, portability outside the declared compiler envelope,
performance, SIMD, GPU, OpenMP, MPI, arbitrary finite-input invertibility,
approximate frames, robust predicates, topology, curves, surfaces, and meshing
remain unqualified.

## 11. Next bounded action

Audit the completed CF0–CF7 admission infrastructure against this fixed
protocol before any separate decision about invoking the manifest creator. No manifest,
formal execution, excluded capability, or change to `CartesianFrame2`,
`CartesianFrame3`, their errors, or their focused contract is authorized.
