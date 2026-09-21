# Curve Representation — Continuous Curve Geometry Regression Protocol

Status: **FIRST FORMAL EXECUTION CONSUMED / TERMINAL AUDIT COMPLETE / CGR0–CGR7 PASS / STAGE QUALIFIED**  
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

## 27. Report-only tooling implementation record

The report-only tooling required by Section 22 is implemented on
`curve/continuous-geometry-regression-tooling`.

Audit authority:

`docs/audits/2026-09-21-continuous-curve-geometry-regression-tooling-audit.md`.

The tooling implements:

- the closed four-cell × two-repetition CGR profile;
- the exact fourteen-test semantic allowlist;
- curve scientific certificate export;
- independent analytic/metamorphic evidence validation;
- same-cell scientific-projection equality and cross-cell categorical/numeric
  relation validation;
- real fail-closed negative/adversarial evidence;
- deterministic CSV/JSON/SVG derived evidence;
- report-only plan/simulation of the exact 56-command / 112-log / eight-
  certificate / 112-semantic-test shape;
- focused GCC 13 Debug and Clang 18/libc++ Debug tooling contracts.

Final focused tooling run `35601303879` passed in both declared tooling
cells.

All eleven Section 4 semantic files remain byte-identical to baseline
`438620efa1f93d29b442e9ba199882a09d2359d9`.

The report-only runner exposes no `prepare` or `execute` command and all
CGR0–CGR7 gates remain `NOT_EXECUTED`.

This implementation record does not authorize formal preparation or execution
by itself. After tooling integration, ordinary post-merge validation, and
tooling-checkpoint closure, the next permitted work item is one separate formal
preparation design / PREPARED-package phase. Formal execution remains blocked
until a future PREPARED package is independently audited and separately
authorized.

## 28. Report-only tooling integration checkpoint

PR #75 integrated the report-only CGR tooling as
`6e3952f2bdee5ca9bdfe076d5af2932b359d130f`.

Validation:

- final branch-head tooling run `35602067270`: PASS in GCC 13 Debug and
  Clang 18/libc++ Debug;
- PR FAST `35602217696`: PASS;
- PR INTEGRATION `35602217726`: PASS;
- post-merge FAST `35603677515`: PASS;
- post-merge INTEGRATION `35603677377`: PASS.

No Section 4 semantic file changed. No formal PREPARED package, authorization,
claim, terminal package or CGR gate result exists.

The report-only tooling checkpoint is closed. The sole permitted continuation
is one separate formal PREPARED-package design decision that defines exact
candidate cleanliness, preparation sealing, planned evidence, lifecycle state
and revision-bound execution identity before formal preparation implementation.

Formal execution remains unauthorized.

## 29. Formal PREPARED lifecycle design

The formal preparation/lifecycle design is specified by:

`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md`.

The decision preserves this protocol unchanged and defines the mechanism by
which one future clean candidate may become PREPARED without executing CGR.

Key invariants:

- report-only planning remains the scientific plan authority;
- formal execution code exists and is revision-bound before preparation;
- all eleven Section 4 semantic files remain frozen;
- complete tracked-source and cloud-environment identities are sealed;
- the exact 56-command / 112-log / eight-certificate plan is retained;
- CGR0–CGR7 remain `NOT_EXECUTED` during preparation;
- the PREPARED archive contains only seven control files;
- independent PREPARED audit precedes any authorization;
- authorization and execution remain separate future lifecycle steps.

This design decision does not itself create a PREPARED package or authorize
execution.

After its integration and checkpoint closure, only formal campaign
infrastructure implementation is permitted.

## 30. Formal PREPARED lifecycle design integration checkpoint

PR #77 integrated the PREPARED lifecycle design as
`7ff5c6e5f52f9f5bd8153d00856239649aac0eff`.

Validation:

- PR FAST `35604864218`: PASS;
- PR INTEGRATION `35604864050`: PASS;
- post-merge FAST `35604986634`: PASS;
- post-merge INTEGRATION `35604986654`: PASS.

The design checkpoint is closed. No formal PREPARED package, authorization,
claim, terminal package or CGR gate result exists.

The sole permitted continuation is formal campaign infrastructure
implementation according to
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md`.

Formal preparation dispatch and formal execution remain unauthorized.

## 31. Formal campaign infrastructure validation

The formal campaign infrastructure is implemented on
`curve/cgr-formal-campaign-infrastructure` according to
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md`.

The implementation preserves the Section 4 frozen semantic baseline and reuses
the report-only plan authority. It adds no alternate scientific matrix or
semantic allowlist.

Formal tooling runs `35613409036` and `35613745665` passed in GCC 13
Debug and Clang 18/libc++ Debug across the exact eight-contract inventory:

- report-only evidence;
- report-only runner;
- report-only tooling workflow;
- formal lifecycle runner;
- preparation-only workflow;
- reusable executor;
- authorization record;
- authorization controller.

The synthetic lifecycle verifies the exact 56-command / 112-log /
eight-semantic-repetition / 112-individual-semantic-test / eight-certificate
shape while leaving CGR0–CGR7 outside formal execution as `NOT_EXECUTED`.

No real PREPARED package, authorization record, claim, terminal package or CGR
gate result is created by this infrastructure phase.

After integration and checkpoint closure, one formal PREPARED dispatch from
canonical clean `main` is permitted. The resulting artifact must receive an
independent **PASS / PREPARED / NOT EXECUTED** audit before any authorization
or formal execution becomes admissible.

## 32. Formal campaign infrastructure integration checkpoint

PR #79 integrated the formal CGR campaign infrastructure as
`a0232e0c00aae1338b55ba0b45997db1a3c00464`.

Exact-tree identity confirms the final formally validated branch content is the
content integrated into `main`:

`2ece9368e9b48a0c1db7fc1494a8713a833a9b43`.

Validation:

- final CGR FORMAL TOOLING `35614078272`: PASS in both declared tooling
  cells;
- PR FAST `35614279053`: PASS;
- PR INTEGRATION `35614278985`: PASS;
- post-merge FAST `35614426119`: PASS;
- post-merge INTEGRATION `35614426153`: PASS.

The formal infrastructure checkpoint is closed. The scientific protocol,
frozen baseline, matrix, allowlist, certificate model and CGR0–CGR7 criteria
remain unchanged.

The sole permitted continuation is one preparation-only dispatch from
canonical clean `main`, followed by an independent PREPARED audit. No
authorization or formal execution is permitted before that audit.

## 33. First formal PREPARED package

Formal preparation run `35620525792` produced one sealed PREPARED package on
candidate `f7dc8d82d881858b6481d6d2d1383d8a561684c5`.

Independent preparation audit is retained in
`docs/audits/2026-09-21-continuous-curve-geometry-regression-preparation-audit.md`
and records **PASS / PREPARED / NOT EXECUTED**.

The audited package binds prepared-manifest SHA-256
`201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`
and preparation-seal SHA-256
`686c5192f5373c42e54339fdd38519e62ebef009dc74ae927c34fe97919b5353`.

The audit verifies the complete candidate/source identity, Section 4 frozen
semantic baseline, exact critical-input hashes, admitted cloud matrix, and
pre-registered formal plan. No command has executed and CGR0–CGR7 remain
`NOT_EXECUTED`.

After audit integration and closure, the sole permitted continuation is one
separate exact manifest-bound `EXECUTE_ONCE` authorization-record PR. Its
protected-main merge will be the formal one-shot execution authorization event.

## 34. PREPARED audit integration checkpoint

PR #81 integrated the formal PREPARED audit as
`988d0877d78ccd0c1ed4d368a802a8a4cad28d7b`. Post-merge FAST
`35629633009` and INTEGRATION `35629632897` passed.

The audited PREPARED package remains unconsumed and binds manifest
`201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`.
CGR0–CGR7 remain `NOT_EXECUTED`.

The PREPARED-audit checkpoint is closed. The sole permitted continuation is one
separate exact manifest-bound `EXECUTE_ONCE` authorization-record PR. Its
protected-main merge will be the formal execution authorization event.

## 35. First formal execution and terminal scientific audit

PR #83 introduced exactly one new manifest-bound authorization record and
merged it to protected `main` as:

`e8b17256924e907d0859b8ac7061600ffc404b9e`

The authorization binds:

- candidate
  `f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
- preparation run `35620525792`;
- prepared artifact `10649325906`;
- prepared artifact SHA-256
  `952cadc3d5cc761105d5100319cf24077cd9b0ac5d0a42000a8ae1e3eac91063`;
- prepared-manifest SHA-256
  `201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`;
- preparation-seal SHA-256
  `686c5192f5373c42e54339fdd38519e62ebef009dc74ae927c34fe97919b5353`.

Protected-main run `35630423134` validated the complete authorization
commit, machine-readable PREPARED audit, unclaimed manifest, candidate and
artifact provenance, admitted cloud toolchain, and restored PREPARED package
before creating the immutable claim:

`cgr-execution-claim-201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`

The claim is an annotated Git tag object
`027d5c033a7d5b91e97b5ce8c35d14030f801037` targeting the exact
candidate. The tag is unsigned; signing was not a pre-registered protocol
requirement.

The runner then invoked the sealed CGR campaign exactly once, removed
reproducible build trees, verified terminal retention and retained terminal
artifact `10654358199` with independently recomputed archive SHA-256:

`a4b59453dcc9f9cacb4265ba540e1a3a443f3b6c80509411e0d6d4b18eff7aa6`

The first formal attempt is consumed.

Independent terminal audit authority:

- `docs/audits/2026-09-21-continuous-curve-geometry-regression-terminal-audit.md`;
- `docs/audits/2026-09-21-continuous-curve-geometry-regression-terminal-audit.json`.

The retained terminal package independently satisfies the pre-registered
cardinality and closure requirements:

- 56 unique successful command records;
- 112 command logs;
- eight CTest discovery records;
- eight exact semantic CTest records;
- fourteen tests per repetition, 112 individual semantic tests total, all
  passing;
- eight validated scientific certificates;
- same-cell scientific-projection equality;
- cross-cell categorical equality and independently valid numeric relations;
- complete negative/adversarial evidence;
- compile/runtime dependency inventories;
- deterministic derived CSV/JSON/SVG evidence;
- exact source and frozen-semantic identity;
- detached verification;
- complete lifecycle/sealing/retention evidence.

Independent gate decision:

- CGR0: `PASS`;
- CGR1: `PASS`;
- CGR2: `PASS`;
- CGR3: `PASS`;
- CGR4: `PASS`;
- CGR5: `PASS`;
- CGR6: `PASS`;
- CGR7: `PASS`;
- overall: `PASS`.

Per Section 24, **Curve Representation — Continuous Geometry Before
Discretization is QUALIFIED only in the exact formally admitted GitHub-hosted
Ubuntu 24.04 x86_64 cloud environment**.

This decision establishes no WSL/cloud equivalence and does not extend the
declared nonclaims into physical discretization, surfaces, meshing,
Quad-Dominant, parallel execution, GPU/SIMD, anisotropy or native-Windows
qualification.

After terminal-audit integration and checkpoint closure, the sole permitted
scientific continuation is one separate entry decision for **Curve Differential
Geometry — Curvature, Regularity, and Features**. No physical Boundary Curve
Discretization implementation begins automatically.

## 36. Qualification integration checkpoint

PR #84 integrated the terminal scientific audit as
`d0045767d5a4c7fb910fd3e8aaccea73673fb558`.

Validation:

- PR FAST `35634165038`: PASS;
- PR INTEGRATION `35634165026`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35634295936`: PASS;
- post-merge INTEGRATION `35634295812`: PASS in both cells.

The Curve Representation qualification checkpoint is closed. This protocol is
now historical stage-exit authority. The sole permitted continuation is the
separate Curve Differential Geometry scientific entry decision described in
Section 24. No Boundary Curve Discretization implementation is authorized by
this closure.


