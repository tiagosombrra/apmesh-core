# Oriented Trimmed Parametric Subcurve — Bounded Semantic Decision

Status: DECISION APPROVED / IMPLEMENTATION NOT STARTED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Curve Representation Breadth Gate

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation — polynomial cubic Bézier baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- Bounded Parametric Curve Contract: integrated;
- Bounded Directed Line Segment 2D/3D: integrated focused work unit;
- Positive-Weight Rational Quadratic Bézier 2D/3D: integrated focused work
  unit and closed.

## 1. Question

What is the next smallest representation-breadth step after the repository has
three bounded curve representations but still lacks analytic-conic,
arbitrary-degree, spline, NURBS and trim/composition semantics?

The required comparison includes:

1. dedicated analytic circle/general conic representation;
2. arbitrary-degree polynomial/rational Bézier;
3. B-spline;
4. NURBS;
5. composition/trimming.

The chosen work unit must advance future CAD/boundary semantics, reuse the
existing bounded-parametric contract, remain independently verifiable, and not
combine several unresolved semantic layers in one implementation.

## 2. Fresh repository evidence

Canonical decision-entry `main`:
`7579254ebd0d6843fdc3761376132a2b7d9fa43c`.

Terminal rational-quadratic closure evidence:

- implementation PR #109:
  `6600875dfbb33d1a37603e32bcf452625373c462`;
- implementation closure PR #110:
  `93b082ce660fd8d2c012b96ef7319b240de6d9d2`;
- closure post-merge FAST `35727653016`: PASS;
- closure post-merge INTEGRATION `35727652961`: PASS;
- terminal sync `7579254ebd0d6843fdc3761376132a2b7d9fa43c`;
- terminal sync FAST `35728104539`: PASS;
- terminal sync INTEGRATION `35728104607`: PASS.

Production curve values at entry:

- `CubicBezier2`, `CubicBezier3`;
- `LineSegment2`, `LineSegment3`;
- `RationalQuadraticBezier2`, `RationalQuadraticBezier3`.

All expose finite bounded parameter domains through the existing
`BoundedParametricCurve2/3` concepts.

Absent at entry:

- dedicated analytic circle/conic arc classes;
- arbitrary-degree polynomial/rational Bézier;
- B-spline;
- NURBS;
- trimmed-curve wrapper semantics;
- heterogeneous composite/polycurve semantics;
- production surfaces.

## 3. Literature and mature-kernel evidence

### Open CASCADE `Geom_TrimmedCurve`

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___trimmed_curve.html

Relevant evidence:

- a trimmed curve is explicitly a portion of a basis curve limited by two
  parameter values;
- orientation of the trimmed result is a separate semantic concern;
- reversal and reversed-parameter mapping are explicit operations;
- trimming is modeled separately from the mathematical representation of the
  basis family;
- periodic basis curves introduce additional ambiguity and period adjustment.

Decision impact:

- trimming is an orthogonal semantic layer and can be introduced without first
  implementing a new mathematical curve family;
- the first AP Mesh trim work unit should remain non-periodic and bounded;
- AP Mesh does not adopt Open CASCADE inheritance, ownership, exception or
  tolerance behavior.

### CGAL polycurve traits

Official reference:
https://doc.cgal.org/latest/Arrangement_on_surface_2/classCGAL_1_1Arr__polycurve__traits__2.html

Relevant evidence:

- piecewise curves may be chains of line segments, conic/circular arcs,
  Bézier curves or other subcurve types;
- neighboring subcurves must share endpoints and maintain orientation;
- composition is a distinct concern layered over individual curve
  representations.

Decision impact:

- trimming/subcurve semantics should be isolated before heterogeneous
  composition;
- the first work unit does not need a closed runtime variant of every future
  curve family.

### Open CASCADE B-spline reference

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Relevant evidence:

- B-spline curves may be uniform/non-uniform, rational/non-rational and
  periodic/non-periodic;
- construction includes degree, poles, knots and multiplicities;
- knot multiplicity affects continuity;
- reversal modifies knot ordering while preserving parameter bounds.

Decision impact:

- B-spline is a major later family, but introducing degree, knots,
  multiplicities, continuity partitions and periodicity together would be a
  substantially larger semantic step than trimming already integrated curves.

### Open CASCADE conic reference

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___conic.html

Relevant evidence:

- analytic 3D conics are positioned by a local coordinate system in a
  supporting plane;
- circle/ellipse/hyperbola/parabola have family-specific analytic and periodic
  semantics.

Decision impact:

- the repository's qualified Cartesian-frame claim still does not admit
  arbitrary-angle orientation;
- a general analytic 3D conic therefore remains blocked by a separate
  arbitrary supporting-plane/orientation decision.

### Rational Bézier / NURBS relationship

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB.html

Relevant evidence:

- rational Bézier is a knot-free special case of NURBS;
- arbitrary-degree rational Bézier adds degree/storage breadth but still lacks
  the local knot structure of B-spline/NURBS.

Decision impact:

- the repository already has a bounded rational layer sufficient to validate
  weighted denominator semantics;
- arbitrary degree remains valuable, but it is not required before trimming
  portions of the existing three production families.

## 4. Candidate comparison

| Candidate | Immediate value | New semantic burden | Reuses all current families | Blocking prerequisite | Decision |
| --- | --- | --- | --- | --- | --- |
| Dedicated analytic circle/general conic | Native analytic CAD semantics | Plane frame, angular/periodic semantics, family-specific degeneracies | No | Arbitrary 3D supporting-plane/orientation | DEFER |
| Arbitrary-degree polynomial/rational Bézier | Broader free-form degree | Dynamic degree/storage and generalized derivative machinery | No | None, but limited immediate boundary gain | DEFER |
| B-spline | Major CAD/free-form family | Degree, knots, multiplicities, local support, continuity, periodicity | No | Larger validation surface | DEFER |
| NURBS | Broad CAD target | B-spline burden plus weights/rational denominator | No | B-spline semantics should be isolated first | DEFER |
| Heterogeneous composition/polycurve | Boundary-chain semantics | Runtime storage/dispatch and join continuity | Yes | Requires a family-erasure/composition decision | DEFER |
| Oriented trimming of one bounded basis curve | Direct subcurve/boundary semantics; future trimming foundation | Basis ownership, subdomain validation, orientation mapping, derivative sign | **Yes** | None for current non-periodic families | **SELECTED** |

## 5. Decision

Authorize exactly one future implementation work unit:

**Oriented Trimmed Parametric Subcurve Semantics in 2D and 3D.**

This work unit introduces a static value wrapper over one existing bounded
parametric basis curve.

It does **not** introduce another mathematical curve family.

The first implementation is admitted scientifically only for basis types
already present at decision entry:

2D:

- `LineSegment2`;
- `CubicBezier2`;
- `RationalQuadraticBezier2`.

3D:

- `LineSegment3`;
- `CubicBezier3`;
- `RationalQuadraticBezier3`.

The C++ template may be syntactically reusable by a future conforming type, but
no future family receives scientific admission merely by satisfying the
compile-time concept.

## 6. Trim representation

Let the basis curve be

`C : [a,b] -> R^d`

with finite closed domain `[a,b]`.

Choose two finite basis parameters:

- oriented source parameter `u_s`;
- oriented target parameter `u_e`;

such that

`u_s, u_e ∈ [a,b]`

and

`u_s != u_e`.

Define the exposed trimmed domain

`D_T = [min(u_s,u_e), max(u_s,u_e)]`.

The wrapper stores:

- the basis curve by value;
- `u_s`;
- `u_e`.

No hidden normalization to `[0,1]` is performed.

## 7. Orientation-preserving parameter map

For an exposed parameter `u ∈ D_T`, define the basis parameter map

`phi(u) = u`

when

`u_s < u_e`.

When

`u_s > u_e`, define

`phi(u) = r_D(u)`

where `r_D` is the already integrated overflow-aware
`reversed_parameter(D_T,u)`.

Thus:

- the lower endpoint of `D_T` always evaluates to the oriented source point;
- the upper endpoint always evaluates to the oriented target point;
- forward trim preserves the basis parameter;
- reverse trim preserves parameter magnitude/range while reversing
  orientation.

The implementation must not compute a naïve `u_s + u_e - u` if the existing
`reversed_parameter` avoids an intermediate-overflow risk.

## 8. Value and derivative semantics

Define

`T(u) = C(phi(u))`.

### Forward orientation

If `u_s < u_e`:

- `T(u)=C(u)`;
- `T'(u)=C'(u)`;
- `T''(u)=C''(u)`.

### Reverse orientation

If `u_s > u_e`:

- `T(u)=C(phi(u))`;
- `T'(u)=-C'(phi(u))`;
- `T''(u)=C''(phi(u))`.

The second derivative keeps its sign because the reversal map is affine with
first derivative `-1` and second derivative zero.

No derivative magnitude scaling occurs because the exposed domain retains the
basis-parameter interval rather than normalizing the trim to a different
interval.

## 9. Construction semantics

The wrapper must use validated construction.

A bounded error vocabulary may distinguish at minimum:

- non-finite source parameter;
- non-finite target parameter;
- source parameter outside the basis domain;
- target parameter outside the basis domain;
- zero-width trim (`u_s == u_e`).

No epsilon defines equality or domain inclusion.

No clamping is permitted.

No periodic wrapping is permitted.

The basis curve is copied/stored by value. The first work unit does not
introduce borrowed lifetime semantics or shared ownership.

## 10. Proposed C++23 shape

The implementation may add one public header:

`include/apmesh/geometry/trimmed_curve.hpp`.

Conceptually, it may expose static templates equivalent to:

- `TrimmedCurve2<Curve>` constrained by `BoundedParametricCurve2<Curve>`;
- `TrimmedCurve3<Curve>` constrained by `BoundedParametricCurve3<Curve>`.

A validated factory is required.

The public semantic surface may include:

- `make(basis, source_parameter, target_parameter)`;
- `basis_curve()`;
- `source_parameter()`;
- `target_parameter()`;
- `parameter_domain()`;
- `evaluate(parameter)`;
- `first_derivative(parameter)`;
- `second_derivative(parameter)`;
- `reversed()`.

The implementation should remain header-only unless a later concrete need
justifies source-level explicit instantiation. This avoids artificially
restricting the generic semantic wrapper to a closed set of runtime types.

## 11. Query failure semantics

For a query parameter:

- non-finite input -> `CurveError::non_finite_parameter`;
- finite parameter outside the **trimmed** domain ->
  `CurveError::parameter_out_of_domain`;
- basis evaluation/derivative error after a valid parameter mapping ->
  propagate the basis `CurveError`;
- a failed reverse mapping -> propagate its typed `CurveError`.

The wrapper must not:

- clamp to a trim endpoint;
- query the basis outside the selected trim;
- retry using another orientation;
- normalize or wrap periodic parameters;
- convert an error into a zero derivative.

## 12. Reversal

`reversed()` swaps only the oriented trim endpoints:

- basis value remains the same stored basis;
- `u_s <-> u_e`;
- exposed parameter domain remains identical.

Double reversal recovers the exact stored trimmed representation.

For the common reversed parameter map on the trimmed domain:

`reverse(T)(r_D(u)) = T(u)`.

Required derivative covariance:

- `reverse(T)'(r_D(u)) = -T'(u)`;
- `reverse(T)''(r_D(u)) = T''(u)`.

The first implementation does not call `basis.reversed()`; orientation is a
property of the trim wrapper.

## 13. Invariants

### Domain subset

The trimmed domain is a nonzero finite closed subset of the basis domain.

### Oriented endpoint identity

The lower exposed parameter evaluates to `C(u_s)`.

The upper exposed parameter evaluates to `C(u_e)`.

### Forward identity

A forward trim queries the basis at the same parameter with the same D1/D2.

### Reverse covariance

A reverse trim queries the reflected basis parameter, negates D1 and preserves
D2.

### Full-domain forward equivalence

Trimming from `a` to `b` over the full basis domain reproduces basis
value/D1/D2 queries.

### Full-domain reverse equivalence

Trimming from `b` to `a` must agree with basis reversal under the existing
common reversed-parameter contract for every admitted basis family.

### No scientific widening by template satisfaction

A future type compiling with the template is not automatically part of the
admitted representation envelope.

### Determinism

Repeated trim construction and queries on identical basis values/parameters
produce identical scientific fields and typed failures.

## 14. Focused evidence required

A single focused contract must cover at minimum:

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Trim wrappers over all admitted 2D/3D basis families satisfy the corresponding bounded concepts. |
| Construction validation | NaN/infinite endpoints, out-of-basis-domain endpoints and equal endpoints fail explicitly. |
| Forward line trim | Exact endpoint/subdomain and direct basis parity. |
| Reverse line trim | Endpoint orientation and D1 sign/D2 parity. |
| Cubic Bézier forward/reverse | Value/D1/D2 agree with independent basis queries at mapped parameters. |
| Rational quadratic forward/reverse | Value/D1/D2 agree with independent basis queries at mapped parameters. |
| Full-domain identity | Forward trim reproduces basis queries. |
| Reversal involution | Double reversal recovers exact trim storage. |
| Reversal covariance | Value/D1/D2 relations hold under common reversed parameter. |
| Query failures | Non-finite and outside-trim queries preserve typed failures. |
| Extreme finite domain behavior | Mapping uses existing overflow-aware reversal primitive; no naïve-sum overflow. |
| 2D/3D parity | Corresponding embedded basis/trim fixtures preserve x/y and zero z. |
| Determinism | Repeated values/failures are identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |
| Prerequisite preservation | Existing 20 ordinary semantic tests remain passing plus one trim contract. |

If exactly one new focused test is registered, the expected ordinary inventory
becomes **21 tests**.

## 15. Why composition is not included

Heterogeneous composition requires a separate storage/dispatch decision.

The repository currently avoids:

- a universal virtual curve base;
- a closed `std::variant` containing every future family;
- generic type erasure.

A polycurve joining different concrete families would force one of those
architectural choices or another explicit alternative.

Trimming one statically known basis type does not.

Therefore:

**trimming is selected now; heterogeneous composition is deferred.**

## 16. Why analytic conics are not next

Dedicated analytic conics remain scientifically valuable.

However, a general 3D analytic conic requires a supporting-plane/local-frame
semantics. The currently qualified Cartesian frames explicitly do not establish
arbitrary-angle orientation.

The already integrated rational quadratic family supplies conic-segment
geometry without weakening that frame qualification.

Therefore analytic conic work remains deferred until its frame/orientation
prerequisite is addressed explicitly.

## 17. Why arbitrary-degree Bézier is not next

Arbitrary degree would introduce:

- dynamic or compile-time-variable control storage;
- generalized degree-dependent derivative machinery;
- new degree bounds/resource semantics.

The repository already has polynomial cubic Bézier for free-form boundary
regression and rational quadratic Bézier for conic-segment capability.

Trimming provides broader immediate reuse across **all three** existing
families with a smaller semantic surface.

## 18. Why B-spline/NURBS are not next

B-spline requires simultaneous decisions around:

- degree;
- knot storage/order;
- multiplicity;
- local support;
- continuity at knots;
- endpoint/clamping conventions;
- periodicity policy;
- derivative behavior at continuity breaks.

NURBS adds weights and rational denominator behavior on top of that.

The rational quadratic work already isolates the weighted rational layer.

Trimming should be established before spline/NURBS so later bounded pieces can
reuse the same subcurve/orientation semantics rather than invent family-specific
trimming.

## 19. Explicit exclusions

This decision does not authorize:

- heterogeneous polycurve/composite storage;
- type erasure;
- a universal runtime curve base;
- a closed variant of all families;
- periodic trimming or wraparound;
- unbounded basis curves;
- zero-width trims;
- reparameterization to `[0,1]`;
- arbitrary parameter scaling;
- analytic circle/conic classes;
- arbitrary 3D plane/frame expansion;
- arbitrary-degree polynomial/rational Bézier;
- B-spline;
- NURBS;
- knot vectors;
- continuity classification;
- surface trimming;
- topology ownership;
- curve intersections;
- projection/closest point;
- arc length/regularity/curvature generic refactors;
- boundary discretization;
- surfaces;
- sizing/meshing;
- Quad-Dominant work;
- parallel execution;
- third-party runtime dependencies;
- a representation-breadth qualification campaign.

## 20. Repository mapping for the future implementation

Authorized mapping is limited to:

- public template/value semantics:
  `include/apmesh/geometry/trimmed_curve.hpp`;
- focused semantic/header contract:
  `tests/trimmed_curve.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No production `.cpp` is expected for the generic wrapper.

No change to `parametric_curve.hpp` is expected.

If the common concepts must change, stop and require a new decision.

## 21. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 20 ordinary semantic tests must remain passing;
- the new trim contract must pass.

Passing yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify composite curves, analytic conics, splines, NURBS or
surface trimming.

## 22. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricCurve2/3`;
- periodic wraparound;
- a basis curve with an unbounded domain;
- runtime type erasure;
- heterogeneous composition;
- shared/borrowed ownership semantics;
- hidden tolerance for trim endpoint equality;
- parameter normalization/scaling beyond orientation reversal;
- a new concrete mathematical curve family;
- topology/surface/meshing code;
- a third-party dependency.

## 23. Planned sequence after trimming

Planning only, not authorization.

After trim implementation/closure, a fresh decision must compare the remaining
breadth steps again.

Likely candidates remain:

1. arbitrary-degree polynomial/rational Bézier;
2. bounded non-periodic B-spline;
3. NURBS after B-spline semantics;
4. analytic conic plus arbitrary 3D supporting-plane/orientation;
5. heterogeneous composition/polycurve.

The order is not pre-authorized.

## 24. Effect if integrated and closed

After decision integration, post-merge validation and decision checkpoint
closure, the sole next implementation work item is:

**Oriented Trimmed Parametric Subcurve Semantics in 2D and 3D.**

No implementation starts on this decision branch.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization remains blocked as a stage until the
representation-breadth gate is explicitly closed by later decisions.

Surface Representation remains blocked, but this decision establishes a
curve-level trimming semantic that later surface-boundary work may reference
without retroactive reinterpretation.


## 25. Decision integration checkpoint

PR #112 integrated this bounded decision as
`13b5b0c77c5ff96ecc30326ff10970b3976d6e84`.

Final decision-head validation:

- FAST `35729461953`: PASS;
- INTEGRATION `35729462003`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35729581995`: PASS;
- INTEGRATION `35729581937`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Oriented Trimmed Parametric Subcurve Semantics in 2D and 3D.**

After this separate closure checkpoint is integrated and its post-merge
validation passes, Sections 5–22 become the sole authorized implementation
scope.

No heterogeneous composition, periodic trim, analytic conic,
arbitrary-degree Bézier, B-spline, NURBS, surface, discretization, sizing,
meshing, Quad-Dominant or parallel implementation is authorized by this
checkpoint.


## 26. Active implementation mapping

Decision closure PR #113 merged as
`74cafc0f7e64abe159303fe7116dcbaac4d8fad7`.

Closure post-merge validation:

- FAST `35729923695`: PASS;
- INTEGRATION `35729923468`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The sole authorized implementation is active on:

`curve/trimmed-parametric-subcurve`.

Candidate repository mapping:

- `include/apmesh/geometry/trimmed_curve.hpp`;
- `tests/trimmed_curve.cpp`;
- `CMakeLists.txt`;
- synchronized `STATE`, `ROADMAP`, `WORKLOG` and this decision.

Candidate semantics:

- generic header-only wrappers constrained by the existing bounded 2D/3D
  concepts;
- basis curve stored by value;
- explicit construction failures for non-finite, out-of-domain and zero-width
  trim endpoints;
- exposed parameter domain retains the basis parameter magnitudes;
- reverse orientation reuses `reversed_parameter`;
- reverse first derivative is sign-negated; second derivative is preserved;
- reversal swaps trim endpoints without reversing the stored basis.

Focused evidence includes all currently admitted basis families in 2D and 3D,
plus an extreme-domain local probe that is evidence only and does not widen the
scientifically admitted basis set.

Expected ordinary FAST/INTEGRATION inventory after registration: **21 tests**.

Status before CI:

**IMPLEMENTED CANDIDATE / FOCUSED VALIDATION PENDING / NOT QUALIFIED.**

No heterogeneous composition or new mathematical curve family is implied.
