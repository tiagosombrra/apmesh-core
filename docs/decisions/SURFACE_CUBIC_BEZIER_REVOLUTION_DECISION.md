# Bounded Cubic Bézier Revolution Surface — Surface Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-23  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites at entry:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation polynomial cubic Bézier baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- bounded 3D surface abstraction: integrated;
- bicubic polynomial/rational/NURBS surfaces: integrated focused work units;
- NURBS U/V simple/double-knot continuity semantics: integrated;
- oriented cubic Bézier Coons patch: integrated;
- static oriented rectangular trim: integrated;
- bounded cubic Bézier linear extrusion: integrated;
- Right-Handed Arbitrary 3D Axis Placement:
  integrated and terminally closed;
- ordinary semantic inventory at entry: 33 tests.

## 1. Question

After closing the arbitrary-axis placement prerequisite, which single Surface
Representation breadth seam should be introduced next?

Required candidates:

1. bounded revolution surface;
2. analytic elementary surfaces enabled by `AxisPlacement3`;
3. general trimming / p-curve / topological-face semantics;
4. broader Coons/transfinite boundaries;
5. remaining NURBS degree/multiplicity/periodic breadth.

The next unit must add one bounded scientific concept only. It must not
simultaneously open periodic surfaces, a runtime surface hierarchy, general
topological faces, or a collection of unrelated analytic families.

## 2. Fresh repository authority

Decision-entry `main`:

`2f6b4bc269146bc9d27bd7351622f5d37093754c`.

Terminal arbitrary-axis-placement lineage:

- implementation PR #176:
  `6d90036300671656c3bbde459dd2a783f8457cc1`;
- implementation post-merge FAST `35902586443`: PASS, 33/33;
- implementation post-merge INTEGRATION `35902586537`: PASS, 33/33;
- closure PR #177:
  `fda3d1284ed500cdd97a7b6b153791f9d0884818`;
- closure post-merge FAST `35903368279`: PASS;
- closure post-merge INTEGRATION `35903368467`: PASS;
- terminal reconciliation PR #178:
  `2f6b4bc269146bc9d27bd7351622f5d37093754c`;
- terminal reconciliation post-merge FAST `35904119775`: PASS;
- terminal reconciliation post-merge INTEGRATION `35904119725`: PASS;
- no open PR and no active production work item at decision entry.

## 3. Literature and mature-kernel evidence

### 3.1 Surface of revolution is a generator + axis + angular sweep

Open CASCADE `Geom_SurfaceOfRevolution`:

https://dev.opencascade.org/doc/occt-7.8.0/refman/html/Geom__SurfaceOfRevolution_8hxx.html

Relevant evidence:

- a surface of revolution rotates a generating curve around an axis;
- the revolution parameter is angular;
- the second parameter follows the generating curve;
- a complete revolution uses a periodic angular interval.

IGES Type 120 surface of revolution:

https://dev.opencascade.org/doc/refman/html/class_i_g_e_s_geom___surface_of_revolution.html

Relevant evidence:

- the model stores an axis, a generatrix, a start angle and an end angle;
- bounded angular sweeps are therefore a mature CAD representation separate
  from complete periodic revolution.

Decision impact:

- AP Mesh can admit one **bounded non-periodic revolution segment** now that an
  arbitrary right-handed axis placement exists;
- a complete 2*pi periodic seam remains a separate later decision.

### 3.2 Elementary analytic surfaces now have their placement prerequisite, but
they are multiple families

Open CASCADE spherical surface:

https://dev.opencascade.org/doc/refman/html/class_geom___spherical_surface.html

Open CASCADE cylindrical surface construction:

https://dev.opencascade.org/doc/refman/html/class_g_c___make_cylindrical_surface.html

Relevant evidence:

- sphere/cylinder use an explicit local coordinate system;
- angular parameters are periodic;
- sphere and cylinder have distinct second-parameter semantics and singular or
  unbounded natural domains;
- a sphere uses U in [0,2*pi] and V in [-pi/2,pi/2];
- a cylinder uses U in [0,2*pi] and an unbounded axial parameter.

Decision impact:

- `AxisPlacement3` removes the arbitrary-placement blocker;
- however, admitting "analytic elementary surfaces" as one work item would
  still combine several families plus periodic/unbounded-domain decisions;
- they remain the next major family group after the bounded revolution seam.

### 3.3 General trimming remains a topology/p-curve seam

Open CASCADE B-rep curve-on-surface tooling:

https://dev.opencascade.org/doc/refman/html/class_b_rep___tool.html

Relevant evidence:

- general trimmed faces use parameter-space curves and explicit face/edge
  topology in addition to the supporting surface;
- this is materially broader than the already integrated static rectangular
  trim wrapper.

Decision impact:

- general trimming remains deferred until p-curve, loop orientation and
  topology-binding semantics receive their own decision;
- it should not be approximated by another value-only subdomain wrapper.

### 3.4 Broader Coons/transfinite filling remains a construction-family breadth
question

Open CASCADE `GeomFill_BSplineCurves`:

https://dev.opencascade.org/doc/occt-7.9.0/refman/html/_geom_fill___b_spline_curves_8hxx.html

Relevant evidence:

- mature filling supports two, three or four B-spline boundary curves;
- filling style and compatible spline profiles are construction semantics;
- the current AP Mesh Coons work unit intentionally covers only four oriented
  cubic Bézier boundaries.

Decision impact:

- broader Coons/transfinite work remains a real obligation;
- it does not exploit the newly closed arbitrary-axis prerequisite as directly
  as revolution does.

### 3.5 Existing project sequencing

Internal authorities:

- `docs/decisions/SURFACE_CUBIC_BEZIER_LINEAR_EXTRUSION_DECISION.md`;
- `docs/decisions/SURFACE_ARBITRARY_AXIS_PLACEMENT_PREREQUISITE_DECISION.md`.

Decision impact:

- linear extrusion already isolates one swept-surface family;
- the placement decision explicitly identified bounded revolution as the first
  candidate to reconsider after `AxisPlacement3`;
- the new placement type is intentionally separate from the already qualified
  exact `CartesianFrame3`.

External sources are design/scientific evidence only. No external runtime
dependency or numerical oracle is admitted.

## 4. Candidate comparison

| Candidate | New semantics now | Prerequisite status | Downstream value | Decision |
| --- | --- | --- | --- | --- |
| Bounded cubic Bézier revolution | axis + signed angular sweep + trigonometric rotation of one qualified curve | **AxisPlacement3 + CubicBezier3 ready** | **Very high**; closes the second core swept family and exercises arbitrary axes | **SELECTED** |
| Analytic elementary surfaces | several shape families, family-specific domains, periodicity/singularities | placement ready; family/domain policies not yet isolated | Very high | DEFER |
| General trimming / p-curves / topological face | parameter-space loops, curve-on-surface, holes, topology identity | major new topology seam | Critical later | DEFER |
| Broader Coons/transfinite | heterogeneous/rational/NURBS boundary filling | current cubic Coons baseline exists | High | DEFER |
| Remaining NURBS breadth | arbitrary degree, C0/multiplicity-three, periodic U/V | independent of bounded revolution | High CAD import breadth | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Bounded Cubic Bézier Surface of Revolution in 3D.**

Proposed public type:

`CubicBezierRevolutionSurface3`.

The surface owns a cubic Bézier generatrix and rotates it around the Z axis of
a stored `AxisPlacement3` through one signed bounded sweep.

This work unit is intentionally **not a complete periodic revolution**.

## 6. Construction inputs

Proposed factory inputs:

- one finite validated `CubicBezier3` generatrix;
- one validated `AxisPlacement3`;
- one finite signed `sweep_angle`.

The angular source is the stored generatrix itself at U=0.

The angular target is the same generatrix rotated by `sweep_angle`.

No separate start angle is required in this first work unit.

## 7. Sweep-angle semantics

Required construction rules:

- `sweep_angle` must be finite;
- `sweep_angle != 0`;
- `abs(sweep_angle) < 2*pi` exactly under the represented double constant.

Positive sweep follows the right-handed sense about
`AxisPlacement3::z_direction()`.

Negative sweep reverses that orientation.

The strict sub-full-revolution bound is deliberate:

- no U-periodic identity is introduced;
- U=0 and U=1 remain distinct representation boundaries;
- seam equivalence at 2*pi is deferred to a later periodic-surface decision.

No epsilon participates in zero/full-sweep classification.

## 8. Proposed construction error vocabulary

A dedicated error set should distinguish at minimum:

- `non_finite_sweep_angle`;
- `zero_sweep_angle`;
- `full_or_multiple_revolution_not_admitted`;
- `non_finite_rotated_control`.

Validation order is deterministic.

The first three are domain/policy failures.

The final error covers inability to precompute a finite stored end generatrix.

## 9. Stored representation

Preferred stored state:

- exact input `CubicBezier3 start_curve`;
- precomputed finite `CubicBezier3 end_curve`;
- exact input `AxisPlacement3 axis`;
- exact input signed `sweep_angle`.

The end curve is obtained by rotating all four start controls through the full
sweep around the axis.

This mirrors the existing extrusion strategy of storing both parameter-boundary
curves and makes U reversal exact at representation level.

## 10. Parameter domain

The concrete family uses exactly:

`(u,v) in [0,1] x [0,1]`.

- U is normalized angular progress;
- V is the existing cubic Bézier parameter.

Physical angle:

`theta(u) = u * sweep_angle`.

The common `BoundedParametricSurface3` contract remains unchanged.

No clamping or periodic wrapping is permitted.

## 11. Rotation model

Let:

- `O` be the axis origin;
- `Z` be the unit axis direction;
- `P(v)` be the generatrix point;
- `r = P(v)-O`;
- `R_Z(theta)` be right-handed rotation around axis `Z`.

Then:

`S(u,v) = O + R_Z(theta(u)) r`.

A Rodrigues-style production formula is preferred:

- split `r` into axis-parallel and perpendicular components;
- rotate the perpendicular component with one `sin/cos` pair;
- preserve the parallel component.

The production result depends geometrically only on axis origin/direction.
The X/Y reference used to construct `AxisPlacement3` must not change the
represented revolution locus for the same origin/Z axis.

## 12. Deterministic trigonometric policy

Production uses the standard C++ trigonometric functions under the admitted
environment.

Required policy:

- compute one sine/cosine pair for the physical angle per query;
- no random or adaptive alternate formula;
- no tolerance-based angle canonicalization;
- U=0 and U=1 may use stored boundary curves directly;
- repeated identical queries in one admitted environment must return identical
  success/failure and components.

This work unit does not make a cross-libm bitwise-equivalence claim.

## 13. Analytic partial derivatives

Let `alpha = sweep_angle`.

Let `Q = R_Z(theta)(P(v)-O)`.

Then:

- `S_u = alpha * (Z x Q)`;
- `S_v = R_Z(theta) P'(v)`;
- `S_uu = alpha^2 * [Z x (Z x Q)]`;
- `S_uv = alpha * [Z x R_Z(theta) P'(v)]`;
- `S_vv = R_Z(theta) P''(v)`.

All five partials are analytic.

No finite-difference derivative is permitted.

Rank deficiency is not a representation failure.

## 14. Boundary semantics

Required U boundaries:

- U=0 equals stored `start_curve(v)`;
- U=1 equals stored `end_curve(v)`.

Required V boundaries:

- V=0 is the bounded circular arc traced by the first generatrix endpoint;
- V=1 is the bounded circular arc traced by the last generatrix endpoint.

No public circle-arc type is required by this work unit.

Tests may use independent analytic circle formulas.

## 15. U reversal

U reversal must preserve the represented locus and domain.

Preferred stored transformation:

- swap `start_curve` and `end_curve`;
- negate `sweep_angle`;
- preserve the exact `AxisPlacement3`.

Required covariance for `r(u)=1-u`:

- reversed value at `(r(u),v)` equals original value at `(u,v)`;
- reversed Su changes sign;
- reversed Sv preserves sign;
- reversed Suu preserves sign;
- reversed Suv changes sign;
- reversed Svv preserves sign.

Double U reversal must recover exact stored representation.

## 16. V reversal

V reversal:

- reverses both stored cubic Bézier boundary curves;
- preserves axis and sweep.

Required covariance for `r(v)=1-v`:

- value preserved under mapped V;
- Sv changes sign;
- Su preserves sign;
- Svv preserves sign;
- Suv changes sign;
- Suu preserves sign.

Double V reversal must recover exact stored representation.

## 17. Independent revolution oracle

Focused tests must implement an independent long-double reference using:

- direct cubic Bernstein evaluation of the generatrix;
- independent Rodrigues rotation;
- independent derivative formulas from Section 13.

The oracle must not call production revolution helpers.

Required parameters include:

- positive and negative sweeps;
- non-axis-aligned `AxisPlacement3`;
- multiple interior U/V pairs;
- U/V boundaries and corners.

## 18. Cylinder fixture

A cubic Bézier generatrix that is exactly a straight line parallel to the
rotation axis and at constant radius produces a cylindrical patch.

For identity placement with axis Z and radius R:

`S(u,v) = (R cos(theta), R sin(theta), z(v))`.

This fixture supplies an analytic reference independent of the general
production path.

Required comparison:

- value;
- Su/Sv;
- Suu/Suv/Svv.

This does not introduce a public cylinder type.

## 19. Planar annular-sector fixture

A radial linear cubic Bézier generatrix lying in a plane normal to the axis
produces an annular-sector patch.

This gives a second simple analytic fixture with different tangent structure.

No public plane or annulus type is introduced.

## 20. Axis-reference invariance

Two valid `AxisPlacement3` instances with:

- identical origin;
- identical Z direction;
- different X-reference-derived X/Y directions

represent the same axis of revolution.

For the same world-space generatrix and sweep, value/partials must agree within
the declared numeric comparison contract.

This evidence confirms that the revolution uses the axis line, not an
accidental azimuth convention.

## 21. Degenerate but representable cases

Admitted cases include:

- generatrix entirely on the axis;
- one generatrix endpoint on the axis;
- constant generatrix off the axis;
- generatrix tangent parallel to the axis at isolated parameters.

These may produce rank-deficient surface derivatives.

They remain valid representation values when finite.

No normal/regularity failure is introduced here.

## 22. Affine/placement evidence

Focused evidence must include:

- translated axis origin and translated generatrix;
- exact power-of-two coordinate scaling where representable;
- identity-axis fixture;
- genuinely arbitrary axis direction;
- parity with signed-permutation `CartesianFrame3`-compatible axes where
  applicable.

No widening of the qualified `CartesianFrame3` claim is implied.

## 23. Extreme finite evidence

At least one fixture must combine:

- large finite axis origin/generatrix coordinates;
- finite near-limit radius/offset;
- bounded nonzero sweep.

Required behavior:

- succeed if final value/partials are representable with scale-aware
  intermediates;
- otherwise return an explicit construction error or
  `SurfaceError::non_finite_result`.

No universal epsilon or silent sanitization is permitted.

## 24. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Type satisfies existing `BoundedParametricSurface3`. |
| Sweep validation | Non-finite/zero/full-or-multiple revolution rejected. |
| Exact domain | Exactly [0,1]^2. |
| U=0 boundary | Exact/declared parity with start generatrix. |
| U=1 boundary | Parity with stored rotated end generatrix. |
| Independent oracle | Value and five partials match Bernstein + Rodrigues reference. |
| Cylinder fixture | Analytic cylindrical patch relation. |
| Annular fixture | Analytic radial-sector relation. |
| Positive/negative sweep | Orientation and partial signs follow signed sweep. |
| Arbitrary axis | Non-signed-permutation placement succeeds. |
| Axis-reference invariance | Same axis line, different X/Y reference gives same geometry. |
| U reversal | Stored-boundary swap, sign covariance, exact involution. |
| V reversal | Generatrix reversal covariance and exact involution. |
| Degenerate generatrix | Representation succeeds without premature regularity rejection. |
| Translation/scale | Declared covariance. |
| Extreme finite | Explicit success/failure without universal epsilon. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/trim/mesh/I/O/threading/external dependency. |
| Prerequisite preservation | Existing 33 ordinary tests remain PASS. |

If exactly one new focused contract is added, ordinary FAST/INTEGRATION
inventory becomes **34 tests**.

## 25. Why bounded revolution is selected now

The prerequisite that previously blocked it has just been closed:

- a separate arbitrary right-handed 3D axis placement exists;
- the qualified exact `CartesianFrame3` remains untouched;
- cubic Bézier generatrix value/D1/D2 is already qualified;
- the bounded surface value/partial contract is integrated;
- linear extrusion already established the first swept-surface family.

Revolution therefore adds one focused new seam: bounded angular sweep around an
arbitrary axis.

## 26. Why analytic elementary surfaces are deferred

Placement is now ready, but the group still contains multiple materially
different families:

- plane;
- cylinder;
- cone;
- sphere;
- torus.

They differ in:

- bounded/unbounded natural domains;
- periodic dimensions;
- singularities;
- radius/angle parameter policies.

A later decision should admit them one family or one rigorously shared
abstraction at a time rather than bundle them into this sweep work unit.

## 27. Why general trimming is deferred

General trimming still requires:

- p-curves;
- arbitrary loops and holes;
- orientation of loop components;
- curve-on-surface consistency;
- explicit edge/face topology identity.

Those concerns are much broader than an untrimmed value-oriented revolution
patch.

## 28. Why broader Coons and NURBS are deferred

The existing Coons and NURBS surface work units already provide bounded
baselines.

The unresolved breadth there is primarily:

- heterogeneous/rational/NURBS boundary filling;
- arbitrary degree;
- C0/multiplicity-three;
- periodic U/V.

None is required to isolate bounded angular sweep semantics.

## 29. Repository mapping for future implementation

Authorized future mapping is limited to:

- public family:
  `include/apmesh/geometry/revolution_surface.hpp`;
- production:
  `src/geometry/revolution_surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_revolution.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No change to `parametric_surface.hpp`, `curve.hpp`, or
`geometry.hpp` is expected.

If those common contracts must change, stop and require a new decision.

## 30. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 33 ordinary semantic tests must remain passing;
- one new revolution contract must pass.

Passing yields only:

**BOUNDED REVOLUTION IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation, periodic surfaces, analytic
elementary surfaces or general swept surfaces.

## 31. Stop conditions

Stop and require a new decision if implementation needs:

- full 2*pi or multi-turn periodic seam semantics;
- a generic runtime generatrix family;
- arbitrary-degree generatrix;
- new `SurfaceError` semantics;
- new `AxisPlacement3` semantics;
- a public rotation/transform hierarchy;
- analytic cylinder/sphere/cone/torus production types;
- trimming/topology/p-curves;
- surface differential geometry;
- discretization/meshing;
- universal epsilon;
- third-party runtime dependency.

## 32. Planned next comparison

Planning only, not authorization.

After bounded revolution implementation/closure, freshly compare:

1. analytic elementary surfaces, now fully placement-unblocked;
2. general trimming/p-curves/topological faces;
3. broader Coons/transfinite boundaries;
4. remaining NURBS degree/C0/periodic breadth;
5. whether Surface Representation breadth is sufficient to open Surface
   Differential Geometry.

No option is pre-authorized.

## 33. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Bounded Cubic Bézier Surface of Revolution in 3D.**

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

No analytic elementary surface, general trimming, broader Coons/NURBS,
differential geometry or meshing capability is authorized.


## 34. Decision integration checkpoint

PR #179 integrated this bounded decision.

Final decision head:

`92db678f92fbf869e00540536156de25a1647113`.

Final decision-head validation:

- FAST `35909899142`: PASS;
- INTEGRATION `35909899200`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #179 merged as:

`112f3b7ae3c439d729380fec07d065997bf11e56`.

Post-merge validation:

- FAST `35910017412`: PASS;
- INTEGRATION `35910017524`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint is ready for documentation closure.

After closure integration and its post-merge validation, the sole next work
item is the bounded cubic Bézier revolution implementation defined by
Sections 5–31.

No periodic revolution, analytic elementary surface, general trimming,
broader Coons/NURBS or downstream capability is authorized.


## 35. Decision closure checkpoint

Decision closure PR #180 used final head:

`2f9f832febe91ab6bcd2d95550e944bd66c48f6f`.

Closure PR validation:

- FAST `35910276757`: PASS;
- INTEGRATION `35910276791`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #180 merged as:

`715ad5dc0ef068bec5f68b260df0dd3abd0fcf52`.

Closure post-merge validation:

- FAST `35910422052`: PASS;
- INTEGRATION `35910422072`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is the bounded cubic Bézier revolution
surface defined by Sections 5–31.

No complete periodic revolution, analytic elementary surface, general
trimming, broader Coons/NURBS or downstream capability is authorized.

## 36. Active implementation mapping

The sole authorized implementation is active on:

`surface/cubic-bezier-revolution`.

Candidate mapping is restricted to:

- `include/apmesh/geometry/revolution_surface.hpp`;
- `src/geometry/revolution_surface.cpp`;
- `tests/surface_revolution.cpp`;
- `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

Expected ordinary semantic inventory after registration: **34 tests**.

Current status:

**IMPLEMENTATION ACTIVE / PRE-PR VALIDATION PENDING / NOT QUALIFIED.**
