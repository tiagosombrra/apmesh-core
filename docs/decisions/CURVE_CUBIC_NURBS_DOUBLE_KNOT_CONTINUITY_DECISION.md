# Cubic NURBS Double-Knot C1 Continuity — Bounded Breadth Decision

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
  integrated focused work unit;
- fixed two-span cubic positive-weight NURBS 2D/3D:
  integrated focused work unit;
- multi-span clamped cubic positive-weight NURBS 2D/3D with simple interior
  knots: integrated and terminally closed focused work unit.

## 1. Question

What is the next smallest curve-representation breadth step after runtime
multi-span cubic NURBS is available with only simple interior knots?

The required comparison includes:

1. repeated-knot / continuity breadth;
2. arbitrary-degree polynomial/rational Bézier and/or arbitrary spline degree;
3. analytic conics after arbitrary 3D supporting-plane/orientation semantics;
4. heterogeneous composition/polycurve;
5. whether curve breadth is already sufficient to open the separate
   Surface Representation stage.

The next work unit must resolve one semantic gap without silently weakening the
existing bounded-parametric contract or mixing curve and surface work.

## 2. Fresh repository evidence

Canonical decision-entry `main`:
`a37e8c266b0057b2d813f5f690faa6a1e6a710a1`.

Terminal multi-span NURBS lineage:

- implementation PR #130:
  `153bf6b874b0deac304ea07562cd897785f631df`;
- final implementation head:
  `3a600bba2b521ba4fea12be0b85dd55161205f15`;
- final PR FAST `35752117689`: PASS, 24/24;
- final PR INTEGRATION `35752117850`: PASS, 24/24 in GCC and Clang;
- implementation post-merge FAST `35752335649`: PASS, 24/24;
- implementation post-merge INTEGRATION `35752335629`: PASS, 24/24;
- implementation closure PR #131:
  `d71ada7b280b443c4eb303c44b57e7ce429fb24a`;
- closure post-merge FAST `35752844947`: PASS, 24/24;
- closure post-merge INTEGRATION `35752844817`: PASS, 24/24;
- terminal synchronization PR #132:
  `a37e8c266b0057b2d813f5f690faa6a1e6a710a1`;
- sync PR FAST `35753336320`: PASS;
- sync PR INTEGRATION `35753336303`: PASS;
- sync post-merge FAST `35753496462`: PASS;
- sync post-merge INTEGRATION `35753496400`: PASS.

Production curve breadth at entry includes:

- `CubicBezier2/3`;
- `LineSegment2/3`;
- `RationalQuadraticBezier2/3`;
- static `TrimmedCurve2/3`;
- `TwoSpanCubicBSpline2/3`;
- `TwoSpanCubicNURBS2/3`;
- `MultiSpanCubicNURBS2/3` for degree-three, positive-weight,
  non-periodic splines with simple interior knots.

The current `CurveError` has no explicit insufficient-continuity outcome.

No production surface type exists.

## 3. Literature and mature-kernel evidence

### 3.1 Knot multiplicity determines guaranteed continuity

Open CASCADE knot-splitting references:

- https://dev.opencascade.org/doc/refman/html/class_geom_convert___b_spline_curve_knot_splitting.html
- https://dev.opencascade.org/doc/refman/html/class_law___b_spline_knot_splitting.html

Relevant evidence:

- B-spline discontinuities are localized at knot values;
- between distinct knot values the spline is infinitely differentiable;
- at a knot, guaranteed continuity is `degree - multiplicity`;
- a degree-three spline with multiplicity one is C2;
- degree three with multiplicity two is C1;
- local derivative APIs are separated from global continuity assumptions.

Decision impact:

- the current simple-knot family covers only the C2 case;
- the next smallest multiplicity step is exactly **interior multiplicity two**;
- multiplicity three / C0 is a distinct later step because D1 itself ceases to
  be guaranteed.

### 3.2 D1 and D2 have different continuity requirements

Open CASCADE `Geom_Curve` reference:

https://dev.opencascade.org/doc/refman/html/class_geom___curve.html

Relevant evidence:

- D1 requires at least C1 continuity;
- D2 requires at least C2 continuity;
- continuity order is an explicit geometric contract rather than an epsilon
  inference.

Decision impact:

- a cubic double knot still admits a unique D1 by representation guarantee;
- D2 is not guaranteed to be unique at the double knot;
- AP Mesh must not silently choose a left or right second derivative for its
  ordinary `second_derivative(u)` query.

### 3.3 Surface splines have the same U/V multiplicity problem

Open CASCADE surface knot-splitting reference:

https://dev.opencascade.org/doc/refman/html/class_geom_convert___b_spline_surface_knot_splitting.html

Relevant evidence:

- B-spline surfaces localize continuity changes on U/V knot lines;
- continuity in each parametric direction is degree minus knot multiplicity;
- local derivative APIs exist when one needs derivatives within a continuity
  interval.

Decision impact:

- carrying unresolved repeated-knot derivative semantics into Surface
  Representation would duplicate the same ambiguity in two parameter
  directions;
- resolving the curve-level C1 case first is a direct prerequisite for a
  defensible bounded NURBS-surface entry.

### 3.4 Surface data models explicitly retain multiplicities

Open CASCADE B-spline surface/conversion references:

- https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html
- https://dev.opencascade.org/doc/refman/html/class_convert___cone_to_b_spline_surface.html

Relevant evidence:

- practical spline surfaces expose U/V knots and U/V multiplicities separately;
- rational weights and multiplicity are independent stored semantics.

Decision impact:

- Surface Representation is not yet the smallest next step;
- multiplicity must become explicit in the reusable 1D spline semantics first.

### 3.5 Polycurve and arbitrary-degree candidates remain distinct

CGAL polycurve reference:

https://doc.cgal.org/latest/Arrangement_on_surface_2/classCGAL_1_1Arr__polycurve__traits__2.html

Relevant evidence:

- a polycurve is a chain of continuous, well-oriented subcurves;
- composition introduces endpoint compatibility and runtime chain semantics
  independent of a single spline's knot continuity.

Open CASCADE Bézier reference:

https://dev.opencascade.org/doc/refman/html/class_geom___bezier_curve.html

Relevant evidence:

- arbitrary-degree/rational Bézier is a degree/control-count generalization of
  a globally supported single-span family.

Decision impact:

- neither candidate resolves the derivative ambiguity that already exists as
  soon as repeated spline knots are admitted.

External references are design/scientific evidence only. No external runtime
dependency is admitted.

## 4. Candidate comparison

| Candidate | New semantic burden | Immediate prerequisite value | Interaction with current D1/D2 contract | Surface relevance | Decision |
| --- | --- | --- | --- | --- | --- |
| Cubic NURBS interior multiplicity 2 / C1 | Multiplicity storage + explicit D2 failure at exact double knot | **Direct** next spline gap | **Must be resolved now** | **Direct U/V continuity precursor** | **SELECTED** |
| Arbitrary-degree Bézier / spline degree | Degree-dependent storage/evaluation | High but orthogonal | Does not resolve repeated-knot ambiguity | High later | DEFER |
| Analytic conics | Arbitrary 3D placement + periodic/angular semantics | Orthogonal | No | Important analytic surface precursor later | BLOCKED/DEFER |
| Heterogeneous polycurve | Runtime family storage/dispatch + join semantics | Orthogonal | Join continuity is a separate layer | Boundary-loop relevance later | DEFER |
| Open Surface Representation now | U/V grid + partials + multiplicities + trimming | Very high combined burden | Would duplicate unresolved knot-continuity policy | Ultimate target | **DEFER UNTIL C1 CURVE SEMANTICS CLOSE** |

## 5. Decision

Authorize exactly one future implementation work unit:

**Cubic Positive-Weight Multi-Span NURBS with Interior Knot Multiplicity One or
Two, with explicit C1/D2 semantics.**

This is an extension of the existing `MultiSpanCubicNURBS2/3` family, not a
new unrelated mathematical representation.

The implementation must preserve the existing simple-knot construction and
scientific outputs.

Newly admitted representation breadth:

- degree remains exactly 3;
- endpoint multiplicity remains exactly 4;
- each unique interior knot has multiplicity exactly 1 or 2;
- at least two distinct nonzero knot spans;
- finite strictly positive weights;
- non-periodic;
- bounded;
- runtime-variable controls/weights/unique interior knots;
- explicit interior multiplicities;
- ordinary value/D1/D2 query surface retained.

Multiplicity 3 / C0 and periodicity remain excluded.

## 6. Mathematical representation

Let unique interior knots be

`k1 < k2 < ... < kr`

with multiplicities

`m1,...,mr`

where each

`mi in {1,2}`.

Let finite endpoint knots satisfy

`a < k1 < ... < kr < b`.

The flat degree-three knot sequence is:

`[a,a,a,a, k1 repeated m1 times, ..., kr repeated mr times,
  b,b,b,b]`.

Define:

- unique interior-knot count: `r >= 1`;
- distinct nonzero span count: `S = r + 1 >= 2`;
- total interior flat multiplicity:
  `M = sum_i mi`;
- control count:
  `N = M + 4`;
- weight count:
  exactly `N`.

The rational curve equation remains unchanged.

## 7. Guaranteed continuity

For degree `p=3`:

- interior multiplicity 1 guarantees C2;
- interior multiplicity 2 guarantees C1.

The API uses **representation-guaranteed continuity**.

It does not inspect controls/weights numerically to infer accidental higher
smoothness.

Therefore a multiplicity-two knot is treated as C1 for ordinary derivative
semantics even if a special control configuration happens to make the physical
curve C2 or smoother.

No epsilon or derivative-comparison heuristic upgrades continuity.

## 8. Required common error extension

Authorize exactly one addition to the existing common query error vocabulary:

`CurveError::insufficient_continuity`.

This is a typed scientific/domain failure.

It is not:

- `singular_parameter`;
- `non_finite_result`;
- an exception;
- a tolerance-triggered classification.

The `BoundedParametricCurve2/3` concepts themselves remain unchanged.

No existing enumerator meaning changes.

## 9. Query semantics at simple knots

At an interior multiplicity-one knot:

- value succeeds;
- D1 succeeds;
- D2 succeeds;
- exact right-span location remains the deterministic implementation policy;
- results must preserve the current multi-span simple-knot behavior.

Existing simple-knot regression is a frozen prerequisite.

## 10. Query semantics at double knots

At an interior multiplicity-two knot `k`:

### Value

`evaluate(k)` succeeds when finite.

The curve is at least C1 and therefore continuous.

### D1

`first_derivative(k)` succeeds when finite.

Because C1 is guaranteed, left and right first derivatives are mathematically
identical in exact arithmetic.

Production may use the existing deterministic right-span policy.

### D2

`second_derivative(k)` returns:

`CurveError::insufficient_continuity`.

No second derivative side is selected implicitly.

The ordinary D2 query therefore represents a derivative guaranteed by the
representation at that exact parameter, not an arbitrary local one-sided
choice.

### Away from the double knot

For finite `u != k` inside either neighboring open span:

- value succeeds when representable;
- D1 succeeds when representable;
- D2 succeeds when representable.

The left and right second-derivative limits are allowed to differ.

## 11. Why D2 failure is parameter-local rather than global

Some mature curve APIs predicate D2 on global C2 continuity.

AP Mesh instead already exposes a parameter-specific expected-value query.

For this bounded work unit:

- a double knot does not disable D2 over all other smooth spans;
- failure occurs exactly at a parameter whose representation guarantee is below
  C2;
- this keeps valid local information available without silently inventing a
  side at the knot.

This is an explicit AP Mesh semantic decision, not a claim that another CAD
kernel uses the identical API.

## 12. No one-sided derivative API in this work unit

This decision does not authorize:

- `second_derivative_left(k)`;
- `second_derivative_right(k)`;
- generic span-local derivative APIs;
- side-selection enum parameters.

Those may be introduced later if Surface Representation or discretization
requires explicit one-sided jets.

The first task is to make ordinary D2 failure unambiguous and typed.

## 13. Storage extension

The existing multi-span family should retain unique interior knot storage and
add explicit multiplicity storage.

Conceptual fields:

- `std::vector<Point2/Point3> control_points`;
- `std::vector<double> weights`;
- `std::vector<double> interior_knots`;
- `std::vector<std::uint8_t> interior_multiplicities`;
- `a`, `b`;
- constant-curve cache/flag if already used.

Read-only public accessor:

`std::span<const std::uint8_t> interior_multiplicities() const noexcept`.

The existing simple-knot factory must remain source-compatible and behave as if
all interior multiplicities equal one.

A new validated overload/factory may accept explicit multiplicities.

No custom allocator or third-party container is introduced.

## 14. Construction validation

New construction rules:

- multiplicity count equals unique interior-knot count;
- every multiplicity is exactly 1 or 2;
- control count equals
  `4 + sum(interior_multiplicities)`;
- weight count equals control count;
- all existing knot-order, finiteness and positive-weight rules remain.

New bounded construction errors may include:

- `interior_multiplicity_count_mismatch`;
- `unsupported_interior_multiplicity`.

Existing error values retain their meaning.

No epsilon changes knot equality or multiplicity.

## 15. Span count and flat-knot semantics

`span_count()` continues to mean the number of distinct nonzero parameter
spans:

`unique_interior_knots + 1`.

It does not count repeated flat-knot entries as additional spans.

Production may construct/access flat-knot indices logically from
(unique knot, multiplicity) storage without materializing a new vector per
query.

No per-query dynamic allocation is allowed.

## 16. Span selection

Existing exact span selection remains:

- `u == b` -> final span;
- otherwise the unique span whose upper unique knot is the first knot strictly
  greater than `u`;
- exact interior-knot query -> right span.

For D2:

- before ordinary local evaluation, if `u` is exactly a multiplicity-two
  interior knot, return `insufficient_continuity`.

No tolerance participates.

## 17. Simple-knot backward compatibility

The existing factory/API path with no explicit multiplicity vector must remain
scientifically unchanged.

Required exact/focused evidence:

- all-simple construction produces multiplicity-one storage;
- control/weight/knot accessors retain existing values;
- span count unchanged;
- value/D1/D2 results unchanged;
- reversal unchanged;
- typed failures unchanged;
- current multi-span test remains passing.

No existing simple-knot baseline is rewritten as a different mathematical
algorithm merely to simplify implementation.

## 18. Double-knot independent reference

Focused tests must include a generic 2D cubic rational fixture with at least:

- one multiplicity-two interior knot;
- one multiplicity-one interior knot;
- nonuniform knot spacing;
- nonuniform positive weights;
- enough controls implied by the flat-knot count.

An independent Cox-de Boor/rational basis oracle using a flat knot vector with
duplicates must validate:

- value inside every span;
- D1 inside every span;
- D2 inside every open span;
- value at each interior knot;
- D1 at each interior knot;
- D2 at simple knots.

At the double knot, the oracle must additionally show:

- left/right value agreement;
- left/right D1 agreement within the declared reference contract;
- a fixture whose left/right D2 values are materially different, establishing
  that a unique ordinary D2 cannot be assumed.

Production must return `insufficient_continuity` there.

## 19. Geometry-preserving repeated-knot insertion oracle

Focused test code may take an existing simple-knot multi-span NURBS and insert
the **same existing knot once** in homogeneous coordinates.

This produces a multiplicity-two representation of the same physical curve.

Required parity:

- value parity everywhere;
- D1 parity everywhere;
- D2 parity away from the repeated knot;
- at the repeated knot:
  original simple representation D2 succeeds;
  repeated-knot representation ordinary D2 returns
  `insufficient_continuity`.

This deliberately demonstrates the distinction between:

- physical curve smoothness for one special representation; and
- continuity guaranteed by the repeated-knot representation.

No production knot insertion is authorized.

## 20. Reversal

Reversal must:

- reverse controls;
- reverse weights;
- reflect and reverse unique interior knots;
- reverse the multiplicity sequence correspondingly.

For common mapped parameter `r(u)`:

- value covariance remains;
- D1 sign covariance remains where D1 succeeds;
- D2 covariance remains where D2 succeeds;
- a double knot maps to a double knot and D2 must fail with
  `insufficient_continuity` on both representations.

Double reversal recovers exact stored representation.

## 21. Constant-curve semantics

A constant control net with double knots is admitted.

Value and D1 remain exact where current implementation preserves exactness.

At a double knot, ordinary D2 still returns
`insufficient_continuity` even though the physical constant curve has zero
derivatives of all orders.

This is intentional: query success follows representation-guaranteed
continuity, not accidental control-data analysis.

Away from double knots, D2 is exact zero.

## 22. Weight, affine and embedding invariants

All current multi-span invariants remain:

- common positive weight-scale invariance;
- translation covariance;
- exact power-of-two coordinate-scale covariance where representable;
- 2D/3D embedding parity;
- deterministic repeat success/failure.

Multiplicity metadata must not alter these relations.

## 23. Extreme finite evidence

At least one repeated-knot fixture must combine:

- extreme finite knot scales;
- positive highly unbalanced weights;
- representable value/D1 away from and at a double knot;
- representable D2 away from the double knot;
- exact `insufficient_continuity` at the double knot.

Existing `non_finite_result` behavior for truly unrepresentable results
remains distinct from continuity failure.

## 24. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept preservation | Extended 2D/3D types still satisfy unchanged bounded concepts. |
| Legacy factory | Existing simple-knot factory remains source/semantic compatible. |
| Multiplicity storage | Read-only unique-knot multiplicities preserved exactly. |
| Construction validation | Count mismatch and multiplicities other than 1/2 rejected. |
| Size relation | controls == weights == 4 + sum(multiplicities). |
| Span count | Counts distinct nonzero spans, not repeated flat entries. |
| Simple-knot parity | Existing 24 ordinary tests remain passing. |
| Independent double-knot oracle | Value/D1/D2 verified over open spans and allowed knots. |
| C1 knot value | Value succeeds at multiplicity-two knot. |
| C1 knot D1 | D1 succeeds and left/right references agree. |
| C1 knot D2 | Exact typed `insufficient_continuity`. |
| One-sided evidence | Test fixture demonstrates differing left/right D2 limits. |
| Repeated-knot insertion | Test-only insertion preserves geometry but changes guaranteed D2 semantics. |
| Reversal | Multiplicities reverse; success/failure covariance preserved. |
| Local support | Multiplicity does not introduce distant-data leakage. |
| Weight scale | Positive common scaling preserves valid jets. |
| Constant curve | D2 still fails exactly at double knot by representation policy. |
| Affine/embedding | Translation, scale, 2D/3D parity preserved. |
| Extreme finite | Continuity failure remains distinct from numeric failure. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |

If exactly one new focused test is added, ordinary FAST/INTEGRATION inventory
becomes **25 tests**.

## 25. Why multiplicity two is selected before multiplicity three

For cubic degree three:

- multiplicity two -> C1: D1 remains guaranteed, only D2 semantics change;
- multiplicity three -> C0: even D1 is no longer guaranteed.

Adding both in one work unit would simultaneously introduce two derivative
failure levels and likely one-sided D1/D2 needs.

The double-knot C1 case isolates exactly one new semantic boundary.

## 26. Why this precedes Surface Representation

A NURBS surface has independent knot/multiplicity structures in U and V.

If repeated-knot derivative semantics are unresolved at the curve level, a
surface implementation must invent rules for:

- first partials on reduced-continuity knot lines;
- second partials on those lines;
- mixed partials at U/V knot-line intersections;
- side selection.

That would duplicate unresolved 1D policy in a more complex stage.

Therefore Surface Representation remains blocked until at least this C1 curve
semantics work unit is integrated and closed.

This decision does not claim that every curve family must be complete before
any surface work. It identifies this specific continuity gap as a direct
surface prerequisite.

## 27. Why arbitrary degree is deferred

Arbitrary degree changes:

- degree storage;
- derivative degree;
- active-control count;
- de Boor work size;
- degree/multiplicity continuity relationship.

The current problem is already observable at fixed degree three and should be
resolved without simultaneously generalizing degree.

## 28. Why analytic conics are deferred

Dedicated 3D analytic conics still require arbitrary supporting-plane/frame
semantics and periodic/angular parameterization beyond the qualified
signed-permutation Cartesian-frame envelope.

Rational spline geometry already supplies conic-capable control
representations, so bypassing that prerequisite is not necessary here.

## 29. Why heterogeneous composition is deferred

Polycurve composition requires:

- runtime storage/dispatch among family types;
- endpoint compatibility;
- segment orientation;
- global parameter allocation;
- join-continuity policy.

Those are distinct from the internal continuity of one spline representation.

The CGAL model confirms that continuous/well-oriented subcurve chaining is its
own semantic layer.

## 30. Repository mapping for future implementation

Authorized mapping is limited to:

- common error-vocabulary extension only:
  `include/apmesh/geometry/parametric_curve.hpp`;
- public NURBS API/storage:
  `include/apmesh/geometry/nurbs.hpp`;
- existing multi-span production implementation:
  `src/geometry/multi_span_nurbs.cpp`;
- focused semantic/reference contract:
  `tests/cubic_nurbs_double_knot_continuity.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

The fixed two-span sources remain frozen.

No surface source/header may be added.

## 31. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 24 ordinary semantic tests must remain passing;
- the new double-knot continuity focused contract must pass.

Passing yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify:

- multiplicity-three C0 splines;
- arbitrary-degree NURBS;
- periodic NURBS;
- analytic conics;
- heterogeneous polycurves;
- NURBS surfaces;
- the representation-breadth stage.

## 32. Stop conditions

Stop and require a new decision if implementation needs:

- changing the `BoundedParametricCurve2/3` concept signatures;
- multiplicity three or greater;
- arbitrary degree;
- periodicity;
- non-positive weights;
- one-sided derivative public APIs;
- production knot insertion/removal;
- custom allocators;
- a second new curve family;
- any surface/topology/meshing code;
- a universal tolerance;
- continuity inferred numerically from controls.

## 33. Planned sequence after this work unit

Planning only, not authorization.

After the C1 double-knot work unit is integrated and closed, a fresh decision
must compare:

1. whether curve breadth is now sufficient for a bounded Surface
   Representation entry decision;
2. multiplicity-three / C0 curve semantics;
3. arbitrary-degree spline/Bézier breadth;
4. analytic conics after arbitrary-placement prerequisites;
5. heterogeneous composition/polycurve.

No winner is pre-authorized.

## 34. Effect if integrated and closed

After decision integration, post-merge validation and separate decision
checkpoint closure, the sole next implementation work item becomes:

**Cubic Positive-Weight Multi-Span NURBS with Interior Knot Multiplicity One or
Two and explicit C1/D2 failure semantics.**

No implementation begins on this decision branch.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization remains blocked.

Surface Representation remains blocked until this continuity work unit closes
and a separate surface-readiness/entry decision explicitly opens it.


## 35. Decision integration checkpoint

PR #133 integrated this bounded decision as
`ea65372bb6a9ed9a6bde94a3e3eed551e96fd3b9`.

Final decision-head validation:

- FAST `35754225028`: PASS;
- INTEGRATION `35754225224`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35754389148`: PASS;
- INTEGRATION `35754389151`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and post-merge validation, the sole next work item is
the implementation bounded by Sections 5–32.

No multiplicity-three, arbitrary-degree, periodic, conic, composite, surface
or downstream capability is authorized.


## 36. Decision closure checkpoint

Decision closure PR #134 merged as
`8e47a35922f5f0dd246b238294e58980baf91277`.

Closure post-merge validation:

- FAST `35754917335`: PASS;
- INTEGRATION `35754917556`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is the multiplicity-1/2 cubic
positive-weight multi-span NURBS extension bounded by Sections 5–32.

No multiplicity-three, arbitrary-degree, periodic, conic, composite, surface
or downstream capability is authorized.


## 37. Active implementation mapping

The sole authorized implementation is active on:

`curve/cubic-nurbs-double-knot-continuity`.

Candidate mapping:

- common error vocabulary:
  `include/apmesh/geometry/parametric_curve.hpp`;
- public multiplicity API/storage:
  `include/apmesh/geometry/nurbs.hpp`;
- production:
  `src/geometry/multi_span_nurbs.cpp`;
- focused contract:
  `tests/cubic_nurbs_double_knot_continuity.cpp`;
- build/test registration:
  `CMakeLists.txt`.

Candidate implementation preserves the existing simple-knot factory and adds:

- explicit multiplicities restricted to one/two;
- construction-time flat-knot cache;
- exact right-span selection on the cached flat sequence;
- parameter-local `CurveError::insufficient_continuity` for ordinary D2 at
  multiplicity-two knots;
- unchanged value/D1 at those knots;
- no one-sided derivative API.

The focused contract includes an independent flat-knot Cox-de Boor rational
oracle, a generic C1 fixture with materially distinct one-sided D2, test-only
homogeneous insertion of an already-simple knot, reversal/multiplicity
reflection, 2D/3D embedding and accidental-smoothness protection.

Expected ordinary semantic inventory: **25 tests**.

Candidate validation:

- candidate head:
  `e13a06feb01a11a18f17495f6b9a0f8cd4c6f038`;
- FAST `35756479210`: PASS, 25/25 ordinary semantic tests;
- INTEGRATION `35756479108`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 25/25 tests per cell;
- `apmesh_core.cubic_nurbs_double_knot_continuity`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**


## 38. Implementation integration checkpoint

The bounded implementation was integrated by PR #135 as
`eb62de8b4c7b09c671801e4b54c04e5d62dde0a9`.

Candidate validation:

- candidate head:
  `e13a06feb01a11a18f17495f6b9a0f8cd4c6f038`;
- FAST `35756479210`: PASS, 25/25;
- INTEGRATION `35756479108`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 25/25 tests per cell.

Final PR-head validation:

- final head:
  `ba0f5ec0ee038f176cc9abc405e2aa879a2e9b95`;
- FAST `35756643568`: PASS, 25/25;
- INTEGRATION `35756643616`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 25/25 tests per cell.

Post-merge validation:

- FAST `35756910304`: PASS, 25/25;
- INTEGRATION `35756910340`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 25/25 tests per cell.

Integrated result:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSURE PENDING / NOT QUALIFIED.**

The implementation preserves simple-knot behavior, adds only multiplicities
1/2 and `CurveError::insufficient_continuity`, and keeps every broader
curve/surface capability outside scope.

After closure, a fresh decision must compare Surface Representation readiness
against the remaining curve-breadth candidates. No winner is pre-authorized.


## 39. Implementation closure checkpoint

Implementation closure PR #136 used head
`bb5c524bc1642376689c6b8aa1a845364844de09`.

Closure PR validation:

- FAST `35757407115`: PASS;
- INTEGRATION `35757407483`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #136 merged as
`affc7a46b7fd4c5cc419679e6192fc293501654e`.

Closure post-merge validation:

- FAST `35757554252`: PASS;
- INTEGRATION `35757554247`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Terminal result:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
NOT QUALIFIED.**

No implementation work item remains active.

The sole next admissible work is a fresh literature-backed decision comparing
Surface Representation entry readiness against multiplicity-three/C0,
arbitrary-degree spline/Bézier breadth, analytic conics and heterogeneous
composition/polycurve. No candidate is pre-authorized.
