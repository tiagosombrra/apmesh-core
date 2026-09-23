# Surface Coons Patch — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-23  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites at entry:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- polynomial cubic Bézier Curve Representation baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- bounded 3D surface abstraction: integrated focused prerequisite;
- tensor-product bicubic polynomial Bézier patch: integrated and closed;
- positive-weight rational bicubic Bézier patch: integrated and closed;
- bicubic positive-weight NURBS surface with variable simple U/V spans:
  integrated and closed;
- bicubic positive-weight NURBS surface with unique U/V multiplicities 1/2 and
  explicit C1/second-jet semantics: integrated and closed.

## 1. Question

After closing the bicubic NURBS C1 continuity seam, which remaining Surface
Representation breadth concept should be introduced next?

Required candidates:

1. Coons/transfinite patch construction;
2. analytic elementary surfaces;
3. ruled/extrusion/revolution surfaces;
4. rectangular/general trimmed-surface semantics;
5. multiplicity-three/C0 surface semantics, only if currently required;
6. arbitrary degree/periodicity, only if currently required.

The next work unit must add exactly one new representation/construction seam
without simultaneously introducing trimming topology, arbitrary 3D placement,
periodicity or a runtime surface hierarchy.

## 2. Fresh repository entry authority

Canonical decision-entry `main`:

`925cf43f3a0cc6239ac6fb2f9f7e913e79141d86`.

Terminal bicubic-NURBS C1 implementation lineage:

- implementation PR #155:
  `fea1cf536336dc5bef19217a879338bac495555f`;
- final PR head:
  `6453279c77883440b5cd86e7ab08052b318ecf22`;
- final PR FAST `35837794984`: PASS, 29/29;
- final PR INTEGRATION `35837794969`: PASS, 29/29 in GCC and Clang;
- implementation post-merge FAST `35837956530`: PASS, 29/29;
- implementation post-merge INTEGRATION `35837956529`: PASS, 29/29;
- implementation closure PR #156 head:
  `def351bde58b9afdc71571effbcad73d0ae53ed5`;
- closure PR FAST `35838274382`: PASS;
- closure PR INTEGRATION `35838274390`: PASS;
- closure PR #156 merged as:
  `925cf43f3a0cc6239ac6fb2f9f7e913e79141d86`;
- closure post-merge FAST `35838387139`: PASS;
- closure post-merge INTEGRATION `35838387121`: PASS.

Retained mechanical history:

- initial C1 implementation runs `35837423080` / `35837423118` selected
  only 28 tests because the new contract lacked FAST/INTEGRATION labels;
- corrected head `b913895065f1d1184ecebf2565667fdd7fcaca2b`
  and all later acceptance runs executed the full 29-test inventory.

No production work item or open PR exists at decision entry.

## 3. Literature and mature-kernel evidence

### 3.1 Coons — boundary-defined surface patches

Steven A. Coons.
*Surfaces for Computer-Aided Design of Space Forms*.
MIT Project MAC Technical Report MAC-TR-41, 1967.
DOI: 10.21236/AD0663504.

Bibliographic record:
https://ci.nii.ac.jp/ncid/BC08350718

Project relevance:

- establishes boundary-defined free-form surface construction as a distinct
  geometric mechanism;
- motivates a patch whose interior is blended from prescribed boundary curves;
- directly aligns with AP Mesh workflows in which patch geometry is organized
  around explicit boundary curves.

### 3.2 Open CASCADE — filling from contiguous boundary curves

Open CASCADE `GeomFill_BezierCurves`:

https://dev.opencascade.org/doc/refman/html/class_geom_fill___bezier_curves.html

Open CASCADE `GeomFill_BSplineCurves`:

https://dev.opencascade.org/doc/refman/html/_geom_fill___b_spline_curves_8hxx.html

Open CASCADE `GeomFill_Boundary`:

https://dev.opencascade.org/doc/refman/html/class_geom_fill___boundary.html

Project relevance:

- mature CAD kernels treat filling from two/three/four contiguous curves as a
  separate construction layer;
- the four-curve case requires boundary contiguity;
- boundary objects provide value and derivative information independently from
  the resulting filled surface;
- supports isolating exact boundary compatibility and orientation before any
  trimming/topology layer.

Open CASCADE is design/reference evidence only and is not a runtime dependency
or numerical oracle.

### 3.3 Gmsh — transfinite surfaces are boundary/corner driven

Gmsh reference manual, transfinite surface meshing:

https://gmsh.info/doc/texinfo/gmsh.html

Project relevance:

- transfinite surface workflows identify three/four boundary corners and use
  boundary structure to drive the interior;
- reinforces the downstream relevance of a deterministic boundary-defined
  surface seam to meshing;
- this source supports sequencing only: AP Mesh does not adopt Gmsh's mesh
  algorithm or runtime dependency here.

### 3.4 Trimmed surfaces are a different semantic layer

Open CASCADE `Geom_RectangularTrimmedSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___rectangular_trimmed_surface.html

Project relevance:

- trimming wraps a basis surface and restricts/orients a subdomain;
- periodicity and trim orientation participate in the semantics;
- therefore trimming should not be conflated with constructing a new
  boundary-interpolating supporting surface.

### 3.5 Analytic surfaces require explicit placement/parameterization

Open CASCADE `Geom_SphericalSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___spherical_surface.html

Project relevance:

- an elementary analytic surface uses a local 3D coordinate system and
  orientation/parameterization conventions;
- the repository's current qualified frame envelope does not yet establish a
  general arbitrary-angle placement contract;
- analytic plane/cylinder/cone/sphere/torus admission remains important but is
  not the smallest unblocked next seam.

### 3.6 Swept/revolution surfaces introduce a generator/axis seam

Open CASCADE surface-of-revolution model:

https://dev.opencascade.org/doc/refman/html/class_i_g_e_s_geom___surface_of_revolution.html

Project relevance:

- a revolution surface depends on a generatrix, an axis and angular bounds;
- sweep/revolution therefore introduces generator ownership, arbitrary spatial
  axis placement and angular semantics;
- those are distinct from a four-boundary Coons patch.

## 4. Candidate comparison

| Candidate | New semantic burden | Current prerequisites | Direct AP Mesh value | Decision |
| --- | --- | --- | --- | --- |
| Four-boundary Coons/transfinite patch | Oriented boundary ownership, exact corner compatibility, transfinite blend | Cubic Bézier curves + bounded surface contract already integrated | **Very high**; first explicit boundary-driven supporting patch | **SELECTED** |
| Analytic elementary surfaces | Arbitrary 3D placement, family-specific periodic/singular parameterization | General placement contract still unresolved | High for CAD breadth and validation | BLOCKED/DEFER |
| Ruled/extrusion/revolution | Generator curve + direction/axis + angular semantics | Curve families exist, arbitrary axis/placement policy not closed | High but less directly boundary-quadrilateral | DEFER |
| Rectangular/general trimming | Basis-surface ownership, trim domain/loops, orientation, later p-curves/topology | Supporting surfaces exist, but trimming/topology seam is broader | Very high downstream | DEFER |
| Multiplicity-three/C0 | D1 failure and likely side/component derivative semantics | Current surface NURBS supports multiplicity 1/2 | Important for broader imported CAD | DEFER |
| Arbitrary degree/periodicity | Runtime degree and/or periodic U/V topology | Not required by current first boundary-fill seam | Important later if input class requires | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Oriented Four-Boundary Cubic Bézier Coons Patch in 3D.**

Proposed public representation:

- `CubicBezierCoonsPatch3`.

The work unit is a supporting continuous surface representation constructed
from four already integrated `CubicBezier3` values.

It is not:

- a trimmed surface;
- a topological face;
- a generic runtime curve-composition mechanism;
- a rational Coons patch;
- a NURBS Coons patch;
- a mesh transfinite algorithm.

## 6. Boundary convention

The patch owns four oriented cubic Bézier boundary curves:

- `bottom(u)`: lower-left corner to lower-right corner;
- `top(u)`: upper-left corner to upper-right corner;
- `left(v)`: lower-left corner to upper-left corner;
- `right(v)`: lower-right corner to upper-right corner.

All boundary curves use the exact domain `[0,1]`.

Required exact corner compatibility:

- `bottom(0) == left(0)`;
- `bottom(1) == right(0)`;
- `top(0) == left(1)`;
- `top(1) == right(1)`.

Compatibility is exact because all four endpoints are finite stored
`Point3` values.

No epsilon or geometric-proximity rule is admitted.

The constructor must not silently reverse a boundary to make it fit.

## 7. Construction failure vocabulary

A dedicated construction error should distinguish the four orientation/corner
failures, for example:

- lower-left mismatch;
- lower-right mismatch;
- upper-left mismatch;
- upper-right mismatch.

The exact final enum names may be mechanical, but they must remain
deterministic and corner-specific.

No common `SurfaceError` extension is expected for construction.

## 8. Mathematical representation

Let:

- `B(u)` be bottom;
- `T(u)` be top;
- `L(v)` be left;
- `R(v)` be right.

Let corners be:

- `P00 = B(0) = L(0)`;
- `P10 = B(1) = R(0)`;
- `P01 = T(0) = L(1)`;
- `P11 = T(1) = R(1)`.

Define the bilinear corner blend:

`C(u,v) =
 (1-u)(1-v)P00 +
 u(1-v)P10 +
 (1-u)vP01 +
 uvP11`.

The Coons patch is:

`S(u,v) =
 (1-u)L(v) + uR(v) +
 (1-v)B(u) + vT(u) -
 C(u,v)`.

The representation domain is exactly:

`(u,v) in [0,1] x [0,1]`.

## 9. Production arithmetic policy

Production may evaluate the boundary curves through their integrated
`CubicBezier3` APIs.

The Coons blend itself must use deterministic, explicitly ordered arithmetic.

Preferred value order:

1. evaluate `L(v)`;
2. evaluate `R(v)`;
3. blend those in U;
4. evaluate `B(u)`;
5. evaluate `T(u)`;
6. blend those in V;
7. evaluate the bilinear corner blend;
8. combine the two ruled blends and subtract the bilinear blend.

No algebraically equivalent reorder may be chosen opportunistically between
runs.

Finite final coordinate failure maps to
`SurfaceError::non_finite_result`.

## 10. Common bounded-surface contract

The patch must satisfy the already integrated
`BoundedParametricSurface3` concept without changing its signatures.

Required operations:

- `parameter_domain()`;
- `evaluate(u,v)`;
- `first_derivatives(u,v)`;
- `second_derivatives(u,v)`.

No common `SurfaceError` extension is expected.

If the common concept or error vocabulary must change, stop and require a new
decision.

## 11. Exact boundary identities

For every representable valid parameter:

- `S(u,0) = B(u)`;
- `S(u,1) = T(u)`;
- `S(0,v) = L(v)`;
- `S(1,v) = R(v)`.

At exact boundaries, production should prefer direct boundary evaluation when
needed to preserve exact identity instead of relying on cancellation in the
general blend.

All four corner identities must therefore also be exact.

## 12. First derivatives

Required analytic partials:

`S_u =
 -L(v) + R(v)
 + (1-v)B'(u) + vT'(u)
 - C_u(u,v)`.

`S_v =
 (1-u)L'(v) + uR'(v)
 - B(u) + T(u)
 - C_v(u,v)`.

No finite-difference derivative is permitted.

The four boundary tangential derivatives must agree with their source curves:

- at V=0/1, `S_u` matches bottom/top derivative;
- at U=0/1, `S_v` matches left/right derivative.

The transverse derivative is defined by the Coons blend and need not match any
additional boundary derivative constraint.

## 13. Second derivatives

Required analytic second partials:

`S_uu = (1-v)B''(u) + vT''(u)`.

`S_vv = (1-u)L''(v) + uR''(v)`.

`S_uv =
 -L'(v) + R'(v) - B'(u) + T'(u) - C_uv`.

The bilinear corner blend has zero pure second derivatives.

The mixed corner term is constant:

`C_uv = P00 - P10 - P01 + P11`.

Production exposes the already integrated aggregate
`SurfaceSecondDerivatives3` result.

No new differential-geometry quantity is introduced.

## 14. Boundary ownership and immutability

The patch owns four `CubicBezier3` values by value.

Public read-only accessors should expose:

- `bottom()`;
- `top()`;
- `left()`;
- `right()`.

The stored orientation is scientific state.

No pointer ownership, virtual interface, type erasure or mutable boundary
replacement is admitted.

## 15. U/V reversal

The patch should support:

- `u_reversed()`;
- `v_reversed()`.

U reversal:

- swaps left/right;
- reverses bottom/top.

V reversal:

- swaps bottom/top;
- reverses left/right.

Required covariance:

- U-reversed value at `(1-u,v)` equals original value at `(u,v)`;
- U-reversed `S_u` changes sign;
- U-reversed `S_v` preserves sign;
- analogous V rules;
- corresponding second-partial signs follow the chain rule;
- double reversal in one direction recovers the exact four stored boundary
  curves.

## 16. Independent reference

Focused tests must implement the Coons expression independently from
production.

The independent oracle may call boundary curve value/D1/D2 methods, but it
must not call production Coons helpers.

At minimum validate:

- multiple asymmetric interior parameter pairs;
- exact corners;
- all four boundaries;
- value;
- `S_u`, `S_v`;
- `S_uu`, `S_uv`, `S_vv`.

## 17. Analytic fixtures

Focused evidence must include at least:

### 17.1 Bilinear surface

Four straight cubic-Bézier boundary curves that define a bilinear quadrilateral
surface.

Required result:

- Coons patch equals the analytic bilinear surface;
- pure second derivatives vanish where the analytic fixture requires;
- mixed derivative matches the analytic constant.

### 17.2 Planar rectangle/parallelogram

Required result:

- exact/analytic plane position;
- constant first partials;
- zero second partials.

### 17.3 Nonplanar asymmetric boundary set

Required result:

- independent Coons oracle agreement;
- no symmetry masks boundary-order errors.

## 18. Degenerate and constant boundaries

A constant patch, where all four boundaries collapse to the same finite point,
is representable if corner compatibility holds.

Required behavior:

- exact constant point;
- exact zero first partials;
- exact zero second partials.

Collapsed or rank-deficient Coons patches are representation-valid.

No constructor rejects them merely because a later normal/metric would be
singular.

## 19. Affine and finite-behavior evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two coordinate scaling where representable;
- Cartesian signed-axis permutation/reflection cases already admitted by the
  current numeric/frame envelope;
- extreme finite coordinates where the final result remains representable;
- explicit `SurfaceError::non_finite_result` when a final value/partial is
  not representable.

No universal epsilon, clamping or hidden fallback is permitted.

## 20. Determinism

Focused evidence must include repeated identical:

- successful value queries;
- successful first/second derivative queries;
- non-finite parameter failures;
- out-of-domain failures;
- construction corner mismatch failures.

Results and typed failures must be identical.

## 21. Header isolation

The future Coons header must not depend on:

- topology;
- mesh entities;
- trimming;
- I/O;
- threading;
- OpenMP/MPI;
- external CAD kernels.

The work unit remains in continuous supporting-surface geometry.

## 22. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Coons patch satisfies existing bounded 3D surface concept. |
| Exact domain | [0,1]²; existing U/V query errors retained. |
| Corner validation | Four orientation-specific mismatch failures. |
| Boundary identity | Four source boundaries reproduced exactly. |
| Boundary tangents | Tangential partials match source `CubicBezier3`. |
| Independent oracle | Value and all five partials match direct Coons expression. |
| Bilinear fixture | Analytic bilinear surface reproduced. |
| Plane fixture | Analytic plane/parallelogram reproduced. |
| Constant/degenerate | Representation remains valid; zero jets where exact. |
| U reversal | Boundary state, value and jet covariance; involution. |
| V reversal | Boundary state, value and jet covariance; involution. |
| Translation/scale | Declared affine covariance. |
| Extreme finite | Explicit success/failure; no universal epsilon. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/meshing/trimming/threading/external dependency. |
| Prerequisite preservation | Existing 29 ordinary tests remain passing. |

If exactly one focused contract is added, ordinary FAST/INTEGRATION inventory
becomes **30 tests**.

## 23. Why Coons/transfinite is selected now

The current Surface Representation stage already supports:

- polynomial bicubic tensor-product geometry;
- positive-weight rational bicubic geometry;
- multi-span bicubic NURBS with simple knots;
- U/V multiplicity-one/two C1 semantics.

What remains missing is a surface whose **construction is defined by explicit
boundary curves rather than an interior control net**.

That seam is directly relevant to:

- patch construction from known boundary curves;
- later patch networks;
- transfinite mesh/parameterization workflows;
- future boundary-consistency reasoning.

It can be introduced without simultaneously opening trimming topology,
arbitrary placement, angular periodicity or runtime surface polymorphism.

## 24. Why analytic elementary surfaces are deferred

Analytic plane/cylinder/cone/sphere/torus families remain required breadth.

However cylinder/cone/sphere/torus need explicit spatial placement,
orientation, periodic/angular parameter semantics and singular-pole policy.

The repository's current qualified Cartesian-frame envelope does not establish
general arbitrary-angle placement.

A later dedicated analytic-surface entry decision must resolve that seam
instead of smuggling it into a boundary-fill work unit.

## 25. Why ruled/extrusion/revolution is deferred

Ruled/swept surfaces are also retained.

They introduce:

- generator-curve ownership;
- direction or axis state;
- angular bounds for revolution;
- arbitrary spatial placement/orientation;
- possible periodicity.

Those semantics are independent from four-boundary transfinite blending.

## 26. Why trimming is deferred

Trimming remains essential for CAD-like face representation.

It introduces at least:

- basis-surface ownership/reference semantics;
- parameter-space trim curves;
- loop closure and orientation;
- possible periodic-domain handling;
- future curve-on-surface consistency;
- explicit topology identity.

The project deliberately keeps continuous supporting geometry separate from
topological identity.

Opening trimming before a first explicit boundary-driven supporting patch would
mix two larger architectural seams.

## 27. Why C0 / arbitrary degree / periodicity are deferred

Multiplicity-three/C0 would require a new first-derivative failure or
one-sided/component-specific API policy.

Arbitrary degree changes active-control algorithms and storage identity.

Periodicity changes parameter-domain and seam semantics.

None is required by the fixed cubic Bézier Coons patch.

They remain retained breadth candidates when the admitted input class demands
them.

## 28. Repository mapping for future implementation

Authorized future mapping is limited to:

- new public representation:
  `include/apmesh/geometry/coons_surface.hpp`;
- new production:
  `src/geometry/coons_surface.cpp`;
- focused contract:
  `tests/surface_coons_patch.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No change to:

- `parametric_surface.hpp`;
- existing polynomial/rational/NURBS surface source semantics;
- curve APIs;

is expected.

If a common contract must change, stop and require a new decision.

## 29. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18/libc++ Debug / INTEGRATION must pass;
- all existing 29 ordinary semantic tests must remain passing;
- the new Coons focused contract must pass.

Passing yields only:

**SURFACE REPRESENTATION STAGE OPEN /
CUBIC BÉZIER COONS PATCH IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation, transfinite meshing, trimming or
any other surface family.

## 30. Stop conditions

Stop and require a new decision if implementation needs:

- rational boundary weights;
- B-spline/NURBS boundary dispatch;
- runtime heterogeneous curve polymorphism;
- approximate corner matching;
- automatic boundary reversal;
- topology identity;
- trim loops;
- p-curves/curve-on-surface binding;
- arbitrary spatial placement frames;
- normals/metric/curvature;
- a universal tolerance;
- third-party runtime dependency;
- mesh generation.

## 31. Planned sequence after the Coons work unit

Planning only, not authorization.

After Coons implementation/closure, a fresh Surface Representation breadth
decision should compare at minimum:

1. analytic elementary surfaces;
2. ruled/extrusion/revolution surfaces;
3. rectangular/general trimmed-surface semantics;
4. broader Coons/transfinite boundary families, including rational/NURBS
   boundaries if required;
5. multiplicity-three/C0, arbitrary-degree and periodic NURBS breadth only if
   required by the admitted model class.

No option is pre-authorized.

## 32. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Oriented Four-Boundary Cubic Bézier Coons Patch in 3D.**

No production implementation begins on this decision branch.

Surface Differential Geometry remains blocked.

Boundary Curve Discretization remains blocked.

The remaining surface and curve breadth obligations remain retained, not
cancelled.


## 33. Decision integration checkpoint

PR #157 integrated this bounded Coons surface decision.

Final decision head:

`dbdee00c53595dd203c9d44a7e18138b8b85afce`.

Final decision-head validation:

- FAST `35841081221`: PASS;
- INTEGRATION `35841081217`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #157 merged as:

`639565e047a15a5b947f73fcadce10e51dde6bb0`.

Post-merge validation:

- FAST `35841180741`: PASS;
- INTEGRATION `35841180924`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the oriented four-boundary cubic Bézier Coons patch implementation
bounded by Sections 5–30.

No other surface family or downstream capability is authorized.
