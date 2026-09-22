# Second Concrete Curve Family — Positive-Weight Rational Quadratic Bézier Decision

Status: DECISION APPROVED / IMPLEMENTATION NOT STARTED / FAMILY NOT QUALIFIED  
Date: 2026-09-22  
Stage: Curve Representation Breadth Gate

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- polynomial cubic Bézier Curve Representation baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- Bounded Parametric Curve Contract:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Bounded Directed Line Segment Representation:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.

## 1. Question

Which concrete curve family should follow bounded directed line segments?

Candidates required by the previous closure are:

1. bounded circular/general-conic arc;
2. rational and/or arbitrary-degree Bézier;
3. B-spline;
4. NURBS;
5. composite/trimmed boundary curves where ordering requires them.

The choice must advance the declared future `line/arc/Bezier` boundary
regression and the eventual CAD/surface-trimming envelope while keeping the
next work unit bounded, independently verifiable and compatible with the
existing parametric-curve concept.

## 2. Fresh repository evidence

Canonical `main` at decision entry:
`b435ddbbf93f94741014b26d081dbf5bdbb7c9e6`.

Closure evidence for the prior family:

- PR #106 merged as
  `b435ddbbf93f94741014b26d081dbf5bdbb7c9e6`;
- closure PR FAST `35722026674`: PASS;
- closure PR INTEGRATION `35722026746`: PASS;
- closure post-merge FAST `35722144861`: PASS;
- closure post-merge INTEGRATION `35722144806`: PASS.

Production curve families at entry:

- `CubicBezier2`, `CubicBezier3`;
- `LineSegment2`, `LineSegment3`.

Absent at entry:

- analytic circle/conic arc types;
- rational Bézier;
- arbitrary-degree Bézier;
- B-spline;
- NURBS;
- composite/trimmed curves;
- production surfaces.

## 3. Internal architectural constraint discovered by regression

The qualified `CartesianFrame2`/`CartesianFrame3` capability is not a
general arbitrary-orientation frame.

Its qualification authority explicitly admits only:

- exact signed-permutation bases; and
- reciprocal-safe power-of-two uniform scales.

It explicitly excludes arbitrary-angle rotations and approximate frames.

Consequently, implementing a general analytic `CircularArc3` by reusing the
qualified frame as if it represented an arbitrary plane orientation would be a
scope violation.

A separate arbitrary-plane/frame decision is possible later, but it is not a
prerequisite for representing planar conic segments in arbitrary 2D/3D
positions using rational Bézier control geometry.

Authority:
`docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`.

## 4. External literature and mature-kernel evidence

### Rational Bézier as a bounded NURBS special case

Michigan Technological University spline notes:

- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB-conics.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB-circles.html

Relevant evidence:

- rational Bézier curves arise as a special case of NURBS without internal
  knots;
- rational quadratic Bézier curves represent conic sections;
- suitable weights represent circular arcs, including the standard
  quarter-circle construction;
- weights affect the rational basis and therefore introduce precisely the
  denominator/projective semantics needed before NURBS.

These sources are mathematical/reference evidence only and are not runtime
dependencies.

### The NURBS Book

Les Piegl and Wayne Tiller. *The NURBS Book*, 2nd ed.

Reference copy already registered:
https://home.zcu.cz/~bastl/GM1/the-nurbs-book.pdf

Decision impact:

- rational Bézier is structurally closer to future NURBS than a dedicated
  analytic circle class because both use weighted rational basis functions;
- fixed-degree rational Bézier avoids knot vectors, multiplicity, local support
  and periodic B-spline machinery in the next work unit.

### Open CASCADE circle/conic and trimmed-curve semantics

References:

- https://dev.opencascade.org/doc/refman/html/class_geom___conic.html
- https://dev.opencascade.org/doc/refman/html/class_geom2d___circle.html
- https://dev.opencascade.org/doc/refman/html/class_geom___trimmed_curve.html

Relevant evidence:

- mature CAD kernels keep analytic conics as explicit families;
- a circle has angular periodic semantics;
- trimming is a separate operation over a basis curve and introduces
  orientation/parameter-bound behavior;
- therefore a dedicated analytic arc plus trimming is scientifically valuable,
  but it is not the smallest next family in the current AP Mesh repository.

No Open CASCADE inheritance, tolerance, ownership or dependency is admitted.

## 5. Candidate comparison

| Candidate | Immediate value | New semantic burden | 2D/3D without new arbitrary frame | Bridge to NURBS | Decision |
| --- | --- | --- | --- | --- | --- |
| Dedicated bounded circle/conic arc | Exact analytic arc family and direct CAD semantics | Angular periodic basis, arc span, arbitrary 3D supporting-plane orientation, later trimming relation | **No**, not with currently qualified frame semantics | Low/moderate | DEFER |
| Positive-weight rational quadratic Bézier | Conic-segment capability, exact real-arithmetic conic representation, weighted rational semantics | Three weights, denominator, rational D1/D2 | **Yes** | **High** | **SELECTED** |
| Arbitrary-degree polynomial Bézier | Broader polynomial free-form family | Dynamic degree/storage, but no conic exactness | Yes | Moderate | DEFER |
| B-spline | Major free-form CAD family | Knots, multiplicities, local support, continuity partitions, possible periodicity | Yes | High | DEFER |
| NURBS | Broad CAD curve envelope | B-spline complexity plus weights/rational denominator | Yes | Final target | DEFER |
| Composite/trimmed curves | Required eventually for topology/boundaries | Ownership/composition, parameter remapping, continuity and basis-curve semantics | Depends on basis family | Orthogonal | DEFER |

## 6. Decision

Select exactly one future implementation work unit:

**Positive-Weight Rational Quadratic Bézier Representation in 2D and 3D.**

The family is fixed degree two.

It must:

- use three finite control points;
- use three finite strictly positive weights;
- use exact parameter domain `[0,1]`;
- satisfy `BoundedParametricCurve2` and `BoundedParametricCurve3`;
- implement point evaluation plus first and second parameter derivatives;
- support exact representation reversal by reversing control-point/weight
  order;
- preserve typed query failures;
- add no knots, periodicity or trimming semantics.

No other family is authorized by this decision.

## 7. Why fixed degree two

Degree two is selected because it is the smallest rational Bézier family that
can represent general conic segments.

This gives the project a bounded way to introduce:

- positive weights;
- rational basis normalization;
- denominator reasoning;
- projective/common-weight-scale invariance;
- conic/circular-arc fixtures;
- 2D/3D weighted curve semantics.

It avoids simultaneously introducing:

- arbitrary-degree storage;
- dynamic derivative-control structures;
- knot vectors;
- continuity partitions;
- periodic spline semantics;
- composite curve ownership.

## 8. Mathematical representation

For control points `P0,P1,P2`, strictly positive weights `w0,w1,w2` and
`t ∈ [0,1]`, define the quadratic Bernstein basis

`B0(t)=(1-t)^2`

`B1(t)=2t(1-t)`

`B2(t)=t^2`.

Define

`A(t)=w0 B0 P0 + w1 B1 P1 + w2 B2 P2`

and

`W(t)=w0 B0 + w1 B1 + w2 B2`.

Then

`C(t)=A(t)/W(t)`.

Because the weights are strictly positive and the Bernstein basis is
nonnegative with partition of unity on `[0,1]`, the real-arithmetic
denominator satisfies

`W(t) > 0`

for every valid parameter.

The floating implementation must nevertheless avoid avoidable overflow,
underflow-induced false singularity and non-finite output.

## 9. Construction semantics

The proposed family should be created through validated factories, conceptually:

- `RationalQuadraticBezier2::make(P0,P1,P2,w0,w1,w2)`;
- `RationalQuadraticBezier3::make(P0,P1,P2,w0,w1,w2)`.

Construction must reject at minimum:

- non-finite weight;
- zero weight;
- negative weight.

A dedicated construction error vocabulary is authorized, for example:

- `non_finite_weight`;
- `non_positive_weight`.

Control-point finiteness remains guaranteed by the existing `Point2/Point3`
constructors.

No epsilon defines positivity.

## 10. Parameter and endpoint semantics

The parameter domain is exactly `[0,1]`.

Required exact endpoint relations:

- `C(0)=P0`;
- `C(1)=P2`.

Non-finite parameters use the existing
`CurveError::non_finite_parameter`.

Finite parameters outside `[0,1]` use
`CurveError::parameter_out_of_domain`.

No clamp or periodic wrapping is permitted.

## 11. Evaluation strategy

A rational de Casteljau evaluation is preferred because it can avoid directly
forming large weighted coordinates.

For a pair of weighted points `(Pa,wa)`, `(Pb,wb)`, one admissible
interpolation step computes

`w=(1-t)wa+t wb`

and the corresponding point as a finite interpolation between `Pa` and
`Pb` using the normalized positive blend coefficient.

The second de Casteljau level repeats that operation.

An algebraically equivalent strategy is allowed only if it preserves the same
finite/error guarantees.

The implementation must not blindly form `wi * Pi` when an equivalent
positive-weight computation can avoid an intermediate overflow.

## 12. First and second derivative contract

Let derivatives with respect to `t` be denoted by primes.

The standard quotient relations are

`C' = (A' - C W') / W`

and

`C'' = (A'' - 2 C' W' - C W'') / W`.

The implementation may use an algebraically equivalent rational Bézier
derivative algorithm.

Required semantics:

- finite representable derivatives succeed;
- an unrepresentable nonzero derivative returns
  `CurveError::non_finite_result`;
- no derivative is silently clamped or normalized;
- zero derivative is a successful value for a degenerate/constant rational
  curve;
- this work unit does not introduce regularity certification.

If a robust bounded implementation cannot avoid systematic false failure for
finite representable derivatives without introducing new numerical policy,
stop and require a new decision.

## 13. Reversal

Reversal is exact representation reversal:

- control points become `P2,P1,P0`;
- weights become `w2,w1,w0`;
- domain remains `[0,1]`.

For `r(t)=1-t`:

- `reverse(C)(r(t)) = C(t)`;
- `reverse(C)'(r(t)) = -C'(t)`;
- `reverse(C)''(r(t)) = C''(t)`.

Double reversal must recover the exact stored representation.

## 14. Common-weight-scale invariance

For every positive finite common factor `λ` whose scaled weights remain
finite and positive,

`(w0,w1,w2)`

and

`(λw0, λw1, λw2)`

represent the same geometric curve and derivatives.

Focused tests must use at least moderate exact power-of-two factors so the
invariance test does not depend on decimal-rounding accidents.

The implementation must not normalize stored weights silently unless a later
representation-canonicalization decision authorizes it.

## 15. Polynomial parity when weights are equal

If

`w0=w1=w2>0`,

the rational curve reduces to an ordinary quadratic polynomial Bézier curve.

The repository does not currently expose a `QuadraticBezier` type, so the
independent regression oracle will use exact degree elevation into the already
qualified cubic-Bézier family.

For quadratic controls `P0,P1,P2`, the degree-elevated cubic controls are

- `Q0=P0`;
- `Q1=(1/3)P0+(2/3)P1`;
- `Q2=(2/3)P1+(1/3)P2`;
- `Q3=P2`.

For selected representable fixtures, rational-equal-weight value, D1 and D2
must agree with the existing cubic-Bézier result under the project's declared
numeric comparison policy.

This is required to avoid validating the rational implementation solely against
itself.

## 16. Conic and circular-arc evidence

At minimum the focused contract must include:

1. one independently derived non-circular conic fixture;
2. the standard quarter-circle rational quadratic construction;
3. an implicit geometric residual check independent of production evaluation
   formulas.

For the quarter-circle fixture, the mathematically exact middle weight is
`sqrt(2)/2`.

Because this number is not exactly representable in binary `double`, the
floating implementation must not claim bit-exact membership on the real unit
circle.

Instead, the test must:

- construct the stored double weight explicitly;
- compute expected/residual values independently;
- use an explicit local `ProximityPolicy` tied to fixture scale;
- record that the residual validates the stored finite representation, not an
  impossible exact-real constant.

No universal epsilon is introduced.

## 17. Degenerate geometry

Coincident or collinear control points are allowed as value representations.

Examples:

- all control points equal: constant rational curve;
- collinear controls: rationally parameterized line locus.

Construction must not reject such geometry solely because later regularity or
feature algorithms may classify it as singular.

This continues the separation between representation and regularity already
used by cubic Bézier and line segments.

## 18. Focused evidence required

A dedicated test must cover at minimum:

| Case | Required observation |
| --- | --- |
| Concept satisfaction | 2D/3D rational quadratic types satisfy the common bounded parametric concepts. |
| Weight validation | NaN, infinities, zero and negative weights are rejected explicitly. |
| Exact domain/endpoints | Domain is exactly `[0,1]`; endpoint values are exact. |
| Interior value | Independent rational quadratic reference fixture. |
| Equal-weight polynomial parity | Value/D1/D2 agree with independently degree-elevated `CubicBezier2/3`. |
| D1/D2 | Analytic reference values at endpoints and interior. |
| Reversal | Representation reversal, involution and value/D1/D2 covariance. |
| Weight-scale invariance | Common exact power-of-two weight scaling preserves value/D1/D2. |
| Constant curve | Equal control points remain a successful value representation. |
| Conic fixture | Independent conic residual. |
| Quarter-circle fixture | Scale-aware residual consistent with stored double middle weight. |
| Parameter failures | Existing typed non-finite/out-of-domain failures. |
| Extreme finite controls/weights | No avoidable value-evaluation overflow; unrepresentable derivatives fail explicitly. |
| Translation | Values translate; derivatives remain invariant. |
| 2D/3D embedding | Embedded planar fixture preserves x/y outputs and zero z. |
| Determinism | Repeated results/failures are identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |
| Prerequisite preservation | Existing ordinary semantic inventory remains passing plus one focused rational-quadratic test. |

If exactly one focused test is added, the expected ordinary inventory increases
from 19 to 20 tests.

## 19. Explicit exclusions

This decision does not authorize:

- dedicated analytic circle or conic classes;
- arbitrary 3D plane/frame semantics;
- arbitrary-degree polynomial Bézier;
- arbitrary-degree rational Bézier;
- zero or negative rational weights;
- B-spline;
- NURBS;
- knot vectors;
- periodicity;
- composite/piecewise curves;
- trimming;
- curve intersections;
- projection/closest point;
- rational-curve regularity certification;
- arc length/inverse length for this new family;
- curvature APIs for this new family;
- generic refactoring of existing cubic algorithms;
- boundary discretization;
- surfaces;
- sizing or meshing;
- Quad-Dominant work;
- parallel execution;
- third-party runtime dependencies;
- a representation-breadth qualification campaign.

## 20. Alternatives

### Dedicated circular/conic arc next

Deferred.

It is scientifically valuable, but a general 3D analytic arc requires an
arbitrary supporting-plane orientation semantics that the currently qualified
Cartesian frame explicitly does not provide.

Using the current qualified frame as a general rotation frame would be invalid.

### Arbitrary-degree polynomial Bézier next

Deferred.

It broadens polynomial degree but cannot provide the rational conic semantics
needed for exact-real conic representation and future NURBS.

### B-spline next

Deferred.

Knots, multiplicity, continuity partitions, local support and periodicity are
too much semantic surface before weighted rational evaluation has been
isolated.

### NURBS next

Deferred.

It combines both B-spline and rational complexity. A fixed rational quadratic
family isolates the rational layer first.

### Composite/trimmed curves next

Deferred.

Composition is more useful after at least one curved non-polynomial family
exists and its own parameter/reversal semantics are understood.

### Positive-weight rational quadratic Bézier

Selected.

It is a bounded 2D/3D family, exercises the common concept, adds rational
denominator semantics, supplies conic/circular-arc geometry without requiring a
new arbitrary 3D frame, and forms a direct conceptual bridge toward NURBS.

## 21. Repository mapping for future implementation

Authorized mapping is limited to:

- public family:
  `include/apmesh/geometry/rational_bezier.hpp`;
- production implementation:
  `src/geometry/rational_bezier.cpp`;
- focused semantic/header contract:
  `tests/rational_quadratic_bezier.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No change to `parametric_curve.hpp` is expected.

If common concept semantics must change, stop and require a new decision.

## 22. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- the line-segment contract must remain passing;
- all existing cubic-Bézier contracts must remain passing;
- all prior ordinary prerequisite tests must remain passing.

Passing yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify a general rational/NURBS representation stage.

## 23. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricCurve2/3`;
- zero/negative weights;
- arbitrary-degree containers;
- knot vectors;
- periodicity;
- trimming/composition;
- a universal tolerance;
- an arbitrary 3D frame/plane primitive;
- a new generic regularity/length/curvature subsystem;
- a second concrete family;
- surface or meshing code;
- a new third-party dependency.

## 24. Planned sequence after this family

Planning only, not authorization:

1. bounded directed line segment — integrated;
2. positive-weight rational quadratic Bézier — selected here;
3. new decision comparing:
   - analytic circle/general conic representation,
   - arbitrary-degree polynomial/rational Bézier,
   - B-spline,
   - NURBS,
   - composition/trimming;
4. continue one bounded family at a time.

The sequence after item 2 may change only through a fresh literature-backed
decision.

## 25. Effect if integrated and closed

After decision integration, post-merge validation and checkpoint closure, the
sole next implementation work item is:

**Positive-Weight Rational Quadratic Bézier Representation in 2D and 3D.**

No production implementation begins on this decision branch.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization remains blocked until its declared exact
line/arc/Bezier regression envelope is scientifically admitted.

Surface Representation remains blocked pending its later explicit family and
trimming decision.


## 26. Decision integration checkpoint

PR #107 integrated this bounded decision as
`4ae5a81cec0c3f6512f81a47b7a4d1a6f97fd6ad`.

Final decision-head validation:

- FAST `35722805362`: PASS;
- INTEGRATION `35722805446`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35722894744`: PASS;
- INTEGRATION `35722894725`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Positive-Weight Rational Quadratic Bézier Representation in 2D and 3D.**

After the separate decision closure is integrated and its post-merge
validation passes, that bounded implementation becomes the sole authorized
production work item.

No analytic circle/conic class, arbitrary-degree Bézier, B-spline, NURBS,
composition/trimming, surface, discretization, sizing, meshing, Quad-Dominant
or parallel implementation is authorized by this checkpoint.


Decision closure authority: PR #108.

No production implementation is present in this closure change. After PR #108
is integrated and its post-merge FAST/INTEGRATION pass, the implementation
scope in Sections 6–23 becomes the sole authorized work item.


## 27. Active implementation mapping

Decision closure PR #108 merged as
`30f32997dec0aa7937c9730eb5ce24e2f80bb964`.

Closure post-merge validation:

- FAST `35723209087`: PASS;
- INTEGRATION `35723209143`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The sole authorized implementation is active on:

`curve/rational-quadratic-bezier`.

Candidate repository mapping:

- `include/apmesh/geometry/rational_bezier.hpp`;
- `src/geometry/rational_bezier.cpp`;
- `tests/rational_quadratic_bezier.cpp`;
- `CMakeLists.txt`;
- synchronized `STATE`, `ROADMAP`, `WORKLOG` and this decision.

Candidate semantics:

- three finite control points and three finite strictly positive stored weights;
- construction rejects non-finite and non-positive weights without epsilon;
- exact `[0,1]` parameter domain;
- exact endpoint value return;
- positive-weight scaling for internal rational computation without
  canonicalizing stored weights;
- point evaluation, D1 and D2 with final representability checks;
- exact representation reversal;
- constant-curve zero-derivative handling;
- unchanged common bounded-parametric concepts.

Focused evidence covers all Section 18 obligations with independent cubic
degree-elevation and conic residual fixtures.

Expected ordinary FAST/INTEGRATION inventory after registration: **20 tests**.

Candidate validation:

- candidate head:
  `a68134daae06207f1ec32cf7df1f613a7e8cb693`;
- FAST `35726985299`: PASS, 20/20 tests;
- INTEGRATION `35726985356`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 20/20 tests in each cell;
- `apmesh_core.rational_quadratic_bezier`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS / FINAL DOCUMENTATION-SYNC
REVALIDATION PENDING / NOT QUALIFIED.**

No other family or downstream capability is implied.


## 28. Implementation integration checkpoint

PR #109 integrated the authorized work unit as
`6600875dfbb33d1a37603e32bcf452625373c462`.

Validation lineage:

- first complete candidate head:
  `a68134daae06207f1ec32cf7df1f613a7e8cb693`;
- candidate FAST `35726985299`: PASS, 20/20 tests;
- candidate INTEGRATION `35726985356`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug, 20/20 tests per cell;
- final documentation-synchronized head:
  `0c2f0f72a9c7ae9d5c400a0685d72df3303d7273`;
- final PR FAST `35727147220`: PASS;
- final PR INTEGRATION `35727147303`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug;
- post-merge FAST `35727296946`: PASS;
- post-merge INTEGRATION `35727296931`: PASS.

Integrated production scope:

- `RationalQuadraticBezier2`;
- `RationalQuadraticBezier3`;
- positive finite validated weights;
- exact `[0,1]` bounded-domain semantics;
- point evaluation, D1 and D2;
- representation reversal;
- common-weight-scale invariance evidence;
- polynomial degree-elevation parity;
- non-circular conic and quarter-circle residual evidence;
- degenerate/constant and extreme-finite evidence.

The common bounded-parametric concepts were not changed.

Work-unit result:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

This result does not qualify arbitrary rational Bézier, analytic conics,
B-spline, NURBS, trimming, surfaces or downstream meshing.

After implementation closure integration/post-merge validation, a fresh
literature-backed decision is mandatory before another concrete family or
representation semantic step.


## 29. Implementation closure checkpoint

Implementation closure PR #110 merged as
`93b082ce660fd8d2c012b96ef7319b240de6d9d2`.

Closure validation:

- PR FAST `35727517602`: PASS;
- PR INTEGRATION `35727517607`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35727653016`: PASS;
- post-merge INTEGRATION `35727652961`: PASS.

The work unit is terminally:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
NOT QUALIFIED.**

This closure does not widen the original cubic-Bézier CGR0–CGR7 qualification.

No analytic conic, arbitrary-degree Bézier, B-spline, NURBS,
composition/trimming, surface, discretization or meshing capability is
authorized by this closure.

The sole next admissible scientific action after terminal documentation sync is
a fresh literature-backed bounded comparison decision for the next
representation-breadth step.
