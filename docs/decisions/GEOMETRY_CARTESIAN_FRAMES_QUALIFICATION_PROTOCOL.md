# Geometry Primitives — Cartesian Frames Qualification Protocol

Status: **PRE-REGISTERED / ADMISSION REVIEW REQUIRED / NOT EXECUTED**

Date: 2026-09-14

Authority: `docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md`

## 1. Question and boundary

Can the bounded `CartesianFrame2` and `CartesianFrame3` implementation be
qualified as a deterministic exact Cartesian local/world mapping layer while
preserving the qualified Foundation, Point/Vector, and Minimal Small Linear
Algebra capabilities?

This protocol qualifies only the admitted Cartesian frame capability. It does
not qualify general transforms, arbitrary rotations, nonuniform scale,
orientation predicates, topology, curves, surfaces, meshes, native Windows, or
the complete Geometry Primitives stage. Focused CTest success is implementation
evidence, not `CF0`–`CF7` closure.

## 2. Fixed claim

Within the declared WSL Ubuntu 24.04 compiler envelope, the candidate may claim
only that:

1. `CartesianFrame2` and `CartesianFrame3` are distinct immutable value types;
2. each frame contains a finite origin, an exact signed-permutation basis, and
   a reciprocal-safe power-of-two scale;
3. point and vector local/world maps obey the entry-decision formulas, with
   translation applied only to points;
4. invalid bases, invalid scales, and non-finite mapped results fail with the
   declared layer-specific classification;
5. fixed exact round-trip, affine, difference, and metric-scaling relations
   hold for the preregistered cases;
6. repeated GCC/Clang Debug/Release observations satisfy the exact equivalence
   rules below; and
7. the qualified prerequisite contracts remain passing on the same candidate.

Reflection is admitted as an algebraic signed permutation only. No determinant
sign, handedness, orientation, incidence, topology, conditioning, or general
geometric-robustness claim is made.

## 3. Candidate and preparation binding

Preparation is admissible only from one committed, clean, published candidate
whose upstream resolves to the same full commit. The frame production files in
that candidate must be identical to the implementation reviewed at
`a32cf67b63a287596d9afbdd6f4b5fe0a80b7a59`; the focused contract must contain
at least the cases published at
`e4b80982be59e8ef2abe614b47c376c2b583c588`. Any production change requires a
new implementation audit before preparation.

One immutable `PREPARED` manifest must bind by SHA-256:

- the full candidate commit and complete tracked-source inventory;
- this protocol, its entry authority, roadmap/state, profile, runner,
  exporter, collector, comparer, focused contract, public headers,
  implementation sources, CMake input, and prerequisite authorities;
- exact toolchains, presets, commands, working directory, environment,
  external control/evidence roots, planned compile-command paths, runtime
  dependency executables, and planned source/artifact inventories; and
- fixed cases, repetitions, claim fields, equivalence rules, gates, and
  retained limitations.

The preparation must reuse `tools/experiment_runtime.py` for clean-candidate
identity, input hashes, lifecycle state, command records, inventories, and
retention manifests. It must not create a competing general execution or
retention framework. The prepared state records `execution_requested=false`.
A separate authorization may execute that unchanged manifest once.

Preparation and execution cannot decide qualification. A report-only collector
may emit only `EVIDENCE_COLLECTED_PENDING_AUDIT`; an independent scientific
audit records `QUALIFIED` or `BLOCKED`.

## 4. Fixed execution matrix

Exactly four clean build cells are required:

| Cell | Compiler/library | Build |
| --- | --- | --- |
| `gcc-debug` | GCC 13 / libstdc++ | Debug |
| `gcc-release` | GCC 13 / libstdc++ | Release |
| `clang-debug` | Clang 18 / libc++ | Debug |
| `clang-release` | Clang 18 / libc++ | Release |

Each cell builds the library, focused frame contract, and evidence exporter from
the same candidate, then runs three independent semantic-certificate processes.
The formal runner is launched once and fails fast while retaining partial
evidence. No automatic retry is permitted.

Each cell also runs once an exact versioned allowlist for the qualified
Architecture, Numeric, Reproducible Experiment, Point/Vector, and Minimal Small
Linear Algebra preservation contracts. The profile must enumerate the expected
test names and reject missing or additional selections. Broad labels and
historical qualification runners are not admissible as the scientific
definition of preservation.

## 5. Fixed evidence cases

The profile must enumerate, not infer:

- identity maps for points and vectors in both directions and dimensions;
- read-only origin, basis, and exponent observations;
- two-dimensional quarter-turn mapping of both Cartesian axes as points and
  vectors;
- three-dimensional cyclic mapping of all Cartesian axes;
- a reflecting signed permutation, with explicit orientation non-claim;
- exponents `{-8, -1, 0, 1, 8}` for point and vector mappings;
- reciprocal-safe boundary exponents and separate zero, infinite, and
  non-finite-reciprocal rejection cases;
- local-to-world-to-local and world-to-local-to-world round trips for points and
  vectors in both dimensions;
- translation separation, affine compatibility, difference compatibility,
  dot scaling by `2^(2k)`, and norm scaling by `2^k`;
- compile-time rejection of cross-dimension point and vector operands;
- signed-zero basis entries plus missing, duplicate, and non-unit finite bases;
- non-finite matrix-entry rejection at qualified `Mat2`/`Mat3` construction,
  explicitly separated from finite non-permutation frame rejection; and
- deliberate point and vector mapping overflow.

Expected results are encoded independently of production mapping calls. All
first-qualification cases are exact: finite nonzero values use hexadecimal
floating representation, and both signed zeros canonicalize to `0x0p+0` for
semantic comparison while raw encodings remain diagnostic provenance. No
tolerance or rounded fixture is admitted.

## 6. Certificate and equivalence

Every case record contains at least:

- schema version, stable case ID, dimension, operation, and claim category;
- origin, basis, exponent, and operand components in hexadecimal form;
- expected and observed result kind and exact error classification;
- expected and observed point/vector components or accessor values;
- exact-match status and signed-zero policy; and
- explicit non-claims relevant to reflection, orientation, topology, and
  general transformations.

The independent validator recomputes basis admissibility, scale
representability, mapping formulas, round trips, compatibility laws, and metric
scaling from the recorded inputs. It must not call `CartesianFrame2/3` or copy
observed values into expected fields.

Within a cell, all three semantic projections must be identical. Across cells,
case sets, result kinds, errors, canonical exact fields, and non-claims match
exactly. Only declared compiler, path, process, timing, dependency, and artifact
hash provenance may differ. Missing, duplicated, additional, schema-invalid,
non-finite, or unclassified claim data is `BLOCKED`.

## 7. CF0–CF7 gates

| Gate | PASS condition |
| --- | --- |
| `CF0` — Scope and revision identity | Candidate is clean, published, immutable, fully hash-bound, and preserves the reviewed frame production files; excluded capabilities are absent. |
| `CF1` — Type, API, and dependency boundary | Dimension and point/vector separation, read-only access, and `numeric/base -> math -> geometry` hold; no general transform, inverse, predicate, or new dependency appears. |
| `CF2` — Construction and explicit failure | All admitted bases/scales construct exactly; invalid finite bases, non-finite matrix input, unsafe exponents, and non-finite mapped results receive the declared layer-specific classifications without clamp, retry, or fallback. |
| `CF3` — Exact mapping agreement | Identity, translation, quarter-turn, axis-cycle, reflection, and power-of-two point/vector maps agree with independent component formulas. |
| `CF4` — Round-trip and compatibility laws | Both-direction point/vector round trips, affine and difference compatibility, and exact dot/norm scaling pass for all fixed cases. |
| `CF5` — Repeatability and cross-cell equivalence | Three processes per cell agree semantically and all four GCC/Clang Debug/Release cells satisfy the fixed exact equivalence rule. |
| `CF6` — Prerequisite preservation | Exact Architecture, Numeric, Reproducible Experiment, Point/Vector, and Minimal Small Linear Algebra allowlists pass in every cell on the same candidate. |
| `CF7` — Evidence integrity and bounded closure | Prepared/terminal manifests, commands, certificates, comparisons, negative outcomes, inventories, hashes, limitations, partial-failure evidence, retention seal, and detached-candidate verification are complete. |

Overall `PASS` requires `CF0`–`CF7` to pass together. No gate may be inferred
from a global test count, focused CTest success, or another gate's result.

## 8. Failure and retention policy

The attempt is `BLOCKED` if any command fails, evidence is absent, repetitions
disagree, a cross-cell difference is unexplained, a prerequisite fails, or the
production implementation differs from the reviewed baseline. It is also
`BLOCKED` if passing requires a tolerance, arbitrary rotation, general inverse,
orientation/topology interpretation, hidden retry, fallback, clamp, or
fixture-specific rescue.

The first failed gate opens one bounded diagnosis. Failed evidence remains
immutable; implementation and expectations are not corrected inside the
scientific audit.

The retained package must contain or hash-link prepared/terminal manifests,
state history, commands, source/input inventories, twelve semantic
certificates, four prerequisite records, per-cell and cross-cell comparisons,
negative outcomes, dependency records, compact gate summaries, limitations,
retention manifest, and detached verification. Reproducible build trees,
caches, binaries, and transient logs are excluded.

## 9. Decision effect and limitations

- `PASS`: only **Cartesian Frame Semantics** becomes `QUALIFIED` within the
  declared WSL Ubuntu 24.04 GCC 13 / Clang 18 libc++ envelope. Geometry
  Primitives remains `IN INVESTIGATION` until its cumulative regression closes.
- `BLOCKED`: frames remain implemented but unqualified; one bounded diagnosis
  may be authorized.
- A prerequisite contradiction reopens that prerequisite instead of becoming a
  retained frame limitation.

Native Windows, performance, parallel execution, arbitrary transformations,
orientation predicates, topology, curves, surfaces, meshes, and the cumulative
Geometry Primitives stage remain unqualified.

## 10. Current decision

**Protocol pre-registered; a new independent admission remains required.**
Admission of `a3b1f4e` was blocked by incomplete recorded inputs, formula
recomputation, prerequisite discovery, process provenance and retention. This
is an infrastructure finding, not evidence of a Cartesian-frame production
defect.

The amended report-only package enumerates 142 cases with actual hexadecimal
inputs in both dimensions. The exporter calls the qualified APIs; the separate
rational oracle recomputes construction, maps and laws from those inputs.
Negative observations include their exact baseline mutation and rejection
reason. Raw signed zero is retained; only semantic comparison canonicalizes it.

Preparation binds the reviewed production and focused-contract Git objects,
all prerequisite authorities, source inventory, toolchains, planned commands
and artifacts. Control and evidence roots are separate: the control root is
created during preparation and the evidence root is created exclusively on
execution. The initial state/history and input table are sealed. Discovery
must match the exact prerequisite selection before tests execute.

Execution records independent exporter child PIDs, command results, source and
binary identities, compile commands, cache, object dependencies and runtime
libraries. Retention rechecks the semantic payload in a detached candidate
process and publishes the staged archive only after verification. A failed
command or failed sealing remains explicit; no retry is automatic. Transient
log files are excluded; hash-bound command stdout/stderr observations remain
in structured evidence for revalidation.

Focused development contracts, including disposable lifecycle/retention
fixtures, passed in GCC 13 and Clang 18 libc++ Debug/Release (four selected
CTests per build). They are not a formal PREPARED manifest or CF0–CF7 qualification.

The formal attempt on `008030f` was retained as
`BLOCKED_BY_EVIDENCE_OUTPUT_DIRECTORY_DEFECT`: its first GCC Debug certificate
exporter could not create its output because `evidence/certificates/` had not
been created. The terminal manifest records `gcc-debug-certificate-1` and
`retry_attempted=false`; no CF gate was interpreted. Published commit `067df4d`
creates the certificate and semantic output directories as part of the exclusive
evidence claim, with a focused lifecycle contract. It changes neither production
frame code nor the scientific profile. A new independent admission may now
consider one new manifest with new external roots; no manifest is currently
prepared and no scientific gate is closed.
