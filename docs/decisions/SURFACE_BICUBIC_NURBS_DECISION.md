# Bicubic Positive-Weight NURBS Surface — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites:

- Surface Representation stage: OPEN / NOT QUALIFIED;
- `BicubicBezierPatch3`: integrated focused work unit;
- `RationalBicubicBezierPatch3`: integrated and terminally closed focused work unit;
- common `BoundedParametricSurface3` contract: integrated;
- cubic positive-weight multi-span NURBS curves with simple/double interior
  knot support: integrated focused work units;
- Curve Representation CGR0–CGR7 qualification remains limited to polynomial
  cubic Bézier curves and is not widened by this decision.

## 1. Question

Which Surface Representation breadth family should be introduced next after the
positive-weight rational bicubic Bézier patch?

Candidates:

1. cubic B-spline/NURBS surface;
2. Coons/transfinite patch;
3. analytic elementary surfaces;
4. ruled/extrusion/revolution surfaces;
5. rectangular/general trimmed-surface semantics.

The selected work unit must advance the CAD/free-form surface model while
introducing only one new scientific seam.

## 2. Fresh repository entry authority

Canonical decision-entry main:

`b1537c0697604ee5bc37ac46f25bb271b9be52a1`.

Terminal rational-bicubic lineage:

- implementation PR #145:
  `8ac1abd913bf15ff1dc4d60595f809491902c055`;
- implementation post-merge FAST `35783312495`: PASS, 27/27;
- implementation post-merge INTEGRATION `35783312402`: PASS, 27/27;
- closure PR #146:
  `c7f7b32b180082421cadc74c39d2919f713ec775`;
- closure post-merge FAST `35783715705`: PASS;
- closure post-merge INTEGRATION `35783715671`: PASS;
- terminal reconciliation PR #147:
  `6577683e65a35d802a05f812e2763b738094c87f`;
- PR #147 FAST `35796641879`: PASS;
- PR #147 INTEGRATION `35796641967`: PASS;
- PR #147 post-merge FAST `35796719734`: PASS;
- PR #147 post-merge INTEGRATION `35796719674`: PASS;
- compact checkpoint normalization PR #148:
  `b1537c0697604ee5bc37ac46f25bb271b9be52a1`;
- PR #148 FAST `35797027086`: PASS;
- PR #148 INTEGRATION `35797027093`: PASS;
- PR #148 post-merge FAST `35797161908`: PASS;
- PR #148 post-merge INTEGRATION `35797161957`: PASS.

Ordinary semantic inventory at entry: **27 tests**.

No production work item or open PR exists at decision entry.

## 3. Literature and mature-kernel basis

### 3.1 Tensor-product B-spline/NURBS surfaces are the direct free-form extension

Open CASCADE Technology 8.0.1 `Geom_BSplineSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html

MIT Hyperbook — B-spline surfaces:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node19.html

Relevant evidence:

- a B-spline surface is a tensor product over a rectangular control net;
- U and V have independent knot vectors and spline basis functions;
- rationality adds a weight net without changing the tensor-product surface
  identity;
- isoparametric lines are spline curves in the corresponding direction.

Decision impact:

- the integrated rational bicubic patch already establishes positive-weight
  homogeneous surface evaluation;
- the integrated curve NURBS family already establishes deterministic
  span-selection and cubic knot semantics;
- the next bounded seam can therefore isolate **runtime U/V span counts and
  simple U/V knot vectors**.

### 3.2 Knot multiplicity is an independent continuity decision

MIT Hyperbook — B-spline basis properties:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node16.html

Project relevance:

- degree and knot multiplicity determine continuity independently in each
  parametric direction;
- simple interior knots in cubic splines preserve C2;
- admitting double surface knots would introduce lines where `S_uu` or
  `S_vv` is not representation-guaranteed and would require a new
  surface-specific continuity/failure decision.

Decision impact:

- this work unit admits **simple interior knots only**;
- existing `SurfaceSecondDerivatives3` semantics remain valid everywhere.

### 3.3 Coons/transfinite filling is a boundary-construction problem

Open CASCADE `GeomFill_Coons` and `GeomFill_BSplineCurves`:

https://dev.opencascade.org/doc/refman/html/class_geom_fill___coons.html

https://dev.opencascade.org/doc/refman/html/_geom_fill___b_spline_curves_8hxx.html

Relevant evidence:

- Coons-style filling is defined from boundary curves and blending/filling
  rules;
- boundary compatibility and corner consistency are intrinsic to the
  construction;
- it is a different semantic seam from owning/evaluating an existing NURBS
  surface.

Decision impact:

- Coons/transfinite remains explicit future work;
- it is not required before a NURBS value representation.

### 3.4 Analytic elementary surfaces require placement and periodic semantics

Open CASCADE elementary surface family:

https://dev.opencascade.org/doc/refman/html/class_geom___elementary_surface.html

Relevant evidence:

- plane/cylinder/cone/sphere/torus use explicit local coordinate systems;
- surfaces of revolution have angular/periodic parameter semantics;
- their natural domains and singularities differ materially from bounded
  clamped NURBS patches.

Decision impact:

- analytic surfaces remain a separate family decision;
- the currently bounded NURBS seam does not require arbitrary-placement or
  periodic-angle policy.

### 3.5 Swept surfaces are generator/trajectory constructions

Open CASCADE linear extrusion and revolution surfaces:

https://dev.opencascade.org/doc/refman/html/class_geom___surface_of_linear_extrusion.html

https://dev.opencascade.org/doc/occt-7.8.0/refman/html/Geom__SurfaceOfRevolution_8hxx.html

Relevant evidence:

- swept surfaces are defined from a basis curve plus an extrusion direction or
  axis/revolution law;
- their parameter domains inherit semantics from generator curves and sweep
  parameters.

Decision impact:

- swept surfaces depend on representation composition rather than the new
  U/V-knot seam;
- defer them until the supporting curve/placement contract is deliberately
  selected.

### 3.6 Trimming remains distinct from the supporting surface

Open CASCADE `Geom_RectangularTrimmedSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___rectangular_trimmed_surface.html

Relevant evidence:

- a trimmed surface is a portion of a basis surface with its own parameter
  bounds/orientation;
- general trimming later also requires parameter-space boundary curves and
  topology identity.

Decision impact:

- trimming should wrap a mature supporting surface family;
- it must not be conflated with NURBS evaluation/storage.

External sources are design/reference evidence only. No external CAD kernel is
a runtime dependency or numerical oracle.

## 4. Candidate comparison

| Candidate | New semantics now | Reuses integrated prerequisites | Downstream value | Principal extra burden | Decision |
| --- | --- | --- | --- | --- | --- |
| Bicubic positive-weight NURBS surface with simple knots | Runtime rectangular control/weight net, independent U/V knots and span search | Rational bicubic surface + cubic NURBS curves | **Very high** for CAD/free-form surfaces | Dynamic tensor-product local support | **SELECTED** |
| Coons/transfinite | Boundary-driven construction/filling | Curve families + surface contract | High for AP Mesh patch construction | Boundary compatibility and blending policy | DEFER |
| Analytic elementary | Placement/orientation, angular/periodic domains, singular charts | Geometry primitives | High for CAD validation breadth | New placement/domain semantics | DEFER |
| Ruled/extrusion/revolution | Generator ownership + sweep law/domain | Curve families | High | Composition and unbounded/periodic sweep semantics | DEFER |
| Rectangular/general trimming | Supporting-surface wrapper, trim orientation/topology | Mature surface family + curve-on-surface | Very high for CAD faces | Topology/parameter-boundary semantics | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Clamped Bicubic Positive-Weight NURBS Surface in 3D with
Runtime-Variable U/V Span Counts and Simple Interior Knots.**

Proposed public family:

`BicubicNURBSSurface3`.

"bicubic" means degree exactly three in each parameter direction. It does not
mean one span.

The family may have one or more spans independently in U and V.

## 6. Fixed scientific scope

The future family is frozen to:

- U degree exactly 3;
- V degree exactly 3;
- finite rectangular control net;
- finite strictly positive weight net of identical dimensions;
- finite clamped U domain `[u0,u1]`;
- finite clamped V domain `[v0,v1]`;
- zero or more **simple** strictly increasing interior U knots;
- zero or more **simple** strictly increasing interior V knots;
- non-periodic in both directions;
- value, first partials and second partials;
- immutable representation after construction;
- no production knot insertion/removal or degree mutation.

## 7. Size relations

Let:

- `Ku` = number of unique simple interior U knots;
- `Kv` = number of unique simple interior V knots.

Then:

- U control count `Nu = Ku + 4`;
- V control count `Nv = Kv + 4`;
- control net size `Nu * Nv`;
- weight net size `Nu * Nv`;
- U span count `Ku + 1`;
- V span count `Kv + 1`.

Flat knot sequences are:

`U = [u0,u0,u0,u0, ui..., u1,u1,u1,u1]`

and

`V = [v0,v0,v0,v0, vj..., v1,v1,v1,v1]`.

Interior vectors may be empty. Therefore the single-span unit-square case is a
strict subset and can be compared directly with
`RationalBicubicBezierPatch3`.

## 8. Storage and indexing policy

Preferred owning representation:

- flattened `std::vector<Point3>` control net;
- flattened `std::vector<double>` weight net;
- `std::vector<double>` U interior knots;
- `std::vector<double>` V interior knots;
- explicit `u_control_count` and `v_control_count`;
- scalar lower/upper domain endpoints.

Flattening convention:

`index(u_index, v_index) = u_index * Nv + v_index`.

This preserves the already documented U-major surface convention:

- fixed U index, varying V index is a V-direction row;
- fixed V index, varying U index is a U-direction column.

Public inspection should use immutable spans and explicit counts.

No nested-vector ragged representation is admitted.

## 9. Construction semantics

Construction is validated.

At minimum reject:

- U control count less than 4;
- V control count less than 4;
- control/weight flattened-size mismatch;
- flattened size not equal to `Nu*Nv`;
- U interior-knot count not equal to `Nu-4`;
- V interior-knot count not equal to `Nv-4`;
- non-finite domain endpoints;
- non-strict U or V domain;
- non-finite interior knot;
- interior knot outside the corresponding open domain;
- non-strictly-increasing interior-knot vector;
- non-finite weight;
- weight `<= 0`.

No epsilon defines knot order, positivity or domain inclusion.

Allocation exhaustion remains an ordinary C++ resource failure, not a
scientific `SurfaceError`.

## 10. Common bounded-surface contract

`BicubicNURBSSurface3` must satisfy the existing
`BoundedParametricSurface3` concept unchanged.

Required operations remain:

- `parameter_domain()`;
- `evaluate(u,v)`;
- `first_derivatives(u,v)`;
- `second_derivatives(u,v)`.

If the common concept or `SurfaceError` vocabulary must change, stop and
require a new decision.

Simple cubic interior knots guarantee C2 in each direction, so no continuity
failure is expected in this work unit.

## 11. Domain and query failures

The exact domain is:

`[u0,u1] x [v0,v1]`.

Existing deterministic validation order remains:

1. U finiteness;
2. V finiteness;
3. U domain;
4. V domain;
5. numerical result.

No clamping, normalization or periodic wrapping is permitted.

## 12. Deterministic span policy

For each direction independently:

- query at the upper endpoint selects the final span;
- otherwise select the span whose upper flat-knot boundary is the first knot
  strictly greater than the query;
- query exactly at a simple interior knot selects the span to the right.

A standard `upper_bound`-equivalent policy is preferred.

No tolerance participates in span selection.

## 13. Local tensor-product production strategy

Production should evaluate only the local active 4x4 control/weight block.

Required query sequence:

1. validate U/V parameters;
2. locate U span;
3. locate V span;
4. identify the 4x4 active original control/weight block;
5. select a positive local common weight scale from those 16 weights;
6. form local homogeneous controls;
7. evaluate deterministic tensor-product cubic de Boor in **V then U** order;
8. dehomogenize with explicit finite/positive denominator checks.

Expected query complexity:

- `O(log Su + log Sv)` span location;
- bounded constant cubic tensor evaluation;
- no per-query dynamic allocation;
- no global weight scan after span location.

The V-then-U order preserves the established surface deterministic convention.

## 14. Analytic partial derivatives

Production must provide homogeneous derivatives for:

- value;
- `S_u`;
- `S_v`;
- `S_uu`;
- `S_uv`;
- `S_vv`.

Then apply the already integrated rational quotient identities.

Finite differences are forbidden.

Because all admitted interior knots are simple cubic knots:

- value is C2 across each interior knot line;
- first partials are representation-guaranteed;
- second partials are representation-guaranteed.

No one-sided partial API is introduced.

## 15. Rational bicubic subset parity

When:

- U interior knots are empty;
- V interior knots are empty;
- U domain is `[0,1]`;
- V domain is `[0,1]`;
- net is 4x4;

the general NURBS surface must reproduce
`RationalBicubicBezierPatch3`.

Focused evidence must compare:

- stored controls/weights;
- domain;
- value;
- first partials;
- second partials;
- U reversal;
- V reversal;
- typed query failures.

The existing rational patch remains a frozen prerequisite, not deprecated.

## 16. Polynomial subset evidence

For the same one-span 4x4 case with all weights equal, the NURBS surface must
also reproduce `BicubicBezierPatch3`.

For true multi-span cases, an independent polynomial Cox-de Boor tensor oracle
must verify the equal-weight subset.

No separate public B-spline surface type is authorized by this decision.

## 17. Boundary-curve parity

Each parameter-domain boundary must reproduce the corresponding integrated
`MultiSpanCubicNURBS3` curve.

Required mappings:

- U lower/upper boundaries: V-direction NURBS curves;
- V lower/upper boundaries: U-direction NURBS curves.

Focused evidence must compare:

- boundary value;
- tangential first derivative;
- tangential second derivative;
- parameter domain;
- reversal consistency.

Boundary parity provides cross-family evidence that U/V knot interpretation
matches the already integrated curve semantics.

## 18. Independent rational tensor-product oracle

Focused tests must compute an independent reference using:

- Cox-de Boor basis functions in U;
- Cox-de Boor basis functions in V;
- rational tensor normalization;
- analytic first/second basis derivatives.

The oracle must not reuse production span-location, local-control or de Boor
helpers.

At minimum cover:

- at least three U spans;
- at least four V spans;
- nonuniform simple interior knots;
- asymmetric non-planar controls;
- nonuniform positive weights;
- one parameter pair in every representative span combination;
- every interior U knot with interior V sample;
- every interior V knot with interior U sample;
- value, first partials and second partials.

## 19. Geometry-preserving test-only knot insertion

Focused evidence must create multi-span NURBS data from an integrated
rational bicubic surface using test-only homogeneous knot insertion.

At minimum:

- insert one simple U knot;
- insert one simple V knot;
- preserve the same physical surface.

Compare:

- value;
- first partials;
- second partials;
- new and original knot lines.

Production knot insertion remains unauthorized.

## 20. Local-support evidence

A query uses only 4 controls in U and 4 controls in V.

Focused evidence must perturb controls and weights outside that 4x4 support and
verify unchanged results.

At least one fixture must have out-of-support data on both sides in U and V.

The production query must not globally normalize or traverse the entire net
after span selection.

## 21. Weight-scale invariance

A positive common scale applied to all weights must leave value and all first/
second partials unchanged within the declared numerical contract.

Focused evidence must include:

- exact power-of-two scaling;
- strongly unbalanced finite positive weights;
- a case where local normalization prevents avoidable overflow/underflow.

Stored public weights remain exact input values.

## 22. U/V reversal

U reversal must:

- reverse U rows;
- preserve order within each V row;
- reflect/reverse U interior knots;
- preserve V interior knots.

V reversal must:

- reverse V entries within every U row;
- reflect/reverse V interior knots;
- preserve U interior knots.

Required covariance:

- mapped value equality;
- sign flip of derivative in the reversed direction;
- unchanged sign in the other first derivative;
- `S_uu` or `S_vv` sign preserved;
- `S_uv` sign flips for exactly one reversed direction;
- reversing both directions preserves `S_uv` sign;
- double reversal recovers exact stored data.

Parameter reflection must reuse the overflow-aware bounded-parameter primitive.

## 23. Constant and degenerate geometry

A constant rectangular control net with arbitrary admitted positive weights is
representable.

Required behavior:

- exact common point where identity is preserved;
- exact zero first partials;
- exact zero second partials.

Collapsed/rank-deficient nets remain valid representation values if finite.

No normal/regularity condition is introduced here.

## 24. Affine, embedding and determinism evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two coordinate scaling where representable;
- signed Cartesian axis permutation/reflection cases already admitted by the
  deterministic geometry envelope;
- deterministic repeated value/partial success;
- deterministic repeated typed failures.

No arbitrary-angle frame qualification is implied.

## 25. Extreme finite evidence

At least one multi-span surface fixture must combine:

- extreme finite coordinates;
- strongly unbalanced positive weights;
- extreme finite U/V knot values;
- multiple spans in both directions.

Required outcome:

- succeed where final value/partials are representable and local scale-aware
  arithmetic can preserve them;
- otherwise fail explicitly with `SurfaceError::non_finite_result`.

No universal epsilon, NaN sanitization or hidden fallback is permitted.

## 26. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | General bicubic NURBS satisfies existing bounded-surface concept. |
| Dynamic rectangular storage | Exact U/V counts and U-major flattened layout. |
| Construction failures | Count, domain, knot, weight violations typed explicitly. |
| Exact domain | Arbitrary finite U/V domains preserved. |
| Span search | Interior knot and endpoint right-span policy deterministic. |
| Rational subset | One-span unit-square case matches rational bicubic patch. |
| Polynomial subset | Equal weights match polynomial bicubic / independent spline oracle. |
| Boundary curves | Four boundaries match `MultiSpanCubicNURBS3`. |
| Rational tensor oracle | Value, Su, Sv, Suu, Suv, Svv over multi-span grid. |
| Knot insertion parity | Test-only U/V insertion preserves surface. |
| Local support | Distant rows/columns/weights do not affect query. |
| Weight scale | Common positive scaling preserves full jet. |
| U/V reversal | Data reflection, involution and derivative covariance. |
| Constant patch | Exact point and zero partials. |
| Affine evidence | Translation / power-of-two scale / signed-axis transforms. |
| Extreme finite | No avoidable overflow; explicit unrepresentable-result failure. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/meshing/I/O/threading/external CAD dependency. |
| Prerequisite preservation | Existing 27 ordinary tests remain PASS. |

If exactly one focused semantic contract is added, ordinary FAST/INTEGRATION
inventory becomes **28 tests**.

## 27. Why NURBS is selected now

This work unit reuses two already-separated seams:

1. positive rational tensor-product surface evaluation;
2. cubic NURBS knot/span semantics from curves.

The unresolved surface-specific question is now the interaction of:

- independent U/V dynamic span counts;
- a rectangular dynamic control/weight net;
- two-direction local support;
- first/second tensor-product rational partials.

This is the most direct next step toward CAD free-form face support.

## 28. Why Coons/transfinite is deferred

Coons construction begins from boundary curves and requires explicit boundary
compatibility/corner/blending policy.

It is valuable for AP Mesh patch construction, but it does not close the
free-form CAD surface storage/evaluation seam.

It remains a strong later Surface Representation candidate.

## 29. Why analytic elementary surfaces are deferred

Plane/cylinder/cone/sphere/torus require a separate placement/orientation and,
for several families, periodic/singular parameter semantics.

Those semantics should be decided together rather than introduced implicitly
through a NURBS conversion.

## 30. Why swept surfaces are deferred

Ruled/extrusion/revolution representations require generator ownership,
orientation and sweep-domain rules.

They are representation-composition questions independent of the U/V knot
seam.

## 31. Why trimming is deferred

Trimming depends on a mature supporting-surface model and parameter-space
boundary semantics.

General trimming also approaches topology identity and curve-on-surface
binding.

It must remain separate from the supporting NURBS surface value type.

## 32. Deferred surface NURBS breadth

This decision does not authorize:

- U or V degree other than 3;
- repeated interior knots in U or V;
- C1/C0 surface knot lines;
- periodic U or V;
- zero/negative weights;
- production knot insertion/removal;
- surface interpolation/fitting;
- arbitrary-degree NURBS;
- a public polynomial B-spline surface family.

A later surface-specific continuity decision may admit multiplicity two after
the simple-knot surface is closed.

## 33. Relationship to Surface Differential Geometry

This work unit provides value and first/second partials only.

It does not provide:

- regularity certification;
- unit normals;
- tangent planes;
- first/second fundamental forms;
- Gaussian/mean/principal curvature;
- singularity classification.

Those remain Surface Differential Geometry responsibilities.

## 34. Repository mapping for future implementation

Authorized future mapping is limited to:

- public NURBS surface value/API:
  `include/apmesh/geometry/nurbs_surface.hpp`;
- production:
  `src/geometry/nurbs_surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_bicubic_nurbs.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

Frozen prerequisites:

- `include/apmesh/geometry/parametric_surface.hpp`;
- `include/apmesh/geometry/surface.hpp`;
- `src/geometry/surface.cpp`;
- `src/geometry/rational_surface.cpp`;
- all curve production semantics.

If the common surface contract must change, stop and require a new decision.

## 35. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 27 ordinary semantic tests must remain passing;
- one new bicubic NURBS focused contract must pass.

Passing yields only:

**SURFACE REPRESENTATION STAGE OPEN /
BICUBIC NURBS SURFACE IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or any other surface family.

## 36. Stop conditions

Stop and require a new decision if implementation needs:

- repeated U/V knot multiplicity;
- a new `SurfaceError`;
- arbitrary degree;
- periodicity;
- Coons/transfinite construction;
- analytic elementary surface production;
- swept surface production;
- trimming/curve-on-surface/topology;
- surface differential geometry;
- a universal tolerance;
- runtime virtual surface hierarchy;
- third-party dependency;
- discretization/meshing.

## 37. Planned sequence after this work unit

Planning only, not authorization.

After bicubic simple-knot NURBS surface integration/closure, a fresh Surface
Representation breadth decision should recompare:

1. surface knot multiplicity-two / C1 continuity semantics;
2. Coons/transfinite patch;
3. analytic elementary surfaces;
4. ruled/extrusion/revolution;
5. rectangular/general trimmed-surface semantics;
6. arbitrary degree/periodicity only if the admissible input class requires it.

No option is pre-authorized.

## 38. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Clamped Bicubic Positive-Weight NURBS Surface in 3D with
Runtime-Variable U/V Span Counts and Simple Interior Knots.**

No implementation begins on this decision branch.

Surface Differential Geometry, Boundary Curve Discretization and all meshing
stages remain blocked.
