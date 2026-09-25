# Bounded Analytic Cylinder Surface — Surface Representation Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-25  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation: QUALIFIED only in the admitted cloud envelope;
- Surface Representation stage: OPEN / IN INVESTIGATION / NOT QUALIFIED;
- bicubic polynomial Bézier patch: integrated/closed;
- rational bicubic Bézier patch: integrated/closed;
- bicubic NURBS patch: integrated/closed;
- bicubic NURBS double-knot continuity: integrated/closed;
- Coons patch: integrated/closed;
- rectangular trimmed surface: integrated/closed;
- bounded cubic Bézier linear extrusion: integrated/closed;
- arbitrary-axis placement `AxisPlacement3`: integrated/closed;
- bounded cubic Bézier revolution: integrated/closed;
- bounded analytic plane: integrated/closed;
- ordinary semantic inventory at entry: 35 tests.

## 1. Question

After terminal closure of the bounded analytic plane work unit, what is the next
smallest Surface Representation breadth step that:

1. adds a materially new analytic family;
2. reuses already validated `AxisPlacement3` and surface-domain semantics;
3. produces a canonical non-zero-curvature fixture for later Surface
   Differential Geometry;
4. avoids prematurely opening periodic seams, poles, apex singularities,
   double-periodicity, general p-curves/topological faces or meshing?

Required candidates:

- bounded analytic cylinder;
- bounded analytic cone;
- bounded analytic sphere;
- bounded analytic torus;
- general trimming / p-curves / topological faces;
- opening Surface Differential Geometry now.

## 2. Decision-entry authority

Canonical entry `main`:

`1c9283498ed9563ff8193f432811c3fb2f7ae7e9`.

Terminal analytic-plane lineage:

- implementation PR #186:
  `89ec9946b7438b30a0b3b3218c8c9c1981b31fd8`;
- implementation closure PR #187:
  `57f647e488bf3498168bc9c3b8c63fc5442d034a`;
- closure post-merge FAST `35992410876`: PASS;
- closure post-merge INTEGRATION `35992410395`: PASS;
- terminal sync PR #188:
  `1c9283498ed9563ff8193f432811c3fb2f7ae7e9`;
- sync PR FAST `36084372900`: PASS;
- sync PR INTEGRATION `36084372892`: PASS;
- sync post-merge FAST `36084432186`: PASS;
- sync post-merge INTEGRATION `36084432183`: PASS.

No open PR and no active production work item existed at decision entry.

## 3. Literature and mature-kernel evidence

### 3.1 Cylinder data model and parameterization

Open CASCADE Technology cylinder construction/reference:

- https://dev.opencascade.org/doc/refman/html/class_g_c___make_cylindrical_surface.html
- https://dev.opencascade.org/doc/occt-7.8.0/refman/html/classgp__Cylinder.html

Relevant evidence:

- a cylindrical surface is positioned by a 3D local coordinate system;
- the local Z direction is the cylinder axis;
- the cylinder is defined by a radius and local placement;
- the canonical parameterization uses an angular U coordinate and an axial V
  coordinate;
- the full analytic cylinder is U-periodic over a complete revolution.

Project impact:

- `AxisPlacement3` already supplies the required orthonormal local frame;
- the next bounded work unit can admit only a strict angular subinterval and a
  finite axial interval, avoiding periodic seam semantics for now;
- radius/angle/axis evaluation is independent of NURBS or surface trimming.

### 3.2 Bounded cylinder patches are meaningful independent objects

Open CASCADE `GeomBndLib_Cylinder` supports both the full cylinder and a
bounded parameter rectangle:

https://dev.opencascade.org/doc/refman/html/class_geom_bnd_lib___cylinder.html

Project impact:

- a finite `[u_min,u_max] x [v_min,v_max]` analytic cylinder patch is a
  standard geometric object;
- admitting a bounded sector before full periodic topology is scientifically
  coherent and directly compatible with the existing
  `BoundedParametricSurface3` contract.

### 3.3 Differential-geometry relevance

Open CASCADE `GeomLProp_SLProps` derives normals and curvature quantities from
surface derivatives:

https://dev.opencascade.org/doc/occt-6.9.1/refman/html/class_geom_l_prop___s_l_props.html

MIT Hyperbook, Differential Geometry of Surfaces:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node26.html

Project impact:

- the current common surface contract already exposes first and second
  derivatives;
- a regular circular cylinder is a canonical future differential-geometry
  fixture with one zero principal curvature and one non-zero principal
  curvature;
- implementing the cylinder now strengthens independent future validation
  without prematurely mixing representation and curvature code.

### 3.4 Why the later analytic families are semantically larger

Sphere reference:

https://dev.opencascade.org/doc/refman/html/class_geom___spherical_surface.html

Torus reference:

https://dev.opencascade.org/doc/occt-7.9.0/refman/html/class_geom___toroidal_surface.html

Cone construction reference:

https://dev.opencascade.org/doc/refman/html/class_g_c___make_conical_surface.html

Relevant evidence:

- sphere introduces periodic U plus pole degeneracies in V;
- torus introduces two angular directions and double periodicity;
- cone introduces a varying radius and apex singularity;
- these are not merely alternate formulas for the same bounded-cylinder
  contract.

External sources are scientific/design evidence only. No Open CASCADE runtime
dependency or numerical oracle is admitted.

## 4. Candidate comparison

| Candidate | New semantics now | Reuse of current prerequisites | Immediate doctoral value | Main deferred burden | Decision |
| --- | --- | --- | --- | --- | --- |
| Bounded analytic cylinder sector | Positive radius + angular/axial analytic coordinates | **High**: AxisPlacement3, bounded surface contract, reversal | **High**: canonical nonzero-curvature fixture | Full periodic seam deferred | **SELECTED** |
| Bounded analytic cone | Radius varying with axial coordinate | High | High | Apex singularity and sign/radius policy | DEFER |
| Bounded analytic sphere | Two angular coordinates | High | Very high | Pole singularities + U periodicity | DEFER |
| Bounded analytic torus | Two angular coordinates + two radii | High | High | Double periodicity and ring/horn/spindle classification | DEFER |
| General trimming / p-curves / topological faces | Parameter-space curves + topology binding | Moderate | Very high for CAD faces | Ownership/orientation/identity architecture | DEFER |
| Surface Differential Geometry | Normal/metric/curvature semantics | Representation derivatives are largely ready | Very high | Stage-ordering requires stronger representation closure first | DEFER |

## 5. Decision

Authorize exactly one future production work unit:

**Bounded Analytic Circular Cylinder Sector in 3D.**

Proposed public type:

`BoundedCylinderSurface3`.

The work unit is deliberately **not** the complete periodic cylinder.

## 6. Frozen representation

The future type stores:

- one validated `AxisPlacement3`;
- one finite strictly positive radius `R`;
- one finite strict angular `CurveParameterDomain` U;
- one finite strict axial `CurveParameterDomain` V;
- two reversal flags, one per parameter direction.

The representation remains immutable after construction.

No dynamic allocation is required by the value type.

## 7. Construction semantics

Construction is validated through a static factory.

Required construction failure vocabulary:

`CylinderSurfaceConstructionError` with at minimum:

- `non_finite_radius`;
- `non_positive_radius`;
- `full_or_multiple_revolution_not_admitted`.

The factory receives already validated U/V `CurveParameterDomain` objects.

Radius policy:

- radius must be finite;
- radius must satisfy `R > 0`;
- no epsilon defines positivity.

Angular-domain policy:

- U lower/upper are already finite and strictly ordered by
  `CurveParameterDomain`;
- angular width is evaluated in an overflow-aware widened representation;
- require `0 < U.upper - U.lower < 2*pi`;
- exactly full or multiple revolutions are not admitted;
- no modulo normalization or periodic wrapping occurs.

V may be any finite strict interval already representable by
`CurveParameterDomain`.

## 8. Concrete parameterization

Let the placement origin be `O` and unit directions be `X,Y,Z`.

For effective parameters `u,v`:

`S(u,v) = O + R cos(u) X + R sin(u) Y + v Z`.

U is the angular coordinate in radians.

V is signed axial distance in placement units.

The parameterization must be deterministic and must not reduce U modulo
`2*pi`.

## 9. Parameter validation order

Reuse the established bounded-surface failure order exactly:

1. U finiteness;
2. V finiteness;
3. U domain;
4. V domain;
5. finite representability of the result.

Use existing `SurfaceError` values unchanged.

No new common `SurfaceError` is authorized.

## 10. First and second partial derivatives

Required analytic partials:

`S_u = R[-sin(u) X + cos(u) Y]`

`S_v = Z`

`S_uu = -R[cos(u) X + sin(u) Y]`

`S_uv = 0`

`S_vv = 0`.

No finite-difference derivative is permitted.

All final point/vector components must be checked for finite representability.

## 11. Reversal semantics

The concrete family must implement:

- `u_reversed()`;
- `v_reversed()`.

Like the bounded plane, reversal preserves the stored U/V domains and records
orientation flags.

Effective parameters are obtained only through the existing
`reversed_parameter` primitive.

Covariance requirements:

- U reversal preserves physical locus and negates `S_u`;
- U reversal preserves `S_v`;
- U reversal preserves `S_uu` and negates `S_uv` where applicable;
- V reversal negates `S_v`;
- V reversal preserves `S_u` and `S_uu`;
- `S_uv` remains exact zero in this family;
- double U or V reversal recovers exact stored state.

## 12. Boundary semantics

The two constant-U boundaries are straight axial segments.

Required focused evidence:

- U=U.lower boundary matches `LineSegment3` between the corresponding V
  endpoints;
- U=U.upper boundary matches the analogous `LineSegment3`;
- value and V-tangent parity are checked.

The two constant-V boundaries are circular arcs.

Because a dedicated analytic circle curve family is not yet admitted, those
boundaries must be checked against an independent analytic long-double
reference rather than a production circle type.

This does not cancel the retained future analytic-conic curve obligation.

## 13. Independent analytic oracle

Focused tests must implement an independent long-double oracle using the
placement basis and trigonometric formulas.

It must not call production cylinder helpers.

At minimum cover:

- non-axis-aligned placement;
- asymmetric non-special angular domain;
- arbitrary finite axial domain;
- interior parameter pairs;
- all four corners;
- value;
- `S_u,S_v,S_uu,S_uv,S_vv`;
- both reversal directions.

## 14. Canonical differential-geometry fixture evidence

Representation tests may verify the exact derivative identities needed later
without implementing differential geometry.

At a regular point:

- `S_u dot S_v = 0`;
- `|S_u| = R`;
- `|S_v| = 1`;
- `S_uu` is radial inward;
- `S_uv = S_vv = 0`.

Tests may also derive the unnormalized orientation vector
`S_u x S_v` to verify outward orientation under the selected placement.

No unit-normal, metric tensor, curvature or principal-direction API is added.

## 15. Affine/placement evidence

Required focused evidence:

- identity placement;
- genuinely non-axis-aligned `AxisPlacement3`;
- translated placement;
- power-of-two scaled radius and axial coordinates where representable;
- placement round-trip evidence already remains frozen in its own contract.

No arbitrary frame semantics are reimplemented in the cylinder source.

## 16. Extreme finite evidence

At least one fixture must combine:

- large finite placement coordinates;
- finite positive radius;
- angular values near a domain boundary;
- large finite axial values.

Required behavior:

- succeed when final value/partials are representable;
- otherwise return `SurfaceError::non_finite_result`.

No universal epsilon, angle normalization, NaN sanitization or hidden retry is
permitted.

## 17. Determinism

Repeated identical calls must return byte-identical value/error alternatives
under the serial reference build where the underlying scalar results are
exactly repeatable.

At minimum repeat:

- one interior value;
- one first-derivative query;
- one second-derivative query;
- one out-of-domain failure;
- one non-finite parameter failure.

## 18. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Cylinder satisfies existing `BoundedParametricSurface3`. |
| Construction | finite R>0 accepted; non-finite/zero/negative rejected. |
| Angular width | strict width < 2π; full/multiple revolution rejected. |
| Domain | exact stored U/V domains; established validation order. |
| Independent oracle | value and all five partials match long-double formula. |
| U boundaries | two axial edges match `LineSegment3`. |
| V boundaries | circular arc values match independent analytic oracle. |
| Orthogonality | Su·Sv=0 and derivative identities hold. |
| U reversal | locus/partial covariance and involution. |
| V reversal | locus/partial covariance and involution. |
| Orientation | Su×Sv follows placement/radius orientation. |
| Arbitrary placement | genuinely non-axis-aligned frame. |
| Translation/scale | declared covariance. |
| Extreme finite | explicit success/failure without epsilon. |
| Determinism | repeated successes/failures identical. |
| Header isolation | no topology, meshing, I/O, threading or external kernel. |
| Regression | existing 35 ordinary semantic tests remain PASS. |

If exactly one new focused contract is added, ordinary FAST/INTEGRATION
inventory becomes **36 tests**.

## 19. Repository mapping for future implementation

Authorized future files:

- extend public elementary-surface family:
  `include/apmesh/geometry/elementary_surface.hpp`;
- extend existing elementary-surface production:
  `src/geometry/elementary_surface.cpp`;
- focused contract:
  `tests/surface_cylinder.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  this decision.

No common surface contract change is expected.

If `SurfaceError`, `AxisPlacement3` or
`BoundedParametricSurface3` must change, stop and require a new decision.

## 20. Explicit exclusions

This decision does not authorize:

- complete/full periodic cylinder;
- U wrapping/modulo behavior;
- cone;
- sphere;
- torus;
- analytic circle/ellipse curve type;
- general trim loops;
- p-curves;
- topological face binding;
- broader Coons/NURBS;
- normals, metrics or curvature APIs;
- Surface Differential Geometry;
- Boundary Curve Discretization;
- sizing or meshing;
- Quad-Dominant or parallel work;
- external CAD runtime dependency.

## 21. Why cylinder is selected before cone

A cone adds both circular angular semantics and a radius varying with V.

It must decide:

- apex admission;
- zero radius at the apex;
- parameterization singularity;
- semi-angle/radius construction semantics.

The cylinder isolates angular analytic geometry first.

## 22. Why cylinder is selected before sphere

A sphere requires:

- U periodicity;
- finite latitude bounds;
- two poles where one parametric tangent vanishes;
- explicit representation-versus-regularity semantics at the poles.

Those are larger semantics than the bounded non-periodic cylinder sector.

## 23. Why cylinder is selected before torus

A torus requires:

- major and minor radius policy;
- two angular directions;
- double periodicity;
- classification of ring/horn/spindle parameter regimes if broader radii are
  admitted.

The bounded cylinder sector is a smaller first analytic curved surface.

## 24. Why general trimming/topological faces remain deferred

The existing rectangular trim proves static subdomain semantics for a single
supporting surface.

General trimming requires a new architecture for:

- parameter-space boundary curves;
- orientation and loops;
- correspondence with physical boundary curves where required;
- topological face identity;
- holes/multiple loops;
- seam behavior.

That is independent of the cylinder formula and should remain a dedicated
decision.

## 25. Why Surface Differential Geometry does not open yet

The existing common surface contract already exposes the derivatives required
for normals, metric and curvature, but the authoritative roadmap places
Surface Representation before Surface Differential Geometry.

Before moving stages, representation should contain at least one genuinely
curved elementary analytic family in addition to free-form/constructed
families and the plane.

The bounded cylinder provides:

- an exact nonzero-curvature analytic fixture;
- no poles/apex/double-periodic complications;
- a direct later reference for principal curvature `0` and `1/R`.

After this cylinder work unit closes, a fresh decision should again compare
opening Surface Differential Geometry against continuing analytic breadth.

## 26. Validation boundary

Before implementation integration:

- FAST / GCC 13 Debug must pass;
- INTEGRATION / GCC 13 Debug must pass;
- INTEGRATION / Clang 18 libc++ Debug must pass;
- all existing 35 ordinary semantic tests must remain passing;
- the new cylinder focused contract must pass.

Passing yields only:

**ANALYTIC CYLINDER SECTOR IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or authorize the full periodic
cylinder.

## 27. Stop conditions

Stop and require a new scientific decision if implementation needs:

- full-periodic seam semantics;
- angle wrapping/modulo;
- zero radius;
- cone/sphere/torus behavior;
- analytic conic curve production;
- a common surface error change;
- an `AxisPlacement3` change;
- a new surface concept method;
- topology/trimming state;
- normal/curvature state;
- a universal epsilon;
- third-party runtime dependency.

## 28. Planned next comparison

Planning only, not authorization.

After bounded cylinder integration and closure, compare:

1. opening Surface Differential Geometry;
2. bounded analytic sphere;
3. bounded analytic cone;
4. bounded analytic torus;
5. general trimming / p-curves / topological faces.

No option is pre-authorized.

## 29. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Bounded Analytic Circular Cylinder Sector in 3D.**

No production implementation begins on this decision branch.

Boundary Curve Discretization and meshing remain blocked.
