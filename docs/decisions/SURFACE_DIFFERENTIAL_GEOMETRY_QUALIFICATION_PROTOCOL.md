# Surface Differential Geometry — Cumulative Qualification Protocol

Status: **PROTOCOL INTEGRATED / CLOSURE ACTIVE / TOOLING NOT AUTHORIZED /
FORMAL EXECUTION NOT AUTHORIZED / NOT QUALIFIED**  
Date: 2026-09-27  
Stage: **Surface Differential Geometry — Metric, Normals, and Curvatures**  
Entry authority:
`docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_READINESS_DECISION.md`

## 1. Scientific question

Can the already-integrated AP Mesh local surface-differential layer be
qualified, on one clean revision and inside one explicitly admitted compiler /
environment envelope, as a deterministic finite-valued pointwise implementation
of:

- first fundamental form;
- area density;
- oriented unit normal;
- pointwise Jacobian 2-norm condition number;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state;
- typed parameter/domain/continuity/singularity/non-representability failures;

while preserving every currently admitted ordinary semantic contract and
without promoting Surface Representation, meshing, anisotropy or any other
excluded capability?

A workflow success, ordinary test success, certificate generation or
report-only tooling success is not itself a qualification decision.

## 2. Protocol-entry authority and preconditions

Protocol-entry protected `main`:

`744dbce1553a537dc22a0830d6ea878c0efb0fe7`.

Entry validation:

- FAST #625: PASS;
- INTEGRATION #616 / GCC 13 Debug: PASS;
- INTEGRATION #616 / Clang 18/libc++ Debug: PASS;
- ordinary semantic registration inventory: **42 tests**;
- open PRs at entry: none;
- active production/scientific/repository-transition work item at entry: none;
- entry/structural audit:
  `docs/audits/2026-09-27-surface-differential-geometry-qualification-protocol-entry-audit.md`.

Formal preparation is forbidden until all of the following are true:

1. this protocol is integrated, post-merge validated and separately closed;
2. report-only Surface Differential Geometry qualification tooling is
   separately designed, implemented, validated, integrated and closed;
3. any preparation/launch infrastructure is separately decided, implemented,
   validated, integrated and closed;
4. one clean, committed, published candidate is selected;
5. the protected candidate and the remote upstream resolve to the same commit;
6. all frozen semantic files in Section 5 remain unchanged unless a separately
   reviewed protocol amendment is integrated before preparation;
7. no prerequisite authority has been reopened;
8. one explicit formal-execution authorization is issued separately.

This protocol document does not authorize preparation or execution.

## 3. Bounded qualification claim

A terminal PASS may qualify only:

**Deterministic pointwise local differential geometry for the currently
admitted bounded parametric-surface derivative contract in 3D, inside the
protocol's admitted GitHub-hosted Ubuntu 24.04 x86_64 compiler envelope.**

The admitted mathematical outputs are exactly those listed in Section 1.

The claim is local and pointwise. It does not establish global surface
regularity, global curvature bounds, global distortion bounds or any meshing
acceptance threshold.

Concrete surface-family executions are conformance and oracle evidence for the
generic differential layer. They do not qualify Surface Representation as a
stage.

## 4. Explicit nonclaims

This protocol cannot qualify or authorize:

- Surface Representation as a stage;
- principal directions, curvature-line integration or line-field continuity;
- near-umbilic classification thresholds;
- an acceptable/ill-conditioned threshold for meshing;
- analytic cone or torus production;
- periodic-seam topology;
- general p-curves, wires or topological-face binding;
- Boundary Curve Discretization;
- physical sizing;
- shared-boundary certification;
- triangular or quadrilateral meshing;
- adaptive meshing;
- anisotropic/tensor metrics;
- Quad-Dominant behavior;
- parallel/GPU/SIMD equivalence;
- native-Windows qualification;
- arbitrary precision;
- any third-party runtime numerical dependency.

## 5. Frozen semantic baseline

The formal candidate must preserve the semantics represented by the following
files byte-for-byte from protocol-entry baseline
`744dbce1553a537dc22a0830d6ea878c0efb0fe7`, unless a separate protocol
amendment is integrated before formal preparation:

### Common surface and differential API

- `include/apmesh/geometry/parametric_surface.hpp`;
- `include/apmesh/geometry/surface.hpp`;
- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface.cpp`;
- `src/geometry/surface_differential.cpp`.

### Current admitted surface representations used as conformance evidence

- `include/apmesh/geometry/elementary_surface.hpp`;
- `include/apmesh/geometry/trimmed_surface.hpp`;
- `include/apmesh/geometry/extrusion_surface.hpp`;
- `include/apmesh/geometry/revolution_surface.hpp`;
- `include/apmesh/geometry/coons_surface.hpp`;
- `include/apmesh/geometry/nurbs_surface.hpp`;
- `src/geometry/elementary_surface.cpp`;
- `src/geometry/extrusion_surface.cpp`;
- `src/geometry/revolution_surface.cpp`;
- `src/geometry/coons_surface.cpp`;
- `src/geometry/nurbs_surface.cpp`;
- `src/geometry/rational_surface.cpp`.

### Current surface semantic contracts

- `tests/surface_bicubic_bezier.cpp`;
- `tests/surface_rational_bicubic_bezier.cpp`;
- `tests/surface_bicubic_nurbs.cpp`;
- `tests/surface_bicubic_nurbs_double_knot_continuity.cpp`;
- `tests/surface_coons_patch.cpp`;
- `tests/surface_rectangular_trim.cpp`;
- `tests/surface_linear_extrusion.cpp`;
- `tests/surface_revolution.cpp`;
- `tests/surface_plane.cpp`;
- `tests/surface_cylinder.cpp`;
- `tests/surface_sphere.cpp`;
- `tests/surface_metric_normal.cpp`;
- `tests/surface_metric_conditioning.cpp`;
- `tests/surface_second_order_curvature.cpp`;
- `tests/surface_principal_curvatures.cpp`.

Qualification tooling, evidence exporters, profiles, workflows, documentation
and qualification-only CMake registration may be added after protocol closure,
but they must not silently alter the frozen semantics.

The formal prepared package must hash-bind the complete tracked-source
inventory, not only the files above.

## 6. Exact ordinary semantic regression allowlist

Every formal repetition must discover and execute exactly the following current
42 ordinary semantic tests:

1. `apmesh_core.scaled_spline_arithmetic`;
2. `apmesh_core.bootstrap_smoke`;
3. `apmesh_core.numeric_contract`;
4. `apmesh_core.geometry_primitives`;
5. `apmesh_core.cartesian_frames`;
6. `apmesh_core.arbitrary_axis_placement`;
7. `apmesh_core.topological_model`;
8. `apmesh_core.curve_representation`;
9. `apmesh_core.parametric_curve_contract`;
10. `apmesh_core.line_segment`;
11. `apmesh_core.rational_quadratic_bezier`;
12. `apmesh_core.trimmed_curve`;
13. `apmesh_core.two_span_cubic_bspline`;
14. `apmesh_core.two_span_cubic_nurbs`;
15. `apmesh_core.multi_span_cubic_nurbs`;
16. `apmesh_core.cubic_nurbs_double_knot_continuity`;
17. `apmesh_core.surface_bicubic_bezier`;
18. `apmesh_core.surface_rational_bicubic_bezier`;
19. `apmesh_core.surface_bicubic_nurbs`;
20. `apmesh_core.surface_bicubic_nurbs_double_knot_continuity`;
21. `apmesh_core.surface_coons_patch`;
22. `apmesh_core.surface_rectangular_trim`;
23. `apmesh_core.surface_linear_extrusion`;
24. `apmesh_core.surface_revolution`;
25. `apmesh_core.surface_plane`;
26. `apmesh_core.surface_cylinder`;
27. `apmesh_core.surface_sphere`;
28. `apmesh_core.surface_metric_normal`;
29. `apmesh_core.surface_metric_conditioning`;
30. `apmesh_core.surface_second_order_curvature`;
31. `apmesh_core.surface_principal_curvatures`;
32. `apmesh_core.curve_differential`;
33. `apmesh_core.curve_curvature`;
34. `apmesh_core.curve_signed_curvature`;
35. `apmesh_core.curve_inflection_isolation`;
36. `apmesh_core.curve_arc_length`;
37. `apmesh_core.curve_cumulative_arc_length`;
38. `apmesh_core.curve_inverse_arc_length`;
39. `apmesh_core.curve_regularity`;
40. `apmesh_core.curve_header_isolation`;
41. `apmesh_core.minimal_small_linear_algebra`;
42. `apmesh_core.math_header_isolation`.

Zero selected tests, missing/duplicate tests, an extra ordinary semantic test or
a different allowlist is a formal gate failure unless a protocol amendment has
been integrated before preparation.

Executing this allowlist is prerequisite-preservation evidence. It does not
promote every unqualified represented capability to `QUALIFIED`.

## 7. Admitted formal environment

The formal matrix is exactly:

| Cell | Compiler | Standard library | Build |
| --- | --- | --- | --- |
| `gcc-debug` | GCC 13.3.0 | libstdc++ | Debug |
| `gcc-release` | GCC 13.3.0 | libstdc++ | Release |
| `clang-debug` | Clang 18.1.3 | libc++ 18.1.3 | Debug |
| `clang-release` | Clang 18.1.3 | libc++ 18.1.3 | Release |

All four cells execute in the already admitted GitHub-hosted Ubuntu 24.04
x86_64 cloud environment and reuse the accepted tool-path/package authorities.

A different runner image, package revision, compiler version, standard library,
architecture, WSL environment or native Windows is outside this protocol and
requires a separate decision or protocol amendment.

Release cells are mandatory because optimized floating-point/data-flow behavior
is material to the qualification claim even though ordinary PR integration uses
Debug cells.

## 8. Repetition and core command cardinality

Each formal cell executes exactly **two independent repetitions**.

Each cell is configured once.

Each repetition executes, in order:

1. build;
2. CTest discovery;
3. exact 42-test ordinary semantic regression;
4. Surface Differential Geometry scientific certificate production;
5. independent certificate validation.

After the two repetitions, each cell executes once:

- negative/adversarial evidence validation;
- compile/dependency inventory;
- runtime dependency inventory.

The fixed core record cardinality is therefore:

- 4 configure records;
- 8 build records;
- 8 discovery records;
- 8 semantic-CTest records;
- 8 scientific-certificate production records;
- 8 certificate-validation records;
- 4 negative-evidence records;
- 4 compile/dependency inventory records;
- 4 runtime-dependency inventory records;
- **56 core command records**.

Because each repetition executes 42 ordinary tests, a successful campaign
contains exactly:

**8 repetitions x 42 tests = 336 individual ordinary semantic test
executions.**

Cross-cell comparison, retained-package sealing, figure derivation/validation
and detached verification are required terminal actions but are not counted as
cell/repetition core records.

No automatic retry is permitted. A failed core command terminates the formal
attempt and preserves the partial evidence.

## 9. Surface Differential Geometry scientific certificate

Every repetition retains one machine-readable certificate. Exactly eight
scientific certificates are required.

Each certificate must include at least:

- schema version and certificate kind;
- candidate identity;
- stable case inventory;
- public result/error vocabulary;
- analytic/reference inputs;
- expected and observed result kind;
- exact typed error where applicable;
- first-form `E,F,G`;
- area density;
- unit-normal components;
- metric condition number;
- second-form `L,M,N`;
- Gaussian and mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state;
- explicit numeric acceptance policy for every rounded comparison;
- residual/reference scale/limit for every rounded comparison;
- transformation/invariance observations;
- explicit nonclaims from Section 4.

Compiler, cell, repetition, timestamps, paths and process identifiers are
provenance and are excluded from the scientific projection used for
same-cell determinism.

### 9.1 Signed-zero policy

Raw floating values are retained in hexadecimal representation.

For scientific projection equality, `+0` and `-0` are canonicalized to one
semantic zero for scalar and vector components unless an individual case
explicitly preregisters signed zero as a scientific claim. No current SDG claim
depends on signed-zero distinction.

The raw value remains available for audit. This policy must be implemented
identically by exporter and independent validator.

### 9.2 Same-cell determinism

The two validated scientific projections from one cell must be byte-identical
after the explicit signed-zero canonicalization above.

### 9.3 Cross-cell equivalence

Across the four cells:

- case inventory is identical;
- result/error classifications are identical;
- exact discrete fields and exact representable analytic fields match under the
  registered exact rule;
- every rounded field independently satisfies the same preregistered
  `ProximityPolicy` against its independent reference;
- invariance/covariance classifications agree;
- no cell adds or omits a scientific claim.

Rounded floating results are not required to be bit-identical across standard
libraries.

No global/default epsilon is permitted. Every rounded case must carry its
explicit Numeric Contract policy in the immutable qualification profile before
formal preparation.

## 10. Mathematical identities under qualification

The independent validator must recompute, where applicable:

`E = Su . Su`

`F = Su . Sv`

`G = Sv . Sv`

`J = ||Su x Sv||`

For a regular parameterization, `EG-F^2 = J^2 > 0`.

With oriented unit normal `n`:

`L = Suu . n`

`M = Suv . n`

`N = Svv . n`

`K = (L*N-M^2)/(E*G-F^2)`

`H = (E*N - 2*F*M + G*L)/(2*(E*G-F^2))`

For ordered principal curvatures `kmax >= kmin`:

`kmax + kmin = 2H`

`kmax * kmin = K`.

The condition number is the pointwise Jacobian 2-norm condition number:

`kappa = sigma_max / sigma_min >= 1`.

The validator must not simply call the production differential functions to
produce expected values.

## 11. Required independent analytic and adversarial fixtures

The qualification profile must enumerate all cases before formal preparation.

### 11.1 First-order metric/normal/conditioning

Required cases include:

- canonical plane with exact orthonormal derivatives;
- arbitrarily placed/translated plane;
- oblique synthetic fixture
  `Su=(2,0,0)`, `Sv=(1,3,0)`, requiring
  `E=4,F=2,G=10,J=6`;
- exact singular fixture with parallel nonzero tangents;
- near-singular regular fixture
  `Su=(1,0,0)`, `Sv=(1,2^-500,0)`;
- the corresponding exact representable condition number expected by the
  analytic singular-value relation;
- extreme finite orthogonal scale-separation fixture using powers of two;
- non-orthogonal conditioning fixture with independently derived eigenvalues;
- non-uniform parameter scaling that changes `kappa`;
- uniform spatial scaling that preserves `kappa`;
- parameter-axis swap and U/V reversal conditioning invariance.

### 11.2 Second-order curvature

Required derivative-level cases include:

- general oblique fixture
  `Su=(2,0,0)`, `Sv=(1,3,0)`,
  `Suu=(0,0,4)`, `Suv=(0,0,-2)`,
  `Svv=(0,0,6)`, independently requiring
  `L=4,M=-2,N=6,K=5/9,H=1`;
- hyperbolic-paraboloid local fixture with identity first form and
  `L=2,M=0,N=-2`, requiring `K=-4,H=0`;
- elliptic principal-value fixture with identity first form and principal
  values `3` and `1`;
- hyperbolic principal-value fixture with principal values `2` and `-4`;
- parabolic principal-value fixture with principal values `2` and `0`;
- exact nonzero umbilic fixture with both principal values `2`;
- near-umbilic fixture based on adjacent representable values, which must not
  be classified as exact umbilic;
- non-orthogonal generalized-eigenvalue fixture with independent closed-form
  reference;
- exact singular first-order data with second-order data present;
- first-order success combined with explicit
  `insufficient_continuity` second-order failure.

### 11.3 Analytic production surfaces

At minimum:

**Plane**

- regular value at multiple interior/boundary parameters;
- `K=0,H=0,kmax=0,kmin=0`;
- exact umbilic state for the zero shape operator;
- placement/reversal/translation evidence.

**Circular cylinder sector**

For declared radius `R` and current orientation convention:

- analytic first/second forms;
- `K=0`;
- one zero and one nonzero principal curvature of magnitude `1/R`;
- `H` and ordered signs consistent with the oriented normal;
- radius power-of-two scale law;
- U/V reversal orientation law.

**Spherical sector**

For declared radius `R`:

- regular equator fixture;
- at least one non-special regular latitude;
- `K=1/R^2`;
- both principal curvatures equal with magnitude `1/R` and sign fixed by the
  oriented normal;
- exact umbilic state at the canonical exact fixture;
- analytic condition relation from
  `E=R^2 cos^2(v),F=0,G=R^2`;
- exact pole parameterization remains a representation success but generic
  differential queries fail with `singular_parameterization`;
- near-pole representable regular points are not rejected by a hidden
  tolerance.

### 11.4 Current surface-family conformance

At least one admitted regular parameter must exercise the generic differential
overload on each current family:

- bicubic polynomial Bezier;
- positive-weight rational bicubic Bezier;
- bicubic NURBS;
- bicubic NURBS with double-knot continuity;
- Coons patch;
- rectangular trimmed surface;
- linear extrusion;
- revolution;
- plane;
- cylinder;
- sphere.

For non-analytic constructed/spline families, this is conformance/metamorphic
evidence, not an independent proof of the representation itself.

The double-knot fixture must include a C1 knot location where first-order
differential quantities remain admissible and second-order quantities propagate
`insufficient_continuity` exactly.

## 12. Required invariance and covariance evidence

The immutable profile must cover, where mathematically applicable:

- translation invariance;
- proper rigid-frame invariance/covariance;
- signed-frame/orientation behavior admitted by current placement contracts;
- U reversal;
- V reversal;
- both-direction reversal;
- U/V parameter-axis interchange on a synthetic fixture;
- uniform positive power-of-two spatial scaling.

Expected laws include:

- first fundamental form and area density are orientation independent;
- unit normal flips under exactly one parameter-direction reversal and is
  restored by both reversals;
- second fundamental form, mean curvature and both principal curvatures change
  sign when the oriented normal flips;
- Gaussian curvature is orientation invariant;
- metric condition number is orientation and uniform-spatial-scale invariant;
- under positive spatial scale `lambda`,
  `K -> K/lambda^2`,
  `H -> H/lambda`, and
  principal curvatures scale by `1/lambda`;
- exact umbilic state is preserved by admitted rigid/orientation/positive-scale
  transformations when representability is preserved.

No transformation may be accepted solely because two production calls agree;
the expected law is independently encoded.

## 13. Error and negative evidence

The certificate and negative artifact must exercise all relevant public errors:

- `non_finite_u_parameter`;
- `non_finite_v_parameter`;
- `u_parameter_out_of_domain`;
- `v_parameter_out_of_domain`;
- `insufficient_continuity`;
- `singular_parameterization`;
- `non_representable_result`.

The negative validator must also reject at least:

- missing, duplicate or extra scientific cases;
- forged candidate identity;
- forged typed error;
- forged exact-umbilic classification;
- principal-curvature ordering violation;
- inconsistent `K=kmax*kmin` or `2H=kmax+kmin`;
- invalid first-form determinant/area relation;
- condition number below one;
- forged orientation or scale covariance;
- undeclared tolerance/policy;
- non-finite successful scientific output;
- malformed signed-zero policy declaration;
- undeclared scientific claim;
- incomplete figure/source-data hash binding.

Negative evidence validates the evidence mechanism; it does not alter
production semantics.

## 14. Required reproducible scientific figures

The terminal package must derive deterministic figures from retained
machine-readable scientific data.

At minimum:

1. **Sphere curvature/conditioning latitude profile**
   - regular latitude samples;
   - analytic `K,H,k1,k2` reference;
   - analytic conditioning reference;
   - pole singularities marked as classified failures, not plotted finite
     substitutes.

2. **Cylinder radius/scale comparison**
   - at least three preregistered radii including power-of-two related cases;
   - analytic principal/mean/Gaussian references;
   - curvature scale law and conditioning relation.

3. **Near-singular conditioning progression**
   - preregistered power-of-two tangent-separation cases;
   - expected condition number versus observed value;
   - exact singular endpoint recorded separately as typed failure.

4. **Elliptic/hyperbolic/parabolic principal-curvature comparison**
   - retained synthetic analytic cases;
   - ordered principal values plus `K/H` identity residuals.

Figures are derived evidence, not acceptance oracles.

The authoritative source data must be CSV/JSON with stable schema and hashes.
SVG generation must be deterministic and must not require manual scientific
editing.

## 15. SDG0-SDG7 qualification gates

| Gate | PASS condition |
| --- | --- |
| **SDG0 — Scope, candidate and frozen semantics** | Clean published candidate, exact upstream identity, complete source hashes, protocol/profile/tooling identities bound, frozen semantic files unchanged, excluded production capabilities absent. |
| **SDG1 — Domain, continuity, singularity and finite-failure semantics** | All preregistered parameter/domain/continuity/exact-singular/non-representable cases produce the exact expected result/error; near-singular regular cases are not threshold-rejected. |
| **SDG2 — First-order metric, area, normal and conditioning** | Independent plane/oblique/analytic fixtures and invariance laws validate `E,F,G,J,n,kappa`, including scale-aware extreme finite cases and `kappa>=1`. |
| **SDG3 — Second fundamental form, Gaussian and mean curvature** | Independent derivative-level and analytic-surface fixtures validate `L,M,N,K,H`, orientation covariance and spatial scale laws. |
| **SDG4 — Principal values and exact umbilic semantics** | Ordered values satisfy independent generalized-eigenvalue references plus `K/H` identities; exact umbilics are detected exactly and near-umbilics remain distinct. |
| **SDG5 — Representation conformance, invariance and continuity boundaries** | Every admitted current surface family satisfies the generic differential seam at declared regular points; trim/reversal/frame/scale laws and NURBS continuity failures are preserved without qualifying Surface Representation. |
| **SDG6 — Repeatability, cross-cell equivalence and prerequisite preservation** | Two repetitions per cell agree scientifically; all four cells satisfy the same exact/proximity rules; exact 42-test allowlist passes in every repetition with no prerequisite contradiction. |
| **SDG7 — Evidence integrity, figures and bounded closure** | Manifests, command history, eight certificates, comparisons, negatives, inventories, source data, deterministic figures, limitations, retention seal and detached verification are complete and independently auditable; no excluded claim appears. |

Overall PASS requires **SDG0 through SDG7** to pass together.

No gate may be inferred from another gate, a workflow conclusion, or an
aggregate test count.

## 16. Failure, stop and retention policy

The formal attempt is `BLOCKED` if:

- any core command fails;
- a required artifact/case is absent or duplicated;
- repetitions disagree;
- an unexplained cross-cell difference remains;
- the exact 42-test allowlist changes or fails;
- a prerequisite contradiction appears;
- evidence requires changing expected results after observation;
- a hidden clamp/retry/fallback/tolerance is needed;
- a non-finite success is emitted;
- a frozen semantic file differs without a prior protocol amendment;
- an excluded capability is needed to make a gate pass.

The first scientific failure opens exactly one bounded diagnosis. Partial/failed
evidence is retained immutably. Production code, expectations and the active
formal candidate are not corrected inside the same formal attempt.

A prerequisite contradiction reopens that prerequisite and suspends
qualification.

## 17. Required retained terminal package

The terminal package must contain or hash-link at least:

- prepared manifest;
- terminal manifest;
- exact candidate and tracked-source inventory;
- protocol/profile/tool hashes;
- state history;
- core command records and stdout/stderr logs;
- exact CTest discovery records;
- eight scientific certificates;
- eight certificate-validation records;
- same-cell and cross-cell comparisons;
- per-cell negative evidence;
- compile/dependency inventories;
- runtime dependency inventories;
- gate summary in JSON and Markdown;
- retained limitation/nonclaim record;
- figure-source CSV/JSON;
- deterministic SVG figures;
- artifact/source hash index;
- retention manifest;
- detached-candidate verification.

Rebuildable build trees, caches and binaries are excluded unless a later
retention decision explicitly admits them.

## 18. Formal campaign separation

The required sequence after this protocol is:

1. integrate and separately close this protocol;
2. design and implement **report-only** qualification tooling;
3. validate/integrate/close that tooling;
4. independently audit tooling against this protocol;
5. decide and implement fail-closed preparation infrastructure;
6. validate/integrate/close preparation infrastructure;
7. prepare one immutable candidate only when explicitly authorized;
8. audit the PREPARED package;
9. issue one exact formal-execution authorization separately;
10. execute once;
11. retain terminal evidence without automatic retry;
12. independently audit SDG0-SDG7;
13. only a later audit/decision may record `QUALIFIED` or `BLOCKED`.

Protocol integration, tooling success and preparation success cannot change the
stage status.

## 19. Stop conditions for protocol/tooling work

Stop and require a new scientific decision or protocol amendment if fulfilling
this protocol needs:

- principal directions or line-field semantics;
- a new surface representation;
- cone/torus support;
- p-curves/topological faces;
- a universal/default epsilon;
- an acceptable-conditioning threshold;
- third derivatives;
- arbitrary precision;
- a third-party runtime numerical package;
- boundary discretization, sizing or meshing;
- changed public differential error semantics;
- changed frozen surface semantics;
- a different formal environment or execution matrix.

## 20. Work-class plan

This protocol-pre-registration work item:

- Implementation: 0%;
- Tests/validation: 15%;
- Evidence/experiments design: 35%;
- Documentation/governance: 50%.

These percentages are process accounting, not scientific completion scores.

## 21. Effect if integrated and separately closed

Protocol integration and closure may establish only:

**SURFACE DIFFERENTIAL GEOMETRY QUALIFICATION PROTOCOL PRE-REGISTERED /
REPORT-ONLY TOOLING MAY BE CONSIDERED / NOT QUALIFIED.**

After protocol closure, the sole next admissible work item is one bounded
decision/implementation unit for **report-only Surface Differential Geometry
qualification tooling** that conforms exactly to this protocol.

No preparation, formal execution, workflow dispatch or qualification status is
authorized by protocol closure.


## 22. Protocol integration checkpoint

Protocol PR #233 used final head:

`9bd09926734d0f2fce2abdd096380e4478c8c38d`.

Required PR validation:

- FAST #626: PASS;
- INTEGRATION #617 / GCC 13 Debug: PASS;
- INTEGRATION #617 / Clang 18/libc++ Debug: PASS;
- no reviews or unresolved review threads;
- branch relation at merge gate: ahead=10, behind=0;
- diff restricted to six documentation/audit/reference files;
- exact 42-test protocol allowlist matched the ordinary CMake registration
  exactly and all 32 frozen semantic paths existed.

PR #233 squash-merged as:

`d34b8d5620f6166b6e8570225bbf243fef3c90f7`.

Protected-main validation on that exact protocol revision:

- FAST #627: PASS;
- INTEGRATION #618 / GCC 13 Debug: PASS;
- INTEGRATION #618 / Clang 18/libc++ Debug: PASS.

Ordinary semantic registration inventory remains **42 tests**.

Protocol integration result:

**SURFACE DIFFERENTIAL GEOMETRY QUALIFICATION PROTOCOL PRE-REGISTERED /
INTEGRATED / CLOSURE PENDING / TOOLING NOT YET AUTHORIZED /
FORMAL EXECUTION NOT AUTHORIZED / NOT QUALIFIED.**

The active closure branch is:

`docs/surface-differential-geometry-qualification-protocol-closure`.

This closure is documentation-only. It cannot implement report-only tooling,
prepare a manifest, dispatch a workflow, authorize a formal attempt or set the
stage to `QUALIFIED`.

Only after this separate closure itself receives required checks, merges and
passes protected-main validation may a fresh bounded decision/work item consider
report-only qualification tooling.
