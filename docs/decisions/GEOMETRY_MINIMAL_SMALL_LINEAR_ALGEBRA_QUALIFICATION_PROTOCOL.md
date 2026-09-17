# Geometry Primitives — Minimal Small Linear Algebra Qualification Protocol

Status: QUALIFIED / LA0-LA7 PASS / CANDIDATE 3804e90 / WSL Ubuntu 24.04
Date: 2026-09-11
Stage: Geometry Primitives — Exact Semantics Before Curves
Authority: `docs/contracts/APMESH_CORE_MINIMAL_SMALL_LINEAR_ALGEBRA_CONTRACT.md`

## 1. Question and boundary

Can the bounded `Mat2`/`Mat3` implementation be qualified as a deterministic,
finite-valued, fixed-size algebra layer while preserving the qualified
Foundation and Point/Vector semantics?

This protocol qualifies only the capability admitted by the authority above.
It does not qualify transforms, frames, inverses, solves, predicates, topology,
curves, surfaces, meshes, native Windows, or the complete Geometry Primitives
stage. Focused CTest success is implementation evidence, not LA0–LA7 closure.

## 2. Fixed claim

The candidate may claim only that:

- `Mat2` and `Mat3` are distinct finite-valued types with checked access;
- zero, identity, transpose, same-dimension composition, and an algebraic
  determinant satisfy the fixed analytic cases;
- the Geometry layer applies a matching matrix to `Vector2` or `Vector3` and
  rejects non-finite results explicitly;
- the logical dependency remains `numeric/base -> math -> geometry`; and
- repeated GCC/Clang Debug/Release observations satisfy the equivalence rules
  below.

The determinant remains an algebraic scalar. No sign, rank, degeneracy,
orientation, incidence, conditioning, or robustness-predicate claim is made.

## 3. Candidate and preparation binding

Preparation is admissible only from one committed, clean, published candidate
whose upstream resolves to the same full commit. One immutable `PREPARED`
manifest must bind by SHA-256:

- candidate commit and complete tracked-source inventory;
- this protocol, its authority, current roadmap/state, profile, runner,
  collector, comparer, focused contracts, public headers, implementation
  sources, CMake input, and prerequisite authorities;
- exact toolchains, presets, commands, working directory, environment, output
  roots, fixtures, repetitions, fields, equivalence rules, gates, and retained
  limitations; and
- planned source, compile-command, runtime-dependency, and artifact inventories.

The manifest records `execution_requested=false`. A separate authorization may
execute that unchanged manifest once. A failed attempt is terminal evidence;
no automatic retry, expectation change, or in-place repair is allowed.

Preparation and execution do not decide qualification. A report-only collector
may emit only `EVIDENCE_COLLECTED_PENDING_AUDIT`. A separate scientific audit
records `QUALIFIED` or `BLOCKED`.

## 4. Fixed execution matrix

Exactly four clean cells are required:

| Cell | Compiler/library | Build |
| --- | --- | --- |
| `gcc-debug` | GCC 13 / libstdc++ | Debug |
| `gcc-release` | GCC 13 / libstdc++ | Release |
| `clang-debug` | Clang 18 / libc++ | Debug |
| `clang-release` | Clang 18 / libc++ | Release |

Each cell builds the admitted library, focused contract, isolated math-header
contract, and evidence exporter from the same candidate. Each cell executes
three independent semantic-certificate processes. The formal launcher runs
once and fails fast while retaining partial evidence.

Each cell also runs once an exact, versioned prerequisite allowlist containing
only the qualified Architecture, Numeric, Reproducible Experiment, and
Point/Vector preservation contracts. A broad `geometry` or `contract` label is
not an admissible selector because it may include the candidate or historical
self-tests. The profile must enumerate the expected test names and reject
missing or additional entries.

## 5. Fixed evidence cases

The profile must enumerate, not infer:

- zero and identity for both dimensions;
- every valid component and out-of-range row and column access;
- fixed signed diagonal, 2D swap, 3D cyclic permutation, 2D quarter-turn,
  non-commuting composition, transpose, and zero-row cases;
- matching `Mat2`–`Vector2` and `Mat3`–`Vector3` applications;
- compile-time rejection of point operands, cross-dimension application, and
  cross-dimension composition;
- quiet NaN and positive/negative infinity in every matrix entry;
- deliberate non-finite application, composition, and determinant results;
- exact power-of-two scaling with exponents `{-8, -1, 0, 1, 8}`, restricted to
  cases whose declared intermediates and results are representable; and
- source/dependency checks for every excluded capability and prohibited
  `math -> geometry` dependency.

Expected values are encoded independently of production calls. Exact integer
and power-of-two cases compare canonical hexadecimal numeric representations.
Finite nonzero values retain their exact hexadecimal representation; `+0.0`
and `-0.0` both canonicalize to `0x0p+0` for semantic comparison. Raw expected
and observed encodings remain recorded as diagnostic provenance, and every
exact comparison record declares
`signed_zero_policy=normalize_to_positive`. This is not a tolerance.
Any rounded case must preregister its independent reference, scale,
`ProximityPolicy`, residual, and limit. No default epsilon is permitted.

## 6. Certificate and equivalence

Every case record contains at least:

- schema version, stable case ID, dimension, operation, and claim category;
- input entries/components in hexadecimal floating representation;
- expected and observed result kind and exact error classification;
- expected and observed entries, components, or scalar result;
- exact-match status or explicit proximity fields; and
- the declared signed-zero policy for exact hexadecimal comparison; and
- an explicit non-claim for determinant predicate/rank/topology semantics where
  determinant is observed.

Within a cell, the three semantic projections must be identical. Across cells:

- case sets, result kinds, errors, exact fields, and non-claims match exactly;
- each rounded field independently satisfies the same preregistered bound; and
- only declared compiler, path, process, timing, dependency, and artifact-hash
  provenance may differ.

Missing, duplicate, additional, non-finite, schema-invalid, or unclassified
claim data is `BLOCKED`.

The first formal attempt on candidate `6fa00bd` remains `BLOCKED`: its
`mat2_quarter_turn_square` certificate recorded raw `0x0p+0` versus
`-0x0p+0`, while the validator asserted `exact_match=true` through numeric
equality without declaring canonical signed-zero handling. The scientific
semantics and minimum correction are fixed in
`docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_SIGNED_ZERO_DECISION.md`.

## 7. LA0–LA7 gates

| Gate | PASS condition |
| --- | --- |
| `LA0` — Scope and revision identity | Clean published candidate, immutable manifest, complete hashes, and no implementation outside the admitted capability or its evidence infrastructure. |
| `LA1` — Dimension, dependency, and access | Distinct `Mat2`/`Mat3`; isolated `math`; one-way Geometry adapters; matching operands admitted; point/cross-dimension operands rejected; every access case classified. |
| `LA2` — Finite construction and failure | Every non-finite placement and every deliberate non-finite arithmetic result yields the exact declared error without clamp, retry, or fallback. |
| `LA3` — Algebraic agreement | Zero, identity, transpose, application, composition, permutation, and quarter-turn observations agree with independent component formulas. |
| `LA4` — Determinant boundary | Fixed determinant values agree algebraically; code, certificate, and report attach no predicate, rank, degeneracy, orientation, incidence, or topology meaning. |
| `LA5` — Scale and deterministic equivalence | All fixed scale laws pass; three repeats per cell agree; all four cells satisfy the declared exact/proximity equivalence. |
| `LA6` — Prerequisite preservation | The exact Architecture, Numeric, Reproducible Experiment, and Point/Vector allowlist passes in every cell on the same candidate with no semantic contradiction. |
| `LA7` — Evidence integrity and bounded closure | Prepared/terminal manifests, commands, certificates, comparisons, failures, inventories, hashes, limitations, retention seal, and detached-candidate verification are complete and independently reproducible. |

Overall PASS requires LA0–LA7 to pass together. No gate may be inferred from a
global test count or another gate's result.

## 8. Failure and retention policy

The attempt is `BLOCKED` if a command fails, evidence is absent, repetitions
disagree, a cross-cell difference is unexplained, or a prerequisite contract
fails. It is also `BLOCKED` if passing requires an undeclared tolerance,
inverse, solve, decomposition, predicate, topology interpretation, third-party
runtime dependency, hidden retry, fallback, clamp, or fixture-specific rescue.

The first failed gate opens one bounded diagnosis. The failed evidence remains
immutable, and neither implementation nor expectations are changed during the
scientific audit.

The retained package must contain or hash-link the prepared and terminal
manifests, state history, command records, source/input inventories, twelve
semantic certificates, four prerequisite records, per-cell and cross-cell
comparisons, negative outcomes, dependency records, compact gate summaries,
limitations, retention manifest, and detached verification. Reproducible build
trees, caches, and binaries are excluded.

No spatial figure is required because this capability has no coordinate-frame,
topology, curve, surface, or mesh semantics.

## 9. Decision effect and limitations

- `PASS`: only Minimal Small Linear Algebra becomes `QUALIFIED` inside the WSL
  Ubuntu 24.04 GCC 13 / Clang 18 libc++ envelope. Geometry Primitives remains
  `IN INVESTIGATION`.
- `BLOCKED`: Mat2/Mat3 remains implemented but unqualified; one bounded
  diagnosis may be authorized.
- A prerequisite contradiction reopens that prerequisite rather than being
  absorbed as an LA limitation.

Native Windows, performance, SIMD, GPU, OpenMP, MPI, arbitrary conditioning,
general numerical stability, robust determinant signs, transforms, topology,
and meshing remain unqualified.

## 10. Formal decision

The independent audit of the one revision-bound execution on clean published
candidate `3804e903f56a226d38319fd44255b5832639815d` records `PASS` for
LA0-LA7. The execution retained 12 semantic certificates from four GCC/Clang
Debug/Release cells, 40 expected negative rejections, observed source and
runtime inventories, command records, a retention manifest, and detached
verification. The detailed decision is recorded in
`docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION.md`.

The first attempt on `6fa00bd` remains immutable `BLOCKED` evidence. The
signed-zero correction did not alter `Mat2`, `Mat3`, Geometry adapters, or the
admitted mathematical capability.

## 11. Next bounded action

Minimal Small Linear Algebra is closed within its declared WSL envelope. Do
not start Transformations and Coordinate Frames from this protocol; a separate
entry decision is required.
