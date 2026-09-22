# Surface Representation Entry — Bounded Stage Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites at entry:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- polynomial cubic Bézier Curve Representation baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- directed line segment 2D/3D: integrated focused work unit;
- positive-weight rational quadratic Bézier 2D/3D:
  integrated focused work unit;
- oriented static trimmed parametric subcurve 2D/3D:
  integrated focused work unit;
- fixed two-span cubic polynomial B-spline 2D/3D:
  integrated focused work unit;
- fixed and multi-span cubic positive-weight NURBS 2D/3D:
  integrated focused work units;
- multi-span cubic positive-weight NURBS with simple/double interior knots and
  explicit C1/D2 semantics:
  integrated and terminally closed focused work unit.

## 1. Question

After closing the bounded cubic-NURBS C1 continuity seam, should the next
scientific work remain in curve-representation breadth or should AP Mesh open
the separate Surface Representation stage?

The required comparison is:

1. bounded Surface Representation entry readiness;
2. multiplicity-three / C0 curve semantics;
3. arbitrary-degree spline/Bézier breadth;
4. analytic conics after arbitrary 3D placement prerequisites;
5. heterogeneous composition/polycurve.

If Surface Representation is ready, the decision must also identify exactly one
first surface work unit while explicitly mapping the broader surface-family
obligations so that one patch is never misrepresented as complete surface
coverage.

## 2. Fresh repository entry authority

Canonical decision-entry main:
`c40175174a9487acb7dca09eeb2fa9b3615766ee`.

Terminal cubic-NURBS C1 lineage:

- implementation PR #135:
  `eb62de8b4c7b09c671801e4b54c04e5d62dde0a9`;
- implementation post-merge FAST `35756910304`: PASS, 25/25;
- implementation post-merge INTEGRATION `35756910340`: PASS, 25/25;
- closure PR #136:
  `affc7a46b7fd4c5cc419679e6192fc293501654e`;
- closure post-merge FAST `35757554252`: PASS;
- closure post-merge INTEGRATION `35757554247`: PASS;
- terminal reconciliation PR #137 head:
  `22bb27133b626b3455e560294cc738fda387908b`;
- terminal reconciliation FAST `35761691320`: PASS;
- terminal reconciliation INTEGRATION `35761691184`: PASS;
- terminal reconciliation merge:
  `c40175174a9487acb7dca09eeb2fa9b3615766ee`;
- terminal reconciliation post-merge FAST `35761803424`: PASS;
- terminal reconciliation post-merge INTEGRATION `35761803482`: PASS.

Ordinary semantic inventory at entry: **25 tests**.

No production work item or open PR exists at entry.

## 3. Repository roadmap requirement

The existing authoritative roadmap already defines Surface Representation as a
separate scientific stage whose admissible envelope must explicitly account
for:

- tensor-product polynomial Bézier patches;
- Coons/transfinite patches where used by the AP Mesh construction;
- rational Bézier patches;
- B-spline and NURBS surfaces;
- analytic surfaces required by the admissible model class and independent
  validation set, including at least plane, cylinder, cone, sphere and torus
  when admitted;
- ruled, extrusion and revolution surfaces when required by the chosen input
  class;
- trimmed-surface semantics;
- separation of continuous supporting geometry, trimming curves and explicit
  topology identity;
- deterministic parameter-domain/orientation semantics and boundary
  consistency.

This decision must preserve that full obligation even though it authorizes only
one first work unit.

## 4. Literature and mature-kernel evidence

### 4.1 Bounded surfaces have a genuine 2D parameter domain

Open CASCADE `Geom_BoundedSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___bounded_surface.html

Relevant evidence:

- a bounded surface is finite over a rectangular 2D parameter domain;
- the rectangle has independent U and V parameter intervals;
- four isoparametric boundary curves delimit the patch;
- Bézier, B-spline and rectangularly trimmed surfaces are concrete bounded
  surface families.

Decision impact:

- surface parameter-domain semantics are not a cosmetic extension of the curve
  API;
- AP Mesh needs an explicit bounded two-parameter surface contract;
- boundary consistency must be a first-class representation invariant.

### 4.2 Bézier surfaces are tensor-product control-net geometry

Open CASCADE `Geom_BezierSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___bezier_surface.html

MIT Hyperbook — Bézier curves and surfaces:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node8.html

Relevant evidence:

- polynomial and rational Bézier surfaces are represented by a 2D control net;
- the U and V directions are independent parametric directions;
- fixed polynomial tensor-product Bézier geometry isolates the new 2D
  parameter/control-net semantics without introducing knots, weights,
  periodicity or trimming.

Decision impact:

- a bicubic polynomial Bézier patch is the smallest surface representation that
  directly reuses the already qualified cubic Bézier curve basis;
- its four parameter-domain edges can be independently checked against
  `CubicBezier3`.

### 4.3 B-spline/NURBS surfaces extend the same idea in two directions

MIT Hyperbook — B-spline surface:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node19.html

Open CASCADE B-spline surface family:

https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html

Relevant evidence:

- B-spline surfaces are tensor products over a rectangular control net;
- U and V have independent knot structures;
- NURBS adds rational weights while retaining the two-direction spline model;
- curve-level knot/multiplicity semantics therefore provide reusable
  prerequisites but do not themselves constitute a surface implementation.

Decision impact:

- the now-closed curve NURBS multiplicity work removes a direct prerequisite
  ambiguity for later U/V NURBS continuity;
- it is not necessary to implement arbitrary curve degree or C0 knots before a
  first bicubic polynomial surface patch.

### 4.4 Mature kernels keep analytic, free-form, swept and trimmed surfaces
separate

Open CASCADE geometry overview:

https://dev.opencascade.org/sites/default/files/pdf/Geometry.pdf

Relevant model classes include:

- elementary surfaces: plane, cylindrical, spherical, toroidal, conical;
- free-form surfaces: B-spline and Bézier;
- swept surfaces: linear extrusion and revolution;
- offset surfaces;
- trimming as a distinct concept.

Open CASCADE `Geom_RectangularTrimmedSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___rectangular_trimmed_surface.html

Decision impact:

- no single free-form patch may be documented as equivalent to the full CAD
  surface envelope;
- analytic surfaces, swept surfaces and trimming remain explicit later
  representation obligations;
- trimming is not to be encoded by altering supporting-surface evaluation.

### 4.5 Coons/transfinite filling is its own construction mechanism

Open CASCADE `GeomFill_BSplineCurves` / Coons filling:

https://dev.opencascade.org/doc/refman/html/_geom_fill___b_spline_curves_8hxx.html

Relevant evidence:

- contiguous boundary curves may define a filled B-spline surface;
- Coons-style filling is a construction/filling mechanism distinct from a
  tensor-product Bézier value type.

Decision impact:

- Coons/transfinite patches remain a required AP Mesh construction family where
  the doctoral algorithm uses them;
- they should not be conflated with the first tensor-product Bézier patch.

External sources are scientific/design evidence only. No OCCT or other external
runtime dependency is admitted.

## 5. Candidate comparison

| Candidate | New semantic burden | Dependency status | Direct doctoral value | Decision |
| --- | --- | --- | --- | --- |
| Surface Representation entry | 2D parameter domain, control net, partial derivatives, U/V orientation, boundary consistency | Curve representation and C1 spline prerequisite now available | **Directly unlocks patch geometry and later meshing** | **SELECTED** |
| Multiplicity-three / C0 curve semantics | D1 failure / side semantics at C0 knots | Independent of first polynomial surface patch | Important later for broader spline import | DEFER |
| Arbitrary-degree curve breadth | runtime degree and variable active-control count | Independent of first bicubic surface patch | Important later, but not a first-surface blocker | DEFER |
| Analytic conic curves | arbitrary 3D placement + periodic/angular semantics | Qualified arbitrary-angle frame support still absent | Important for analytic model breadth | BLOCKED/DEFER |
| Heterogeneous polycurve | runtime family composition + join semantics | Not needed for an untrimmed tensor-product patch | Important for trimming/boundary loops later | DEFER |

## 6. Decision

Open the **Surface Representation — Continuous Patch Geometry** scientific
stage.

Authorize exactly one future implementation work unit:

**Tensor-Product Bicubic Polynomial Bézier Patch in 3D.**

Proposed public representation:

- `BicubicBezierPatch3`.

This is a first surface work unit only. It is not a claim that Surface
Representation is complete or qualified.

## 7. Surface-stage family coverage map

The following map is a retained obligation, not blanket implementation
authorization.

### Family A — polynomial tensor-product patches

Required:

- bicubic polynomial Bézier patch — **selected first work unit**;
- later decision on arbitrary bi-degree only if the admissible model class
  requires it.

### Family B — rational free-form patches

Required later:

- rational Bézier patches;
- explicit positive-weight and homogeneous evaluation semantics.

### Family C — spline surfaces

Required later:

- B-spline surfaces;
- NURBS surfaces;
- independent U/V degree, knot and multiplicity semantics as justified by the
  admissible input class;
- parameter-local continuity/failure rules in each direction.

### Family D — transfinite / boundary-generated surfaces

Required when used by AP Mesh:

- Coons/transfinite patches;
- boundary compatibility and corner consistency;
- no implicit conversion of topology identity into coordinate equality.

### Family E — analytic surfaces

Must be explicitly admitted or explicitly excluded by a later decision:

- plane;
- cylinder;
- cone;
- sphere;
- torus;
- other quadrics only if required by the admitted model/validation envelope.

Their supporting-frame/orientation requirements must be decided explicitly.

### Family F — swept/constructed surfaces

Must be considered when required:

- ruled surfaces;
- linear extrusion;
- revolution.

### Family G — trimmed surfaces

Required before a CAD-like trimmed-face envelope can be claimed:

- supporting continuous surface distinct from trim;
- parameter-space trimming curves;
- corresponding physical-space boundary evidence where needed;
- explicit orientation;
- explicit topology identity;
- no geometric-proximity identity inference.

No family above is implicitly covered by the first bicubic patch.

## 8. Retained curve-breadth obligations

Opening Surface Representation does **not** cancel the remaining curve work.

Still retained for later literature-backed decisions:

- multiplicity-three / C0 B-spline/NURBS curve semantics;
- arbitrary-degree polynomial/rational Bézier;
- arbitrary-degree B-spline/NURBS where the input class requires it;
- dedicated analytic line/conic family breadth beyond currently admitted
  representations;
- arbitrary 3D supporting-plane/orientation prerequisites for analytic conics;
- heterogeneous composition/polycurve;
- periodic curve semantics;
- one-sided derivative APIs if later stages demonstrate they are required.

These items may become prerequisites for trimmed surfaces, analytic surfaces,
CAD exchange or Boundary Curve Discretization, but they are not prerequisites
for the first bicubic patch.

## 9. Common bounded-surface abstraction

The future implementation should introduce a minimal static/value-oriented
bounded surface contract, analogous in spirit to the bounded curve contract.

Conceptual types:

- `SurfaceParameterDomain`;
- `SurfaceError`;
- `SurfaceFirstDerivatives3`;
- `SurfaceSecondDerivatives3`;
- `BoundedParametricSurface3` concept.

`SurfaceParameterDomain` should contain independent valid U and V finite
intervals, preferably reusing `CurveParameterDomain` as the already validated
1D interval type.

The concept should require:

- `parameter_domain()`;
- `evaluate(u,v)`;
- `first_derivatives(u,v)`;
- `second_derivatives(u,v)`.

No runtime virtual hierarchy is authorized in this first work unit.

## 10. Surface error semantics

The first bounded surface error vocabulary should distinguish at minimum:

- non-finite U parameter;
- non-finite V parameter;
- U parameter out of domain;
- V parameter out of domain;
- non-finite final result.

Validation order is deterministic:

1. U finiteness;
2. V finiteness;
3. U domain;
4. V domain;
5. numerical result.

No universal epsilon participates in domain membership.

Continuity and singular-parameterization errors are not added speculatively in
this first polynomial patch work unit.

## 11. First patch parameter domain

`BicubicBezierPatch3` uses the exact normalized rectangular domain:

`(u,v) in [0,1] x [0,1]`.

This normalized choice is specific to this first Bézier family.

The common surface abstraction must **not** claim that all future surfaces use
`[0,1]^2`.

Future NURBS and analytic surfaces may use family-specific finite domains or
periodic semantics.

## 12. Control-net convention

The patch owns exactly 16 finite `Point3` controls.

Preferred public layout:

`std::array<std::array<Point3,4>,4>`.

Index convention:

`control_points[u_index][v_index]`.

Therefore:

- fixed U index, varying V index defines a V-direction control row;
- fixed V index, varying U index defines a U-direction control column.

The convention must be documented in the header and tested explicitly.

## 13. Mathematical representation

Let cubic Bernstein basis functions be `B_i^3`.

The surface is:

`S(u,v) = sum_i=0^3 sum_j=0^3 B_i^3(u) B_j^3(v) P_ij`.

Production should evaluate through a deterministic tensor-product de Casteljau
scheme.

Frozen evaluation order for reproducibility:

1. for each fixed U-index row, reduce its four controls in V;
2. reduce the four resulting points in U.

Equivalent mathematical orders may not be switched opportunistically because
floating execution order is part of the deterministic baseline.

## 14. Exact corner and boundary identities

Required exact corner identities:

- `S(0,0)=P00`;
- `S(0,1)=P03`;
- `S(1,0)=P30`;
- `S(1,1)=P33`.

Each boundary must reproduce an existing `CubicBezier3` exactly under the
same parameter:

- U=0 boundary: controls `P0j`, parameter V;
- U=1 boundary: controls `P3j`, parameter V;
- V=0 boundary: controls `Pi0`, parameter U;
- V=1 boundary: controls `Pi3`, parameter U.

Boundary value and tangential derivative parity are mandatory focused evidence.

## 15. First and second partial derivatives

The first work unit includes analytic partials needed by later surface
differential geometry:

First derivatives:

- `S_u`;
- `S_v`.

Second derivatives:

- `S_uu`;
- `S_uv`;
- `S_vv`.

No finite-difference derivative is permitted.

The mixed derivative is one mathematical quantity; production must not expose
competing `uv` and `vu` results.

A direct Bernstein derivative oracle independent of production de Casteljau
must verify the partials.

## 16. Representation versus differential geometry

This work unit is representation only.

It does not compute:

- normals;
- unit normals;
- tangent planes;
- first fundamental form;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- principal curvatures/directions;
- regularity certificates.

A patch whose `S_u x S_v` vanishes at a parameter is still a valid
representation value if evaluation/partials are finite.

Surface singularity/regularity belongs to the later
**Surface Differential Geometry** stage.

## 17. U/V reversal and orientation

The concrete bicubic patch must support:

- `u_reversed()`;
- `v_reversed()`.

U reversal reverses the outer U control order.

V reversal reverses each inner V control order.

Required covariance:

- U-reversed value at `(1-u,v)` equals original value at `(u,v)`;
- U-reversed `S_u` is the negative original `S_u`;
- U-reversed `S_v` preserves sign;
- analogous V rules;
- reversing one direction flips the orientation implied by
  `S_u x S_v`;
- reversing both directions preserves that orientation;
- double reversal recovers the exact stored control net.

No normal vector API is introduced here; orientation evidence may use the
already integrated vector cross product in tests.

## 18. Independent polynomial oracle

Focused validation must compute an independent direct Bernstein sum.

It must not call production surface de Casteljau helpers.

At minimum validate:

- generic asymmetric non-planar control net;
- multiple interior parameter pairs;
- all four boundaries;
- all four corners;
- value;
- `S_u`, `S_v`;
- `S_uu`, `S_uv`, `S_vv`.

## 19. Analytic fixture set

Focused evidence should include control nets representing or degree-elevating
simple analytic polynomial surfaces, for example:

- plane;
- bilinear saddle;
- parabolic cylinder or another low-degree polynomial surface.

Expected values and partials should come from the analytic formula, not from
production code.

Sphere/cylinder/cone/torus are **not** claimed as production representations by
this work unit.

They remain future analytic-surface families and/or independent regression
oracles as later decisions specify.

## 20. Constant and degenerate patches

A constant 4x4 control net is representable.

Required behavior:

- exact constant point for every valid parameter where exact identity is
  preserved;
- exact zero first partials;
- exact zero second partials.

Collinear/collapsed/otherwise rank-deficient control nets are also
representable if finite.

No representation constructor rejects them merely because later normal or
metric computation would be singular.

## 21. Affine and deterministic evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two coordinate scaling where representable;
- signed Cartesian axis permutation/reflection cases already admitted by the
  existing deterministic frame/numeric envelope;
- repeated identical value/partial results;
- repeated identical typed failures.

No arbitrary-angle frame qualification is implied.

## 22. Extreme finite evidence

At least one patch must use extreme finite coordinates and parameter values near
the domain boundary.

Required behavior:

- succeed when the final point/partial is representable and the chosen
  scale-aware/interpolation arithmetic can preserve it;
- otherwise return explicit `SurfaceError::non_finite_result`.

No NaN/inf sanitization, silent clamping or hidden fallback is permitted.

## 23. Header isolation and architecture boundary

The first surface headers must not depend on:

- topology;
- mesh entities;
- discretization;
- I/O;
- threading;
- OpenMP/MPI;
- external CAD kernels.

Surface geometry stays a reusable scientific value layer.

## 24. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Bicubic patch satisfies the new bounded 3D surface concept. |
| Domain | Exact [0,1]² domain; U/V failures typed independently. |
| Control layout | 4x4 U-major convention preserved exactly. |
| Corners | Four exact control-point identities. |
| Independent oracle | Direct Bernstein value and partials match production. |
| Boundary parity | Four boundaries match `CubicBezier3` values/tangents. |
| Partial derivatives | Su, Sv, Suu, Suv, Svv analytic and finite where representable. |
| Mixed partial | One deterministic Suv representation. |
| U reversal | Value/partial covariance and involution. |
| V reversal | Value/partial covariance and involution. |
| Orientation relation | One reversal flips cross-product orientation; two preserve it where regular. |
| Plane fixture | Exact/analytic polynomial reference. |
| Saddle fixture | Analytic bivariate reference. |
| Constant patch | Exact point and zero partials. |
| Degenerate patch | Representation remains valid without premature regularity rejection. |
| Translation/scale | Declared affine covariance. |
| Extreme finite | Explicit success/failure without universal epsilon. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/meshing/I/O/threading/external dependency. |
| Prerequisite preservation | Existing 25 ordinary tests remain PASS. |

If exactly one new focused contract is added, ordinary FAST/INTEGRATION
inventory becomes **26 tests**.

## 25. Why Surface Representation is selected now

The curve work needed for the first surface patch is already sufficient:

- finite 3D point/vector primitives exist;
- polynomial cubic Bézier value and derivative semantics are qualified;
- deterministic failure behavior exists;
- curve NURBS multiplicity/C1 semantics have been resolved before future
  two-direction NURBS surface work.

A fixed bicubic polynomial patch does not require:

- C0 curve knots;
- arbitrary curve degree;
- analytic conic curves;
- heterogeneous polycurve dispatch;
- periodic curves.

Continuing curve breadth indefinitely before allowing any surface code would
delay a separate doctoral geometry stage without resolving a prerequisite of
the first patch.

## 26. Why multiplicity-three/C0 curves are deferred

Cubic multiplicity three introduces a new D1 continuity boundary and likely
one-sided derivative needs.

Those semantics matter for broader imported NURBS and eventually trimmed
boundaries, but they are not required for a smooth polynomial bicubic patch.

They remain an explicit later curve-breadth obligation.

## 27. Why arbitrary-degree curves are deferred

The first patch intentionally fixes degree three in each parameter.

Arbitrary-degree curves and surfaces should be introduced only when the
admissible input class requires runtime degree.

They are not a prerequisite for a bicubic tensor-product baseline.

## 28. Why analytic conic curves are deferred

Dedicated analytic 3D conics still require an arbitrary supporting-plane and
orientation contract beyond the currently qualified signed-permutation frame
envelope.

The first polynomial surface patch does not depend on that contract.

Analytic surfaces will receive their own placement/orientation decision before
production admission.

## 29. Why heterogeneous polycurve is deferred

An untrimmed tensor-product patch does not require runtime heterogeneous
boundary-chain storage.

Polycurve semantics become more directly relevant when the project opens:

- trimmed surfaces;
- curve-on-surface boundaries;
- general boundary loops;
- Boundary Curve Discretization.

The obligation remains visible but is not a blocker for surface entry.

## 30. Repository mapping for the future first implementation

Authorized future mapping is limited to:

- common bounded surface contract:
  `include/apmesh/geometry/parametric_surface.hpp`;
- public bicubic patch:
  `include/apmesh/geometry/surface.hpp`;
- production:
  `src/geometry/surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_bicubic_bezier.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No topology, trimming, analytic surface, NURBS surface, Coons, sweep or meshing
file is authorized by this first work unit.

## 31. Validation boundary

Before first implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 25 ordinary semantic tests must remain passing;
- one new bicubic surface contract must pass.

Passing yields only:

**SURFACE REPRESENTATION STAGE OPEN /
BICUBIC BÉZIER PATCH IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or any other surface family.

## 32. Stop conditions

Stop and require a new decision if implementation needs:

- rational weights;
- B-spline/NURBS knots;
- periodic U/V semantics;
- arbitrary degree;
- Coons/transfinite filling;
- analytic plane/cylinder/cone/sphere/torus production types;
- swept surfaces;
- trimming loops;
- curve-on-surface binding;
- topology identity;
- normals/curvature/metric;
- a universal tolerance;
- runtime virtual surface hierarchy;
- third-party runtime dependency;
- surface meshing/discretization.

## 33. Planned sequence after the first surface patch

Planning only, not authorization.

After the bicubic patch is integrated and closed, a fresh Surface
Representation breadth decision should compare:

1. rational Bézier patch;
2. B-spline/NURBS surface;
3. Coons/transfinite patch;
4. analytic elementary surfaces;
5. ruled/extrusion/revolution surfaces;
6. rectangular/general trimmed-surface semantics.

The selected next item must be justified by the doctoral admissible input class
and the needs of downstream Surface Differential Geometry / meshing.

No option is pre-authorized.

## 34. Effect if this decision is integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Tensor-Product Bicubic Polynomial Bézier Patch in 3D.**

Surface Representation becomes the active scientific stage.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization remains blocked.

Remaining curve breadth remains retained, not cancelled.

No surface family beyond the bicubic polynomial Bézier patch is authorized by
this decision.


## 35. Decision integration checkpoint

PR #138 integrated this bounded Surface Representation entry decision.

Final decision head:

`0593131d7380147ef87e9fcba5122ffbcbedd546`.

Final decision-head validation:

- FAST `35762844157`: PASS;
- INTEGRATION `35762844171`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #138 merged as:

`50403e5780b30c69ecea5bc8ae2857bad31b18ea`.

Post-merge validation:

- FAST `35762956742`: PASS;
- INTEGRATION `35762956709`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the bicubic polynomial Bézier patch implementation bounded by
Sections 6–32.

No other surface family or downstream capability is authorized.


## 36. Decision closure checkpoint

Decision closure PR #139 used head
`050d1b5b919b9bba0b360b37657c07c5bac70eef`.

Closure PR validation:

- FAST `35763315317`: PASS;
- INTEGRATION `35763315277`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #139 merged as
`2300c5fdac3e79d4106f0a7821749dfc5de97ffd`.

Closure post-merge validation:

- FAST `35763548131`: PASS;
- INTEGRATION `35763548244`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / SURFACE REPRESENTATION STAGE OPEN /
BICUBIC BÉZIER PATCH IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is the tensor-product bicubic polynomial
Bézier patch bounded by Sections 6–32.

No other surface family or downstream capability is authorized.


## 37. Active bicubic patch implementation mapping

The sole authorized implementation is active on:

`surface/bicubic-bezier-patch`.

Candidate mapping:

- common bounded-surface contract:
  `include/apmesh/geometry/parametric_surface.hpp`;
- public bicubic patch API:
  `include/apmesh/geometry/surface.hpp`;
- production:
  `src/geometry/surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_bicubic_bezier.cpp`;
- build/test registration:
  `CMakeLists.txt`.

The candidate implements:

- exact [0,1]^2 domain for the bicubic family;
- 4x4 U-major Point3 control storage;
- deterministic V-then-U tensor-product de Casteljau;
- analytic Su, Sv, Suu, Suv and Svv;
- U/V reversal and exact stored-net involution;
- exact four-corner identity;
- boundary value/tangent parity against the existing `CubicBezier3`;
- independent direct Bernstein value/partial reference;
- analytic plane and saddle fixtures;
- constant and rank-deficient representation acceptance;
- translation/power-of-two scale covariance;
- extreme finite success and explicit non-finite-result failure;
- deterministic repeated successes/failures.

Expected ordinary semantic inventory: **26 tests**.

Candidate validation:

- candidate head:
  `f9c94d95540d93eace7bbf1401c35f17a27d145b`;
- FAST `35765755483`: PASS, 26/26 ordinary semantic tests;
- INTEGRATION `35765755475`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 26/26 tests per cell;
- `apmesh_core.surface_bicubic_bezier`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**

No rational, B-spline/NURBS, Coons, analytic elementary, swept, trimmed,
surface differential geometry or meshing capability is implied.
