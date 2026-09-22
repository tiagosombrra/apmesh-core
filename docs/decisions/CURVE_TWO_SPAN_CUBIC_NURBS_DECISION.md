# Two-Span Clamped Cubic Positive-Weight NURBS — Bounded Decision

Status: DECISION APPROVED / IMPLEMENTATION NOT STARTED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Curve Representation Breadth Gate

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- polynomial cubic Bézier Curve Representation baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- bounded parametric curve contract: integrated;
- directed line segment 2D/3D: integrated focused work unit;
- positive-weight rational quadratic Bézier 2D/3D:
  integrated focused work unit;
- oriented trimmed parametric subcurve 2D/3D:
  integrated focused work unit;
- fixed two-span clamped cubic polynomial B-spline 2D/3D:
  integrated focused work unit and terminally closed.

## 1. Question

Which remaining representation-breadth concept should be introduced next after
the project has independently demonstrated:

- knot-free rational-weight semantics; and
- fixed cubic B-spline knot/local-support semantics?

The required comparison includes:

1. general bounded clamped B-spline expansion beyond the fixed two-span family;
2. NURBS;
3. arbitrary-degree polynomial/rational Bézier;
4. analytic conic after arbitrary 3D supporting-plane/orientation semantics;
5. heterogeneous composition/polycurve.

The next work unit should add one new semantic interaction only and preserve all
current qualified/integrated baselines.

## 2. Fresh repository evidence

Canonical decision-entry `main`:
`6e549d4f4b989108f8faacad326a207ee88238e7`.

Terminal B-spline closure evidence:

- implementation PR #118:
  `c336460b751fa600c893aa6a96f9d594cdcd9a9e`;
- implementation closure PR #119:
  `5f9c6b2c324d5c2519114784dd3705277dd9b06e`;
- closure post-merge FAST `35737514686`: PASS;
- closure post-merge INTEGRATION `35737514493`: PASS;
- terminal sync PR #120:
  `6e549d4f4b989108f8faacad326a207ee88238e7`;
- terminal sync FAST `35740316433`: PASS;
- terminal sync INTEGRATION `35740316620`: PASS.

Production at entry includes:

- `CubicBezier2/3`;
- `LineSegment2/3`;
- `RationalQuadraticBezier2/3`;
- static `TrimmedCurve2/3`;
- `TwoSpanCubicBSpline2/3`.

Absent at entry:

- any NURBS production class;
- any general/multi-span/dynamic B-spline container;
- arbitrary-degree Bézier;
- dedicated analytic conics;
- heterogeneous composition/polycurve;
- production surfaces.

## 3. Literature and mature-kernel evidence

### NURBS basis as weighted B-spline basis

Michigan Technological University reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS-property.html

Relevant evidence:

- NURBS basis functions inherit local support from B-spline basis functions;
- on one degree-`p` span, at most `p+1` rational basis functions are
  nonzero;
- continuity at a knot of multiplicity `m` follows the underlying B-spline
  continuity `C^(p-m)` for positive finite weights;
- when all weights are equal nonzero constants, the rational basis reduces to
  the ordinary B-spline basis.

Decision impact:

- all-one-weight parity with the already integrated two-span B-spline is a
  mandatory oracle;
- one simple cubic interior knot retains the existing C2 representation scope;
- no new continuity policy is needed in the first NURBS work unit.

### NURBS weight semantics

Michigan Technological University reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS-mod-weight.html

Relevant evidence:

- a NURBS curve is defined by control points, degree, knot vector and weights;
- changing a control weight changes its rational influence on the curve;
- all-one weights reproduce the corresponding B-spline.

Decision impact:

- positive finite weights are the only new stored scientific field relative to
  the fixed polynomial B-spline;
- common positive weight scaling must preserve the represented curve and
  derivatives.

### B-spline/NURBS data model in mature CAD kernels

Open CASCADE references:

- https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html
- https://dev.opencascade.org/doc/refman/html/class_step_geom___b_spline_curve_with_knots_and_rational_b_spline_curve.html

Relevant evidence:

- mature CAD models combine degree, control points, knots/multiplicities and
  weights for rational B-spline curves;
- rational/non-rational and periodic/non-periodic are distinct semantics;
- the same knot/multiplicity structure supports both polynomial B-spline and
  rational B-spline/NURBS representations.

Decision impact:

- AP Mesh should compose its already isolated positive-weight rational layer
  with its already isolated two-span cubic knot layer before generalizing
  degree/count/periodicity;
- Open CASCADE is reference evidence only, not a dependency.

### Rational Bézier is a knot-free NURBS special case

Michigan Technological University reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB.html

Relevant evidence:

- rational Bézier curves are a special NURBS case with endpoint-only clamped
  knots;
- rational Bézier degree elevation can be performed in homogeneous
  coordinates;
- B-spline knot insertion preserves the represented curve.

Decision impact:

- the integrated `RationalQuadraticBezier2/3` family can supply a strong
  cross-family oracle:
  first elevate degree two to degree three in homogeneous coordinates, then
  insert one simple knot;
- the resulting five-control two-span cubic NURBS must reproduce the original
  rational quadratic physical curve.

### Existing project evidence

Internal authorities:

- `docs/decisions/CURVE_RATIONAL_QUADRATIC_BEZIER_DECISION.md`;
- `docs/decisions/CURVE_TWO_SPAN_CUBIC_BSPLINE_DECISION.md`.

Decision impact:

- positive finite weight validation, rational denominator handling, common
  weight-scale invariance and conic residual evidence already exist in the
  rational-quadratic work unit;
- degree-three de Boor evaluation, two spans, simple interior knot, local
  support, C2 continuity, D1/D2 and reversal already exist in the polynomial
  two-span B-spline work unit;
- the next work unit should test the interaction of those two foundations,
  not broaden both simultaneously.

External sources are design/reference evidence only. No external runtime
dependency is admitted.

## 4. Candidate comparison

| Candidate | New semantics introduced now | New engineering burden | Reuses current foundations | Downstream value | Decision |
| --- | --- | --- | --- | --- | --- |
| Fixed two-span cubic positive-weight NURBS | Interaction of B-spline knots/local support with rational weights | **Bounded**: same degree/count/spans as current B-spline; five weights added | **Both** existing B-spline and rational work | Very high bridge to NURBS surfaces/CAD | **SELECTED** |
| General bounded clamped polynomial B-spline | Variable span/control/knot count | Container/resource/capacity and arbitrary-span semantics | Existing B-spline | High | DEFER |
| Arbitrary-degree polynomial/rational Bézier | Variable degree/control count | Degree/storage generalization | Bézier/rational work | Moderate | DEFER |
| Analytic conic | Native analytic CAD family | Arbitrary 3D supporting-plane/orientation and periodic/angular semantics | Rational conic already covers physical conic arcs | High | BLOCKED/DEFER |
| Heterogeneous composition/polycurve | Runtime family chain | Storage/dispatch/ownership/join semantics | Trim already available | High | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Two-Span Clamped Cubic Positive-Weight NURBS Representation in 2D and 3D.**

The family is deliberately fixed:

- degree exactly three;
- exactly five finite control points;
- exactly five finite strictly positive weights;
- full knot vector exactly
  `[a,a,a,a,k,b,b,b,b]`;
- finite strict `a < k < b`;
- exactly two nonzero knot spans;
- one simple interior knot;
- non-periodic;
- bounded;
- D1 and D2 required;
- no dynamic degree, control count, knot count or weight count.

This combines previously isolated knot/local-support and rational-weight
semantics while avoiding a simultaneous general-container expansion.

## 6. Mathematical representation

Let control points be

`P0,P1,P2,P3,P4`

and strictly positive weights be

`w0,w1,w2,w3,w4`.

Let finite knot values satisfy

`a < k < b`.

The degree-three knot vector is

`U=[a,a,a,a,k,b,b,b,b]`.

Let `N_i,3(u)` be the corresponding polynomial B-spline basis functions.

Define

`W(u) = sum_i N_i,3(u) w_i`.

Because all admitted weights are strictly positive and the active basis forms a
partition of unity,

`W(u) > 0`

throughout the admitted domain.

The NURBS curve is

`C(u) = [sum_i N_i,3(u) w_i P_i] / W(u)`

for `u in [a,b]`.

## 7. Continuity scope

The single interior knot has multiplicity one.

Degree is three.

Therefore the fixed representation remains C2 at the interior knot.

Strictly positive finite weights do not introduce a denominator zero in the
admitted domain.

The first NURBS work unit does not admit:

- zero or negative weights;
- repeated interior knots;
- C1/C0/discontinuous joins;
- periodic knots;
- degree other than three;
- more than two spans.

## 8. Construction semantics

The proposed public family may be named:

- `TwoSpanCubicNURBS2`;
- `TwoSpanCubicNURBS3`.

Construction is validated.

At minimum reject:

- non-finite `a`, `k`, or `b`;
- non-strict knot order;
- any NaN/infinite weight;
- any weight `<= 0`.

No epsilon defines positivity or knot order.

Control-point finiteness remains guaranteed by existing `Point2/Point3`
construction.

The work unit may introduce one bounded NURBS construction-error vocabulary.

## 9. Stored representation

The implementation stores exactly:

- five controls;
- five original input weights;
- `a`, `k`, and `b`.

Stored weights are not canonicalized.

A common positive scale factor applied to every weight must leave
value/D1/D2 unchanged within the declared numeric contract.

The implementation may scale weights internally during evaluation to avoid
avoidable overflow/underflow, but public stored values must remain the exact
validated inputs.

## 10. Parameter domain

`parameter_domain()` returns exactly `[a,b]`.

Required failures:

- NaN/infinity query:
  `CurveError::non_finite_parameter`;
- finite query below `a` or above `b`:
  `CurveError::parameter_out_of_domain`.

No clamping or periodic wrapping is permitted.

Required endpoint values:

- `C(a)=P0`;
- `C(b)=P4`.

Endpoint values should be returned exactly.

## 11. Production evaluation strategy

The preferred production strategy is fixed-degree homogeneous de Boor
evaluation.

For each control:

`H_i = (w_i P_i, w_i)`

in homogeneous coordinates.

Production may normalize all weights by a positive common scale before building
homogeneous controls, provided this does not alter the represented curve.

Apply the same fixed two-span cubic de Boor knot structure to homogeneous
controls.

After evaluation:

- denominator/homogeneous weight must be finite and strictly positive;
- spatial components are divided by that weight;
- final public point must be representable as finite `double`;
- otherwise return `CurveError::non_finite_result`.

The implementation must not evaluate a point by separately running one
polynomial B-spline per coordinate and then a differently parameterized
denominator algorithm.

## 12. D1 and D2 semantics

D1 and D2 must be analytic, not finite-difference approximations.

The implementation may:

1. derive/evaluate homogeneous B-spline derivatives and dehomogenize through
   quotient identities; or
2. use an algebraically equivalent rational-basis derivative method.

Let homogeneous spatial numerator be `A(u)` and homogeneous denominator be
`W(u)`.

Then

`C=A/W`.

Required first derivative:

`C' = (A' - C W') / W`.

Required second derivative:

`C'' = (A'' - 2 C' W' - C W'') / W`.

Production must use an overflow-aware implementation of these identities.

An unrepresentable final D1/D2 returns
`CurveError::non_finite_result`.

No derivative is normalized, clamped or replaced with zero after failure.

## 13. All-one/equal-weight polynomial parity

If all five weights are the same positive finite constant, the NURBS must
represent exactly the corresponding
`TwoSpanCubicBSpline2/3` physical curve.

Focused evidence must compare at multiple values on both spans and at the
interior knot:

- value;
- D1;
- D2;
- reversal.

Power-of-two common weight rescaling is required as an exact/metamorphic
fixture where arithmetic permits exact identity.

This is the primary prerequisite-family parity oracle.

## 14. Rational quadratic cross-family parity

The existing `RationalQuadraticBezier2/3` family supplies an independent
rational curve oracle.

Construct homogeneous rational quadratic controls:

`H0,H1,H2`.

Perform degree elevation from degree two to degree three in homogeneous
coordinates:

- `G0=H0`;
- `G1=(1/3)H0 + (2/3)H1`;
- `G2=(2/3)H1 + (1/3)H2`;
- `G3=H2`.

Choose a non-endpoint knot `k` and normalized insertion parameter

`t=(k-a)/(b-a)`.

Insert one knot in homogeneous coordinates:

- `J0=G0`;
- `J1=lerp(G0,G1,t)`;
- `J2=lerp(G1,G2,t)`;
- `J3=lerp(G2,G3,t)`;
- `J4=G3`.

Dehomogenize each `J_i` into a control point and positive weight.

The resulting two-span cubic NURBS must represent the same physical rational
quadratic curve after parameter mapping.

Required parity:

- value;
- D1 with the `du` parameter-scale factor;
- D2 with the squared parameter-scale factor.

At least one parity fixture should use the existing rational quadratic
quarter-circle construction or another conic fixture.

This gives a second cross-family oracle independent of all-one weights.

## 15. Independent rational-basis oracle

Focused tests must additionally compute an independent reference through
Cox-de Boor polynomial basis functions followed by rational normalization:

`R_i(u)=N_i,3(u) w_i / W(u)`.

The reference must not reuse production homogeneous de Boor helpers.

It must cover:

- asymmetric controls;
- nonuniform positive weights;
- a non-midpoint interior knot;
- both spans;
- the interior knot;
- value;
- D1;
- D2.

This is required because equal-weight and rational-quadratic parity fixtures
cover only special subclasses.

## 16. Local support

The rational curve retains the support of its underlying B-spline basis.

On the first span, `P4,w4` must have no effect on a query strictly inside the
left span.

On the second span, `P0,w0` must have no effect on a query strictly inside the
right span.

Focused evidence must perturb both the out-of-support control and its weight,
then verify unchanged value/D1/D2 in the opposite span.

## 17. Weight-scale invariance

For any positive common scale `lambda` such that all stored scaled weights
remain finite:

`(w0,...,w4) -> (lambda w0,...,lambda w4)`

must leave the represented value, D1 and D2 invariant.

Focused evidence must include:

- at least one exact power-of-two scale;
- one strongly unbalanced but finite positive weight set;
- one common scale that would make naïve homogeneous intermediates vulnerable
  to overflow/underflow but whose normalized computation remains representable.

Stored-weight identity is not invariant: the public stored weights remain the
actual inputs.

## 18. Reversal

Reversal preserves physical locus and domain `[a,b]`.

Required reversed representation:

- controls:
  `P4,P3,P2,P1,P0`;
- weights:
  `w4,w3,w2,w1,w0`;
- lower/upper knots remain `a,b`;
- reversed interior knot:
  `k_r = reversed_parameter([a,b],k)`.

For `r(u)` from the common reversal primitive:

- `reverse(C)(r(u)) = C(u)`;
- `reverse(C)'(r(u)) = -C'(u)`;
- `reverse(C)''(r(u)) = C''(u)`.

Double reversal must recover the exact stored representation.

No naïve `a+b-k` is permitted when the common primitive avoids intermediate
overflow.

## 19. Degenerate/constant semantics

A curve with five identical control points and arbitrary admitted positive
weights is representable.

Required behavior:

- every valid query returns the exact common point where the implementation can
  preserve exact identity;
- D1 and D2 are exact zero;
- reversal preserves value representation except for the intentional weight
  order reversal.

No regularity certification is introduced.

## 20. Affine/embedding evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two uniform coordinate scaling where representable;
- 2D/3D embedding parity for `z=0`;
- deterministic repeated success/failure evidence.

No arbitrary-angle frame qualification is implied.

## 21. Extreme finite numeric evidence

At least one fixture must combine:

- extreme finite coordinate magnitudes;
- highly unbalanced but positive finite weights;
- extreme finite knot scale.

The required outcome is:

- succeed when the final point/D1/D2 is representable and an equivalent
  scale-aware computation exists;
- otherwise fail explicitly with
  `CurveError::non_finite_result`.

No hidden coordinate renormalization changes the scientific output.

No universal epsilon is introduced.

## 22. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | 2D/3D NURBS types satisfy existing bounded-parametric concepts. |
| Knot validation | Non-finite or non-strict `a<k<b` rejected. |
| Weight validation | NaN/infinite/zero/negative weights rejected. |
| Exact domain | Domain exactly `[a,b]`. |
| Endpoint values | Exact P0/P4. |
| Independent rational basis | Value/D1/D2 on both spans and interior knot. |
| Equal-weight B-spline parity | Value/D1/D2/reversal match existing two-span B-spline. |
| Rational quadratic parity | Homogeneous degree-elevation + knot-insertion parity, including one conic fixture. |
| Local support | Out-of-support control+weight perturbation leaves opposite-span query unchanged. |
| Common weight scale | Value/D1/D2 invariant under positive common scaling. |
| Reversal | Controls/weights/knot reversal, involution and value/D1/D2 covariance. |
| Constant curve | Exact value and zero D1/D2. |
| Query failures | Existing typed non-finite/out-of-domain failures. |
| Translation/scale | Declared affine covariance. |
| 2D/3D embedding | x/y parity and zero z. |
| Extreme finite | No avoidable overflow; explicit failure when final result is unrepresentable. |
| Determinism | Repeated fields/failures identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |
| Prerequisite preservation | Existing 22 ordinary tests remain passing plus one NURBS contract. |

If exactly one new focused test is added, ordinary FAST/INTEGRATION inventory
becomes **23 tests**.

## 23. Why fixed NURBS before general B-spline expansion

General B-spline expansion would introduce at least:

- variable control count;
- variable knot count;
- runtime or compile-time capacity policy;
- more than two spans;
- generalized span search;
- resource/allocation or bounded-capacity semantics.

Those are independent from the question now ready to be answered:

**Can AP Mesh correctly combine its already integrated rational-weight layer
with its already integrated knot/local-support layer?**

The fixed NURBS work unit answers that question without simultaneously changing
container/resource semantics.

## 24. Why not arbitrary-degree Bézier next

Arbitrary-degree Bézier primarily generalizes:

- degree;
- control count;
- degree-dependent evaluation/derivative storage.

It does not exercise the newly integrated knot/local-support layer.

The fixed NURBS work unit composes two already isolated foundations and is a
more direct prerequisite for future NURBS surfaces.

## 25. Why not analytic conic next

Rational quadratic Bézier already provides exact conic-segment capability in
2D/3D control geometry.

A dedicated analytic 3D conic still requires supporting-plane/orientation
semantics beyond the qualified signed-permutation Cartesian frames.

This prerequisite remains explicit and must not be bypassed.

## 26. Why not heterogeneous composition next

Heterogeneous composition still requires a runtime representation decision:

- type erasure;
- closed variant;
- virtual base;
- another bounded dispatch mechanism.

The integrated trim layer is static and deliberately does not choose among
those alternatives.

NURBS does not require that unresolved runtime-storage decision.

## 27. Explicit exclusions

This decision does not authorize:

- more than two spans;
- more or fewer than five controls/weights;
- degree other than three;
- repeated interior knots;
- periodic NURBS;
- zero/negative weights;
- general B-spline/NURBS containers;
- arbitrary-degree polynomial/rational Bézier;
- knot insertion/removal as production mutation;
- degree elevation/reduction as production mutation;
- analytic conic classes;
- heterogeneous composition;
- surface NURBS;
- interpolation/fitting;
- generic regularity/length/curvature refactors;
- boundary discretization;
- sizing/meshing;
- Quad-Dominant work;
- parallel execution;
- third-party runtime dependencies;
- a representation-breadth qualification campaign.

## 28. Repository mapping for future implementation

Authorized future mapping is limited to:

- public family:
  `include/apmesh/geometry/nurbs.hpp`;
- production implementation:
  `src/geometry/nurbs.cpp`;
- focused semantic/header contract:
  `tests/two_span_cubic_nurbs.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

Existing B-spline and rational-Bézier production semantics should be reused as
scientific references but should not be modified unless a strictly mechanical
test-access need is demonstrated.

No change to `parametric_curve.hpp` is expected.

If common concept semantics must change, stop and require a new decision.

## 29. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 22 ordinary semantic tests must remain passing;
- the new NURBS focused contract must pass.

Passing yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify general NURBS, arbitrary-span B-spline, spline surfaces or
the representation-breadth stage.

## 30. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricCurve2/3`;
- variable degree/count/container semantics;
- repeated interior knots;
- periodicity;
- non-positive weights;
- a second new representation family;
- topology/surface/meshing code;
- a universal tolerance;
- a third-party dependency.

## 31. Planned sequence after this work unit

Planning only, not authorization.

After fixed two-span cubic NURBS implementation/closure, a fresh decision should
compare:

1. general bounded clamped cubic B-spline/NURBS span-count expansion;
2. arbitrary-degree polynomial/rational Bézier;
3. repeated-knot/continuity breadth;
4. analytic conic after orientation prerequisites;
5. heterogeneous composition/polycurve.

The next winner must be selected from fresh evidence.

## 32. Effect if integrated and closed

After decision integration, post-merge validation and decision checkpoint
closure, the sole next implementation work item is:

**Two-Span Clamped Cubic Positive-Weight NURBS Representation in 2D and 3D.**

No implementation begins on this decision branch.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization and Surface Representation remain blocked until
their prerequisite representation-breadth decisions are explicitly closed.


## 33. Decision integration checkpoint

PR #121 integrated this bounded decision as
`bd7a50144535ee0a9b9774b1c4e7d7490aca5a85`.

Final decision-head validation:

- FAST `35740954735`: PASS;
- INTEGRATION `35740954945`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35741064912`: PASS;
- INTEGRATION `35741064906`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Two-Span Clamped Cubic Positive-Weight NURBS Representation in 2D and 3D.**

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the implementation bounded by Sections 5–30.

General/multi-span B-spline or NURBS, variable degree/count, repeated knots,
periodicity, arbitrary-degree Bézier, analytic conics, heterogeneous
composition, surfaces and downstream meshing remain unauthorized.


## 35. Active implementation mapping

Terminal decision closure synchronization PR #123 merged as
`01a4f7f9e88b7df0ebddbec7e3c745893b86512b`.

Sync validation:

- PR FAST `35743947099`: PASS;
- PR INTEGRATION `35743946835`: PASS;
- post-merge FAST `35744071581`: PASS;
- post-merge INTEGRATION `35744071575`: PASS.

The sole authorized implementation is active on:

`curve/two-span-cubic-nurbs`.

Candidate repository mapping:

- public value family:
  `include/apmesh/geometry/nurbs.hpp`;
- production implementation:
  `src/geometry/nurbs.cpp`;
- focused semantic/reference contract:
  `tests/two_span_cubic_nurbs.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

Candidate production semantics:

- validated five-control/five-positive-weight storage;
- exact fixed knot topology and parameter domain;
- common positive internal weight scaling before homogeneous control creation;
- fixed cubic homogeneous de Boor value evaluation;
- first/second homogeneous derivative control polygons;
- analytic rational D1/D2 dehomogenization;
- exact endpoint values;
- exact constant-curve value and zero derivatives;
- reflected interior-knot reversal through the common reversal primitive;
- unchanged bounded-parametric concepts.

The focused contract includes every Section 22 evidence category, including
the independent rational-basis oracle and the rational-quadratic
degree-elevation/knot-insertion conic parity fixture.

Expected ordinary FAST/INTEGRATION inventory after registration: **23 tests**.

Candidate validation:

- candidate head:
  `a096b00438f8acf08adce58327439e888037818f`;
- FAST `35745044617`: PASS, 23/23 tests;
- INTEGRATION `35745044483`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 23/23 tests in each cell;
- `apmesh_core.two_span_cubic_nurbs`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS / FINAL DOCUMENTATION-SYNC
REVALIDATION PENDING / NOT QUALIFIED.**

No broader NURBS/B-spline or downstream capability is implied.
