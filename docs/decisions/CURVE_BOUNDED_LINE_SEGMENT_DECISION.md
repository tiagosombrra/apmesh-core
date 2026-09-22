# First Concrete Curve Family — Bounded Directed Line Segment Decision

Status: DECISION APPROVED / IMPLEMENTATION NOT STARTED / FAMILY NOT QUALIFIED  
Date: 2026-09-22  
Stage: Curve Representation Breadth Gate

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation — Polynomial Cubic Bézier baseline:
  QUALIFIED by CGR0–CGR7 PASS in the admitted cloud envelope;
- Parametric Curve Family Abstraction decision: integrated and closed;
- Bounded Parametric Curve Contract and Cubic Bézier Conformance:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.

## 1. Question

Which concrete bounded curve representation should be implemented first after
the common parametric-curve semantic seam?

Candidates required by the closure checkpoint:

1. bounded line/segment;
2. bounded circle/conic arc;
3. rational and/or arbitrary-degree Bézier;
4. B-spline;
5. NURBS.

The selection must optimize scientific sequencing rather than feature count.
The first family should exercise the new common contract, add direct value to
Boundary Curve Discretization, admit strong independent analytic evidence, and
avoid importing unrelated complexity that belongs to later families.

## 2. Repository evidence

Fresh reconciliation on canonical `main`
`63479c8a7414a61be6f8ac1629e934f506d4f3de` establishes:

- `CurveParameterDomain`, `reversed_parameter`, and static
  `BoundedParametricCurve2/3` concepts are integrated;
- `CubicBezier2` and `CubicBezier3` satisfy those concepts without changing
  their qualified semantics;
- closure post-merge FAST `35713788156` and INTEGRATION
  `35713788216` pass;
- no concrete curve family other than polynomial cubic Bézier exists;
- Boundary Curve Discretization explicitly requires future
  `line/arc/Bezier` regression evidence;
- no surface representation exists.

The next work item is therefore a representation-breadth decision, not a new
differential-geometry operation.

## 3. Candidate comparison

| Candidate | Immediate boundary-discretization value | New semantic complexity | Independent analytic oracle | Exact CAD/free-form value | Decision |
| --- | --- | --- | --- | --- | --- |
| Bounded directed line segment | Directly required by the declared `line/arc/Bezier` envelope; simplest non-Bézier family | Very low: two endpoints, closed finite interval, constant first derivative, zero second derivative | Exact | Essential linear boundary primitive | **SELECTED FIRST** |
| Bounded circle/conic arc | Directly required for exact curved analytic boundaries | Moderate: angular parameterization, frame/orientation, arc span, periodic parent curve, conic degeneracies | Strong | High | SECOND CANDIDATE |
| Rational/arbitrary-degree Bézier | Important bridge toward exact conics and broader free-form geometry | Moderate/high: dynamic degree and/or weights; rational denominator semantics | Strong but broader | High | DEFER |
| B-spline | Important free-form CAD representation | High: knots, multiplicities, continuity partitions, local support, periodicity | Strong but extensive | High | DEFER |
| NURBS | Broad CAD representation including rational spline geometry | Highest among current candidates: knots + weights + rational denominator + periodicity/continuity | Strong but extensive | Very high | DEFER |

The line segment is selected because it tests whether the new abstraction is
genuinely reusable while adding the smallest possible new scientific surface
area.

## 4. Literature and mature-kernel evidence

### CGAL Segment_2 and Segment_3

Official references:

- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Segment__2.html
- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Segment__3.html

Relevant evidence:

- a segment is a directed closed straight segment between source and target;
- source and target belong to the segment;
- reversing/opposite swaps source and target;
- a segment may be degenerate when source and target coincide;
- 2D and 3D segment value types are natural separate kernel primitives.

Decision impact:

- AP Mesh should model a bounded **directed segment**, not an unbounded line;
- orientation belongs to ordered endpoints;
- reversal is exact endpoint exchange;
- coincident endpoints remain representable as a degenerate curve rather than
  being silently rejected at construction.

CGAL is reference/interface evidence only and is not admitted as a dependency
or numerical oracle.

### CGAL 2D Arrangements — curve-family breadth

Official reference:
https://doc.cgal.org/latest/Arrangement_on_surface_2/index.html

Relevant evidence:

- arrangements distinguish line segments, circular/conic arcs, polylines,
  Bézier curves and other curve families;
- the documentation recommends using the smallest traits model that satisfies
  the actual need.

Decision impact:

- adding the minimal linear family before conic/spline families is consistent
  with a disciplined representation-breadth sequence;
- AP Mesh should not collapse all future curve families into one heavy storage
  representation merely because a more general representation can express
  simpler cases.

### Farin 2002 and Piegl/Tiller 1997

Existing register authorities:

- Gerald Farin, *Curves and Surfaces for CAGD*, 5th ed.;
- Les Piegl and Wayne Tiller, *The NURBS Book*, 2nd ed.

Decision impact:

- polynomial/rational Bézier, B-spline and NURBS families have additional
  representation and derivative semantics that deserve later bounded work
  units;
- none of that machinery is required to represent a straight segment exactly.

## 5. Decision

Authorize exactly one future implementation work unit:

**Bounded Directed Line Segment Representation in 2D and 3D.**

The work unit will add concrete `LineSegment2` and `LineSegment3` value
representations that satisfy the already integrated
`BoundedParametricCurve2/3` concepts.

No other concrete curve family is authorized by this decision.

## 6. Mathematical representation

For finite endpoint points `P0` and `P1`, define the represented segment on

`t ∈ [0,1]`

by

`L(t) = (1-t) P0 + t P1`.

The parameter domain is exactly `[0,1]`.

First derivative:

`L'(t) = P1 - P0`.

Second derivative:

`L''(t) = 0`.

Reversal swaps the endpoints:

`reverse(L) = (P1,P0)`

and, through the common bounded-domain reversal map,

`reverse(L)(1-t) = L(t)`.

The family is directed: `LineSegment(P0,P1)` and
`LineSegment(P1,P0)` have opposite orientation even though their physical
point sets agree.

## 7. Degenerate segment semantics

A segment with `P0 == P1` is allowed as a value representation.

For such a segment:

- evaluation succeeds for every valid `t` and returns the common endpoint;
- first derivative is exactly the zero vector when representable;
- second derivative is exactly the zero vector;
- reversal is value-identical;
- the curve is not regular, but **regularity certification is not part of this
  first family work unit**.

This matches the project's existing separation between value representation and
later regularity/differential claims. Construction must not silently reject a
mathematically representable degenerate curve merely to simplify downstream
algorithms.

## 8. Proposed public API boundary

The implementation may add:

`include/apmesh/geometry/line_segment.hpp`

with concrete value types conceptually equivalent to:

- `LineSegment2(Point2 source, Point2 target)`;
- `LineSegment3(Point3 source, Point3 target)`;
- `source()`;
- `target()`;
- `parameter_domain()`;
- `evaluate(t)`;
- `first_derivative(t)`;
- `second_derivative(t)`;
- `reversed()`;
- exact value equality.

A separate `is_degenerate()` public method is **not required** by this work
unit. Degenerate behavior must nevertheless be covered by tests.

The implementation may add such a method only if it is proven necessary for
the focused contract without widening downstream semantics; otherwise defer it.

## 9. Numerical requirements

### Evaluation

For finite endpoints and `t∈[0,1]`, evaluation should use the same
overflow-aware finite interpolation primitive already accepted for Bézier
evaluation, preferably component-wise `std::lerp`.

Required properties:

- `evaluate(0) == source` exactly;
- `evaluate(1) == target` exactly;
- finite in-domain inputs must not produce avoidable overflow when the standard
  interpolation contract can return a finite result;
- non-finite or out-of-domain parameters fail with the existing
  `CurveError` vocabulary.

### First derivative

The exact mathematical derivative is `P1-P0`.

If the finite endpoint coordinates imply a derivative component whose
magnitude is not representable as `double`, return
`CurveError::non_finite_result`.

Do not clamp or rescale the mathematical derivative into a different vector.

### Second derivative

For every valid parameter, return the exact zero vector.

The method must still validate the parameter so all conforming curve families
retain consistent domain/error behavior.

### No hidden tolerance

No epsilon is permitted for:

- endpoint equality;
- domain membership;
- degeneracy;
- derivative zero classification.

This work unit does not introduce approximate segment identity.

## 10. Required invariants

### Endpoint interpolation

`L(0)=P0`, `L(1)=P1`.

### Affine midpoint

For representable moderate coordinates:

`L(0.5)` equals the analytic midpoint within the existing numeric comparison
contract used by focused tests.

### Constant first derivative

All valid parameter queries return the same first derivative.

### Zero second derivative

All valid parameter queries return exact zero components.

### Reversal involution

`reverse(reverse(L)) == L`.

### Reversal covariance

For `r(t)=1-t`:

- `reverse(L).evaluate(r(t)) = L.evaluate(t)`;
- `reverse(L)'(r(t)) = -L'(t)`;
- `reverse(L)''(r(t)) = L''(t) = 0`.

### Translation covariance

Translating both endpoints translates evaluated points and leaves derivatives
unchanged.

### 2D/3D embedding parity

Embedding a 2D segment in `z=0` must preserve x/y evaluation and derivative
results in the 3D representation.

### Determinism

Identical endpoints and parameter queries yield identical scientific fields and
typed failures under the declared environment.

## 11. Focused evidence required

A new focused contract must cover at minimum:

| Case | Required observation |
| --- | --- |
| Concept satisfaction | `LineSegment2` and `LineSegment3` satisfy the common bounded parametric concepts. |
| Endpoints | Exact source at 0 and target at 1. |
| Interior value | Independent analytic interior interpolation. |
| Constant D1 | Same derivative at multiple parameters. |
| Exact D2 | Zero vector at endpoints and interior. |
| Reversal | Endpoint swap, involution and evaluation/D1/D2 covariance. |
| Degenerate segment | Constant evaluation, zero D1/D2, stable reversal. |
| Non-finite parameter | Existing `non_finite_parameter` failure. |
| Below/above domain | Existing `parameter_out_of_domain` failure. |
| Extreme finite endpoints | Finite interpolation where `std::lerp` guarantees it; unrepresentable D1 fails explicitly. |
| Translation | Value translation and derivative invariance. |
| 2D/3D embedding | x/y parity and zero embedded z. |
| Determinism | Repeated evidence identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |
| Prerequisite preservation | Existing 18-test FAST/INTEGRATION inventory remains passing plus the new segment contract. |

Expected post-addition ordinary inventory: 19 semantic tests if one focused
segment test is added and no unrelated tests change.

## 12. Focused validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- the selected cubic-Bézier regression inventory must remain passing;
- the common parametric-curve contract must remain passing.

Passing evidence yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify the whole representation-breadth stage.

## 13. Explicit exclusions

This decision does not authorize:

- unbounded lines or rays;
- circular arcs;
- general conic arcs;
- arbitrary-degree Bézier;
- rational Bézier;
- B-spline;
- NURBS;
- polylines/composite curves;
- trimmed-curve composition;
- curve intersections;
- projection/closest-point queries;
- exact predicates;
- generic arc length or inverse length for all concepts;
- regularity certification for segments;
- segment curvature APIs;
- boundary discretization;
- surface representation;
- sizing;
- meshing;
- Quad-Dominant construction;
- parallel execution;
- third-party runtime dependencies;
- a new qualification campaign.

## 14. Alternatives considered

### Circle/conic arc first

Deferred.

It is the next high-value analytic family, especially for exact CAD boundaries,
but it introduces arc-frame, span, angular-domain, parent-periodicity and conic
semantics before the common abstraction has been exercised by any second
family.

### Rational or arbitrary-degree Bézier first

Deferred.

These broaden free-form geometry but introduce dynamic degree and/or weight
semantics. They do not provide as small an independent test of the new common
contract as a segment.

### B-spline first

Deferred.

Knots, multiplicity, continuity partitions, local support and possible
periodicity make it too large for the first post-abstraction family.

### NURBS first

Deferred.

NURBS combine the B-spline complexity above with rational weights and
denominator/finiteness concerns. Implementing them first would obscure whether
problems arise from the common abstraction or the spline/rational machinery.

### Bounded directed line segment

Selected.

It is the smallest meaningful non-Bézier family, is already required by the
future boundary-discretization regression, and has exact analytic evidence for
every operation required by the common concept.

## 15. Repository mapping for future implementation

Authorized mapping is limited to:

- public family declaration:
  `include/apmesh/geometry/line_segment.hpp`;
- production implementation:
  `src/geometry/line_segment.cpp`;
- focused contract:
  `tests/line_segment.cpp`;
- public/common header isolation as needed:
  `tests/curve_header_isolation.cpp` and/or one narrow segment-header
  isolation check;
- library/test registration:
  `CMakeLists.txt`;
- decision/state authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`.

Existing `parametric_curve.hpp` should not require semantic changes. If the
new family cannot satisfy the common concept without modifying that contract,
stop and require a new decision.

No topology, surface, qualification workflow or meshing file is expected to
change.

## 16. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricCurve2/3` semantics;
- an unbounded domain;
- a new tolerance;
- approximate endpoint identity;
- runtime type erasure or virtual inheritance;
- family-generic regularity/length/curvature algorithms;
- a second new concrete family;
- topology ownership;
- surface code;
- downstream discretization or meshing;
- a new third-party dependency.

## 17. Planned breadth sequence after this family

This is planning, not blanket authorization:

1. bounded directed line segment;
2. bounded circular/conic arc;
3. rational and/or arbitrary-degree Bézier after a new comparison decision;
4. B-spline;
5. NURBS;
6. composite/trimmed boundary curves.

Every transition requires a fresh remote audit and a bounded decision.

## 18. Effect on roadmap if integrated and closed

After decision integration, post-merge validation and checkpoint closure, the
sole next implementation work item is:

**Bounded Directed Line Segment Representation in 2D and 3D.**

The representation-breadth stage remains NOT QUALIFIED.

Curve Differential Geometry remains paused and unqualified.

Boundary Curve Discretization and Surface Representation remain blocked.

No circle/conic, rational Bézier, B-spline, NURBS or composite/trimmed family
is pre-authorized by this decision.


## 19. Decision integration checkpoint

PR #103 integrated this bounded decision as
`2b42c78a2dbf0ede225144339dbf100900bef672`.

Final decision-head validation:

- FAST `35719338493`: PASS;
- INTEGRATION `35719338492`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35719435059`: PASS;
- INTEGRATION `35719434961`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Bounded Directed Line Segment Representation in 2D and 3D.**

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the selected implementation within Sections 5–16 of this decision.

No circle/conic, rational/arbitrary-degree Bézier, B-spline, NURBS,
composite/trimmed curve, surface, boundary-discretization, sizing, meshing,
Quad-Dominant or parallel implementation is authorized by this checkpoint.


## 20. Active implementation mapping

Decision closure PR #104 merged as
`326ffdf724912e8841a74c3c0b69756ca23e14c2`.

Closure post-merge validation:

- FAST `35719744251`: PASS;
- INTEGRATION `35719744291`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The sole authorized implementation is active on:

`curve/bounded-line-segment`.

Candidate repository mapping:

- public value declarations:
  `include/apmesh/geometry/line_segment.hpp`;
- production implementation:
  `src/geometry/line_segment.cpp`;
- focused semantic and header-isolation contract:
  `tests/line_segment.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized continuity authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`.

Implemented candidate semantics:

- `LineSegment2` and `LineSegment3` are directed endpoint value types;
- the parameter domain is exactly `[0,1]`;
- value evaluation uses component-wise `std::lerp`;
- first derivative is the endpoint difference and returns
  `CurveError::non_finite_result` when that mathematical difference is not
  representable;
- second derivative is exact zero after the same typed parameter validation;
- reversal swaps endpoints;
- degenerate coincident-endpoint segments remain representable and yield
  constant values with exact zero derivatives;
- both concrete families satisfy the already integrated
  `BoundedParametricCurve2/3` concepts;
- the common parametric-curve contract is unchanged.

Focused evidence covers concept satisfaction, exact endpoints, analytic
interior evaluation, D1/D2, reversal/involution/covariance, degeneracy, typed
parameter failures, extreme finite coordinates, unrepresentable derivative,
translation, 2D/3D embedding and determinism.

The ordinary FAST/INTEGRATION semantic inventory is expected to increase from
18 to 19 tests through one new `apmesh_core.line_segment` contract.

Status before CI:

**IMPLEMENTED CANDIDATE / FOCUSED VALIDATION PENDING / NOT QUALIFIED.**

No other curve family or downstream capability is implied.
