# Continuous Curve Geometry Regression — Terminal Audit

Status: **PASS / CGR0–CGR7 PASS / CURVE REPRESENTATION QUALIFIED**  
Audit date: 2026-09-21  
Qualification scope: **exact admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud envelope only**  
Candidate: `f7dc8d82d881858b6481d6d2d1383d8a561684c5`  
Formal execution run: `35630423134`

## 1. Scope

This audit independently evaluates the first formal Continuous Curve Geometry
Regression campaign against the pre-registered CGR0–CGR7 protocol.

Workflow success is treated only as process evidence. The scientific decision
below is recomputed from the retained terminal package, the independently
audited PREPARED package, the immutable execution claim, and the exact
pre-registered protocol.

## 2. Formal authorization and one-shot execution

PR #83 contained exactly one newly added repository file:

`experiments/authorizations/continuous-curve-geometry-regression-201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd.json`

It was squash-merged to protected `main` as:

`e8b17256924e907d0859b8ac7061600ffc404b9e`

The complete commit diff is one added file / 14 added lines and no other
repository change.

This merge was the formal `EXECUTE_ONCE` event.

Protected-main run `35630423134` independently:

1. validated the exact authorization commit;
2. validated the machine-readable PREPARED audit;
3. rejected any pre-existing claim;
4. revalidated the authorization before candidate checkout;
5. checked out candidate
   `f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
6. reconstructed the sealed published-candidate identity;
7. installed and checked the admitted cloud toolchain;
8. validated exact GitHub artifact identity/digest/provenance;
9. restored and revalidated the audited PREPARED package;
10. created one immutable manifest-hash claim;
11. invoked the sealed CGR campaign exactly once;
12. removed reproducible build trees;
13. verified terminal retention; and
14. retained the terminal package.

Every step passed.

The formal attempt is consumed. No rerun/rescue is authorized or required.

## 3. Immutable execution claim

Claim tag:

`cgr-execution-claim-201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`

Annotated tag object:

`027d5c033a7d5b91e97b5ce8c35d14030f801037`

The tag points to:

`f7dc8d82d881858b6481d6d2d1383d8a561684c5`

and binds:

- authorization commit
  `e8b17256924e907d0859b8ac7061600ffc404b9e`;
- exact authorization file;
- preparation run `35620525792`;
- prepared-manifest SHA-256
  `201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`;
- execution run `35630423134`.

The tag is annotated and unsigned. Signing was not a pre-registered CGR
requirement, so this is retained as a provenance limitation rather than a gate
failure.

## 4. Terminal artifact

Terminal artifact:

- artifact ID: `10654358199`;
- name:
  `cgr-terminal-201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd-35630423134`;
- size: `397025` bytes;
- GitHub SHA-256:
  `a4b59453dcc9f9cacb4265ba540e1a3a443f3b6c80509411e0d6d4b18eff7aa6`.

An independent download recomputed the same ZIP SHA-256 exactly.

The authorization commit also passed normal protected-main regression:

- FAST `35630422359`: PASS;
- INTEGRATION `35630422086`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

## 5. Identity, frozen semantics, and lifecycle

The formal candidate remains exactly:

`f7dc8d82d881858b6481d6d2d1383d8a561684c5`

The PREPARED audit had already independently verified:

- 1595 tracked source paths;
- 1595 GitHub candidate blobs;
- path-list SHA-256
  `46b39d68c68f8f580f4c1c61a08ee3014c834ecccb08a3b9e7d5e80a498b8b69`;
- 18/18 critical preparation inputs;
- 11/11 frozen curve-semantic files;
- semantic baseline
  `438620efa1f93d29b442e9ba199882a09d2359d9`;
- all four admitted cloud observations.

The terminal package binds the same candidate, source inventory and semantic
baseline.

The recursive GitHub tree for the candidate still contains exactly 1595 blobs,
and the terminal candidate path-list recomputes to the same
`46b39d68…b8b69` hash.

Detached verification is `PASS`.

Lifecycle is exactly:

`PREPARED → RUNNING → EXECUTED_PENDING_AUDIT`

The four preparation-sealed files remain byte-identical to their sealed hashes.
The first PREPARED state-history line still hashes to the preparation-sealed
state-history SHA-256.

## 6. Retention integrity

The terminal ZIP contains exactly **155 files**.

`retention-manifest.json` hashes **154 files**. Together with the retention
manifest itself, this is the complete 155-file package.

Independent recomputation found:

- missing retained paths: **0**;
- unexpected retained paths: **0**;
- SHA-256 mismatches: **0**;
- size mismatches: **0**;
- retained build directories: **0**.

The successful planned artifact set and actual retained file set agree exactly.

## 7. Command cardinality

The terminal package contains exactly **56 command records**.

All IDs are unique. Every command:

- exits 0;
- has no timeout;
- has no launch error.

Each of four cells contains exactly 14 commands:

- configure: 1;
- build: 2;
- CTest discovery: 2;
- semantic CTest: 2;
- certificate production: 2;
- certificate validation: 2;
- negative evidence: 1;
- dependency inventory: 1;
- runtime dependency inventory: 1.

Global cardinality is therefore:

- configure: 4;
- build: 8;
- CTest discovery: 8;
- semantic CTest: 8;
- certificate production: 8;
- certificate validation: 8;
- negative evidence: 4;
- dependency inventory: 4;
- runtime dependency inventory: 4.

The planned log set is exactly **112** stdout/stderr files and the terminal
package retains exactly **112** command logs.

## 8. Semantic CTest evidence

There are exactly eight semantic CTest records:

4 cells × 2 repetitions.

Each repetition executes exactly the fourteen-test allowlist from Section 5 of
the protocol. Selection order may follow CTest registration order, but the set
is exact, complete and duplicate-free.

Every semantic repetition reports:

- 14 selected tests;
- 14 passed;
- 0 failed;
- 100% passed.

Therefore the campaign retains exactly:

**8 × 14 = 112 individual semantic test executions, all PASS.**

This preserves Foundation, Geometry Primitives and Topological Model
prerequisites while exercising every admitted Curve Representation focused
contract.

## 9. Curve certificates

Exactly eight certificates are retained.

The full certificate byte streams differ only through provenance identity as
allowed by the protocol. Their same-cell scientific projections are identical,
and in this campaign the scientific projections are in fact identical across
all eight certificates.

Canonical scientific-projection SHA-256:

`076571b1cc63f19748726aec394816fcfcdbb0d4f76a271c0a5f00709b4c19be`

The projection contains 15 declared scientific case families:

1. representation/value;
2. differential/speed;
3. regularity regular;
4. regularity degenerate;
5. regularity indeterminate;
6. total length line;
7. total length parabola;
8. cumulative length line;
9. cumulative length parabola;
10. inverse line;
11. inverse parabola;
12. reversal relations;
13. translation/scale relations;
14. planar embedding parity; and
15. failure semantics.

The public type projection remains exactly cubic, polynomial, four controls,
2D/3D, parameter domain `[0,1]`, non-rational.

Declared downstream/nonclaims remain unchanged, including no curvature,
torsion, physical sampling, boundary discretization, meshing, Quad-Dominant,
parallel-equivalence, WSL/cloud-equivalence or native-Windows qualification.

## 10. Independent analytic recomputation

The audit independently recomputed the principal retained references rather
than relying only on the production exporter/validator.

For the value fixture with controls
`(0,0), (0,2), (2,2), (2,0)`:

- endpoint values match exactly;
- `B(0.5) = (1, 1.5)`;
- `B'(0) = (0, 6)`;
- `B''(0) = (12, -12)`;
- speed at `t=0` is exactly `6`;
- all retained value/derivative/speed series points agree with independent
  analytic formulas.

For the exact straight cubic:

- total length is exactly `3`;
- the retained half-prefix enclosure contains `1.5`;
- the inverse target `0.6` brackets exact parameter `0.2`;
- lower/upper cumulative evidence proves the target bracket.

For the parabola fixture `(x,y)=(t,t²)`:

- exact total arc length is approximately `1.4789428575445975`;
- retained total enclosure
  `[1.4736336690827536, 1.4843984095863125]` contains it;
- exact prefix at `t=0.25` is
  `0.2600572048586377`;
- retained cumulative enclosure contains it;
- inverse evidence brackets `t=0.25`;
- the lower/upper cumulative proof is valid;
- the indeterminate total-length status is preserved rather than guessed away.

Regularity classifications are exactly `regular`, `degenerate`, and
`indeterminate` on their declared cases.

Reversal, translation, scale and planar-embedding residuals are zero on the
retained exact relations.

Arc-length enclosure widths and inverse-bracket widths are nonincreasing across
the retained refinement series.

## 11. Repeat and cross-cell equivalence

Each matrix cell retains exactly two certificates.

The two same-cell scientific projections are equal in every cell.

The cross-cell comparison reports:

- certificate count: 8;
- same-cell projection equality: true;
- cross-cell categorical equality: true;
- independently validated numeric relations: true.

The audit additionally compared all eight complete scientific projections and
found them identical.

This exceeds the minimum CGR5 requirement, which permits valid floating
enclosure differences across standard libraries as long as the declared
analytic/metamorphic relations remain satisfied.

## 12. Negative/adversarial evidence

Every cell retains 13 negative outcomes, all `REJECTED`:

- duplicate case;
- forged categorical result;
- forged reference containment;
- forged inverse-bracket ordering;
- undeclared semantic claim;
- invalid regularity policy;
- invalid length policy;
- invalid inverse policy;
- non-finite curve parameter;
- non-finite inverse target;
- inverse target outside domain;
- inverse target with uncertifiable membership; and
- inverse regularity not certified.

The failures are either validator rejection or exact declared production error
classification. No negative case is silently accepted.

## 13. Dependency and isolation evidence

All runtime inventories resolve without `not found`.

The `curve.cpp` object dependency block in every cell has the same
repository-local semantic dependency set:

- `src/geometry/curve.cpp`;
- `include/apmesh/geometry/curve.hpp`;
- `include/apmesh/core/geometry.hpp`;
- `include/apmesh/math/linear_algebra.hpp`;
- `include/apmesh/core/numeric.hpp`;
- `src/geometry/detail/curve_length_interval.hpp`;
- `src/geometry/detail/curve_regularity_interval.hpp`.

No topology, surface, mesh/meshing, filesystem, random, OpenMP, MPI, CUDA or
other prohibited production dependency is observed for the curve object.

No parallel/GPU/SIMD execution flag is introduced for that production unit.

## 14. Derived regression evidence

The terminal package retains all required deterministic derived evidence:

- value/reference CSV + SVG;
- differential/speed CSV + SVG;
- arc-length enclosure CSV + SVG;
- inverse-bracket CSV + SVG;
- derived-evidence manifest.

Total: **9 files**.

The derived manifest binds source certificate
`1596e2ffe2145b8f21672d731cbe6367889204624fb5b543dd63df9fc98c1ff6`
(`gcc-debug-1`).

The CSV data agree with the certificate scientific series, and the SVG files
are deterministic text-only derivations retained under the terminal seal.

## 15. CGR0–CGR7 decisions

| Gate | Decision | Independent basis |
| --- | --- | --- |
| CGR0 — Identity and scope | **PASS** | Exact candidate, source inventory, frozen semantic baseline, critical input/cloud binding, detached verification and retained nonclaims all hold. |
| CGR1 — Representation and value | **PASS** | Ordered cubic representation, endpoint/value analytic fixtures, deterministic repetitions and reversal/translation/scale/embedding relations pass. |
| CGR2 — Differential evaluation and speed | **PASS** | Independent first/second derivative and speed formulas match; reversal/parity and explicit failure semantics remain correct. |
| CGR3 — Global regularity | **PASS** | Regular, degenerate and resource-limited indeterminate cases classify correctly; no sampling-based overclaim is introduced. |
| CGR4 — Arc length and inverse mapping | **PASS** | Straight exact references, parabola total/cumulative containment, monotone cumulative behavior, valid inverse brackets and uncertainty preservation pass. |
| CGR5 — Repeat and cross-cell equivalence | **PASS** | Exactly eight certificates; same-cell projections equal; categorical equality and numeric relations pass across cells; all eight scientific projections are identical. |
| CGR6 — Prerequisite preservation and isolation | **PASS** | Exact 14-test allowlist passes 14/14 in all eight repetitions (112 semantic tests); curve header/dependency isolation and runtime inventories satisfy Section 13. |
| CGR7 — Evidence integrity and closure | **PASS** | Exact 56 commands/112 logs, discoveries, CTests, certificates, comparisons, negatives, dependency inventories, derived evidence, lifecycle, sealing and retention are complete and independently recomputable. |

## 16. Overall scientific decision

**CGR0–CGR7 PASS.**

**Overall CGR decision: PASS.**

**Curve Representation — Continuous Geometry Before Discretization:
QUALIFIED.**

Qualification is scoped only to the exact formally admitted GitHub-hosted
Ubuntu 24.04 x86_64 cloud environment.

This result does not establish WSL/cloud equivalence and does not expand any
claim into physical boundary discretization, surface ownership, meshing,
Quad-Dominant, parallel, GPU/SIMD, anisotropy or native-Windows qualification.

Accepted Foundation, Geometry Primitives and Topological Model qualifications
remain intact.

## 17. Next bounded action

Per the pre-registered stage decision effect, Curve Representation PASS does
not start physical boundary discretization automatically.

After this audit is integrated and its post-merge checkpoint is closed, the
next permitted scientific transition is one separate entry decision for:

**Curve Differential Geometry — Curvature, Regularity, and Features.**

That future decision must bound its first investigation problem and exclusions
before any production implementation begins.

Machine-readable audit:

`docs/audits/2026-09-21-continuous-curve-geometry-regression-terminal-audit.json`.
