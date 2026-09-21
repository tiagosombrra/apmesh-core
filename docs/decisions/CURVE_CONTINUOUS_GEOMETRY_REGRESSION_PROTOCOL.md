# Curve Representation — Continuous Curve Geometry Regression Protocol

Status: **PRE-REGISTERED / INTEGRATED / DOCUMENTATION-ONLY / NO FORMAL EXECUTION AUTHORIZED**  
Date: 2026-09-21  
Stage: **Curve Representation — Continuous Geometry Before Discretization**  
Semantic baseline at protocol entry:
`438620efa1f93d29b442e9ba199882a09d2359d9`

## 1. Scientific question

Does the complete admitted continuous-curve layer preserve its declared
representation, differential, regularity, total/cumulative length and certified
inverse-mapping semantics across the admitted cloud toolchain matrix, repeated
executions, analytic references and metamorphic transformations, while all
qualified prerequisites remain passing and all downstream physical
discretization claims remain absent?

This protocol is the integrated stage-exit authority for Curve Representation.

A workflow success, focused unit-test success or report-only tooling success is
not by itself a Curve Representation qualification decision.

## 2. Preconditions

Formal preparation is forbidden until all of the following are true:

1. Foundation qualification remains accepted;
2. Geometry Primitives qualification remains accepted;
3. Topological Model remains qualified in the admitted GitHub-hosted Ubuntu
   24.04 x86_64 environment;
4. the following Curve Representation work units are integrated and closed:
   - Polynomial Cubic Bézier Value Representation and Evaluation;
   - Cubic Bézier Differential Evaluation and Pointwise Speed;
   - Certified Global Cubic Regularity;
   - Certified Cubic Bézier Total Arc-Length Enclosure;
   - Certified Cumulative Arc-Length Enclosure;
   - Certified Inverse Arc-Length Bracketing;
5. FAST and INTEGRATION on the Work Unit 2B closure baseline are passing;
6. this protocol is integrated and its documentation checkpoint is closed;
7. report-only regression tooling is separately implemented, validated,
   integrated and closed;
8. no production curve semantic file covered by Section 4 has changed without
   a separately reviewed protocol amendment.

No formal campaign is authorized by this protocol document.

## 3. Qualification scope

A PASS may qualify only the currently admitted continuous curve semantics for
polynomial cubic Bézier curves in 2D and 3D:

- immutable ordered four-control-point representation;
- deterministic value evaluation over `t in [0,1]`;
- first derivative;
- second derivative;
- pointwise speed;
- global regularity classification under an explicit resource policy;
- certified total arc-length enclosure;
- certified cumulative arc-length enclosure;
- certified inverse arc-length parameter bracketing for absolute and normalized
  targets;
- reversal semantics;
- admitted translation and power-of-two scale relations;
- 2D/3D planar-embedding parity where declared;
- explicit finite/domain/policy/resource failure semantics.

Qualification does **not** establish:

- curvature, torsion, Frenet frames or feature classification;
- arbitrary-degree Bézier, B-spline, NURBS, rational or transcendental curves;
- generic root solving or public generic interval arithmetic;
- lookup-table or fitted inverse parameterizations;
- physical equal-arc-length sample generation;
- boundary discretization or adaptive sampling;
- surface/trim ownership;
- mesh sizing;
- shared-boundary certification;
- triangular or quadrilateral meshing;
- Quad-Dominant capability;
- parallel, GPU or SIMD equivalence;
- WSL/cloud equivalence;
- native-Windows qualification.

## 4. Frozen semantic baseline

The following existing semantic files define the stage claims at protocol
entry and must remain byte-identical to baseline
`438620efa1f93d29b442e9ba199882a09d2359d9` through the formal PREPARED
candidate unless a separate amendment is integrated before preparation:

- `include/apmesh/geometry/curve.hpp`;
- `src/geometry/curve.cpp`;
- `src/geometry/detail/curve_length_interval.hpp`;
- `src/geometry/detail/curve_regularity_interval.hpp`;
- `tests/curve_representation.cpp`;
- `tests/curve_differential.cpp`;
- `tests/curve_regularity.cpp`;
- `tests/curve_arc_length.cpp`;
- `tests/curve_cumulative_arc_length.cpp`;
- `tests/curve_inverse_arc_length.cpp`;
- `tests/curve_header_isolation.cpp`.

Tooling, evidence exporters, workflow files, documentation and CMake
registration needed for qualification may change after protocol entry, but
they may not modify the scientific semantics above silently.

The formal PREPARED package must also bind the exact complete tracked-source
inventory of its candidate.

## 5. Exact semantic CTest allowlist

Every formal repetition must discover and execute exactly these fourteen tests:

1. `apmesh_core.bootstrap_smoke`;
2. `apmesh_core.numeric_contract`;
3. `apmesh_core.geometry_primitives`;
4. `apmesh_core.minimal_small_linear_algebra`;
5. `apmesh_core.math_header_isolation`;
6. `apmesh_core.cartesian_frames`;
7. `apmesh_core.topological_model`;
8. `apmesh_core.curve_representation`;
9. `apmesh_core.curve_differential`;
10. `apmesh_core.curve_regularity`;
11. `apmesh_core.curve_arc_length`;
12. `apmesh_core.curve_cumulative_arc_length`;
13. `apmesh_core.curve_inverse_arc_length`;
14. `apmesh_core.curve_header_isolation`.

Zero selected tests, missing tests, duplicate selection, extra semantic tests or
a different allowlist is a formal gate failure.

## 6. Admitted formal matrix

The formal cloud matrix is exactly:

| Cell | Compiler | Standard library | Build |
| --- | --- | --- | --- |
| `gcc-debug` | GCC 13.3.0 | libstdc++ | Debug |
| `gcc-release` | GCC 13.3.0 | libstdc++ | Release |
| `clang-debug` | Clang 18.1.3 | libc++ 18.1.3 | Debug |
| `clang-release` | Clang 18.1.3 | libc++ 18.1.3 | Release |

All cells run on the admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud
environment and exact accepted tool paths/packages already governed by the
cloud qualification authorities.

The campaign makes no equivalence claim for WSL, native Windows, a different
runner image, different package revision or different architecture.

## 7. Repetition and command cardinality

Every cell has exactly **two repetitions**.

Each cell is configured exactly once.

Each repetition must execute, in this order:

1. build;
2. CTest discovery;
3. exact fourteen-test semantic CTest;
4. curve scientific certificate production;
5. independent certificate validation.

After both repetitions, each cell executes exactly once:

- negative-evidence validation;
- compile/dependency inventory;
- runtime dependency inventory.

Therefore a successful campaign has exactly:

- 4 configure records;
- 8 build records;
- 8 CTest-discovery records;
- 8 semantic-CTest records;
- 8 certificate-production records;
- 8 certificate-validation records;
- 4 negative-evidence records;
- 4 dependency-inventory records;
- 4 runtime-dependency records;
- **56 command records total**;
- **112 stdout/stderr command logs**.

Because every semantic CTest record executes fourteen tests, the campaign must
contain exactly:

**8 repetitions × 14 tests = 112 individual semantic test executions.**

Repetition identity must be present in every repetition-scoped command,
discovery record and certificate path.

## 8. Curve scientific certificate

Every repetition must retain one machine-readable curve certificate.

There are exactly eight certificates.

Each certificate must contain:

- schema version and certificate kind;
- candidate identity;
- cell and repetition identity outside the scientific projection;
- declared case inventory;
- exact public type/API contract projection;
- exact categorical outcomes;
- retained analytic/reference inputs;
- retained production enclosures/brackets where relevant;
- independently recomputable invariant observations;
- explicit nonclaims from Section 3.

The scientific projection must exclude timestamps, PIDs, runner paths and other
provenance-only data.

### 8.1 Same-cell determinism

The two scientific certificate projections in the same cell must be byte
identical.

### 8.2 Cross-cell equivalence

Cross-cell qualification does not require floating enclosure endpoints to be
bit-identical across libstdc++ and libc++.

It does require:

- identical case inventory;
- identical categorical result/classification;
- identical exact discrete/type/API claims;
- every numeric enclosure or bracket independently satisfies its pre-registered
  analytic or metamorphic condition;
- no cell introduces a claim absent from another cell.

A cross-cell comparison artifact must state the exact equality domains and the
independently validated numerical-relation domains.

## 9. Required certificate cases

At minimum the certificate exporter must independently cover the following
scientific case families.

### Representation and value

- exact endpoint evaluation at `t=0` and `t=1`;
- exactly representable straight cubic;
- nonuniform cubic value fixture;
- reversal value relation;
- exact translation relation;
- power-of-two scaling relation;
- planar 2D/3D embedding parity;
- non-finite/out-of-domain parameter rejection.

### Differential and speed

- analytic first derivative;
- analytic second derivative;
- endpoint derivatives;
- reversal derivative sign/parameter relation;
- speed nonnegativity and reversal relation;
- planar 2D/3D speed parity;
- finite/error behavior at admitted extremes.

### Global regularity

- regular straight cubic;
- exact degenerate endpoint/constant fixture;
- interior-stationary or otherwise non-regular fixture;
- resource-limited `indeterminate`;
- reversal invariance;
- translation invariance;
- power-of-two scale invariance;
- 2D/3D parity where declared.

### Total arc length

- exact axis-monotone length;
- analytic/reference curved fixture;
- lower ≤ reference ≤ upper;
- converged versus resource-limited indeterminate behavior;
- reversal invariance;
- translation invariance;
- power-of-two scale relation;
- 2D/3D planar parity.

### Cumulative arc length

- exact zero prefix;
- `S(1)` equivalence with total-length evidence under the same policy;
- analytic/reference prefix fixture;
- nondecreasing prefix relation;
- reversal relation;
- translation/power-of-two scale/embedding relations;
- finite/domain/policy failure cases.

### Certified inverse arc length

- absolute zero;
- normalized zero and one;
- exact-total endpoint when total evidence is exact;
- analytic straight absolute target;
- analytic straight normalized target;
- nonuniform/analytic curved target;
- reversal relation;
- translation and power-of-two scale relations;
- 2D/3D parity;
- uncertain total-domain rejection;
- degenerate and indeterminate regularity rejection;
- ambiguous midpoint returns `indeterminate` with prior valid bracket;
- iteration exhaustion retains a valid bracket;
- zero parameter tolerance does not fabricate an exact inverse.

## 10. Analytic/reference authority

Expected scientific references must be independent of the production
implementation.

Allowed references include:

- exact algebra for Bézier endpoint/value/derivative fixtures;
- exact straight-line length relations;
- exact power-of-two transformation relations;
- high-precision or analytic parabola arc-length expressions retained as
  reference data;
- independently derived monotonicity/reversal equations.

The legacy AP Mesh implementation is not an acceptance oracle.

A diagnostic numerical quadrature or sampled plot may be retained only as
secondary visualization/reference. It may not override a failed conservative
certificate.

## 11. Derived regression figures

The formal terminal package must retain deterministic derived evidence for at
least:

1. curve value/reference comparison;
2. first/second derivative and speed reference comparison;
3. total/cumulative arc-length enclosure width versus refinement/resource
   setting;
4. inverse parameter-bracket width versus refinement iteration.

The scientifically authoritative input to these figures is retained
machine-readable CSV/JSON data.

SVG figures may be generated deterministically from that data without a
third-party runtime dependency.

Figures are derived evidence, not independent acceptance oracles.

## 12. Negative/adversarial evidence

A separate negative-evidence artifact must prove rejection or correct
`indeterminate` handling for at least:

- malformed/duplicate certificate case inventory;
- forged categorical result;
- forged analytic-reference containment claim;
- forged inverse-bracket ordering claim;
- undeclared semantic claim;
- invalid regularity policy;
- invalid length policy;
- invalid inverse policy;
- non-finite curve parameter;
- non-finite inverse target;
- inverse target outside the certified domain;
- inverse target whose membership is uncertifiable;
- inverse regularity not certified.

The evidence validator must fail closed on missing, extra or inconsistent
scientific case data.

## 13. Dependency and isolation requirements

Curve production evidence may depend on admitted Foundation/Geometry headers
and implementation required by the existing curve layer.

It must not gain a production dependency on:

- topology implementation or topology headers;
- filesystem state;
- environment variables;
- logging;
- random generators;
- wall-clock time;
- a third-party geometry/numerics runtime;
- surface, discretization or meshing modules;
- OpenMP/MPI/GPU execution.

The Topological Model semantic test is a prerequisite-preservation check, not a
production dependency authorization for the curve module.

Compile-command and dependency inventories must be retained and independently
auditable.

## 14. CGR0 — Identity and scope

PASS requires all of:

- exact clean formal candidate identity;
- exact upstream/published-candidate binding;
- exact tracked-source inventory;
- exact protocol/tool/profile/exporter/validator hashes;
- admitted cloud identity PASS in all cells;
- Section 4 semantic baseline preserved or separately amended before
  preparation;
- no physical discretization or downstream claim introduced.

Any identity or scope mismatch is BLOCKED/FAIL according to whether evidence is
missing or contradictory.

## 15. CGR1 — Representation and value

PASS requires the representation/value certificate cases to establish:

- immutable ordered cubic control data;
- endpoint and analytic value correctness;
- deterministic repeatability;
- reversal/translation/scale/embedding relations;
- explicit parameter failure semantics.

## 16. CGR2 — Differential evaluation and speed

PASS requires:

- analytic first/second derivative agreement;
- speed agreement/nonnegativity where declared;
- reversal relations;
- 2D/3D planar parity;
- finite/error semantics;
- deterministic repeatability.

## 17. CGR3 — Global regularity

PASS requires:

- regular cases certified regular;
- exact degenerate cases classified degenerate;
- bounded-resource uncertainty classified indeterminate rather than guessed;
- invariance/parity relations preserved;
- no detached regularity authority admitted.

## 18. CGR4 — Arc-length and inverse mapping

PASS requires:

- certified total-length containment;
- certified cumulative-length containment and endpoint relation;
- monotone cumulative relation;
- certified inverse brackets with the required lower/upper cumulative proof;
- total-length uncertainty preserved for normalized targets;
- ambiguity/resource paths retain valid brackets and return indeterminate;
- analytic/reference, reversal, translation, scale and embedding relations all
  pass.

## 19. CGR5 — Repeat and cross-cell equivalence

PASS requires:

- exactly eight validated certificates;
- two same-cell scientific projections byte-identical in every cell;
- cross-cell categorical/discrete claims identical;
- every cross-cell numeric certificate independently satisfies the same
  analytic/metamorphic relations;
- no cell-specific extra scientific claim.

## 20. CGR6 — Prerequisite preservation and isolation

PASS requires:

- the exact fourteen-test allowlist passes 14/14 in all eight repetitions;
- therefore all 112 individual semantic test executions pass;
- Foundation, Geometry and Topological Model prerequisite tests remain passing;
- curve header isolation remains passing;
- compile/runtime dependency evidence satisfies Section 13.

## 21. CGR7 — Evidence integrity and closure

PASS requires all planned evidence to be retained and independently
recomputable:

- 56 command records;
- 112 command logs;
- eight discovery records;
- eight semantic CTest records;
- eight certificates;
- per-cell repeat comparisons;
- cross-cell comparison;
- negative evidence;
- compile/runtime dependency inventories;
- source inventory;
- environment observations;
- derived regression data/figures;
- lifecycle/state history;
- preparation and terminal sealing;
- exact retention manifest.

Missing required evidence blocks qualification even when executable tests
themselves passed.

## 22. Formal lifecycle

The formal lifecycle is staged:

1. **Protocol decision** — this document only;
2. **Report-only tooling** — exporter, validator, runner, negative contracts,
   focused tooling tests; no formal scientific execution;
3. **Tooling integration checkpoint**;
4. **Formal PREPARED package** from one clean candidate;
5. **Independent PREPARED audit**;
6. **Exact one-shot repository-resident execution authorization**;
7. **Formal execution** exactly once;
8. **Independent terminal audit** against CGR0–CGR7;
9. **Qualification integration checkpoint**.

Preparation is not execution.

Workflow success is not scientific PASS.

Once the immutable execution claim exists, any later formal failure consumes
the attempt. No rerun/rescue is allowed unless a new protocol-governed campaign
is prepared and separately authorized.

## 23. Formal decision rules

### PASS

Curve Representation may be qualified only if CGR0–CGR7 all PASS.

### BLOCKED

Use BLOCKED when required evidence is absent/incomplete or a formal
precondition cannot be established without contradicting the retained
scientific results.

### FAIL

Use FAIL when retained evidence contradicts a pre-registered scientific claim.

A BLOCKED or FAIL campaign may not be relabeled by weakening this protocol
after execution.

## 24. Stage decision effect

On CGR0–CGR7 PASS:

**Curve Representation — Continuous Geometry Before Discretization:
QUALIFIED in the exact admitted cloud environment.**

A PASS does not start physical boundary discretization automatically.

The next permitted scientific transition is one separate entry decision for:

**Curve Differential Geometry — Curvature, Regularity, and Features.**

Boundary Curve Discretization remains blocked until its own prerequisites and
entry decision are satisfied.

On BLOCKED or FAIL, Curve Representation remains stage-unqualified and only the
bounded diagnosis explicitly authorized by the terminal audit may proceed.

## 25. Immediate next action after protocol closure

After this protocol is integrated, FAST/INTEGRATION pass, and its documentation
checkpoint is closed, implement only report-only qualification tooling:

- curve certificate exporter;
- curve certificate/evidence validator;
- cumulative-regression runner;
- negative-evidence generator/validator;
- deterministic regression-data/SVG derivation;
- focused tooling contracts.

That tooling work must not prepare or execute the formal campaign.


## 26. Protocol integration checkpoint

PR #73 integrated this protocol as
`af580a9358428e1607c77f7557355595a0a7c45c`.

Validation:

- PR FAST `35598147692`: PASS;
- PR INTEGRATION `35598147572`: PASS;
- post-merge FAST `35598248171`: PASS;
- post-merge INTEGRATION `35598248036`: PASS.

No production curve semantic file changed and no formal qualification campaign
occurred. The next permitted work item is report-only CGR tooling only.
