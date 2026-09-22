# Multi-Span Clamped Cubic Positive-Weight NURBS — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
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
- fixed two-span cubic polynomial B-spline 2D/3D:
  integrated and closed focused work unit;
- fixed two-span cubic positive-weight NURBS 2D/3D:
  integrated and terminally closed focused work unit.

## 1. Question

Which remaining representation-breadth concept should be introduced next after
the fixed two-span cubic NURBS work unit, and what is the smallest work unit
that advances toward practical CAD/NURBS surfaces without simultaneously
changing degree, continuity class, periodicity and runtime family dispatch?

Required candidates are:

1. general bounded clamped cubic B-spline/NURBS span-count expansion;
2. arbitrary-degree polynomial/rational Bézier;
3. repeated-knot/continuity breadth;
4. analytic conic after arbitrary 3D supporting-plane/orientation;
5. heterogeneous composition/polycurve.

The repository has already independently isolated:

- bounded parameter-domain and reversal semantics;
- positive rational weights;
- cubic B-spline knot/local-support semantics;
- two-span NURBS interaction of knots and rational weights;
- exact typed query failures.

The unresolved concept with the highest direct downstream value is now
**runtime-variable span/control/knot count under otherwise frozen cubic NURBS
semantics**.

## 2. Fresh repository evidence

Canonical decision-entry `main`:
`eb48bd648c75efe044dd4bc34c3aa34b4b72cda2`.

Terminal fixed-NURBS lineage:

- implementation PR #124:
  `9bb810f473977cbadf2e1e2a9a6df111f2ce67f1`;
- implementation closure PR #125:
  `465dc5b5f1948d4d27ca67d777aa2493f9c8968e`;
- terminal synchronization PR #126:
  `eaba130eaa5da79fc827b8f770df4123524fc455`;
- final terminal-state PR #127:
  `eb48bd648c75efe044dd4bc34c3aa34b4b72cda2`;
- PR #127 FAST `35748224182`: PASS;
- PR #127 INTEGRATION `35748224260`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug;
- post-merge FAST `35748370784`: PASS;
- post-merge INTEGRATION `35748370519`: PASS.

Production curve breadth at entry:

- `CubicBezier2/3`;
- `LineSegment2/3`;
- `RationalQuadraticBezier2/3`;
- static `TrimmedCurve2/3`;
- `TwoSpanCubicBSpline2/3`;
- `TwoSpanCubicNURBS2/3`.

No active representation work item exists at entry.

## 3. Literature and engineering basis

### 3.1 General CAD B-spline/NURBS data model

Open CASCADE Technology 8.0.1 `Geom_BSplineCurve`:

https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Relevant evidence:

- mature CAD B-splines use arrays of poles, optional weights, knots,
  multiplicities and degree;
- the flat knot-sequence length is related to pole count and degree;
- rational and non-rational curves share the same spline structure;
- local continuity depends on degree and knot multiplicity;
- value and derivatives are evaluated on a located knot span.

Decision impact:

- AP Mesh should now resolve variable pole/knot/span storage while keeping
  degree, multiplicity and periodicity frozen;
- span-count generalization is a prerequisite to practical imported CAD
  splines.

### 3.2 NURBS piecewise/local-support structure

Michigan Technological University NURBS properties:

https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS-property.html

Relevant evidence:

- a NURBS curve is piecewise degree-`p` rational geometry;
- a clamped NURBS passes through its endpoint controls;
- one parameter in one knot span depends only on the locally active controls;
- increasing span count does not require increasing polynomial degree.

Decision impact:

- the new work unit may expand span count without changing degree 3;
- production query cost should preserve local support after deterministic span
  location.

### 3.3 Simple-knot continuity

Michigan Technological University B-spline basis properties:

https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/bspline-property.html

Relevant evidence:

- at knot multiplicity `m`, degree-`p` continuity is `C^(p-m)`;
- simple cubic interior knots therefore remain `C2`.

Decision impact:

- every admitted interior knot remains simple;
- repeated-knot C1/C0 behavior and one-sided derivative semantics remain a
  separate future decision.

### 3.4 Direct relation to future NURBS surfaces

Open CASCADE Technology 8.0.1 `Geom_BSplineSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html

Relevant evidence:

- a B-spline/NURBS surface combines a control-point grid, weights, independent
  U/V knot arrays, multiplicities and U/V degrees;
- rational and non-rational surface cases share the same knot/control
  structure.

Decision impact:

- resolving variable 1D pole/weight/knot ownership and deterministic span
  location is a direct curve-level prerequisite for later surface work;
- this does **not** authorize a surface representation now.

### 3.5 Runtime sequence ownership

C++ standard-library references:

- https://en.cppreference.com/cpp/container/vector
- https://en.cppreference.com/cpp/container/span

Relevant evidence:

- `std::vector` provides owning contiguous dynamic-size storage;
- `std::span` provides a non-owning contiguous view;
- no custom allocator or third-party small-vector dependency is needed.

Decision impact:

- the first variable-size spline value should own its data with ordinary
  `std::vector`;
- read-only public access should use `std::span<const T>`;
- no arbitrary fixed capacity becomes part of scientific type identity;
- allocation failure remains an ordinary C++ resource failure, not a
  `CurveError` or a silently retried scientific outcome.

### 3.6 Deferred candidates remain genuinely distinct

Open CASCADE `Geom_BezierCurve`:

https://dev.opencascade.org/doc/refman/html/class_geom___bezier_curve.html

supports variable pole count/degree and rational weights, but remains a
single-span globally supported family.

Open CASCADE `Geom_Circle`:

https://dev.opencascade.org/doc/refman/html/class_geom___circle.html

uses an arbitrary 3D placement frame and periodic angular parameterization.

Decision impact:

- arbitrary-degree Bézier does not exercise the newly integrated multi-span
  spline seam;
- analytic conic still requires arbitrary 3D placement/periodic semantics
  beyond the currently qualified frame envelope.

External sources are design/reference evidence only. No runtime dependency or
numerical oracle is admitted.

## 4. Candidate comparison

| Candidate | New semantics introduced now | Reuses integrated foundations | Downstream value | Main unresolved burden | Decision |
| --- | --- | --- | --- | --- | --- |
| Multi-span clamped cubic positive-weight NURBS | Runtime pole/weight/interior-knot count and span search | Fixed NURBS + parametric contract | **Very high**; direct precursor to practical NURBS surfaces | Owning dynamic storage + variable span count | **SELECTED** |
| Arbitrary-degree polynomial/rational Bézier | Variable degree/control count | Bézier + rational layer | Moderate-high | Degree-dependent algorithms/storage; still single-span | DEFER |
| Repeated-knot/continuity breadth | C1/C0 knot joins and derivative-side semantics | Spline layer | High | Requires new continuity/failure semantics at knots | DEFER |
| Analytic conic | Native circle/ellipse/hyperbola/parabola representation | Parametric contract | High | Arbitrary 3D placement + periodic/angular semantics | BLOCKED/DEFER |
| Heterogeneous composition/polycurve | Runtime chain of different families | All current families + trim | High | Ownership/dispatch/join continuity policy | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Multi-Span Clamped Cubic Positive-Weight NURBS Representation in 2D and 3D
with Simple Interior Knots.**

Proposed public family names:

- `MultiSpanCubicNURBS2`;
- `MultiSpanCubicNURBS3`.

The family is deliberately fixed in every dimension except span count:

- degree exactly 3;
- at least two nonzero knot spans;
- finite runtime-variable control count;
- one positive finite weight per control;
- finite runtime-variable simple interior-knot count;
- clamped endpoint multiplicity exactly four;
- every interior knot multiplicity exactly one;
- non-periodic;
- bounded;
- value, D1 and D2 required;
- no production knot insertion/removal or degree mutation.

## 6. Mathematical representation and size relation

Let there be `r >= 1` simple interior knots

`k1 < k2 < ... < kr`

between finite endpoint knots

`a < k1 < ... < kr < b`.

The full degree-three flat knot sequence is

`U = [a,a,a,a,k1,...,kr,b,b,b,b]`.

Then:

- number of spans:
  `S = r + 1 >= 2`;
- number of controls:
  `N = r + 4 = S + 3`;
- number of weights:
  exactly `N`;
- flat knot count:
  `N + 4`.

Let controls be `P0...P(N-1)` and positive weights be
`w0...w(N-1)`.

The curve is

`C(u) = [sum_i N_i,3(u) w_i P_i] / [sum_i N_i,3(u) w_i]`

for `u in [a,b]`.

Because every admitted weight is strictly positive and the active basis forms
a partition of unity, the denominator is positive throughout the admitted
domain in exact arithmetic.

## 7. Continuity scope

Every interior knot has multiplicity one.

With degree three, the represented curve remains `C2` at every admitted
interior knot.

Therefore the existing common D1/D2 query contract can remain unchanged.

This work unit does not admit:

- repeated interior knots;
- C1/C0/discontinuous spline joins;
- one-sided derivative APIs;
- periodic knots;
- degree other than three.

## 8. Ownership and storage policy

The general family must own its runtime-variable representation.

Preferred storage:

- `std::vector<Point2/Point3>` controls;
- `std::vector<double>` weights;
- `std::vector<double>` interior knots;
- scalar `a` and `b`.

Public inspection should expose immutable contiguous views using
`std::span<const T>` rather than mutable vector references.

Rationale:

- the represented input size is inherently runtime data in CAD exchange;
- a compile-time capacity parameter would make an arbitrary resource limit part
  of curve type identity;
- `std::pmr`, custom allocators and third-party small-vector types add
  independent resource-policy semantics without scientific benefit here.

The owning vectors are immutable after successful construction.

Allocation exhaustion is not a scientific/domain failure. It must not be
converted into `CurveError`, retried with a smaller representation, or
silently truncated.

## 9. Construction semantics

Construction is validated.

At minimum reject:

- fewer than five controls;
- controls/weights count mismatch;
- interior-knot count not equal to `control_count - 4`;
- non-finite `a` or `b`;
- `a >= b`;
- any non-finite interior knot;
- any interior knot not strictly inside `(a,b)`;
- non-strictly-increasing interior knots;
- any non-finite weight;
- any weight `<= 0`.

No epsilon defines knot order, knot equality or positivity.

Control-point finiteness remains guaranteed by `Point2/Point3`.

The construction error vocabulary may extend the bounded NURBS construction
vocabulary, but no existing error meaning may be weakened.

## 10. Common bounded-parametric contract

`MultiSpanCubicNURBS2/3` must satisfy the existing
`BoundedParametricCurve2/3` concepts without changing
`parametric_curve.hpp`.

Required operations remain:

- `parameter_domain()`;
- `evaluate(u)`;
- `first_derivative(u)`;
- `second_derivative(u)`;
- `reversed()`.

If the common concept must change, stop and require a new scientific decision.

## 11. Parameter domain and query failures

The parameter domain is exactly `[a,b]`.

Required failures:

- NaN/infinity query:
  `CurveError::non_finite_parameter`;
- finite query below `a` or above `b`:
  `CurveError::parameter_out_of_domain`;
- finite in-domain result that cannot be represented:
  `CurveError::non_finite_result`.

No clamping, normalization to `[0,1]`, periodic wrapping or hidden retry is
permitted.

Required endpoint values:

- `C(a)=P0`;
- `C(b)=P(N-1)`.

Endpoint values should be returned exactly where the fixed family already
preserves exact identity.

## 12. Deterministic span-location policy

Span location is part of the scientific contract.

For finite `u in [a,b]`:

- `u == b` selects the final span;
- otherwise select the unique span whose upper boundary is the first knot
  strictly greater than `u`;
- an interior-knot query `u == ki` therefore selects the span to the right.

A standard binary-search/`upper_bound`-equivalent implementation is preferred.

No tolerance participates in span selection.

The right-span rule is representation policy, not a claim of one-sided
geometry. Because every admitted interior knot is simple cubic, value, D1 and
D2 are `C2` there in exact arithmetic.

## 13. Production evaluation strategy

Production should preserve the fixed-family homogeneous strategy:

1. locate the active span;
2. select only its four active original controls/weights;
3. choose a positive local common weight scale from those active weights;
4. construct homogeneous local controls;
5. apply fixed-degree cubic de Boor evaluation;
6. dehomogenize with explicit finite/positive denominator checks.

The query must not traverse every control after span location.

Expected query complexity:

- span location: `O(log S)`;
- degree-three local evaluation: bounded constant work;
- no per-query allocation is required.

This local normalization rule also prevents an out-of-support weight from
changing arithmetic in an unrelated span.

## 14. D1 and D2 semantics

D1/D2 remain analytic.

Production may derive the locally required homogeneous derivative controls from
the active neighborhood and flat-knot accessors, then apply the already
validated quotient identities:

`C' = (A' - C W') / W`

`C'' = (A'' - 2 C' W' - C W'') / W`.

No finite-difference derivative is permitted.

An unrepresentable final D1/D2 returns
`CurveError::non_finite_result`.

No derivative is normalized, clamped or replaced with zero after failure.

## 15. Fixed two-span parity

For exactly one interior knot, the new general family must reproduce the
integrated `TwoSpanCubicNURBS2/3` family.

Focused evidence must compare:

- stored controls and weights;
- parameter domain;
- value;
- D1;
- D2;
- reversal;
- typed query failures.

The fixed family remains frozen prerequisite evidence and is not replaced or
deprecated by this work unit.

## 16. Polynomial subset evidence

Equal positive weights reduce the NURBS equation to the corresponding
polynomial cubic B-spline.

No new general polynomial B-spline production type is authorized here.

Focused evidence must include:

- two-span equal-weight parity with `TwoSpanCubicBSpline2/3`;
- multi-span all-one/equal-weight comparison against an independent polynomial
  Cox-de Boor basis oracle.

A later decision may introduce a dedicated general polynomial B-spline public
type if data-model fidelity or performance demonstrates a need.

## 17. Geometry-preserving knot-insertion parity

Focused tests must construct a three-or-more-span general NURBS from a frozen
fixed two-span NURBS by **test-only homogeneous knot insertion**.

The test helper may:

- insert one new simple knot strictly inside an existing span;
- create the corresponding additional homogeneous control;
- dehomogenize into controls/weights for the general family.

The represented physical curve must remain unchanged.

Required comparison:

- value;
- D1;
- D2;
- both original and newly inserted interior knots.

Production knot insertion is explicitly not authorized.

## 18. Independent rational-basis oracle

Focused tests must compute an independent reference through polynomial
Cox-de Boor basis functions and rational normalization.

The oracle must not reuse production span-location or de Boor helpers.

At minimum cover:

- four or more spans;
- nonuniform interior-knot spacing;
- asymmetric 2D controls;
- nonuniform positive weights;
- one parameter strictly inside every span;
- every interior knot;
- value;
- D1;
- D2.

This is the primary multi-span numerical reference.

## 19. Local-support evidence

For a query in span `j`, only four original degree-three controls/weights have
support.

Focused evidence must perturb controls and weights that are strictly outside
that support and verify unchanged query results.

At least one fixture must use enough spans that there are out-of-support data
on **both** sides of the queried active set.

The production algorithm must not globally scan or globally normalize all
weights per query.

## 20. Weight-scale invariance

A positive common scale applied to every weight must leave value/D1/D2
unchanged within the declared numerical contract.

Focused evidence must include:

- exact power-of-two common scaling;
- strongly unbalanced finite positive weights;
- a case where naïve unscaled homogeneous products would be vulnerable to
  overflow/underflow but local scale-aware computation remains representable.

Stored public weights remain the exact input values.

## 21. Reversal

Reversal preserves the physical locus and domain `[a,b]`.

Required reversed representation:

- controls reversed;
- weights reversed;
- interior-knot sequence reflected and reversed;
- endpoint knots remain `a,b`.

For each original interior knot `ki`, reflected value is obtained through the
common overflow-aware reversed-parameter primitive.

Required relations:

- reversed value at mapped parameter equals original value;
- reversed D1 is the negative original D1;
- reversed D2 equals original D2;
- double reversal recovers the exact stored representation.

No naïve `a+b-u` is permitted where the common primitive avoids intermediate
overflow.

## 22. Degenerate and constant semantics

A curve with every control point identical and arbitrary admitted positive
weights is representable.

Required behavior:

- exact common point for every valid query where exact identity is preserved;
- exact zero D1/D2;
- reversal preserves geometry.

No regularity certification is introduced.

## 23. Affine, embedding and determinism evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two coordinate scaling where representable;
- 2D/3D embedding parity for `z=0`;
- deterministic repeated value/D1/D2 success;
- deterministic repeated typed failures.

No arbitrary-angle frame qualification is implied.

## 24. Extreme finite evidence

At least one fixture must combine:

- extreme finite coordinate magnitudes;
- strongly unbalanced positive weights;
- extreme finite knot values;
- more than two spans.

Required outcome:

- succeed where final value/D1/D2 is representable and scale-aware local
  computation can preserve it;
- otherwise fail explicitly with
  `CurveError::non_finite_result`.

No universal epsilon or silent fallback is permitted.

## 25. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | 2D/3D general NURBS satisfy existing bounded-parametric concepts. |
| Minimum size | Fewer than 5 controls rejected. |
| Size relation | controls == weights == interior_knots+4. |
| Knot validation | Endpoints/interiors finite and strictly ordered. |
| Weight validation | NaN/infinite/zero/negative weights rejected. |
| Exact domain/endpoints | Exact `[a,b]`, P0/Pn endpoints. |
| Span search | Every span and interior-knot right-span rule validated. |
| Fixed-family parity | General two-span case matches `TwoSpanCubicNURBS2/3`. |
| Polynomial subset | Equal weights match fixed B-spline and independent multi-span polynomial oracle. |
| Knot insertion parity | Test-only insertion adds span without changing geometry. |
| Independent rational basis | Value/D1/D2 across >=4 spans and all interior knots. |
| Local support | Distant controls+weights do not affect unrelated span. |
| Weight scale | Common positive scaling preserves value/D1/D2. |
| Reversal | Data reflection, involution and value/D1/D2 covariance. |
| Constant curve | Exact point and zero D1/D2. |
| Translation/scale | Declared affine covariance. |
| 2D/3D embedding | x/y parity and zero z. |
| Extreme finite | No avoidable overflow; explicit unrepresentable-result failure. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |
| Prerequisite preservation | Existing 23 ordinary tests remain passing plus one focused multi-span contract. |

If exactly one new focused test is added, ordinary FAST/INTEGRATION inventory
becomes **24 tests**.

## 26. Why span-count expansion is selected now

The fixed two-span work units already prove:

- simple cubic knot continuity;
- local support;
- homogeneous de Boor evaluation;
- positive-weight rational normalization;
- D1/D2;
- reversal;
- scale-aware finite behavior.

The next unresolved spline question is therefore not another formula. It is
whether those semantics remain correct when the representation owns an
arbitrary finite number of spans and must locate the active span
deterministically.

This is the smallest direct step toward practical CAD spline data and later
surface control nets.

## 27. Why arbitrary-degree Bézier is deferred

Arbitrary-degree Bézier would resolve variable degree/control count for a
single globally supported span.

It is useful, but it does not exercise:

- multi-span ownership;
- knot search;
- local support over many spans;
- the exact storage pattern required by future NURBS surfaces.

It remains a later breadth candidate.

## 28. Why repeated-knot continuity is deferred

Repeated interior knots change continuity.

For cubic curves:

- multiplicity 1 -> C2;
- multiplicity 2 -> C1;
- multiplicity 3 -> C0.

Admitting them would force a new decision on:

- whether D1/D2 fail at insufficient-continuity knots;
- whether left/right derivative queries are exposed;
- how exact knot-side selection interacts with the common curve contract.

Those semantics are independent of span-count storage and remain deferred.

## 29. Why analytic conic is deferred

A dedicated analytic circle/ellipse family in 3D needs a general supporting
plane/orientation representation and periodic angular semantics.

The currently qualified Cartesian Frames claim intentionally does not provide
arbitrary-angle frame qualification.

Rational control geometry already represents exact conic segments, so bypassing
that prerequisite is not justified.

## 30. Why heterogeneous composition/polycurve is deferred

A polycurve must choose runtime family ownership/dispatch and join semantics,
including at least:

- closed variant vs type erasure vs virtual polymorphism;
- segment ownership;
- orientation of each segment;
- parameter allocation across segments;
- C0/C1 compatibility policy at joins.

That architectural decision should be made after the concrete family set is
more stable.

## 31. Relationship to Surface Representation

This work unit is deliberately a **curve-only prerequisite** for surfaces.

It may establish reusable concepts for:

- variable control storage;
- variable knot storage;
- deterministic span location;
- local homogeneous evaluation.

It does not authorize:

- tensor-product control grids;
- U/V parameter domains;
- U/V derivatives;
- NURBS surfaces;
- analytic planes/cylinders/spheres;
- surface trimming;
- curve-on-surface semantics;
- topology/surface binding.

Surface Representation remains a separate Scientific Stage entry decision.

## 32. Explicit exclusions

This decision does not authorize:

- degree other than three;
- fewer than two spans;
- repeated interior knots;
- periodic NURBS;
- zero/negative weights;
- production knot insertion/removal;
- degree elevation/reduction;
- interpolation/fitting;
- arbitrary-degree polynomial/rational Bézier;
- dedicated analytic conic classes;
- heterogeneous composition/polycurve;
- general regularity/length/curvature refactors;
- any surface representation;
- boundary discretization;
- sizing/meshing;
- Quad-Dominant work;
- parallel execution;
- third-party runtime dependencies;
- representation-breadth qualification.

## 33. Repository mapping for future implementation

Authorized future mapping is limited to:

- public family extension:
  `include/apmesh/geometry/nurbs.hpp`;
- new production translation unit:
  `src/geometry/multi_span_nurbs.cpp`;
- focused semantic/reference contract:
  `tests/multi_span_cubic_nurbs.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

The existing fixed `src/geometry/nurbs.cpp` semantics should remain frozen.
A mechanical declaration-only/header extension is allowed, but no fixed-family
scientific behavior may be refactored merely for code reuse.

No change to `parametric_curve.hpp` is expected.

If the common concept must change, stop and require a new decision.

## 34. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 23 ordinary semantic tests must remain passing;
- the new multi-span focused contract must pass.

Passing yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify general arbitrary-degree/multiplicity/periodic NURBS,
NURBS surfaces or the representation-breadth stage.

## 35. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricCurve2/3`;
- degree variation;
- repeated knot multiplicity;
- periodicity;
- non-positive weights;
- one-span rational cubic Bézier admission;
- a custom allocator or third-party container;
- a second new representation family;
- surface/topology/meshing code;
- a universal tolerance;
- production knot mutation.

## 36. Planned sequence after this work unit

Planning only, not authorization.

After multi-span cubic NURBS implementation/closure, a fresh decision should
recompare:

1. repeated-knot/continuity breadth;
2. arbitrary-degree polynomial/rational Bézier and/or arbitrary spline degree;
3. analytic conic after arbitrary 3D placement prerequisites;
4. heterogeneous composition/polycurve;
5. whether curve breadth is then sufficient to prepare the separate Surface
   Representation entry decision.

No option is pre-authorized.

## 37. Effect if integrated and closed

After decision integration, post-merge validation and a separate decision
checkpoint closure, the sole next implementation work item becomes:

**Multi-Span Clamped Cubic Positive-Weight NURBS Representation in 2D and 3D
with Simple Interior Knots.**

No implementation begins on this decision branch.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization and Surface Representation remain blocked until
their respective prerequisite decisions are explicitly opened and closed.


## 38. Decision integration checkpoint

PR #128 integrated this bounded decision as
`77a7773cdb431469402f773b52c70d171d805201`.

Final decision-head validation:

- FAST `35749650284`: PASS;
- INTEGRATION `35749650175`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35749735576`: PASS;
- INTEGRATION `35749735502`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Multi-Span Clamped Cubic Positive-Weight NURBS Representation in 2D and 3D
with Simple Interior Knots.**

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the implementation bounded by Sections 5–35.

Arbitrary degree, repeated knots, periodicity, analytic conics, heterogeneous
composition, surfaces and downstream meshing remain unauthorized.


## 39. Decision closure checkpoint

Decision closure PR #129 merged as
`4f59898b40717cef91ea0fbf70f72493d715d4a3`.

Closure PR validation:

- FAST `35749973962`: PASS;
- INTEGRATION `35749973927`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Closure post-merge validation:

- FAST `35750063544`: PASS;
- INTEGRATION `35750063493`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is the multi-span cubic positive-weight
NURBS family bounded by Sections 5–35.

No broader spline, conic, composition, surface or downstream capability is
authorized.


## 40. Active implementation validation

The bounded implementation is active on:

`curve/multi-span-cubic-nurbs`.

Candidate repository mapping:

- `include/apmesh/geometry/nurbs.hpp`;
- `src/geometry/multi_span_nurbs.cpp`;
- `tests/multi_span_cubic_nurbs.cpp`;
- `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

The fixed `src/geometry/nurbs.cpp` implementation and
`include/apmesh/geometry/parametric_curve.hpp` remain unchanged.

Candidate head:

`78e086aa6744fb9bcdb2c5077b55b4122f883836`.

Candidate validation:

- FAST `35751863096`: PASS, 24/24 ordinary semantic tests;
- INTEGRATION `35751863450`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 24/24 tests per cell;
- `apmesh_core.multi_span_cubic_nurbs`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Validated candidate scope includes:

- runtime-variable control/weight/simple-knot count;
- deterministic right-span selection;
- local homogeneous value/D1/D2;
- fixed two-span NURBS parity in 2D/3D;
- polynomial B-spline subset parity;
- independent four-span rational-basis oracle;
- test-only geometry-preserving knot insertion;
- two-sided local support;
- weight-scale invariance;
- reversal/involution and derivative covariance;
- constant-curve semantics;
- translation and coordinate-scale covariance;
- 2D/3D embedding parity;
- extreme-finite success;
- explicit unrepresentable-result failure;
- deterministic repeat evidence.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**

No broader spline, conic, composition, surface or downstream capability is
implied.


## 41. Implementation integration checkpoint

Implementation PR #130 merged as
`153bf6b874b0deac304ea07562cd897785f631df`.

Validation lineage:

- first complete candidate head:
  `78e086aa6744fb9bcdb2c5077b55b4122f883836`;
- candidate FAST `35751863096`: PASS, 24/24 ordinary tests;
- candidate INTEGRATION `35751863450`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug, 24/24 tests per cell;
- final documentation-synchronized head:
  `3a600bba2b521ba4fea12be0b85dd55161205f15`;
- final PR FAST `35752117689`: PASS, 24/24;
- final PR INTEGRATION `35752117850`: PASS in GCC 13 Debug and Clang 18
  libc++ Debug, 24/24 tests per cell;
- post-merge FAST `35752335649`: PASS, 24/24;
- post-merge INTEGRATION `35752335629`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug, 24/24 tests per cell.

Integrated production scope:

- `MultiSpanCubicNURBS2`;
- `MultiSpanCubicNURBS3`;
- degree 3;
- runtime-variable control/weight/simple-knot count;
- at least two spans;
- clamped endpoints and simple interior knots;
- finite strictly positive weights;
- immutable owning vectors/read-only spans;
- exact right-span search;
- local homogeneous value/D1/D2;
- no per-query allocation;
- no global weight normalization after span location.

Focused evidence includes every Section 25 category.

The common `BoundedParametricCurve2/3` concepts and fixed
`src/geometry/nurbs.cpp` production semantics remain unchanged.

Work-unit result before closure:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

This does not qualify repeated knots, arbitrary degree, periodic NURBS,
analytic conics, heterogeneous composition, NURBS surfaces or the breadth
stage.

A separate implementation closure is mandatory before any next breadth
decision.
