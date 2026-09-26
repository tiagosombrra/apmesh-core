# Bounded Analytic Spherical Surface — Scientific Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-26  
Parent stage: Surface Representation — Continuous Patch Geometry

## 1. Question

After terminal closure of pointwise ordered principal curvature values and exact
represented-data umbilic state, which single bounded scientific work unit
should be admitted next?

Required comparison:

1. principal directions and their line-field/umbilic semantics;
2. explicit conditioning diagnostics;
3. remaining sphere/cone/torus Surface Representation breadth;
4. general trimming/p-curves/topological faces;
5. Boundary Curve Discretization readiness;
6. bounded Surface Differential Geometry qualification readiness.

The next work unit must address a real prerequisite gap without pre-authorizing
a broader CAD or meshing seam.

## 2. Fresh repository authority

Decision-entry `main`:

`049c55f3ebb81a1e1851cc6f3b147b5a8da6a176`.

Final principal-curvature authority lineage:

- terminal publication PR #210 head:
  `466075be5fcdfaaf81d3855e3f1fe65e7af02b05`;
- PR #210 FAST `36251427419`: PASS;
- PR #210 INTEGRATION `36251427426`: PASS;
- PR #210 merge:
  `7cfd7cc34ecd813f8ec44a0356ec1ba172de26eb`;
- PR #210 post-merge FAST `36251514891`: PASS;
- PR #210 post-merge INTEGRATION `36251514909`: PASS;
- final authority PR #211 head:
  `621cbd0a8ec47375d21d357a2597dd9e20b4f879`;
- PR #211 FAST `36252051886`: PASS;
- PR #211 INTEGRATION `36252051820`: PASS;
- PR #211 merge:
  `049c55f3ebb81a1e1851cc6f3b147b5a8da6a176`;
- PR #211 post-merge FAST `36252124919`: PASS;
- PR #211 post-merge INTEGRATION `36252124966`: PASS.

Protected-main ordinary semantic inventory at decision entry: **39 tests**.

No open PR, no active production work item, and no competing analytic-sphere
branch exists at decision entry.

## 3. Relevant integrated prerequisites

Surface Representation already includes focused implementations for:

- bicubic polynomial Bézier;
- positive-weight rational bicubic Bézier;
- bicubic NURBS with simple/double knot continuity;
- Coons/transfinite patch;
- rectangular trimming;
- linear extrusion;
- revolution;
- arbitrary right-handed `AxisPlacement3`;
- bounded analytic plane;
- bounded analytic circular cylinder sector.

Surface Differential Geometry already includes:

- regularity;
- first fundamental form;
- area density;
- oriented unit normal;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state.

Still absent in production:

- analytic sphere;
- analytic cone;
- analytic torus;
- full-periodic cylinder/sphere seams;
- general trimming/p-curves/topological faces;
- principal directions/line fields;
- explicit conditioning diagnostics.

## 4. External scientific and CAD evidence

### 4.1 Mature sphere parameterization

Open CASCADE `Geom_SphericalSurface`:

https://dev.opencascade.org/doc/occt-7.6.0/refman/html/class_geom___spherical_surface.html

Relevant evidence:

- a sphere is defined by a local 3D placement and radius;
- its standard parameterization is
  `O + R sin(v) Z + R cos(v)(cos(u) X + sin(u) Y)`;
- natural U bounds are `[0,2*pi]`;
- natural V bounds are `[-pi/2,+pi/2]`;
- U is closed/periodic while V is not;
- first and second derivatives are first-class surface operations.

Decision impact:

- AP Mesh already has the required `AxisPlacement3` prerequisite;
- a bounded non-periodic spherical sector can reuse the current bounded-surface
  contract without first solving full periodic seams;
- the latitude endpoints expose the canonical polar parameterization
  singularity needed by the differential layer.

### 4.2 Sphere is the smallest remaining elementary-surface breadth step

Open CASCADE `Geom_ConicalSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___conical_surface.html

Relevant evidence:

- cone representation additionally requires reference radius and semi-angle;
- the V direction is unbounded in the natural surface;
- the apex has family-specific singular semantics;
- U remains periodic.

Open CASCADE `Geom_ToroidalSurface`:

https://dev.opencascade.org/doc/occt-7.9.0/refman/html/class_geom___toroidal_surface.html

Relevant evidence:

- a torus requires major and minor radii;
- both U and V are angular directions;
- both natural directions cover full `[0,2*pi]` periodic ranges.

Decision impact:

- cone and torus each introduce additional independent policy seams;
- sphere requires only placement, positive radius, longitude bounds and
  latitude bounds;
- sphere is therefore the smallest remaining elementary surface that still
  adds materially new differential evidence.

### 4.3 Principal directions require a separate line-field decision

Patrikalakis, Maekawa and Cho, *Shape Interrogation for Computer Aided Design
and Manufacturing*, lines of curvature:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node186.html

Relevant evidence:

- principal directions define the directions of minimum/maximum normal
  curvature;
- at non-umbilic points the two principal directions are orthogonal;
- integration into curvature lines is not a trivial pointwise eigenvector
  operation.

The same differential-geometry framework treats principal direction as
undefined/non-unique at an umbilic.

Decision impact:

- the repository now has exact represented-data umbilic state but no
  line-field/sign-continuity contract;
- a sphere is an especially strong fixture because every regular point is
  geometrically umbilic;
- introducing principal directions before deciding the undefined/line-field
  semantics would conflate eigenvector mechanics with a larger scientific
  contract.

### 4.4 General trimming/topological faces remain a separate cross-layer seam

Open CASCADE face construction requires parameter-space curve information for
general non-planar face boundaries; p-curves connect topological edges to an
underlying surface.

Reference family:
https://dev.opencascade.org/doc/refman/html/class_b_rep_builder_a_p_i___make_face.html

Decision impact:

- general trimming requires parameter-space curves, physical realization,
  loop orientation and explicit topology identity;
- this is much broader than a new analytic surface value type;
- it remains a prerequisite for CAD-like face coverage and later shared
  boundary certification, but not for a bounded sphere sector.

External sources are scientific/design evidence only. No external CAD runtime
dependency or numerical oracle is admitted.

## 5. Candidate comparison

| Candidate | New semantics now | Current prerequisite status | Immediate evidence value | Decision |
| --- | --- | --- | --- | --- |
| Bounded analytic sphere | radius + spherical latitude/longitude + exact polar singularity | arbitrary placement + surface derivatives + differential stack already integrated | **Very high**: missing production family + canonical K/H/principal/umbilic oracle | **SELECTED** |
| Principal directions | eigenvectors + sign/line-field + undefined-at-umbilic policy | principal values exist, but line-field contract absent | high later | DEFER |
| Conditioning diagnostics | condition representation/threshold policy | metric + curvature stack exists | high, but policy still independent | DEFER |
| Analytic cone | semi-angle + reference radius + apex policy | placement exists | high later | DEFER |
| Analytic torus | two radii + two angular/periodic seams | placement exists | high later | DEFER |
| General trim/p-curves/faces | p-curves + loops + topology binding | rectangular trim only | very high but cross-layer | DEFER |
| Boundary Curve Discretization | shared physical trace + parameterization invariance | general face/boundary seam incomplete | downstream-critical | BLOCKED |
| Differential-geometry qualification | stage-level regression/claim | sphere fixture and conditioning/breadth still incomplete | premature | DEFER |

## 6. Decision

Authorize exactly one future implementation work unit:

**Bounded Analytic Spherical Surface Sector in 3D.**

Proposed public type:

`BoundedSphereSurface3`.

This work unit belongs to **Surface Representation**.

It is also required as independent analytic evidence for the already open but
unqualified Surface Differential Geometry stage.

It does not qualify either stage.

## 7. Bounded representation scope

The first spherical family is deliberately bounded and non-periodic.

Stored state:

- `AxisPlacement3 placement`;
- finite strictly positive radius `R`;
- finite strict U domain;
- finite strict V domain;
- U-reversal state;
- V-reversal state.

U is longitude in radians.

V is latitude in radians.

Admitted U-domain rule:

- any already-valid finite strict `CurveParameterDomain`;
- represented angular width strictly less than the represented
  `2*pi` constant;
- full or multiple revolution is rejected;
- no modulo normalization or periodic wrapping.

Admitted V-domain rule:

- any already-valid finite strict `CurveParameterDomain`;
- lower bound must be at least represented `-pi/2`;
- upper bound must be at most represented `+pi/2`;
- endpoints may equal a canonical pole exactly.

This work unit does not implement a globally periodic full sphere.

## 8. Construction failures

Proposed bounded construction vocabulary:

`SphereSurfaceConstructionError`.

At minimum:

- `non_finite_radius`;
- `non_positive_radius`;
- `full_or_multiple_revolution_not_admitted`;
- `latitude_out_of_range`.

Existing `CurveParameterDomain` construction continues to own the finite
strict interval invariant.

No universal epsilon is introduced.

## 9. Parameterization

Let placement origin be `O` and its right-handed unit directions be
`X,Y,Z`.

For effective parameters `u,v`:

`Q(u) = cos(u) X + sin(u) Y`

`S(u,v) = O + R cos(v) Q(u) + R sin(v) Z`.

Production must not reduce U modulo `2*pi`.

U/V reversals use the existing overflow-aware `reversed_parameter` primitive
and preserve stored parameter domains.

## 10. Exact canonical pole semantics

The represented constants

- `north = +pi/2`;
- `south = -pi/2`

are special representation points.

When the **effective** latitude is exactly one of those stored constants,
production must not rely on an approximate library value of
`cos(+/-pi/2)`.

Instead, the represented trigonometric state is exact:

- north: `sin(v)=+1`, `cos(v)=0`;
- south: `sin(v)=-1`, `cos(v)=0`.

Consequences:

- spherical value is exactly independent of U at a pole where arithmetic
  permits;
- `S_u` is exactly zero at a pole;
- the generic differential layer deterministically identifies exact
  parameterization singularity without an epsilon.

No near-pole latitude is coerced to a pole.

This is a representation identity for the canonical stored constants, not a
tolerance rule.

## 11. First derivatives

Required analytic partials before reversal signs:

`S_u = R cos(v) [-sin(u) X + cos(u) Y]`

`S_v = R[-sin(v) Q(u) + cos(v) Z]`.

U reversal multiplies `S_u` by -1.

V reversal multiplies `S_v` by -1.

At a canonical pole:

- `S_u = 0` exactly;
- `S_v` remains finite and may depend on U;
- representation evaluation succeeds;
- differential regularity is singular because the tangent cross product is
  exactly zero.

## 12. Second derivatives

Required analytic partials before reversal signs:

`S_uu = -R cos(v) Q(u)`

`S_uv = R sin(v)[sin(u) X - cos(u) Y]`

`S_vv = -R[cos(v) Q(u) + sin(v) Z]`.

Reversal covariance:

- U reversal preserves `S_uu`;
- V reversal preserves `S_vv`;
- `S_uv` receives the product of the U/V first-derivative reversal signs.

No finite-difference derivative is permitted.

## 13. Existing SurfaceError contract

The implementation reuses the established parameter validation order:

1. U finiteness;
2. V finiteness;
3. U domain;
4. V domain;
5. final finite representability.

Use existing `SurfaceError` values unchanged.

No representation query fails merely because a parameterization is
differentially singular at a pole.

That singularity belongs to `SurfaceDifferentialError`.

## 14. Reversal semantics

The family must implement:

- `u_reversed()`;
- `v_reversed()`.

Reversal preserves the stored U/V domains and toggles representation flags.

Required evidence:

- value covariance through `reversed_parameter`;
- D1/D2 sign laws from Sections 11–12;
- double U reversal recovers exact stored state;
- double V reversal recovers exact stored state;
- a canonical-pole query remains a canonical pole after the corresponding
  represented V reversal when the domain endpoints map that way.

No full-periodic seam behavior is implied.

## 15. Independent analytic oracle

Focused tests must implement an independent long-double oracle using the
placement basis and spherical formulas.

The oracle must not call production sphere helpers.

At minimum cover:

- identity placement;
- genuinely non-axis-aligned `AxisPlacement3`;
- asymmetric non-special U/V subdomains;
- several regular interior parameter pairs;
- U/V domain boundaries;
- value;
- `S_u,S_v,S_uu,S_uv,S_vv`;
- U and V reversal;
- translation;
- positive power-of-two radius/coordinate scaling.

## 16. Canonical exact equator fixture

For identity placement, radius `R=1`, and effective `u=0,v=0`:

- `S=(1,0,0)`;
- `S_u=(0,1,0)`;
- `S_v=(0,0,1)`;
- `S_uu=(-1,0,0)`;
- `S_uv=(0,0,0)`;
- `S_vv=(-1,0,0)`.

Where existing primitive arithmetic preserves exact values, focused tests
should require exact identity for this fixture.

This point supplies a stable cross-layer differential oracle.

## 17. Differential-geometry cross-layer evidence

At regular interior parameters, for outward parameter orientation:

`E = R^2 cos^2(v)`

`F = 0`

`G = R^2`

`J = R^2 cos(v)` for admitted latitude range.

The outward unit normal is the radial unit direction.

With the repository's existing second-fundamental-form sign convention, the
canonical identity/equator fixture must verify:

- Gaussian curvature `K = 1/R^2`;
- mean curvature `H = -1/R`;
- both ordered principal curvature values equal `-1/R`;
- exact represented-data umbilic state is true for the exact canonical
  `R=1,u=0,v=0` fixture.

The work unit does **not** change differential production algorithms.

This is conformance/oracle evidence for the already integrated generic layer.

## 18. Polar singularity evidence

At canonical north/south pole parameters:

- representation value succeeds;
- first derivatives succeed;
- second derivatives succeed;
- `S_u` is exact zero;
- generic `surface_metric_normal` fails with
  `SurfaceDifferentialError::singular_parameterization`;
- second-order and principal-curvature surface overloads propagate the same
  singularity deterministically.

A near-pole but non-pole representable latitude remains regular if the tangent
cross product is nonzero and representable.

No epsilon defines a polar neighborhood.

## 19. Boundary semantics

Constant-U boundaries are bounded meridian arcs.

Constant-V boundaries are bounded latitude circles, degenerating to one point
at an admitted canonical pole.

A dedicated analytic circle curve family is still not required by this work
unit.

Boundary values and tangents are checked against the independent spherical
oracle.

No new curve family is introduced implicitly.

## 20. Affine/placement evidence

Focused evidence must include:

- identity placement;
- arbitrary right-handed non-axis-aligned placement;
- translated placement;
- radius scaled by an exact positive power of two;
- corresponding coordinate scaling where representable.

Rigid placement semantics are reused from `AxisPlacement3`; they are not
reimplemented inside the sphere family.

## 21. Extreme finite evidence

At least one fixture must combine:

- large finite placement coordinates;
- finite positive radius;
- angles near an admitted U or V boundary.

Required behavior:

- succeed when final value/partials remain representable;
- otherwise return `SurfaceError::non_finite_result`.

No angle normalization, NaN sanitization, hidden retry or universal epsilon is
permitted.

## 22. Determinism

Repeated identical calls must produce identical success/failure alternatives
within the admitted serial reference environment.

At minimum repeat:

- regular interior value;
- first derivatives;
- second derivatives;
- canonical pole representation query;
- differential singularity at pole;
- one parameter-domain failure.

## 23. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Sphere satisfies existing `BoundedParametricSurface3`. |
| Construction | finite R>0 accepted; invalid radius rejected. |
| U width | strict width < represented 2pi; full/multiple revolution rejected. |
| Latitude range | V remains within represented [-pi/2,+pi/2]. |
| Domain | established U/V validation order preserved. |
| Independent oracle | value and all five partials match long-double formulas. |
| Canonical equator | exact/simple expected value and derivatives. |
| Canonical poles | exact U-independence and exact zero `S_u`. |
| Polar differential | deterministic singular-parameterization propagation. |
| Near pole | nonzero representable tangent area succeeds without epsilon rejection. |
| U reversal | locus/partial covariance and involution. |
| V reversal | locus/partial covariance and involution. |
| Arbitrary placement | non-axis-aligned `AxisPlacement3` case. |
| Translation/scale | declared covariance. |
| Generic metric | E/F/G/J match analytic sphere. |
| Generic normal | radial outward normal. |
| K/H | analytic `1/R^2` and signed `-1/R`. |
| Principal values | both `-1/R` at canonical regular fixture. |
| Umbilic | exact represented-data umbilic true at canonical fixture. |
| Extreme finite | explicit success/failure without epsilon. |
| Determinism | repeated successes/failures identical. |
| Header isolation | no topology, meshing, I/O, threading or external kernel. |
| Regression | existing 39 ordinary tests remain PASS. |

If exactly one new focused contract is added, ordinary FAST/INTEGRATION
inventory becomes **40 tests**.

## 24. Why sphere is selected before principal directions

The scalar principal values are already available.

Principal directions require additional semantics that scalar eigenvalues do
not settle:

- an eigenvector has sign ambiguity;
- a principal direction is naturally a line rather than an oriented vector;
- a continuous line field needs continuity/branch handling;
- at an umbilic there is no unique principal direction.

A spherical surface is a canonical production fixture where every regular
point is geometrically umbilic.

Implementing the sphere first therefore strengthens the evidence needed to
design principal-direction undefined/umbilic semantics instead of hiding that
problem in an eigenvector API.

## 25. Why explicit conditioning diagnostics are deferred

Conditioning diagnostics remain important for near-singular metrics and
near-umbilic eigensystems.

However:

- no conditioning threshold policy is yet frozen;
- the sphere adds exact singular and exact umbilic fixtures without inventing
  a near-singular threshold;
- implementing diagnostics now would not close the explicit missing
  Surface Representation family.

A later decision can use the expanded analytic fixture set to define
conditioning outputs without treating them as acceptance epsilons.

## 26. Why cone is deferred

A cone additionally introduces:

- reference radius;
- signed/semi-angle semantics;
- natural unbounded V;
- an apex where the physical radius becomes zero;
- apex-specific parameterization singularity/domain policy.

Those semantics are independent of the first bounded sphere sector.

Cone remains an explicit later Surface Representation obligation.

## 27. Why torus is deferred

A torus additionally introduces:

- major radius;
- minor radius;
- two angular parameter directions;
- two periodic seams;
- spindle/horn/self-intersection policy if general radii are admitted.

This is a materially larger representation-policy seam than the bounded
sphere sector.

Torus remains an explicit later Surface Representation obligation.

## 28. Why general trimming/topological faces are deferred

General face semantics require at least:

- p-curves in surface parameter space;
- physical-space edge realization;
- loop ordering/orientation;
- holes;
- curve-on-surface consistency;
- explicit topology face/edge identity.

Rectangular trimming does not solve those problems.

They are essential before CAD-like face coverage and Shared Boundary
Certification, but are intentionally isolated from the analytic sphere value
type.

## 29. Why Boundary Curve Discretization remains blocked

The future Boundary Curve Discretization stage must generate canonical shared
physical traces while respecting parameterization, orientation and topology
identity.

General trimmed-face and boundary-loop semantics remain incomplete.

Opening discretization before resolving that seam would risk making sampled
geometry stand in for topology identity, which the repository explicitly
forbids.

## 30. Why qualification is deferred

Surface Differential Geometry mandatory stage regression explicitly calls for
sphere/paraboloid/saddle and near-degenerate fixtures.

The current repository lacks a production sphere and still retains explicit
conditioning work.

Surface Representation itself also remains unqualified with cone, torus,
full-periodic seams and general trimming obligations retained.

Therefore a formal Surface Differential Geometry qualification campaign now
would freeze an avoidably incomplete analytic/conditioning envelope.

## 31. Repository mapping for future implementation

Authorized future mapping is limited to:

- public elementary surface API:
  `include/apmesh/geometry/elementary_surface.hpp`;
- existing elementary-surface production:
  `src/geometry/elementary_surface.cpp`;
- focused contract:
  `tests/surface_sphere.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision;
- candidate/implementation audits when validation is reached.

Existing plane/cylinder scientific behavior is frozen.

No common `SurfaceError`, `BoundedParametricSurface3`,
`AxisPlacement3` or Surface Differential Geometry production change is
expected.

If implementation requires changing those contracts, stop and require a new
decision.

## 32. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 39 ordinary semantic tests must remain passing;
- the focused sphere contract must pass;
- the generic surface-differential cross-layer assertions in that focused
  contract must pass.

Passing yields only:

**BOUNDED ANALYTIC SPHERE IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED /
SURFACE REPRESENTATION NOT QUALIFIED /
SURFACE DIFFERENTIAL GEOMETRY NOT QUALIFIED.**

## 33. Stop conditions

Stop and require a new scientific decision if implementation needs:

- full-periodic U wrapping/seam identity;
- parameter normalization modulo `2*pi`;
- a new common `SurfaceError`;
- changes to `BoundedParametricSurface3`;
- changes to generic Surface Differential Geometry algorithms;
- principal directions;
- conditioning thresholds;
- analytic cone or torus;
- general trimming/p-curves/topological faces;
- Boundary Curve Discretization;
- Physical Sizing;
- meshing;
- third-party runtime dependencies.

## 34. Planned sequence after sphere

Planning only, not authorization.

After bounded analytic sphere implementation/closure, a fresh decision should
recompare:

1. analytic cone;
2. analytic torus;
3. principal directions / line-field semantics;
4. conditioning diagnostics;
5. general trimming/p-curves/topological faces;
6. Surface Representation qualification readiness;
7. Surface Differential Geometry qualification readiness;
8. Boundary Curve Discretization readiness.

No option is pre-authorized.

## 35. Effect if integrated and closed

After this decision is integrated, protected-main validation passes, and a
separate decision checkpoint closes, the sole next production work item is:

**Bounded Analytic Spherical Surface Sector in 3D.**

No implementation begins on this decision branch.

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

Surface Differential Geometry remains IN INVESTIGATION / NOT QUALIFIED.

Boundary Curve Discretization and downstream meshing remain blocked.
