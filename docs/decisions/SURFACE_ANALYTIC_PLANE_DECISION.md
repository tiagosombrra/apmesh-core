# Bounded Analytic Plane Surface — Surface Representation Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-24  
Stage: Surface Representation — Continuous Patch Geometry

## 1. Question

After terminal closure of the bounded cubic Bézier surface-of-revolution work
unit, which Surface Representation breadth item should be introduced next?

Required candidates:

1. analytic elementary surfaces;
2. general trimming / p-curves / topological faces;
3. broader Coons/transfinite boundaries;
4. remaining NURBS degree/C0/periodic breadth;
5. opening Surface Differential Geometry.

If analytic elementary surfaces are selected, the first work unit must isolate
exactly one elementary family rather than bundling plane, cylinder, cone,
sphere and torus despite their materially different domain, periodicity and
singularity semantics.

## 2. Fresh repository authority

Decision-entry main:

`65ed30fbd4e383a886b78b20ad8480d38557254a`.

Latest closed focused work unit:

**Bounded Cubic Bézier Surface of Revolution in 3D.**

Terminal evidence:

- implementation PR #181:
  `44417fab90973baf9a2be4ec07f6eed8fdcc186e`;
- implementation closure PR #182:
  `e3fd83430ed664c9355ab4c7630316003ad6642d`;
- terminal sync PR #183:
  `65ed30fbd4e383a886b78b20ad8480d38557254a`;
- sync PR FAST `35977741146`: PASS;
- sync PR INTEGRATION `35977741076`: PASS;
- sync post-merge FAST `35977907185`: PASS;
- sync post-merge INTEGRATION `35977907165`: PASS;
- ordinary semantic inventory: **34 tests**;
- no open PR and no active production work item at decision entry.

Current Surface Representation production breadth includes:

- tensor-product bicubic polynomial Bézier;
- positive-weight rational bicubic Bézier;
- bicubic positive-weight NURBS with simple knots;
- bicubic positive-weight NURBS with interior multiplicity one/two;
- oriented cubic Bézier Coons patch;
- static rectangular trimmed surface;
- cubic Bézier linear extrusion;
- cubic Bézier bounded revolution;
- arbitrary right-handed `AxisPlacement3`.

No dedicated analytic elementary surface type exists.

## 3. Literature and mature-kernel evidence

### 3.1 Elementary surfaces are a distinct CAD family

Open CASCADE geometry model taxonomy:

https://dev.opencascade.org/sites/default/files/pdf/Geometry.pdf

The mature CAD model separates:

- plane;
- cylindrical surface;
- conical surface;
- spherical surface;
- toroidal surface;

from free-form Bézier/B-spline, swept and trimmed surfaces.

Decision impact:

- extrusion/revolution/free-form representations do not replace explicit
  elementary-surface coverage;
- Surface Representation remains materially incomplete without an explicit
  elementary-family plan.

### 3.2 Plane parameterization and placement

Open CASCADE `Geom_Plane`:

https://dev.opencascade.org/doc/refman/html/class_geom___plane.html

Relevant evidence:

- a plane is positioned by a 3D axis placement;
- placement X/Y directions define U/V isoparametric directions;
- the natural plane is unbounded in U and V;
- U/V reversal is meaningful orientation behavior;
- first/second partial derivatives are part of the surface contract.

Decision impact:

- AP Mesh can reuse the already integrated `AxisPlacement3`;
- because the project surface concept is bounded, the first production plane
  must explicitly store finite U/V intervals rather than pretending an
  unbounded natural domain fits `BoundedParametricSurface3`;
- plane adds no periodic or singular parameter seam and is therefore the
  smallest analytic elementary work unit.

### 3.3 Other elementary surfaces have materially different seams

Open CASCADE references:

- cylinder:
  https://dev.opencascade.org/doc/refman/html/class_geom___cylindrical_surface.html
- cone:
  https://dev.opencascade.org/doc/refman/html/class_geom___conical_surface.html
- sphere:
  https://dev.opencascade.org/doc/refman/html/class_geom___spherical_surface.html
- torus:
  https://dev.opencascade.org/doc/refman/html/class_geom___toroidal_surface.html

Relevant differences:

- cylinder/cone use an angular U domain and unbounded natural axial direction;
- sphere uses full angular U and latitude V with pole singularities;
- torus has angular structure in both U and V;
- cone has an apex singularity;
- periodic seam, radius constraints and singular parameterization therefore
  cannot be smuggled into a plane work unit.

External references are design/scientific evidence only. No external CAD
runtime dependency is admitted.

## 4. Candidate comparison

| Candidate | New semantics now | Dependency status | Downstream value | Decision |
| --- | --- | --- | --- | --- |
| Bounded analytic plane | Arbitrary finite U/V domain over `AxisPlacement3`; exact affine surface | Placement prerequisite closed | High: first elementary family and exact differential-geometry oracle | **SELECTED** |
| General trimming / p-curves / topological faces | arbitrary loops, holes, curve-on-surface consistency, topology binding | multiple unresolved seams | Very high, but too broad for one bounded step | DEFER |
| Broader Coons/transfinite | heterogeneous/rational/NURBS boundary filling | current cubic Coons baseline exists | Moderate | DEFER |
| Remaining NURBS breadth | arbitrary degree, C0, periodic U/V | bounded NURBS baseline exists | High but independent of plane | DEFER |
| Surface Differential Geometry | normals, metric, curvatures, regularity | Representation stage still missing explicit elementary family coverage | Very high | DEFER until analytic baseline starts |

## 5. Decision

Authorize exactly one future implementation work unit:

**Bounded Analytic Plane Surface in 3D.**

Proposed public type:

`BoundedPlaneSurface3`.

The work unit introduces the first dedicated elementary analytic surface but
does not claim the elementary-surface family is complete.

## 6. Stored representation

The type stores exactly:

- one validated `AxisPlacement3`;
- one finite valid U `CurveParameterDomain`;
- one finite valid V `CurveParameterDomain`;
- one U-reversal flag;
- one V-reversal flag.

No separate normal vector is stored; the placement Z direction is implied by
the right-handed placement.

The reversal flags avoid rebuilding a placement through translated origins and
prevent avoidable overflow from expressions such as `u_min + u_max`.

## 7. Parameter domain

The public domain is:

`SurfaceParameterDomain{u_domain, v_domain}`.

Both intervals may be arbitrary finite strict intervals already admitted by
`CurveParameterDomain`.

The type does not normalize them to `[0,1]`.

This is an intentional new Surface Representation seam: family-specific
bounded domains beyond the normalized unit square.

## 8. Mathematical representation

Let the placement define:

- origin `O`;
- unit X direction `X`;
- unit Y direction `Y`;
- unit right-handed Z direction `Z`.

For effective parameters `u_e`, `v_e`:

`S(u,v) = O + u_e X + v_e Y`.

Without reversal:

- `u_e = u`;
- `v_e = v`.

With U reversal:

- `u_e = reversed_parameter(u_domain,u)`.

With V reversal:

- `v_e = reversed_parameter(v_domain,v)`.

Production should use the existing `AxisPlacement3::point_to_world` on the
local point `(u_e,v_e,0)` or an algebraically equivalent checked path.

No universal epsilon is used.

## 9. Query validation and failures

The common `SurfaceError` vocabulary remains unchanged.

Validation order remains the established common order:

1. U finite;
2. V finite;
3. U in domain;
4. V in domain;
5. finite representable result.

Required failures:

- non-finite U:
  `SurfaceError::non_finite_u_parameter`;
- non-finite V:
  `SurfaceError::non_finite_v_parameter`;
- finite U outside domain:
  `SurfaceError::u_parameter_out_of_domain`;
- finite V outside domain:
  `SurfaceError::v_parameter_out_of_domain`;
- unrepresentable final point/vector:
  `SurfaceError::non_finite_result`.

No clamping or periodic wrapping is permitted.

## 10. First partial derivatives

The analytic first partials are constant.

If U is not reversed:

`S_u = X`.

If U is reversed:

`S_u = -X`.

If V is not reversed:

`S_v = Y`.

If V is reversed:

`S_v = -Y`.

The derivatives are independent of parameter values after validation.

## 11. Second partial derivatives

All second partials are exactly zero:

- `S_uu = 0`;
- `S_uv = 0`;
- `S_vv = 0`.

The zero vectors should be returned exactly.

No numerical differentiation is permitted.

## 12. Reversal semantics

Required public operations:

- `u_reversed()`;
- `v_reversed()`.

They toggle the corresponding stored flag and preserve the same public domain.

Required covariance:

- U reversal:
  `R_u(S)(r_u(u),v)=S(u,v)`;
- V reversal:
  `R_v(S)(u,r_v(v))=S(u,v)`;
- U reversal negates `S_u` and preserves `S_v`;
- V reversal negates `S_v` and preserves `S_u`;
- one reversal flips orientation of `S_u x S_v`;
- two direction reversals preserve orientation;
- double reversal recovers the exact stored representation.

## 13. Boundary semantics

The four bounded edges are analytic line segments in physical space.

Focused evidence must compare each edge against the existing
`LineSegment3` physical locus after affine parameter mapping:

- U lower;
- U upper;
- V lower;
- V upper.

The plane implementation does not own or infer topology identity for those
edges.

Boundary identity remains a later explicit topology/trim concern.

## 14. Independent analytic oracle

Tests must compute an independent long-double affine reference:

`O + uX + vY`.

The oracle must not call the production plane evaluation helper.

It must cover:

- non-origin placement;
- genuinely arbitrary right-handed placement;
- non-unit-square asymmetric U/V domains;
- interior parameters;
- all four corners;
- first partials;
- zero second partials.

## 15. Affine and placement evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two coordinate scaling where representable;
- placement with axis not aligned to signed coordinate permutations;
- exact `AxisPlacement3` stored identity;
- no mutation of the qualified `CartesianFrame3` contract.

## 16. Degenerate/singularity semantics

A valid plane placement is always regular because `AxisPlacement3` stores
orthonormal X/Y directions.

The work unit does not introduce a regularity API, normal API or curvature API.

Those belong to Surface Differential Geometry.

## 17. Extreme finite evidence

At least one fixture must use:

- extreme finite placement origin and/or finite U/V bounds;
- values for which a representable query result exists;
- a separate case where the final result cannot be represented.

Required behavior:

- succeed for representable final values;
- return `SurfaceError::non_finite_result` when arithmetic cannot produce a
  finite result;
- no hidden rescaling changes the represented plane.

## 18. Determinism

Repeated identical:

- value queries;
- first-partial queries;
- second-partial queries;
- typed failures;
- reversal operations

must produce identical fields/results.

## 19. Header isolation

The first elementary-surface header must not depend on:

- topology;
- trimming loops;
- p-curves;
- meshing;
- I/O;
- threading;
- external CAD kernels.

## 20. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept | `BoundedPlaneSurface3` satisfies `BoundedParametricSurface3`. |
| Domain | Arbitrary finite strict U/V intervals preserved exactly. |
| Query failures | Established U/V typed errors preserved. |
| Analytic oracle | Independent affine point oracle matches. |
| First partials | Exact ±X/±Y according to reversal flags. |
| Second partials | Exact zero vectors. |
| Corners | Four analytic corner identities. |
| Boundaries | Four physical edges match `LineSegment3` after parameter mapping. |
| U reversal | Value/partial covariance and exact involution. |
| V reversal | Value/partial covariance and exact involution. |
| Orientation | One reversal flips cross-product direction; two preserve it. |
| Arbitrary placement | Non-axis-aligned `AxisPlacement3` succeeds. |
| Translation/scale | Declared covariance. |
| Extreme finite | Explicit success/failure without universal epsilon. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/trim/mesh/I/O/threading/external dependency. |
| Regression | Existing 34 ordinary tests remain PASS. |

If exactly one focused contract is added, ordinary FAST/INTEGRATION inventory
becomes **35 tests**.

## 21. Why the plane is selected first

The plane introduces the elementary-surface category with the fewest unrelated
seams:

- no radius;
- no angular periodicity;
- no apex;
- no poles;
- no toroidal double periodicity;
- no new common surface errors;
- no new placement abstraction.

It also introduces arbitrary finite U/V domains and provides an exact
zero-curvature production surface for later differential-geometry validation.

## 22. Why cylinder is deferred

The natural cylinder adds:

- angular U semantics;
- a natural periodic seam;
- radius validation;
- an unbounded natural axial parameter.

The existing bounded revolution surface can already serve as an independent
cylindrical-sector oracle, but it does not replace a future dedicated analytic
cylinder representation.

Cylinder should be reconsidered immediately after plane closure.

## 23. Why cone is deferred

Cone additionally introduces:

- semi-angle semantics;
- apex singularity;
- parameter regions crossing or approaching the apex.

Those semantics require a separate bounded decision.

## 24. Why sphere is deferred

Sphere introduces:

- angular periodic U;
- latitude domain;
- pole singularities;
- orientation behavior at degenerate parameter lines.

It is an important later production family and differential-geometry fixture,
but not part of this plane work unit.

## 25. Why torus is deferred

Torus introduces:

- two angular directions;
- two periodic seams;
- major/minor radius constraints;
- spindle/horn/self-intersection policy if non-standard radii are admitted.

It requires a separate decision.

## 26. Why general trimming remains deferred

General trim requires several independent seams at once:

- p-curves;
- arbitrary loops and holes;
- loop orientation;
- curve-on-surface physical consistency;
- edge/face topology identity.

The existing rectangular trim remains a bounded baseline.

General trimming is still mandatory before a broad CAD-face envelope is
claimed.

## 27. Why Surface Differential Geometry remains deferred

The representation stage is still missing the explicit analytic elementary
family that the roadmap requires.

The plane work unit creates the first exact elementary production surface and a
future zero-curvature oracle.

After plane closure, the next breadth decision should reconsider:

- cylinder and the remainder of elementary surfaces;
- general trimming;
- whether current representation breadth is then sufficient to open Surface
  Differential Geometry.

## 28. Explicit exclusions

This decision does not authorize:

- cylinder;
- cone;
- sphere;
- torus;
- complete/periodic analytic surfaces;
- general trimming or p-curves;
- topological face binding;
- broader Coons/NURBS;
- Surface Differential Geometry;
- normals/metrics/curvatures;
- Boundary Curve Discretization;
- meshing;
- Quad-Dominant work;
- parallel execution;
- third-party runtime dependencies.

## 29. Repository mapping for future implementation

Authorized future mapping is limited to:

- public elementary-surface family header:
  `include/apmesh/geometry/elementary_surface.hpp`;
- production:
  `src/geometry/elementary_surface.cpp`;
- focused contract:
  `tests/surface_plane.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No change to `parametric_surface.hpp`, `geometry.hpp`, `curve.hpp` or
existing surface production files is expected.

If a common contract must change, stop and require a new decision.

## 30. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 34 ordinary semantic tests must remain passing;
- the new plane focused contract must pass.

Passing yields only:

**ANALYTIC PLANE IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or any other elementary surface.

## 31. Stop conditions

Stop and require a new decision if implementation needs:

- new `SurfaceError` values;
- changes to `AxisPlacement3`;
- changes to `BoundedParametricSurface3`;
- periodic semantics;
- radius/angle/apex/pole policy;
- general trimming/topology;
- surface differential geometry;
- universal epsilon;
- third-party runtime dependency.

## 32. Planned next comparison

Planning only, not authorization.

After bounded plane implementation/closure, freshly compare:

1. bounded analytic cylinder;
2. bounded analytic cone;
3. bounded analytic sphere;
4. bounded analytic torus;
5. general trimming/p-curves/topological faces;
6. whether Surface Representation breadth is sufficient to open Surface
   Differential Geometry.

No option is pre-authorized.

## 33. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Bounded Analytic Plane Surface in 3D.**

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

No other elementary surface, trimming, differential geometry or meshing
capability is authorized.


## 34. Decision integration checkpoint

Decision PR #184 final head:

`5f0bc6f5967e7409024ee0d5ec4b191f5978397a`.

Final decision-head validation:

- FAST `35978702792`: PASS;
- INTEGRATION `35978702703`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #184 merged as:

`9bc9a2770a1fdd3b2c5034ec4e2e003cddcd3176`.

Post-merge validation:

- FAST `35989404234`: PASS;
- INTEGRATION `35989404276`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Integrated decision result:

**DECISION INTEGRATED / CLOSURE PENDING / IMPLEMENTATION NOT STARTED /
NOT QUALIFIED.**

After this closure is integrated and post-merge validated, the sole next
production work item is `BoundedPlaneSurface3` under Sections 5–31.

No other elementary surface, periodic seam, trimming/topology, broader
Coons/NURBS, Surface Differential Geometry or meshing capability is
authorized.


## 35. Decision closure checkpoint

Decision closure PR #185 final head:

`595b13fa19d7d17548a550633f5f06eb167b81a2`.

Closure PR validation:

- FAST `35989671595`: PASS;
- INTEGRATION `35989671407`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #185 merged as:

`d90bc0c7bac80c38cb02fc2d4e7b72ab8c5aa848`.

Closure post-merge validation:

- FAST `35989828872`: PASS;
- INTEGRATION `35989828884`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is `BoundedPlaneSurface3` under
Sections 5–31.

No cylinder, cone, sphere, torus, periodic seam, trimming/topology, broader
Coons/NURBS, Surface Differential Geometry or meshing capability is
authorized.


## 36. Active implementation mapping

The sole authorized implementation is active on:

`surface/analytic-plane`.

Candidate mapping:

- public family:
  `include/apmesh/geometry/elementary_surface.hpp`;
- production:
  `src/geometry/elementary_surface.cpp`;
- focused contract:
  `tests/surface_plane.cpp`;
- build/test registration:
  `CMakeLists.txt`.

Candidate semantics:

- stores the exact validated `AxisPlacement3` and U/V domains;
- stores only U/V reversal flags in addition;
- preserves the common `SurfaceError` and
  `BoundedParametricSurface3` contracts unchanged;
- validates U finite, V finite, U domain, V domain in established order;
- evaluates the affine plane through the existing placement transform;
- returns exact signed placement X/Y first partials;
- returns exact zero second partials;
- reflects parameters only through the existing overflow-aware
  `reversed_parameter`;
- introduces no topology or differential-geometry state.

The focused contract includes the complete Section 20 matrix, including four
`LineSegment3` boundary-locus checks, arbitrary non-axis-aligned placement,
translation, power-of-two scaling, extreme finite success/failure and
deterministic repeat evidence.

Expected ordinary semantic inventory: **35 tests**.

Candidate validation:

- candidate head:
  `8945b7ff71e7363ad509116c4659cebf180ad3d3`;
- FAST `35991297348`: PASS, 35/35 ordinary semantic tests;
- INTEGRATION `35991297411`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 35/35 tests per cell;
- `apmesh_core.surface_plane`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**


## 37. Implementation integration and closure

Candidate head:

`8945b7ff71e7363ad509116c4659cebf180ad3d3`.

Candidate validation:

- FAST `35991297348`: PASS, 35/35 ordinary semantic tests;
- INTEGRATION `35991297411`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 35/35 tests per cell.

Final PR head:

`7cb3e38d046144282d183dc7e68d9476b36c484a`.

Final PR validation:

- FAST `35991554343`: PASS, 35/35;
- INTEGRATION `35991554362`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 35/35 tests per cell.

PR #186 merged as:

`89ec9946b7438b30a0b3b3218c8c9c1981b31fd8`.

Protected-main validation:

- FAST `35991745316`: PASS, 35/35;
- INTEGRATION `35991745435`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 35/35 tests per cell;
- `apmesh_core.surface_plane`: PASS;
- every prior ordinary semantic contract remained PASS.

Terminal work-unit result:

**ANALYTIC PLANE IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSED / NOT QUALIFIED.**

No plane implementation work remains active.

The next admissible action is a fresh literature-backed comparison of bounded
analytic cylinder, cone, sphere, torus, general trimming/p-curves/topological
faces, and Surface Differential Geometry entry readiness.

No option is pre-authorized.


## 38. Implementation closure checkpoint

Implementation closure PR #187 used head:

`79c9028e56913799613292948d452f53e2b3ead1`.

Closure PR validation:

- FAST `35992244200`: PASS;
- INTEGRATION `35992244354`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #187 merged as:

`57f647e488bf3498168bc9c3b8c63fc5442d034a`.

Closure post-merge validation:

- FAST `35992410876`: PASS;
- INTEGRATION `35992410395`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Terminal result:

**ANALYTIC PLANE IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSED / NOT QUALIFIED.**

No production work item remains active.

After terminal documentation synchronization, the sole next admissible work is
a fresh literature-backed Surface Representation breadth decision comparing
bounded analytic cylinder, cone, sphere, torus, general trimming/p-curves/
topological faces, and Surface Differential Geometry entry readiness.

No candidate is pre-authorized.
