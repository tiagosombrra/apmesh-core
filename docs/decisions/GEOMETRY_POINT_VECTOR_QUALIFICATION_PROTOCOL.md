# Geometry Primitives — Point/Vector Qualification Protocol

Status: ADMISSION CORRECTIONS IMPLEMENTED / FOCUSED CONTRACT PASS / NOT PREPARED / NOT EXECUTED
Date: 2026-09-08
Stage: Geometry Primitives — Exact Semantics Before Curves
Work unit: Qualify Point and Vector Semantics
Entry authority: `docs/decisions/GEOMETRY_PRIMITIVES_ENTRY_DECISION.md`

## 1. Question and decision boundary

Can the implemented `Point2`, `Point3`, `Vector2`, and `Vector3` capability be
qualified, on one clean revision, as a deterministic finite-valued affine and
Euclidean primitive layer inside the already qualified Foundation envelope?

This protocol qualifies only the Point/Vector investigation problem. It cannot
qualify the complete Geometry Primitives stage and cannot authorize matrices,
transforms, predicates, topology, curves, surfaces, meshing, native Windows, or
parallel execution. Small Linear Algebra and Transformations remain separate
future decisions. The Geometry Primitives end-of-stage regression remains
mandatory after all admitted investigation problems are complete.

## 2. Fixed claims

The candidate may establish only that, within the declared WSL Ubuntu 24.04
compiler envelope:

1. points and vectors are distinct public types with the admitted affine result
   types;
2. valid construction excludes non-finite coordinates;
3. finite operations either return their declared finite result or an explicit
   classified failure;
4. dot, cross, norm, and normalization agree with independently declared
   analytic cases and explicit Numeric Contract proximity policies;
5. exact power-of-two rescaling preserves the declared relations whenever the
   result remains representable;
6. structured claim fields are repeatable within a build cell and semantically
   equivalent across the four qualified compiler/build cells; and
7. the qualified Foundation contracts remain passing on the same candidate.

Exact coordinate equality remains a numeric relation only. This protocol makes
no geometric-coincidence, orientation-sign, robustness-predicate, incidence, or
topological-identity claim.

## 3. Candidate and revision binding

Preparation is admissible only from a committed, clean, published candidate
whose upstream resolves to the same commit. The prepared manifest must bind:

- the full candidate commit and complete tracked-source inventory;
- the profile, runner, collector, comparer, focused C++ contract, public header,
  implementation source, CMake configuration, and all prerequisite authorities
  by SHA-256;
- the exact toolchain identities, presets, planned compile-command paths, planned
  runtime dependency executables, working directory, environment observations,
  and output roots; the terminal package must separately hash-bind the observed
  compile commands and runtime dependency inventory;
- the fixed fixtures, repetitions, claim fields, equivalence rules, gates, and
  retained limitations in this protocol; and
- `execution_requested=false` until a separately authorized launch consumes the
  unchanged `PREPARED` manifest.

Preparation, focused test success, and complete report-only evidence cannot
change a scientific status. Only a separate audit of retained terminal evidence
may record `QUALIFIED` or `BLOCKED`.

## 4. Fixed execution matrix

Exactly four clean build cells are required:

| Cell | Compiler and library | Build |
| --- | --- | --- |
| `gcc-debug` | GCC 13 / libstdc++ | Debug |
| `gcc-release` | GCC 13 / libstdc++ | Release |
| `clang-debug` | Clang 18 / libc++ | Debug |
| `clang-release` | Clang 18 / libc++ | Release |

Each cell executes the Point/Vector contract in three independent processes.
The formal runner is launched once. A failed command stops the attempt and its
partial evidence is retained; no automatic retry is permitted.

Each cell also runs the current registered Foundation contract tests once on the
same revision. The full historical REC campaign is not repeated here: its
general execution and retention contract is reused, while the later Geometry
Primitives end-of-stage regression remains responsible for the cumulative
stage campaign.

## 5. Fixed analytic and adversarial fixtures

The evidence profile must enumerate, rather than infer, all cases below:

- origin, Cartesian basis, and positive/negative signed-zero placements;
- compile-time admitted and forbidden affine expressions;
- exact affine closure and basis dot/cross identities;
- axis norms, `(3,4)` norm, unit-axis normalization, cross antisymmetry, and
  parallel-vector cross product;
- every-coordinate placement of quiet NaN and positive/negative infinity in all
  four value types;
- `denorm_min()`, `min()`, `lowest()`, `max()`, ordinary normal values, and safe
  powers of two with exponents `{-500, -100, 0, 100, 500}`;
- zero and signed-zero normalization;
- zero scalar division and non-finite scalar input;
- deliberate non-finite results from vector addition/subtraction, scaling,
  division, point translation/displacement, dot, cross, and norm; and
- representable nonzero subnormal directions that must not be silently
  classified as zero.

Expected results are encoded independently of production calls. Exact cases use
declared exact components. Rounded cases record the reference value, reference
scale, `ProximityPolicy`, residual, limit, and classification. No default epsilon
or fixture-specific rescue is admissible.

For `normalize_three_four`, the independent residual is the Euclidean norm of
the observed-minus-expected components, the reference scale is the Euclidean
norm of the expected unit vector, and `limit = absolute_limit +
relative_limit * reference_scale`. The certificate and validator must both
recompute these finite quantities.

## 6. Claim fields and equivalence

Every case record contains at least:

- stable case and operation identifiers;
- admitted input components in hexadecimal floating representation;
- expected and observed result kind;
- expected and observed `GeometryError` when fallible;
- result components or scalar value when successful;
- exact-match flag, or the explicit policy, reference scale, residual, and limit
  for rounded results; and
- the declared claim/non-claim category.

Within one cell, all three semantic projections must be identical. Across cells:

- result kinds and error classifications must match exactly;
- exact-case components must match exactly;
- rounded cases must independently satisfy the same pre-registered bound; and
- only declared compiler, path, PID, timestamp, duration, and artifact hashes
  are volatile provenance.

Missing, duplicated, additional, non-finite, schema-invalid, or unclassified
claim data is `BLOCKED`, not equivalent.

## 7. Qualification gates

The short identifiers are secondary to the descriptive gate names.

| Gate | PASS condition |
| --- | --- |
| `PV0` — Scope and candidate identity | Candidate is clean, published, revision-bound, and contains only the admitted Point/Vector capability plus evidence infrastructure; excluded geometry and topology capabilities are absent. |
| `PV1` — Point/vector semantic separation | Compile-time evidence accepts every authorized expression/result type and rejects point-plus-point, scalar-times-point, and other excluded expressions. |
| `PV2` — Finite construction and explicit failure | All invalid coordinate/scalar placements and non-finite intermediates are rejected with the declared classification; zero remains a valid vector and zero normalization is explicitly degenerate. |
| `PV3` — Affine and vector algebra | Exact origin, basis, affine closure, dot, cross, antisymmetry, and parallel cases match their independent analytic expectations. |
| `PV4` — Norm, normalization, and scale behavior | Norm and normalization satisfy the fixed ordinary, extrema, subnormal, and power-of-two cases under their explicit exact or proximity rule, with no hidden tolerance, clamp, retry, or fallback. |
| `PV5` — Repeatability and cross-cell equivalence | Three independent processes per cell agree semantically, and all four GCC/Clang Debug/Release cells satisfy the declared cross-cell equivalence rules. |
| `PV6` — Foundation preservation | The current Architecture, Numeric, and reproducible-experiment contract tests pass on the same candidate in every cell; no qualified Foundation claim is contradicted. |
| `PV7` — Evidence integrity and bounded closure | Manifest, command records, certificates, comparisons, dependency inventory, hashes, limitations, partial-failure evidence, and retention seal are complete and independently verifiable from a detached candidate worktree. |

Overall `PASS` requires `PV0`–`PV7` to pass together. A report-only collector
must emit `EVIDENCE_COLLECTED_PENDING_AUDIT`; it cannot declare the scientific
decision.

## 8. Stop and failure policy

The qualification is `BLOCKED` if any command fails, a required case or artifact
is absent, repetitions disagree, an unexpected cross-cell difference remains,
or a Foundation contract fails. It is also `BLOCKED` if qualification requires:

- a global/default tolerance;
- coordinate-based identity or a topological interpretation;
- a robust-predicate claim from raw dot/cross values;
- a new matrix, transform, curve, surface, or mesh abstraction;
- a hidden retry, fallback, clamp, relaxed expectation, or fixture-specific
  constant; or
- adaptation of expected results after observing the formal run.

An unexpected result opens exactly one bounded diagnosis against the first
failed gate. The failed evidence is preserved, and neither tooling nor the
implementation is corrected inside the scientific audit.

## 9. Required retained outputs

The terminal package must contain or hash-link:

- prepared and terminal manifests, launch plan, state history, and command
  records;
- complete candidate source inventory and input hashes;
- twelve Point/Vector semantic certificates and their per-cell/cross-cell
  comparisons;
- four Foundation contract-test records and dependency inventories;
- negative-fixture outcomes;
- compact JSON and Markdown gate summaries;
- explicit retained limitations; and
- a canonical retention manifest with detached-worktree verification.

No spatial figure is required because the candidate has no segment, frame,
curve, surface, topology, or mesh semantics. A compact gate table is sufficient.

## 10. Decision effect

- `PASS`: only **Point and Vector Semantics** becomes `QUALIFIED` within the
  declared WSL compiler envelope. Geometry Primitives remains `IN
  INVESTIGATION`; a separate decision may then consider the next demonstrably
  required geometry capability.
- `BLOCKED`: Point/Vector remains implemented but unqualified. Geometry
  Primitives does not advance, and one bounded diagnosis is opened.

Neither outcome changes Foundation qualification. A contradiction of Foundation
evidence instead reopens the affected prerequisite explicitly.

## 11. Retained limitations

- Native Windows and cross-platform portability are not qualified.
- No performance, SIMD, GPU, OpenMP, MPI, or parallel-equivalence claim is made.
- No matrices, transforms, frames, predicates, topology, curves, surfaces,
  meshes, or geometric-coincidence semantics are qualified.
- Passing finite analytic fixtures is not an unconditional proof of numerical
  stability for arbitrary inputs.
- This work-unit gate does not replace the mandatory Geometry Primitives
  end-of-stage regression.

## 12. Admission-hardening implementation result

The bounded infrastructure now consists of a versioned, case-enumerating
profile; an independent C++ certificate exporter with declared expected and
observed fields; a report-only validator/comparer that requires all twelve
fixed certificate slots; and a `PREPARED`-only launcher. The launcher binds
the Geometry Primitives entry authority and current Foundation authorities,
tool identities, candidate source inventory, planned artifact inventories,
observed compile-command and runtime-dependency hashes, negative-fixture
outcomes, partial command records, separate prepared/terminal manifests, and a
detached-worktree retention seal. A closure/retention failure is recorded as
explicit `BLOCKED` evidence rather than leaving a pending-audit state behind.

Focused contracts now pass; this remains tooling and focused-contract evidence only. No formal manifest was
prepared, no qualification cell was launched, and no `PV0`–`PV7` scientific
gate changed status.

## 13. Next bounded action

Audit the corrected admission package against this protocol and explicitly
authorize or block creation of one `PREPARED` manifest. Do not execute the
formal qualification regression.
