# Curve Representation — Bounded Entry Decision

Status: ALL ADMITTED CONTINUOUS-CURVE WORK UNITS IMPLEMENTED / FOCUSED
CONTRACTS PASS / CGR0–CGR7 PROTOCOL UNDER REVIEW / STAGE UNQUALIFIED
Date: 2026-09-20
Stage: Curve Representation — Continuous Geometry Before Discretization
Prerequisites:
- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the exact admitted GitHub-hosted Ubuntu
  24.04 x86_64 cloud envelope by TMR0–TMR7 PASS.

## Question

What is the smallest continuous-curve capability that can be implemented after
qualified geometry and topology while remaining independent of topology
ownership, differential geometry, integration, discretization, meshing, and
parallel execution?

## Decision

Authorize one bounded implementation work unit named:

**Polynomial Cubic Bézier Value Representation and Evaluation.**

The work unit may introduce immutable two- and three-dimensional polynomial
cubic Bézier value types, exact access to their four control points, evaluation
on the closed normalized parameter interval `[0,1]`, and geometric reversal.

Evaluation must use recursive de Casteljau interpolation directly in the
Bernstein/Bézier representation. It must not convert control points to
power-basis polynomial coefficients.

The implementation must use the qualified `Point2`/`Point3` semantics and
preserve the existing Numeric Contract. For scalar interpolation inside
de Casteljau, `std::lerp` is the preferred C++23 primitive because the C++
library contract guarantees exact endpoint return for `t==0` and `t==1`
and a finite result for finite endpoints with `t` in `[0,1]`.

This decision fixes the scientific semantics and focused evidence obligations.
It does not itself implement curve code and does not authorize stage-level
qualification.

## Sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| Gerald Farin, *Curves and Surfaces for CAGD*, 5th ed., chapters on de Casteljau and Bernstein Bézier curves | Recursive linear-interpolation evaluation and standard polynomial Bézier representation are core CAGD constructions. | That AP Mesh must reproduce a particular library API or extend immediately to splines/rational curves. | Use de Casteljau as the first evaluator and keep the first work unit polynomial cubic only. |
| CGAL `Arr_Bezier_curve_traits_2` documentation | A Bézier curve is defined from ordered control points, is evaluated as a parametric curve, and uses normalized parameter `t∈[0,1]`; first and last control points are the endpoints. | That CGAL is a dependency, numerical oracle, or required storage design. | Fix control-point order, endpoint semantics, and normalized parameter domain without admitting CGAL. |
| Open CASCADE `Geom_BezierCurve` / `Geom2d_BezierCurve` documentation | Mature CAD kernels separate 2D/3D Bézier geometry, use control points, and use parameter range `[0,1]`; rational curves are a distinct extension. | That rational weights, arbitrary degree, CAD ownership, or OCCT APIs belong in the first AP Mesh work unit. | Implement 2D/3D polynomial cubic geometry only and defer rational/arbitrary-degree support. |
| Farouki and Rajan, *On the numerical condition of polynomials in Bernstein form*, CAGD 4(3), 1987 | Bernstein-form computation has favorable numerical properties and geometric-processing motivation. | That every Bernstein-basis algorithm is automatically stable for every floating-point task. | Avoid an unnecessary conversion to power basis and require adversarial numeric fixtures. |
| ISO C++ `std::lerp` contract | For finite interpolation endpoints, `t=0` returns the first endpoint, `t=1` returns the second, and `t∈[0,1]` produces a finite result. | That a complete curve evaluator is thereby scientifically qualified. | Use component-wise `std::lerp` as the primitive for each de Casteljau interpolation step. |

External implementations are semantic and verification references only. No new
third-party runtime dependency is admitted.

## Mathematical representation

For ordered control points `P0, P1, P2, P3`, define the polynomial cubic
Bézier curve on `t∈[0,1]` by

`B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t)t^2 P2 + t^3 P3`.

The production evaluator must not evaluate this expanded Bernstein expression
through direct power-basis coefficient conversion. Instead, it must use three
levels of de Casteljau interpolation:

1. `Q0 = lerp(P0,P1,t)`, `Q1 = lerp(P1,P2,t)`,
   `Q2 = lerp(P2,P3,t)`;
2. `R0 = lerp(Q0,Q1,t)`, `R1 = lerp(Q1,Q2,t)`;
3. `B(t) = lerp(R0,R1,t)`.

The formula above is the mathematical definition and an independent reference
for selected analytic fixtures; the recursive construction is the production
evaluation mechanism.

## Authorized scope

The first implementation work unit may add only:

- immutable `CubicBezier2` and `CubicBezier3` value types;
- exactly four ordered `Point2` or `Point3` control points;
- deterministic read-only indexed/named control-point access;
- normalized parameter domain exactly `[0,1]`;
- explicit rejection of non-finite parameter values;
- explicit rejection of finite parameters outside `[0,1]`;
- evaluation by component-wise de Casteljau interpolation using
  `std::lerp`;
- geometric reversal by reversing the control-point order;
- exact endpoint semantics:
  `evaluate(0)==P0` and `evaluate(1)==P3`;
- focused analytic, adversarial, deterministic, header/dependency, and
  prerequisite-preservation tests.

The control points themselves require no second finite-value validation layer:
the already qualified `Point2` and `Point3` construction boundary excludes
NaN and infinities.

The exact C++ spelling and file split remain implementation choices. A dedicated
geometry-level curve header/source is preferred over expanding
`core/geometry.hpp` indefinitely, but this decision does not require a new
linker target.

## Error semantics

Evaluation is a fallible operation only because the caller supplies a scalar
parameter.

At minimum, the API must distinguish:

- `non_finite_parameter`;
- `parameter_out_of_domain`.

A conforming implementation must not clamp an invalid parameter, silently
extrapolate, retry with another algorithm, or substitute an endpoint.

If the implementation includes an explicit defensive check for an impossible
non-finite de Casteljau result, that condition must be reported as a distinct
internal/numeric failure rather than mapped to either parameter error.

No exception-driven normal control flow and no global tolerance are authorized.

## Required invariants and hypotheses

### Ordered control points define curve state

Two curves with identical control points in identical order are exactly equal as
value representations. Reordering control points changes the represented
parameterized curve unless the particular fixture is algebraically degenerate.

### Endpoint interpolation

For every valid curve:

- `B(0) == P0`;
- `B(1) == P3`.

These are exact value equalities, not proximity claims.

### Finite in-domain evaluation

Finite valid control points and finite `t∈[0,1]` produce a finite evaluated
point. No valid in-domain fixture may escape as NaN or infinity.

### Reversal

If `reverse(C)` has control points `P3,P2,P1,P0`, then geometrically

`reverse(C)(t) = C(1-t)`.

For exactly representable fixtures, tests should require exact equality where
the declared arithmetic permits it. General floating-point fixtures must use
the qualified explicit `ProximityPolicy` and retain residual/limit evidence;
a hard-coded epsilon is forbidden.

Double reversal must recover the original curve value exactly.

### Degenerate control polygons remain valid continuous curves

Repeated control points, all-equal control points, zero-length endpoint chord,
and other polynomial degeneracies are valid representations at this layer.

This work unit does not classify regularity, tangent existence, curvature,
self-intersection, closure, simplicity, or suitability for meshing.

### Translation and admitted-frame covariance

For selected exactly representable fixtures, translating every control point
and then evaluating must agree with translating the evaluated point.

Likewise, the already qualified Cartesian signed-permutation/power-of-two frame
mapping may be used in tests to verify the admitted covariance relation without
introducing a new general transform abstraction.

### Determinism and isolation

The same ordered control points and parameter produce the same scientific result
within the declared compiler/library envelope. Evaluation depends on no
filesystem, clock, locale, environment variable, topology identity, random
state, thread schedule, or unordered iteration.

## Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| 2D endpoint pair | `t=0` returns `P0` exactly and `t=1` returns `P3` exactly. |
| 3D endpoint pair | Same exact endpoint requirement in 3D. |
| Constant curve | Four identical control points evaluate to that point across representative parameters. |
| Collinear cubic | Selected control points on one axis agree with an independently computed scalar Bernstein reference. |
| Planar 2D fixture | Interior values at `t=1/4,1/2,3/4` agree with independent high-precision or exactly representable expectations. |
| Spatial 3D fixture | Independent component-wise reference agrees under an explicit policy where rounding is unavoidable. |
| Reversal endpoints | Reversed control order swaps exact endpoints. |
| Reversal involution | `reverse(reverse(C)) == C` exactly. |
| Reversal evaluation | `reverse(C)(t)` and `C(1-t)` agree under declared exact/proximity rules. |
| Translation covariance | Translating control points commutes with evaluation for selected fixtures. |
| Admitted frame covariance | Qualified frame mapping commutes with evaluation for selected safe power-of-two fixtures. |
| Repeated controls | Consecutive duplicate control points remain valid and deterministic. |
| All-equal controls | No false degeneracy error is emitted. |
| Extreme finite controls | Safe combinations near large/small finite magnitudes remain finite for in-domain parameters, exercising `std::lerp` rather than naïve `a+t(b-a)`. |
| Non-finite parameter | NaN and ±infinity are explicitly rejected. |
| Below-domain parameter | A finite `t<0` is rejected; no extrapolation. |
| Above-domain parameter | A finite `t>1` is rejected; no extrapolation. |
| Signed zero parameter | `+0.0` and `-0.0` both resolve to the exact first endpoint and do not create different curve semantics. |
| Repeatability | Repeated evaluations reproduce equivalent scientific fields. |
| Header/dependency isolation | The curve header introduces no topology, model, boundary, meshing, I/O, threading, or third-party dependency. |

Expected values must be generated independently from the production evaluator.
The legacy AP Mesh implementation is not an acceptance oracle.

## Focused validation boundary

The work unit must provide one focused CTest contract covering both 2D and 3D
cubic value/evaluation semantics.

Before integration, focused validation must pass at minimum in:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

Release cells are not required for this ordinary component work unit unless a
specific optimization-sensitive numerical difference is observed.

The focused contract must also preserve the exact currently accepted semantic
prerequisite tests for:

- Numeric Contract;
- Geometry Primitives;
- Minimal Small Linear Algebra / header isolation where dependency reachability
  applies;
- Cartesian Frames;
- Topological Model.

Passing focused evidence permits only
`IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED`.

## Explicit exclusions

This decision does not authorize:

- derivatives, tangent vectors, speed, regularity, singularity detection, or
  differential geometry;
- curvature, torsion, Frenet frames, feature classification, or inflection
  detection;
- arc length, numerical integration, inverse arc-length mapping, or
  reparameterization;
- public subdivision/splitting, degree elevation/reduction, interpolation, or
  fitting;
- rational Bézier curves, weights, arbitrary degree, B-splines, NURBS, conics,
  composite splines, or CAD-kernel imports;
- line/segment/plane/intersection predicates, closest-point projection, root
  finding, or self-intersection classification;
- bounding boxes or geometric acceleration structures;
- `CurveId`, curve ownership, `EdgeId ↔ Curve` association, trimming,
  patches, surfaces, model integration, or serialization;
- adaptive sampling, boundary discretization, sizing, metric evaluation,
  quadrilateral construction, mesh generation, optimization, or adaptation;
- SIMD, GPU, OpenMP, MPI, task systems, or any parallel execution path;
- third-party runtime dependencies;
- native-Windows qualification;
- a formal Curve Representation qualification campaign.

In particular, quadrilateral meshing remains a later serial scientific stage
and parallelism remains later still; neither may be pulled into continuous curve
representation.

## Admission and stop conditions

Implementation may proceed only if:

1. the representation remains pure continuous geometry with no topological
   ownership;
2. parameter semantics remain exactly the closed normalized interval `[0,1]`;
3. de Casteljau evaluation is retained in Bernstein form without power-basis
   conversion;
4. parameter failures are explicit and no clamping/extrapolation fallback
   exists;
5. the full focused case table is represented by tests;
6. approximate assertions use explicit `ProximityPolicy` evidence rather than
   a global epsilon;
7. no new runtime dependency or threading mechanism is introduced; and
8. all relevant qualified prerequisite semantics remain passing.

Stop and require a new bounded decision if implementation requires derivatives,
integration, a rational representation, arbitrary-degree polymorphism,
topological curve identity/ownership, an undeclared numerical acceptance rule,
or a dependency outside the standard library.

## Alternatives considered

- **Start with arbitrary-degree Bézier curves:** deferred because the roadmap's
  first downstream needs can be exercised with cubic polynomial curves and a
  fixed four-point value type has a smaller error/state space.
- **Start with NURBS/rational curves:** deferred because weights, knot vectors,
  rational homogeneous evaluation, conic exactness, and CAD import materially
  expand the scientific problem.
- **Use power-basis coefficients internally:** rejected for the first evaluator;
  it adds basis conversion with no demonstrated downstream need and weakens the
  direct link between control-point semantics and de Casteljau evidence.
- **Use topology-owned curves immediately:** rejected because the architecture
  explicitly separates a topological edge from its continuous curve geometry.
- **Implement derivatives together with values:** deferred so value
  representation/evaluation can close as an independently testable first
  work unit.
- **Introduce parallel evaluation now:** rejected; deterministic serial
  reference behavior must be established before parallel equivalence can even
  be defined.

## Effect on the roadmap if integrated

Curve Representation moves from `NOT STARTED` to
`IN INVESTIGATION / ENTRY DECISION APPROVED / IMPLEMENTATION NOT STARTED`.

Topological Model remains qualified and is not reopened by this
documentation-only entry decision.

The next bounded work item after this decision is integrated, validated, and
closed is implementation of **Polynomial Cubic Bézier Value Representation and
Evaluation** only.

No derivative, arc-length, discretization, quadrilateral, or parallel work is
authorized until the appropriate later bounded decision.

## Integration checkpoint

PR #46 integrated this entry decision as
`c76e2946c8c9ffec658e4c8aa146f1abdca62f33`.

Validation:

- PR FAST `35536256853`: PASS;
- PR INTEGRATION `35536256854`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35536324878`: PASS;
- post-merge INTEGRATION `35536324875`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug.

The entry-decision checkpoint is closed. The sole next bounded work item is
implementation of **Polynomial Cubic Bézier Value Representation and
Evaluation** within the scope fixed above.

## First work-unit implementation result

Branch `curve/cubic-bezier-value-evaluation` implements only the authorized
Polynomial Cubic Bézier Value Representation and Evaluation work unit:

- `CubicBezier2` and `CubicBezier3` immutable value representations;
- exactly four ordered qualified control points;
- evaluation only for finite `t∈[0,1]`;
- recursive component-wise de Casteljau evaluation using `std::lerp`;
- exact endpoint semantics;
- geometric reversal by reversing control-point order;
- explicit `CurveError::non_finite_parameter`,
  `parameter_out_of_domain`, and defensive `non_finite_result`;
- no derivative, regularity, curvature, integration, topology ownership,
  discretization, meshing or parallel capability.

Focused evidence is implemented by:

- `apmesh_core.curve_representation`;
- `apmesh_core.curve_header_isolation`.

PR validation for branch head `2da0d55b1d0404c108449fb15ae8b0f528a6c3f0`:

- FAST run `35541613233`: PASS, GCC 13 Debug, 9/9 selected tests;
- INTEGRATION run `35541613216`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 9/9 selected tests in each cell.

The selected nine tests include both new curve contracts plus the required
Numeric, Geometry Primitives, Cartesian Frames, Topological Model, Minimal
Small Linear Algebra and Math Header Isolation prerequisite tests.

Scientific status after this focused evidence is exactly:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

No stage-level Curve Representation qualification is claimed by this component
work unit.

## First work-unit integration checkpoint

PR #48 integrated Polynomial Cubic Bézier Value Representation and Evaluation
as `bde874311d9960c5fab7ce03b26b6cd5fbd61b34`.

Validation:

- PR FAST `35541914952`: PASS;
- PR INTEGRATION `35541914958`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35541963486`: PASS;
- post-merge INTEGRATION `35541963489`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug.

The first work unit is closed at **IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT
QUALIFIED**. The next bounded transition is a separate scientific decision for
Curve Derivatives and Regularity. No derivative implementation is authorized by
this checkpoint.

That separate decision is now being specified in
`docs/decisions/CURVE_DERIVATIVES_REGULARITY_DECISION.md`. It deliberately
separates first/second derivative and pointwise-speed evaluation from the
stronger interval-wide global-regularity certification problem.


## Stage implementation completion and cumulative-regression transition

Subsequent bounded decisions and work units completed the admitted
Curve Representation scope without reopening the original representation
semantics:

- Cubic Bézier Differential Evaluation and Pointwise Speed;
- Certified Global Cubic Regularity;
- Certified Cubic Bézier Total Arc-Length Enclosure;
- Certified Cumulative Arc-Length Enclosure;
- Certified Inverse Arc-Length Bracketing.

The final admitted continuous-curve work unit, Certified Inverse Arc-Length
Bracketing, was integrated by PR #70 as
`a82fa1c96fc6665e586753d5e4eb698012a79be3` and closed by the subsequent
checkpoint on `main` at
`438620efa1f93d29b442e9ba199882a09d2359d9`.

Post-closure FAST `35594467964` and INTEGRATION `35594467983` passed.

No component work unit individually qualifies Curve Representation.

The stage-exit authority is now pre-registered in:

`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`.

That protocol freezes the current semantic baseline and requires a separate
report-only tooling phase, formal preparation, one-shot authorization,
execution, retained evidence and independent CGR0–CGR7 terminal audit.

Until CGR0–CGR7 all PASS and the qualification decision is integrated:

**Curve Representation remains STAGE UNQUALIFIED.**

No Boundary Curve Discretization, surface, Quad-Dominant or parallel work is
opened by this transition.
