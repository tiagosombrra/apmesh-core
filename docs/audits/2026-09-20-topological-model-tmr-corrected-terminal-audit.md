# Topological Model TMR — Corrected Terminal Audit

Status: **PASS / TMR0–TMR7 PASS / TOPOLOGICAL MODEL QUALIFIED**  
Audit date: 2026-09-20  
Qualification scope: **exact admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud envelope only**  
Candidate: `37f9af77f38e12af0a92d3c0f57f1ad31a218144`  
Formal execution run: `35533702004`

## 1. Scope

This audit independently evaluates the second formal Topological Model
cumulative-regression campaign after the repetition-cardinality defect found in
the first campaign was diagnosed, corrected, separately prepared, audited, and
authorized.

The workflow/process conclusion is not used as the scientific decision. The
decision below is recomputed from the retained terminal package against the
pre-registered TMR0–TMR7 protocol.

## 2. Formal authorization and one-shot execution

PR #43 contained exactly one newly added repository file:

`experiments/authorizations/topological-model-tmr-f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa.json`

It was squash-merged to protected `main` as:

`cddd959574ed6a677ac755a5b329d53a9cfe32ec`

This merge was the formal `EXECUTE_ONCE` event.

Protected-main run `35533702004` independently:

1. validated the complete authorization commit;
2. validated the exact machine-readable PREPARED audit;
3. rejected any pre-existing claim;
4. revalidated the committed authorization;
5. checked out the exact historical scientific candidate;
6. reconstructed the sealed published-candidate identity;
7. verified the admitted cloud toolchain;
8. verified exact GitHub artifact metadata and digest before download;
9. restored and revalidated the exact PREPARED package;
10. created the immutable manifest-hash claim;
11. invoked the sealed campaign exactly once;
12. removed reproducible build trees;
13. verified retention; and
14. retained the terminal package.

Every workflow step passed.

The formal attempt is consumed. No rerun or rescue is authorized or needed.

## 3. Immutable execution claim

Claim tag:

`tmr-execution-claim-f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`

Annotated tag object:

`5f0b693780a681b6ceb9d17afb7c6c83fde72847`

The tag points to candidate:

`37f9af77f38e12af0a92d3c0f57f1ad31a218144`

and binds:

- authorization commit `cddd959574ed6a677ac755a5b329d53a9cfe32ec`;
- the exact manifest-bound authorization file;
- preparation run `35531261000`;
- prepared-manifest SHA-256
  `f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`;
- execution workflow run `35533702004`.

The annotated tag is unsigned. Tag signing is not a pre-registered protocol
requirement, so this is retained as a provenance limitation rather than a gate
failure.

## 4. Terminal artifact identity

Retained artifact:

- artifact ID: `10612032787`;
- name:
  `tmr-terminal-f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa-35533702004`;
- size: `260602` bytes;
- GitHub SHA-256:
  `dd5c12f54ed60a106059a9f42acf5c07884fa83230ae4dfe81d073cb6f7111b8`.

An independent download recomputed the same ZIP SHA-256 exactly.

The authorization commit also passed ordinary post-merge validation:

- FAST `35533701748`: PASS;
- INTEGRATION `35533701754`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

## 5. Lifecycle, preparation binding, and detached verification

The retained lifecycle is exactly:

`PREPARED → RUNNING → EXECUTED_PENDING_AUDIT`

The terminal package continues to bind the audited PREPARED manifest:

`f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`

The original preparation seal remains valid for all sealed preparation inputs.
The initial state-history record also retains the exact hash sealed during
preparation.

Detached-candidate verification is `PASS`, with the same 1546 tracked-source
inventory established during the independent PREPARED audit.

## 6. Retention integrity

The terminal archive contains exactly **146 files**.

`retention-manifest.json` hashes **145** retained files; together with the
retention manifest itself, that is the complete 146-file archive.

Independent recomputation found:

- missing retained paths: **0**;
- unexpected retained paths: **0**;
- hash mismatches: **0**;
- size mismatches: **0**;
- retained build directories: **0**.

The planned success artifact set and the actual retained set agree exactly.

The planned command-log inventory contains **112** stdout/stderr logs and the
terminal package retains exactly **112** command logs.

## 7. Corrected command cardinality

The terminal package contains exactly **56 command records**.

All 56 record IDs are unique.

All 56 commands:

- exited with code 0;
- did not time out;
- did not report a launch error.

Each of the four cells contains exactly 14 command records:

1. configure once;
2. repetition 1:
   - build;
   - CTest discovery;
   - semantic CTest;
   - certificate;
   - certificate validation;
3. repetition 2:
   - build;
   - CTest discovery;
   - semantic CTest;
   - certificate;
   - certificate validation;
4. negative outcomes;
5. dependency inventory; and
6. runtime dependency inventory.

Global cardinality:

- configure: **4**;
- build: **8**;
- CTest discovery: **8**;
- semantic CTest: **8**;
- certificate production: **8**;
- certificate validation: **8**;
- negative outcomes: **4**;
- dependency inventory: **4**;
- runtime dependency inventory: **4**.

This is exactly the prospective 56-command shape established after the first
campaign's blocked audit.

## 8. Semantic CTest evidence

There are exactly **8 semantic CTest records**:

4 cells × 2 repetitions.

For every cell and every repetition, the observed selected allowlist is exactly:

- `apmesh_core.bootstrap_smoke`;
- `apmesh_core.numeric_contract`;
- `apmesh_core.geometry_primitives`;
- `apmesh_core.minimal_small_linear_algebra`;
- `apmesh_core.math_header_isolation`;
- `apmesh_core.cartesian_frames`; and
- `apmesh_core.topological_model`.

Every repetition reports:

- 7 selected semantic tests;
- 7 passed;
- 0 failed;
- 100% passed.

Therefore the terminal evidence contains exactly:

**8 repetitions × 7 tests = 56 individual semantic test executions.**

The evidence-cardinality gap that blocked TMR6 and TMR7 in the first campaign
is no longer present.

## 9. Topology certificates

The terminal package contains exactly **8 certificates**.

Every certificate hash recorded by the certificate index recomputes exactly.

All eight certificate byte streams are identical.

Certificate SHA-256:

`e574cc589e2dc75c2ffd218af7336bbdc6202b6e2dcf4fe035a5f4e8d4a791a5`

Each certificate covers the same 13 cases:

1. `empty_model`;
2. `identity_and_edge_order`;
3. `transactional_rejection`;
4. `invalid_lookup_and_orientation`;
5. `boundary_valence`;
6. `multiple_loops_repeated_owner`;
7. `structural_classes`;
8. `three_face_multi_use`;
9. `incidence_bijection`;
10. `canonical_snapshot`;
11. `deterministic_equal_construction`;
12. `controlled_orientation_difference`; and
13. `controlled_insertion_difference`.

The retained type contract reports:

- distinct vertex/edge/face types: PASS;
- raw-ID construction rejected: PASS;
- no public `add_vertex` mutation path on the immutable model: PASS.

The certificates explicitly retain nonclaims for coordinate welding,
adjacency, pairing, boundary classification, manifold classification, geometry,
curves, surfaces, and meshing.

## 10. Independent semantic checks

### TMR1 evidence

Identity/order fixtures, transactional rejection, invalid-handle rejection,
orientation handling, arbitrary valence, repeated owners and immutable model
constraints are identical in all eight certificates.

### TMR2 evidence

`incidence_bijection` records:

- forward occurrences: 3;
- reverse records: 3;
- exact bijection: true;

with exact face, loop ordinal, use ordinal and orientation retained.

### TMR3 evidence

All five structural classes are present and independently recomputed:

- `unused`;
- `single_use`;
- `two_use_opposed`;
- `two_use_cooriented`;
- `multi_use`.

The retained consistency summary contains the expected edge/face/loop/use
counts and class counts, identically across all certificates.

### TMR4 evidence

The canonical snapshot fixtures retain:

- canonical schema/order;
- deterministic decimal representation;
- LF-only line endings;
- required final newline;
- equal construction equality;
- controlled orientation difference;
- controlled insertion-order difference.

All eight certificates agree byte-for-byte.

## 11. Repeat and cross-cell equivalence

Each cell retains exactly two certificates.

Every per-cell repeat comparison reports equivalent scientific projections.

The cross-cell comparison contains all eight certificates and reports an
identical 13-case semantic projection and identical type contract.

Byte identity of all eight certificates is stronger than the required
scientific-projection equivalence.

## 12. Negative and dependency evidence

Every cell rejects all five negative cases:

- `duplicate_case`;
- `forged_snapshot`;
- `forged_incidence`;
- `forged_summary`; and
- `undeclared_semantic_claim`.

All runtime dependency inventories resolve without `not found`.

The retained compile-command inventories match their recorded hashes.

The repo-local topology dependencies observed from the retained build evidence
are limited to:

- `src/topology/topology.cpp`;
- `include/apmesh/topology/topology.hpp`.

No excluded geometry, curve, surface, meshing or other semantic dependency is
observed in the topology object dependency evidence.

## 13. TMR0–TMR7 decisions

| Gate | Decision | Independent basis |
| --- | --- | --- |
| TMR0 — Identity and scope | **PASS** | Candidate is revision-bound and published; PREPARED/authorization/claim/terminal hashes bind exactly; detached verification passes; no undeclared scientific-candidate change is evidenced. |
| TMR1 — Construction and immutability | **PASS** | Strong identities, deterministic storage, transactional rejection/finalization and immutability/type contract pass identically in all eight certificates. |
| TMR2 — Incidence bijection | **PASS** | Every tested forward edge-use occurrence maps to exactly one reverse record with exact face, loop/use ordinal and orientation. |
| TMR3 — Structural recomputation | **PASS** | All five structural classes and consistency-summary counts independently recompute identically from authoritative topology. |
| TMR4 — Canonical snapshot | **PASS** | Schema, records, order, decimal representation, LF policy, final newline, equality and controlled differences agree across all eight certificates. |
| TMR5 — Repeat and cross-cell equivalence | **PASS** | Eight certificates are byte-identical; repeat and cross-cell scientific projections are identical. |
| TMR6 — Prerequisite preservation and isolation | **PASS** | The exact seven-test allowlist passes 7/7 in all eight repetitions; Foundation/Geometry prerequisite tests pass in every repetition; topology dependency evidence remains isolated from excluded semantic layers. |
| TMR7 — Evidence integrity and closure | **PASS** | Exact 56-command plan is retained; 112 logs, eight certificates, comparisons, negatives, inventories, sealing, detached verification and retention are complete and independently recomputable. |

## 14. Overall scientific decision

**TMR0–TMR7 PASS.**

**Overall TMR decision: PASS.**

**Topological Model — Explicit Identity and Incidence: QUALIFIED.**

Qualification is scoped to the exact formally admitted GitHub-hosted Ubuntu
24.04 x86_64 cloud envelope for this campaign.

This result does **not** establish WSL/cloud equivalence and does not expand any
scientific claim beyond the declared topology contracts and retained nonclaims.

Accepted Foundation and Geometry qualifications remain intact.

## 15. Decision effect and next bounded action

The protocol states that after a Topological Model PASS, a separate scientific
entry decision may open Curve Representation; curve implementation does not
begin automatically.

Therefore the next bounded action after this terminal audit is integrated and
its post-merge checks pass is:

**close the terminal-audit checkpoint, then open one separate scientific entry
decision for Curve Representation — Continuous Geometry Before
Discretization.**

That future entry decision must define its bounded first investigation problem
before any curve production implementation begins.

Machine-readable audit:

`docs/audits/2026-09-20-topological-model-tmr-corrected-terminal-audit.json`.
