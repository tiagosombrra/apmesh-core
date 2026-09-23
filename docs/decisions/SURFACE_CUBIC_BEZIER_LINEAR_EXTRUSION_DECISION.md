# Cubic Bézier Linear Extrusion Surface — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-23  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites:

- Surface Representation stage: OPEN / NOT QUALIFIED;
- bicubic polynomial Bézier patch: integrated and closed;
- rational bicubic Bézier patch: integrated and closed;
- bicubic/multi-span positive-weight NURBS surface: integrated and closed;
- U/V simple/double-knot continuity semantics: integrated and closed;
- oriented four-boundary cubic Bézier Coons patch: integrated and closed;
- static oriented rectangular trimmed surface: integrated and closed;
- ordinary semantic inventory: 31 tests;
- terminal rectangular-trim sync PR #167 merged and post-merge validated.

## 1. Question

Which Surface Representation breadth item should follow static rectangular
trimming?

Required comparison:

1. analytic elementary surfaces;
2. ruled/extrusion/revolution surfaces;
3. general trimmed-surface / curve-on-surface / face-boundary semantics;
4. broader Coons/transfinite boundary families;
5. remaining NURBS breadth required by the admitted CAD input class.

The next work unit must introduce one bounded semantic seam without silently
requiring an unqualified coordinate-frame capability or topology/trim-loop
architecture.

## 2. Fresh repository authority

Decision-entry main:

`47b1a2b17d9cecd972430bb33e84161eb74cfec1`.

Terminal rectangular-trim lineage:

- implementation PR #165:
  `07a5836ceabace389c4a6bfc2d1f60d644a7a939`;
- implementation post-merge FAST `35874273067`: PASS, 31/31;
- implementation post-merge INTEGRATION `35874273154`: PASS, 31/31;
- implementation closure PR #166:
  `ad95dc048be988ad6f3fd9b29203f6e9d6d92b10`;
- closure post-merge FAST `35875154581`: PASS;
- closure post-merge INTEGRATION `35875154595`: PASS;
- terminal sync PR #167 head:
  `083a9a3263bd24be4f848c3a31ead5a2d13773b3`;
- sync PR FAST `35875571648`: PASS;
- sync PR INTEGRATION `35875571740`: PASS;
- sync merge:
  `47b1a2b17d9cecd972430bb33e84161eb74cfec1`;
- sync post-merge FAST `35875814460`: PASS;
- sync post-merge INTEGRATION `35875814543`: PASS.

No open PR or active production work item exists at decision entry.

## 3. Literature and mature-kernel evidence

### 3.1 Linear extrusion is a distinct swept-surface family

Open CASCADE geometry model:

https://dev.opencascade.org/sites/default/files/pdf/Geometry.pdf

Open CASCADE analytical bounding support:

https://dev.opencascade.org/doc/refman/html/class_geom_bnd_lib___surface_of_extrusion.html

Relevant evidence:

- swept surfaces are represented separately from free-form and analytic
  elementary surfaces;
- a linear extrusion has direct form
  `P(u,v)=BasisCurve(u)+v*Direction`;
- the U direction follows the basis-curve parameter and V follows the extrusion
  direction.

Decision impact:

- extrusion adds a real new surface family while reusing already integrated
  curve value/D1/D2 semantics;
- no knot, rational-weight, trim-loop or topology semantics need be added.

### 3.2 Revolution is more complex than linear extrusion

Open CASCADE `Geom_SurfaceOfRevolution`:

https://dev.opencascade.org/doc/refman/html/_geom__surface_of_revolution_8hxx.html

Relevant evidence:

- revolution uses an axis and angular U parameter;
- complete revolution introduces periodic angular semantics;
- derivative continuity in the generatrix direction depends on the basis curve.

Decision impact:

- revolution is deferred until axis/orientation and periodic parameter semantics
  receive their own bounded decision.

### 3.3 Analytic elementary surfaces require local 3D placement

Open CASCADE `Geom_Plane`:

https://dev.opencascade.org/doc/refman/html/_geom___plane_8hxx.html

Open CASCADE cylindrical/spherical references:

- https://dev.opencascade.org/doc/refman/html/class_g_c___make_cylindrical_surface.html
- https://dev.opencascade.org/doc/refman/html/class_geom___spherical_surface.html

Relevant evidence:

- plane, cylinder and sphere use explicit local 3D coordinate systems;
- cylinder and sphere introduce periodic angular parameterization;
- cylinder and sphere use arbitrary spatial axis placement.

Internal sequencing constraint:

`CartesianFrame3` is qualified only for exact signed-permutation bases and
positive power-of-two scale.

Decision impact:

- arbitrary-orientation analytic elementary surfaces may not silently widen the
  qualified frame claim;
- analytic plane/cylinder/cone/sphere/torus remain required future families but
  are blocked pending an explicit arbitrary-placement/orientation decision or
  an alternative rigorously bounded representation.

### 3.4 General trimming is a topology/parameter-space seam, not a rectangular
subdomain wrapper

Open CASCADE trimming overview:

https://dev.opencascade.org/sites/default/files/pdf/Geometry.pdf

Project evidence:

- `RectangularTrimmedSurface3<Surface>` already covers static oriented
  rectangular parameter subdomains;
- arbitrary loops, p-curves and face-boundary identity are explicitly absent.

Decision impact:

- general trimming should not be approximated by another rectangular wrapper;
- it remains deferred until curve-on-surface and explicit boundary-loop
  semantics are decided.

## 4. Candidate comparison

| Candidate | New semantics | Prerequisite status | Downstream value | Decision |
| --- | --- | --- | --- | --- |
| Cubic Bézier linear extrusion | Swept surface from one curve + one displacement | Existing CubicBezier3 and bounded surface contract sufficient | High; opens swept family with exact formulas | **SELECTED** |
| Analytic elementary surfaces | Local 3D placement, angular/periodic domains | Arbitrary-orientation frame not qualified | Very high | BLOCKED/DEFER |
| Revolution surface | Axis placement + angular periodicity + generatrix | Placement/periodicity unresolved | High | DEFER |
| General trimming / p-curves / face boundary | arbitrary loops, orientation, curve-on-surface, topology identity | Major new architecture seam | Very high | DEFER |
| Broader Coons/transfinite | generic boundary families and compatibility | Current Coons limited to cubic Bézier boundaries | Moderate-high | DEFER |
| Remaining NURBS breadth | arbitrary degree / broader multiplicities / periodicity | Independent of first swept surface | High for CAD import | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Bounded Cubic Bézier Linear Extrusion Surface in 3D.**

Proposed public type:

`CubicBezierLinearExtrusionSurface3`.

The work unit is intentionally concrete rather than generic over every
`BoundedParametricCurve3`.

Reason:

- `CubicBezier3` has already qualified value/D1/D2 representation semantics;
- this avoids introducing a new cross-family error-translation contract in the
  same work unit;
- a later decision may generalize extrusion to additional curve families after
  the swept-surface semantics are isolated.

## 6. Mathematical representation

Let `C(u)` be the stored `CubicBezier3` basis curve and let `E` be a finite
3D extrusion displacement vector.

Use normalized bounded domain:

`(u,v) in [0,1] x [0,1]`.

Define:

`S(u,v)=C(u)+v E`.

Therefore:

- `S_u=C'(u)`;
- `S_v=E`;
- `S_uu=C''(u)`;
- `S_uv=0`;
- `S_vv=0`.

No finite-difference derivative is permitted.

## 7. Construction semantics

Inputs:

- one `CubicBezier3` basis curve;
- one finite `Vector3` extrusion displacement.

The displacement may be exactly zero.

A zero displacement yields a rank-deficient surface representation but is not
a construction failure. Regularity belongs to Surface Differential Geometry.

The implementation may precompute the translated end curve

`C_1(u)=C(u)+E`

at construction time.

If any translated control point is not representable as finite double, return a
typed construction failure rather than creating a partially invalid surface.

No epsilon participates in construction.

## 8. Stored representation and reversal safety

Preferred stored representation:

- exact original start curve;
- exact precomputed translated end curve;
- exact extrusion displacement.

This supports exact representation identity and reversal without recomputing
translated controls.

U reversal:

- reverse both stored curves;
- preserve extrusion displacement.

V reversal:

- swap start/end curves;
- negate extrusion displacement.

Double U or V reversal must recover exact stored representation.

## 9. Parameter-domain semantics

The concrete family uses exactly `[0,1]^2`.

Validation order follows the existing surface contract:

1. U finiteness;
2. V finiteness;
3. U in domain;
4. V in domain;
5. result finiteness.

Required errors remain the existing `SurfaceError` vocabulary.

No clamping or periodic wrapping is allowed.

## 10. Value evaluation

Production may evaluate:

`C(u)+vE`

directly with overflow-aware existing geometry primitives.

Alternatively it may use the precomputed start/end curves through a fixed
algebraically equivalent interpolation, provided the execution order is frozen
and focused tests prove reversal and boundary parity.

Required exact/structural identities:

- `S(u,0)=C(u)`;
- `S(u,1)=C_1(u)`;
- `S(0,v)` and `S(1,v)` are line segments parallel to `E`.

## 11. Derivative semantics

Required first partials:

- U derivative equals basis-curve D1;
- V derivative equals exact stored extrusion vector.

Required second partials:

- UU equals basis-curve D2;
- UV exact zero;
- VV exact zero.

Because `CubicBezier3` D1/D2 do not introduce continuity failure within
`[0,1]`, this work unit requires no extension to `SurfaceError`.

## 12. Boundary parity

Focused evidence must validate all four boundaries.

V=0:

- exact/declared parity with stored basis `CubicBezier3`.

V=1:

- parity with the stored translated end `CubicBezier3`.

U=0 and U=1:

- parity with direct line-segment interpolation between corresponding start and
  end endpoints.

No new public line-surface binding type is required.

## 13. Independent analytic oracle

The focused test must compute an independent reference from the cubic Bernstein
basis for `C(u)` and the explicit extrusion formula.

It must not call production extrusion helpers.

Required comparisons:

- value;
- Su;
- Sv;
- Suu;
- Suv;
- Svv;
- multiple interior U/V pairs;
- every boundary and corner.

## 14. Reversal covariance

For `r(t)=1-t`:

U reversal:

- `S^U(r(u),v)=S(u,v)`;
- `S^U_u(r(u),v)=-S_u(u,v)`;
- `S^U_v(r(u),v)=S_v(u,v)`;
- `S^U_uu(r(u),v)=S_uu(u,v)`;
- `S^U_uv(r(u),v)=-S_uv(u,v)=0`;
- `S^U_vv(r(u),v)=S_vv(u,v)=0`.

V reversal:

- `S^V(u,r(v))=S(u,v)`;
- U derivatives preserve sign;
- V first derivative changes sign;
- UV changes sign and remains exact zero;
- VV preserves sign and remains exact zero.

Reversing exactly one direction flips the orientation of `S_u x S_v` at a
regular point; reversing both preserves it.

No normal API is introduced.

## 15. Degenerate fixtures

Required admitted cases include:

- zero extrusion displacement;
- constant basis curve;
- linear cubic Bézier basis;
- extrusion displacement parallel to the basis tangent at some parameter.

The representation must not reject these merely because a later surface normal
or metric would be singular.

## 16. Affine and scale evidence

Focused evidence must include:

- translation of the basis curve;
- exact power-of-two scaling of basis controls and extrusion displacement;
- admitted signed-permutation Cartesian-frame covariance;
- deterministic repeated results/failures.

No arbitrary-angle frame claim is implied.

## 17. Extreme finite evidence

At least one fixture must combine large finite basis coordinates and extrusion
displacement.

Required outcome:

- succeed when final values/partials are representable;
- otherwise return explicit construction or `SurfaceError::non_finite_result`
  according to whether failure occurs in precomputed stored data or at query
  evaluation.

No universal epsilon or silent fallback is allowed.

## 18. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Type satisfies `BoundedParametricSurface3`. |
| Exact domain | Exactly `[0,1]^2`. |
| V=0 boundary | Basis curve parity. |
| V=1 boundary | Translated-end curve parity. |
| U boundaries | Line-segment extrusion edges. |
| Independent oracle | Value and five partials match Bernstein + extrusion formula. |
| First partials | Su = curve D1; Sv = displacement. |
| Second partials | Suu = curve D2; Suv = Svv = exact zero. |
| U reversal | Value/partial covariance and involution. |
| V reversal | Value/partial covariance and involution. |
| Orientation | One reversal flips cross-product orientation; two preserve it where regular. |
| Zero extrusion | Valid degenerate representation. |
| Constant basis | Valid degenerate representation. |
| Translation/scale | Declared covariance. |
| Admitted frame | Signed-permutation/power-of-two frame covariance only. |
| Extreme finite | Explicit success/failure. |
| Determinism | Repeat results/failures identical. |
| Header isolation | No topology, trimming, meshing, I/O, threading, external kernel. |
| Prerequisites | Existing 31 ordinary tests remain PASS. |

If exactly one focused contract is added, ordinary inventory becomes **32
tests**.

## 19. Why analytic elementary surfaces are not selected yet

Mature CAD kernels define elementary surfaces using arbitrary local spatial
placement.

The qualified AP Mesh frame envelope currently permits only exact
signed-permutation bases and positive power-of-two scale.

Therefore implementing a general plane/cylinder/cone/sphere/torus now would
either:

- silently exceed the qualified frame claim; or
- introduce a new arbitrary-placement representation without first deciding
  its numeric and qualification semantics.

That prerequisite must be explicit.

## 20. Why general trimming is deferred

Static rectangular trimming is now integrated.

General trimming additionally requires:

- parameter-space trimming curves/p-curves;
- loop ordering/orientation;
- multiple loops/holes;
- correspondence to physical-space boundaries;
- explicit topological face/use identity.

Those belong to a larger representation/topology seam and should not be hidden
inside the next surface value type.

## 21. Why broader Coons is deferred

The current Coons patch accepts four oriented cubic Bézier boundary curves.

Generalizing it to arbitrary bounded curve families requires a separate
cross-family boundary/error/continuity contract.

Linear extrusion isolates swept-surface semantics without that additional
generic boundary problem.

## 22. Why remaining NURBS breadth is deferred

Current surface NURBS already includes runtime span counts and simple/double
interior multiplicities in U/V under degree three.

Arbitrary degree, multiplicity three and periodicity remain important CAD
breadth but are not prerequisites for linear extrusion.

## 23. Repository mapping for future implementation

Authorized future mapping is limited to:

- public swept surface:
  `include/apmesh/geometry/extrusion_surface.hpp`;
- production:
  `src/geometry/extrusion_surface.cpp`;
- focused contract:
  `tests/surface_linear_extrusion.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  this decision.

Existing curve/surface headers and production semantics should remain frozen
unless a strictly mechanical include exposure is necessary.

No topology, trim-loop, p-curve, analytic-surface or meshing file is authorized.

## 24. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST PASS;
- GCC 13 Debug / INTEGRATION PASS;
- Clang 18 libc++ Debug / INTEGRATION PASS;
- all existing 31 ordinary tests remain PASS;
- new extrusion contract PASS.

Passing yields only:

**LINEAR EXTRUSION SURFACE IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or the swept family generally.

## 25. Stop conditions

Stop and require a new decision if implementation needs:

- arbitrary basis-curve polymorphism/generic error translation;
- rotation/revolution;
- periodic parameters;
- arbitrary-angle frame admission;
- analytic elementary surface types;
- trim loops/p-curves/topology;
- surface normals/curvature/metric;
- meshing/discretization;
- universal tolerance;
- third-party runtime dependency.

## 26. Planned sequence after this work unit

Planning only, not authorization.

After linear extrusion integration/closure, a fresh Surface Representation
breadth decision should recompare:

1. explicit arbitrary-placement prerequisite for analytic elementary surfaces;
2. bounded revolution surface;
3. general trimmed-surface / p-curve / face-boundary semantics;
4. broader Coons/transfinite boundaries;
5. remaining NURBS degree/multiplicity/periodic breadth.

No option is pre-authorized.

## 27. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item is:

**Bounded Cubic Bézier Linear Extrusion Surface in 3D.**

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

Surface Differential Geometry and Boundary Curve Discretization remain blocked.


## 28. Decision integration checkpoint

PR #168 merged as `b8fe106080513a9736172a2380d5d8c0f162276a`.

Validation:

- PR FAST `35885189655`: PASS;
- PR INTEGRATION `35885189624`: PASS in GCC and Clang;
- post-merge FAST `35885367665`: PASS;
- post-merge INTEGRATION `35885367613`: PASS.

The decision is integrated and ready for documentation closure.

After closure integration and post-merge validation, the sole next work item is
the bounded `CubicBezierLinearExtrusionSurface3` implementation.


## 29. Decision closure checkpoint

Decision closure PR #169 used head
`eadb04268ed167759e5eaa9d1004922c43bb5404`.

Closure PR validation:

- FAST `35885907493`: PASS;
- INTEGRATION `35885907499`: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

PR #169 merged as
`f2c37429a1d3e0f6700c58a0f20c2f290e8148ee`.

Closure post-merge validation:

- FAST `35886133289`: PASS;
- INTEGRATION `35886133213`: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is
`CubicBezierLinearExtrusionSurface3`, bounded by Sections 5–25.


## 30. Active implementation mapping

The sole authorized implementation is active on:

`surface/cubic-bezier-linear-extrusion`.

Candidate mapping:

- public API:
  `include/apmesh/geometry/extrusion_surface.hpp`;
- production:
  `src/geometry/extrusion_surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_linear_extrusion.cpp`;
- build/test registration:
  `CMakeLists.txt`.

Candidate implementation stores:

- exact basis `CubicBezier3`;
- prevalidated translated end `CubicBezier3`;
- exact extrusion displacement.

Candidate production semantics:

- exact `[0,1]^2` parameter domain;
- deterministic U/V validation order;
- `S=C(u)+vE`;
- analytic `Su=C'(u)`, `Sv=E`, `Suu=C''(u)`;
- exact zero `Suv` and `Svv`;
- exact storage-based U/V reversal;
- no generic curve-family error translation contract.

The focused contract includes:

- independent direct Bernstein + extrusion reference;
- all four boundaries;
- U/V reversal and orientation relation;
- zero extrusion and constant-basis degenerates;
- translation and exact power-of-two scale;
- admitted signed-permutation/power-of-two Cartesian-frame covariance;
- extreme finite success;
- construction-time non-representable translated-control rejection;
- deterministic success/failure evidence.

Expected ordinary semantic inventory: **32 tests**.

Current status:

**IMPLEMENTED CANDIDATE / PRE-PR VALIDATION PENDING / NOT QUALIFIED.**


## 31. Active implementation validation

Candidate head:

`c45cfa573c686a5c96c8c3c9fc4cb0fa14c472ba`.

Candidate validation:

- FAST `35887599787`: PASS, 32/32 ordinary semantic tests;
- INTEGRATION `35887599839`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 32/32 tests per cell;
- `apmesh_core.surface_linear_extrusion`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Validated candidate scope includes:

- exact normalized `[0,1]^2` domain;
- stored cubic Bézier basis, translated end curve and extrusion displacement;
- independent Bernstein-plus-extrusion value/partial evidence;
- `Su=C'(u)`, `Sv=E`, `Suu=C''(u)`, exact-zero `Suv/Svv`;
- four boundary families;
- U/V reversal and orientation covariance;
- zero-extrusion and constant-basis degenerates;
- translation, exact power-of-two scale and admitted Cartesian-frame
  covariance;
- extreme finite success;
- construction-time rejection of non-representable translated end controls;
- deterministic repeated success/failure evidence.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**

No generic extrusion, revolution, analytic elementary surface, general
trim/p-curve/topological-face, differential-geometry or meshing capability is
implied.


## 32. Implementation integration checkpoint

The bounded implementation was integrated by PR #170.

Candidate head:

`c45cfa573c686a5c96c8c3c9fc4cb0fa14c472ba`.

Candidate validation:

- FAST `35887599787`: PASS, 32/32 ordinary semantic tests;
- INTEGRATION `35887599839`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 32/32 tests per cell.

Final PR head:

`87d0ecc0dbb7a5dda34164834d8c30c473779102`.

Final-head validation:

- FAST `35891866350`: PASS, 32/32;
- INTEGRATION `35891867091`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 32/32 per cell.

PR #170 merged as:

`63b4d963fbed25e6482d37ae1944a08799043b2c`.

Post-merge validation:

- FAST `35892078309`: PASS, 32/32;
- INTEGRATION `35892078255`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 32/32 per cell.

Integrated result:

**LINEAR EXTRUSION SURFACE IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / CLOSURE PENDING / NOT QUALIFIED.**

No broader swept, analytic, trimming, differential-geometry or meshing
capability is implied.

After implementation closure, the next admissible work is one fresh
literature-backed Surface Representation breadth decision. No option is
pre-authorized.


## 33. Implementation closure checkpoint

Implementation closure PR #171 used head:

`6bd10e79ddc80fc7c1330ecb7d570148b1a0e841`.

Closure PR validation:

- FAST `35892445808`: PASS;
- INTEGRATION `35892445798`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #171 merged as:

`fccb7330e9fb0e5a45b53d9e72efee689f30ea2f`.

Closure post-merge validation:

- FAST `35892574623`: PASS;
- INTEGRATION `35892574146`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Terminal result:

**LINEAR EXTRUSION SURFACE IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / CLOSED / NOT QUALIFIED.**

No implementation work item remains active.

The sole next admissible work is one fresh literature-backed Surface
Representation breadth decision comparing analytic-placement prerequisites,
bounded revolution, general trimming/p-curve/face seams, broader
Coons/transfinite boundaries and remaining NURBS breadth. No option is
pre-authorized.
