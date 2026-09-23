# Static Rectangular Trimmed Surface — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-23  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites at entry:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- bounded 3D surface abstraction: integrated focused prerequisite;
- tensor-product bicubic polynomial Bézier patch: integrated and closed;
- positive-weight rational bicubic Bézier patch: integrated and closed;
- bicubic positive-weight NURBS surface with runtime-variable U/V spans:
  integrated and closed;
- bicubic positive-weight NURBS surface with unique U/V multiplicities 1/2 and
  explicit C1/second-jet semantics: integrated and closed;
- oriented four-boundary cubic Bézier Coons patch:
  integrated and terminally closed.

## 1. Question

After closing the first boundary-defined Coons patch, which remaining Surface
Representation breadth concept should be introduced next?

Required candidates:

1. analytic elementary surfaces;
2. ruled/extrusion/revolution surfaces;
3. rectangular/general trimmed-surface semantics;
4. broader Coons/transfinite boundary families;
5. remaining NURBS breadth, including multiplicity-three/C0,
   arbitrary-degree and periodic semantics when justified.

The next work unit must add exactly one new representation seam without
simultaneously introducing arbitrary 3D placement, periodicity, topological
face loops, p-curves or runtime surface polymorphism.

## 2. Fresh repository entry authority

Canonical decision-entry `main`:

`efd084e8b247cde48e89bb67cd0b0097d005feab`.

Terminal Coons lineage:

- implementation PR #159:
  `3528612fdb875d8298d785a2c32f0fb4e1f8eea4`;
- implementation post-merge FAST `35843296073`: PASS, 30/30;
- implementation post-merge INTEGRATION `35843296147`: PASS, 30/30;
- implementation closure PR #160:
  `3a0edbbed28b43171cf27171525b13e9fb54ad3e`;
- closure post-merge FAST `35845572575`: PASS;
- closure post-merge INTEGRATION `35845572455`: PASS;
- terminal reconciliation PR #161 head:
  `3946ee5079490e68ee26add001a3a83828e06459`;
- terminal reconciliation FAST `35845757475`: PASS;
- terminal reconciliation INTEGRATION `35845757479`: PASS;
- terminal reconciliation merge:
  `efd084e8b247cde48e89bb67cd0b0097d005feab`;
- terminal reconciliation post-merge FAST `35845911399`: PASS;
- terminal reconciliation post-merge INTEGRATION `35845911492`: PASS.

Ordinary semantic inventory at entry: **30 tests**.

No open PR or production work item exists at entry.

## 3. Literature and mature-kernel evidence

### 3.1 Rectangular trimming is a basis-surface subdomain seam

Open CASCADE `Geom_RectangularTrimmedSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___rectangular_trimmed_surface.html

Relevant evidence:

- the representation owns a basis surface and U/V trim bounds;
- the trimmed domain must lie within the basis-surface domain;
- U and V trim directions have independent orientation;
- U/V reversal is surface-parameter orientation, not a change to the
  supporting surface geometry;
- point and derivative evaluation remains based on the supporting surface.

Decision impact:

- AP Mesh can isolate bounded rectangular subdomain/orientation semantics
  without opening arbitrary trimming loops or topology;
- this seam composes directly with every already integrated
  `BoundedParametricSurface3` value.

### 3.2 General B-rep trimming is materially broader

Open CASCADE `BRep_Tool::CurveOnSurface`:

https://dev.opencascade.org/doc/refman/html/class_b_rep___tool.html

Open CASCADE / IGES trimmed surface:

https://dev.opencascade.org/doc/refman/html/class_i_g_e_s_geom___trimmed_surface.html

Relevant evidence:

- general face boundaries may have curves represented in the parametric space
  of a supporting surface;
- p-curves, 3D edge curves, face identity and boundary loops are distinct
  entities;
- general trimming therefore couples representation to 2D boundary curves and
  topology in ways a rectangular subdomain does not.

Decision impact:

- **general trimmed-face semantics remain deferred**;
- the selected work unit must not introduce p-curves, wires, edge identities,
  inside/outside classification or topological face ownership.

### 3.3 Analytic elementary surfaces require placement and periodicity choices

Open CASCADE `Geom_Plane`:

https://dev.opencascade.org/doc/refman/html/class_geom___plane.html

Open CASCADE `Geom_CylindricalSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___cylindrical_surface.html

Relevant evidence:

- elementary surfaces use explicit local coordinate systems / axis placements;
- a plane is naturally unbounded in both parameters;
- a cylinder has a periodic angular parameter and an unbounded axial
  parameter;
- orientation follows the selected local frame.

Decision impact:

- analytic surfaces are important but would simultaneously require a broader
  arbitrary-placement contract, family-specific natural domains and periodic
  semantics;
- those concerns are larger than rectangular subdomain restriction over an
  already bounded surface.

### 3.4 Swept surfaces introduce generator and unbounded/periodic seams

Open CASCADE `Geom_SurfaceOfLinearExtrusion`:

https://dev.opencascade.org/doc/refman/html/class_geom___surface_of_linear_extrusion.html

Open CASCADE `Geom_SurfaceOfRevolution`:

https://dev.opencascade.org/doc/refman/html/class_geom___surface_of_revolution.html

Relevant evidence:

- extrusion owns a basis curve and a direction, with one naturally unbounded
  parameter;
- revolution owns a meridian and an axis, with an angular parameter over a
  full revolution;
- both introduce placement/generator semantics independently of rectangular
  trimming.

Decision impact:

- swept surfaces remain separate later work units;
- they do not block a static trim wrapper over already bounded surfaces.

### 3.5 Broader Coons/transfinite filling is construction, not trimming

Open CASCADE `GeomFill_BSplineCurves`:

https://dev.opencascade.org/doc/refman/html/class_geom_fill___b_spline_curves.html

Relevant evidence:

- B-spline filling constructs a new supporting surface from two/three/four
  contiguous boundary curves;
- filling style and boundary family are construction semantics;
- it does not model a restricted subdomain of an existing supporting surface.

Decision impact:

- broader Coons/transfinite work remains distinct from trimming;
- the integrated cubic Bézier Coons patch is preserved unchanged.

External sources are design/scientific reference evidence only. No Open CASCADE
runtime dependency or numerical oracle is admitted.

## 4. Candidate comparison

| Candidate | New semantic burden | Current prerequisites | Direct downstream value | Decision |
| --- | --- | --- | --- | --- |
| Static rectangular trim of a bounded surface | Basis ownership, U/V subdomain, independent U/V orientation, derivative sign covariance | **All required contracts already integrated** | **Very high** for patch restriction, decomposition and later face semantics | **SELECTED** |
| General trimmed surface / face | 2D p-curves, loops, inside/outside, topology identity, seam edges | Requires additional 2D curve/topology decisions | Critical later | DEFER |
| Analytic elementary surfaces | arbitrary 3D placement, unbounded and periodic family domains | Placement/periodicity not closed | High CAD breadth | DEFER |
| Ruled/extrusion/revolution | generator ownership, direction/axis, unbounded/angular semantics | Placement/axis policy not closed | High CAD breadth | DEFER |
| Broader Coons/transfinite | rational/NURBS/heterogeneous boundary dispatch and filling styles | Existing cubic Coons is sufficient baseline | Useful but not the smallest new seam | DEFER |
| Remaining NURBS breadth | C0 D1 semantics, arbitrary degree, periodic U/V | Not required for bounded rectangular trim | Important for import breadth | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Static Oriented Rectangular Trim of a Bounded Parametric Surface in 3D.**

Proposed public representation:

`template <BoundedParametricSurface3 Surface> class RectangularTrimmedSurface3`.

This is a value-oriented, compile-time generic wrapper.

It is not:

- a topological face;
- a general trimmed surface with arbitrary loops;
- a p-curve container;
- a runtime surface hierarchy;
- a reparameterization to `[0,1]^2`;
- a periodic-surface seam resolver.

## 6. Construction and stored representation

The wrapper owns by value:

- one basis `Surface`;
- `u_source`;
- `u_target`;
- `v_source`;
- `v_target`.

The four parameters are finite and each must lie inside the corresponding
basis-surface domain.

Required nonzero widths:

- `u_source != u_target`;
- `v_source != v_target`.

No epsilon participates in containment or zero-width decisions.

The signed source/target ordering encodes orientation independently in U and V.

## 7. Construction error vocabulary

Proposed bounded error set:

- `non_finite_u_source_parameter`;
- `non_finite_u_target_parameter`;
- `non_finite_v_source_parameter`;
- `non_finite_v_target_parameter`;
- `u_source_parameter_out_of_domain`;
- `u_target_parameter_out_of_domain`;
- `v_source_parameter_out_of_domain`;
- `v_target_parameter_out_of_domain`;
- `zero_width_u_trim`;
- `zero_width_v_trim`.

Deterministic validation order:

1. U source finiteness;
2. U target finiteness;
3. V source finiteness;
4. V target finiteness;
5. U source containment;
6. U target containment;
7. V source containment;
8. V target containment;
9. nonzero U width;
10. nonzero V width.

No new common `SurfaceError` value is expected.

## 8. Public parameter domain

The wrapper exposes the sorted rectangular domain:

- U:
  `[min(u_source,u_target), max(u_source,u_target)]`;
- V:
  `[min(v_source,v_target), max(v_source,v_target)]`.

The domain is **not normalized or rescaled**.

This preserves basis parameter magnitudes and avoids introducing derivative
scale factors unrelated to trimming.

## 9. Parameter mapping

For a valid wrapper query `(u,v)`:

- if `u_source < u_target`, mapped U is `u`;
- otherwise mapped U is the exact existing overflow-aware
  `reversed_parameter(trim_u_domain, u)`;
- V follows the same independent rule.

The mapped pair is then passed to the basis surface.

No clamping, wrapping, epsilon snapping or hidden retry is permitted.

## 10. Value semantics

For every valid query:

`T(u,v) = S(map_u(u), map_v(v))`.

Exact basis values should remain exact where the basis already provides exact
identity.

No new approximation algorithm is introduced.

## 11. First-derivative covariance

Let:

- `su = +1` for forward U orientation and `-1` for reversed U;
- `sv = +1` for forward V orientation and `-1` for reversed V.

If the basis returns:

- `S_u`;
- `S_v`;

the wrapper returns:

- `T_u = su * S_u`;
- `T_v = sv * S_v`.

Any basis `SurfaceError` propagates unchanged.

## 12. Second-derivative covariance

If the basis returns:

- `S_uu`;
- `S_uv`;
- `S_vv`;

the wrapper returns:

- `T_uu = S_uu`;
- `T_uv = su * sv * S_uv`;
- `T_vv = S_vv`.

Therefore a single-direction reversal flips the mixed derivative and preserves
the corresponding pure second derivative.

Any basis continuity failure, including
`SurfaceError::insufficient_continuity`, propagates unchanged at the mapped
parameter.

## 13. Reversal API

The wrapper should provide:

- `u_reversed()`;
- `v_reversed()`.

U reversal swaps `u_source` and `u_target`.

V reversal swaps `v_source` and `v_target`.

Required properties:

- parameter domain unchanged;
- exact stored basis unchanged;
- one-direction reversal flips surface orientation where the basis is regular;
- double reversal in one direction recovers exact stored state;
- U/V reversals commute.

No normal-vector API is introduced here.

## 14. Basis access and ownership

The wrapper exposes:

- `const Surface& basis_surface() const noexcept`;
- source/target parameter accessors.

The basis is owned by value and is immutable after successful construction.

No pointer ownership, virtual base class, shared mutable state or global
registry is introduced.

## 15. Static genericity boundary

The work unit is intentionally compile-time generic over
`BoundedParametricSurface3`.

Focused evidence must instantiate the wrapper over multiple already integrated
families:

- `BicubicBezierPatch3`;
- `RationalBicubicBezierPatch3`;
- `BicubicNURBSSurface3`;
- `CubicBezierCoonsPatch3`.

This demonstrates that trimming is a surface-contract seam rather than a new
closed runtime family hierarchy.

If one of those families cannot be trimmed without changing the common surface
concept, stop and require a new decision.

## 16. Nested-trim semantics

Because the wrapper itself satisfies `BoundedParametricSurface3`, a second
rectangular trim may wrap an existing rectangular trim.

Required evidence:

- nested subdomain fully contained in the first trim succeeds;
- direct trim of the original basis over the same final oriented U/V interval
  matches value and derivatives;
- a nested trim attempting to escape the first trim fails construction even if
  the parameter would still lie inside the original basis domain.

No automatic flattening is required in production.

## 17. Boundary evidence

The four trim boundaries are parameter-domain restrictions of the basis.

Focused evidence must verify:

- exact corner mapping;
- U-boundary value parity with the basis at mapped U;
- V-boundary value parity with the basis at mapped V;
- tangential derivative sign covariance under independent orientation.

No boundary-curve extraction API is authorized by this work unit.

## 18. Continuity evidence

The wrapper does not change basis continuity.

Focused evidence using `BicubicNURBSSurface3` must include:

- a trim whose domain contains a multiplicity-two U or V knot;
- first derivatives continue to follow the basis contract;
- second-jet failure propagates exactly where the basis returns
  `SurfaceError::insufficient_continuity`;
- trimming must not hide or reinterpret that failure.

## 19. Determinism and finite behavior

Focused evidence must include:

- repeated identical successes;
- repeated identical typed failures;
- forward and reversed U/V trims;
- extreme but finite trim parameters from a non-normalized NURBS surface
  domain;
- no intermediate overflow from reversal mapping where the existing
  `reversed_parameter` primitive avoids it.

The wrapper itself introduces no coordinate arithmetic beyond derivative sign
changes.

## 20. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Wrapper satisfies `BoundedParametricSurface3` for each admitted basis fixture. |
| Construction validation | Non-finite/out-of-domain/zero-width U/V trims fail deterministically. |
| Domain | Sorted U/V trim bounds exposed exactly; no normalization. |
| Value forwarding | Basis value at mapped parameter reproduced. |
| First derivatives | Independent U/V sign covariance. |
| Second derivatives | Pure second partials preserved; mixed partial gets product sign. |
| U reversal | Domain preserved, mapping/sign law and involution. |
| V reversal | Domain preserved, mapping/sign law and involution. |
| U/V commutation | Double-direction reversal order is deterministic/equivalent. |
| Polynomial basis | Bicubic Bézier trim parity. |
| Rational basis | Rational bicubic trim parity. |
| NURBS basis | Non-normalized domain and C1 continuity failure propagation. |
| Coons basis | Boundary-defined surface trim parity. |
| Nested trim | Nested/direct final subdomain parity; escape rejected. |
| Corners/boundaries | Exact mapped corner/boundary evidence. |
| Extreme finite | Overflow-aware reversal/domain mapping remains representable where expected. |
| Determinism | Repeated success/failure evidence identical. |
| Header isolation | No topology, p-curve, meshing, I/O, threading or external dependency. |
| Regression | Existing 30 ordinary semantic tests remain passing. |

If exactly one focused test is added, ordinary FAST/INTEGRATION inventory
becomes **31 tests**.

## 21. Why rectangular trimming is selected now

The repository already has several continuous supporting-surface families.

A rectangular trim:

- immediately exercises the common surface abstraction across those families;
- introduces subdomain/orientation semantics needed for later face and meshing
  workflows;
- does not require arbitrary spatial placement;
- does not require periodicity;
- does not require p-curves or topological loops;
- is therefore smaller and better isolated than every other remaining
  candidate.

## 22. Why general trimming is deferred

General trimmed faces require a substantially broader contract:

- arbitrary boundary loops in parameter space;
- p-curves;
- outer/inner classification;
- orientation;
- relationship between 2D trim curves and 3D edge curves;
- explicit face/edge topology identity;
- seam behavior on periodic surfaces.

Those semantics belong to a later decision that explicitly joins Surface
Representation and the already qualified Topological Model.

The present work unit must not infer topology from coordinates.

## 23. Why analytic elementary surfaces are deferred

Plane/cylinder/cone/sphere/torus families remain required breadth.

They are deferred because a scientifically coherent family admission also
requires decisions for:

- arbitrary 3D placement/orientation;
- finite versus natural unbounded domains;
- periodic angular directions;
- singular parameter locations for sphere/cone-like families.

Those concerns are independent of rectangular trimming.

## 24. Why swept surfaces are deferred

Extrusion/revolution/ruled surfaces require:

- basis/generator curve ownership;
- spatial direction or axis representation;
- unbounded and/or angular domains;
- additional degeneracy semantics.

They remain later bounded decisions.

## 25. Why broader Coons/transfinite is deferred

The current cubic Bézier Coons patch establishes the boundary-defined surface
seam.

Generalizing boundaries to rational/NURBS or heterogeneous runtime curve
families would introduce dispatch/composition questions without being required
for rectangular restriction of existing surfaces.

## 26. Why remaining NURBS breadth is deferred

Multiplicity-three/C0, arbitrary degree and periodic U/V remain explicit
surface-breadth obligations.

They are not prerequisites for trimming the currently integrated bounded
NURBS family.

A later import-driven decision must justify which of them is required by the
admitted CAD model class.

## 27. Repository mapping for future implementation

Authorized future mapping is limited to:

- new header-only public/static wrapper:
  `include/apmesh/geometry/trimmed_surface.hpp`;
- focused semantic/reference contract:
  `tests/surface_rectangular_trim.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No change to:

- `include/apmesh/geometry/parametric_surface.hpp`;
- existing production surface sources;
- topology;
- curve APIs

is expected.

If a common surface-contract change becomes necessary, stop and require a new
decision.

## 28. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 30 ordinary semantic tests must remain passing;
- one new rectangular-trim contract must pass.

Passing yields only:

**STATIC RECTANGULAR TRIM IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / NOT QUALIFIED.**

It does not qualify:

- general trimmed surfaces;
- topological faces;
- p-curves;
- analytic or swept surfaces;
- broader Coons/NURBS breadth;
- Surface Differential Geometry;
- surface meshing.

## 29. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricSurface3`;
- runtime surface polymorphism;
- arbitrary trim curves;
- p-curves;
- topology IDs or face loops;
- inside/outside classification;
- periodic wrap semantics;
- analytic placement axes;
- a universal tolerance;
- surface normals/curvature/metric;
- meshing/discretization;
- third-party runtime dependency.

## 30. Planned sequence after this work unit

Planning only, not authorization.

After rectangular trimming is integrated and closed, a fresh Surface
Representation breadth decision should recompare:

1. analytic elementary surfaces;
2. ruled/extrusion/revolution surfaces;
3. general trimmed-surface / curve-on-surface / face-boundary semantics;
4. broader Coons/transfinite boundary families;
5. remaining NURBS breadth required by the admitted CAD input class.

No option is pre-authorized.

## 31. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Static Oriented Rectangular Trim of a Bounded Parametric Surface in 3D.**

No implementation begins on this decision branch.

Surface Representation remains **IN INVESTIGATION / NOT QUALIFIED**.

Surface Differential Geometry and Boundary Curve Discretization remain
blocked.


## 32. Decision integration checkpoint

PR #162 integrated this bounded decision from final head:

`0ee2a2b47592eee055dcc58ccba04c0364353fe5`.

Decision-head validation:

- FAST `35846509153`: PASS;
- INTEGRATION `35846509081`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #162 merged as:

`b0ed7acaf88b3267c0e1af1e79db657b9cc70540`.

Post-merge validation:

- FAST `35846672914`: PASS;
- INTEGRATION `35846673106`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Static Oriented Rectangular Trim of a Bounded Parametric Surface in 3D.**

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the implementation bounded by Sections 5–29.

General trim loops/p-curves/topology, analytic/swept surfaces, broader
Coons/transfinite boundaries, remaining NURBS breadth and downstream
differential/meshing capabilities remain unauthorized.


## 33. Decision closure checkpoint

Decision closure PR #163 used head:

`012703f1f8cd5cdf2ed8922269c2d17e8b3cb236`.

Closure PR validation:

- FAST `35847140517`: PASS;
- INTEGRATION `35847140605`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #163 merged as:

`dbaffc020bdd8d7197f94b17f9f85b44367da1f0`.

Closure post-merge validation:

- FAST `35847281819`: PASS;
- INTEGRATION `35847281719`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

After terminal documentation synchronization, the sole next production work
item is the static rectangular trimmed-surface implementation bounded by
Sections 5–29.

General trimming/topology, analytic/swept surfaces, broader Coons/transfinite,
remaining NURBS breadth and downstream differential/meshing capabilities remain
unauthorized.


## 34. Terminal decision synchronization

Terminal synchronization PR #164 used head:

`8c3b4c623bb3bd5812edce8723bb74eca7f9c667`.

Synchronization validation:

- FAST `35847597802`: PASS;
- INTEGRATION `35847597868`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #164 merged as:

`83a56ee4f1b6a4703956c3e81c1786540bfecb14`.

Synchronization post-merge validation:

- FAST `35847689693`: PASS;
- INTEGRATION `35847690044`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision is terminally closed and the sole authorized implementation is
active on:

`surface/rectangular-trimmed-surface`.

No broader trim/topology, analytic/swept surface, differential-geometry or
meshing capability is authorized.


## 35. Active implementation mapping

The sole authorized implementation is active on:

`surface/rectangular-trimmed-surface`.

Candidate mapping:

- public/static wrapper:
  `include/apmesh/geometry/trimmed_surface.hpp`;
- focused contract:
  `tests/surface_rectangular_trim.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

Candidate production semantics:

- basis surface owned by value;
- deterministic construction validation in the order frozen by Section 7;
- exact sorted U/V trim domains;
- independent U/V orientation encoded by source/target order;
- no normalization/rescaling;
- overflow-aware reflection using the existing `reversed_parameter`;
- exact basis value forwarding;
- first derivative orientation covariance;
- pure second partial preservation;
- mixed-partial product sign;
- U/V reversal, commutation and involution;
- nested wrapper compatibility through `BoundedParametricSurface3`.

The focused contract instantiates the wrapper over:

- `BicubicBezierPatch3`;
- `RationalBicubicBezierPatch3`;
- `BicubicNURBSSurface3`;
- `CubicBezierCoonsPatch3`.

It also covers nested trimming, NURBS C1 continuity failure propagation,
extreme finite non-normalized NURBS domains and deterministic failures.

Expected ordinary semantic inventory: **31 tests**.

Corrected candidate validation:

- corrected candidate head:
  `e1a9d4df3d7e8bb8ce441901a9075caa371f0ded`;
- FAST `35873716331`: PASS, 31/31 ordinary semantic tests;
- INTEGRATION `35873716367`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 31/31 tests in each cell;
- `apmesh_core.surface_rectangular_trim`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS;
- production `trimmed_surface.hpp` remained unchanged from the initial
  failed attempt.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**


## 36. Initial implementation validation attempt

PR #165 initial implementation head:

`296d8fd7f61b2c3f6681e4a27ef8149dfd990809`.

Initial validation:

- FAST `35873420030`: FAIL, 30/31 ordinary tests;
- INTEGRATION `35873419776`: FAIL in GCC 13 Debug and Clang 18/libc++
  Debug, 30/31 tests in each cell;
- only `apmesh_core.surface_rectangular_trim` failed;
- emitted assertion:
  `nested rectangular trim semantics differ`.

Audit diagnosis:

**TEST-ORACLE EXPECTATION MISMATCH / NO PRODUCTION SEMANTIC DEFECT SHOWN.**

The failing assertion compared a nested trim and its mathematically equivalent
direct trim with bitwise equality. Equivalent reversal compositions can reach
the same physical parameter through different intermediate interval arithmetic
and therefore do not guarantee bit-identical floating results.

Corrective action:

- preserve production `trimmed_surface.hpp` unchanged;
- replace only nested/direct exact equality with explicit scale-aware numerical
  parity for value, first partials and second partials;
- revalidate the new immutable PR head in FAST and both INTEGRATION cells.

This failed attempt remains part of the permanent validation history.


## 37. Implementation integration checkpoint

PR #165 final implementation head:

`b672572fc4495b3ff1e369a3bc346673978dba3b`.

Final PR validation:

- FAST `35874000945`: PASS, 31/31 ordinary semantic tests;
- INTEGRATION `35874000992`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 31/31 tests per cell;
- `apmesh_core.surface_rectangular_trim`: PASS in all three jobs.

PR #165 merged as:

`07a5836ceabace389c4a6bfc2d1f60d644a7a939`.

Post-merge validation:

- FAST `35874273067`: PASS, 31/31;
- INTEGRATION `35874273154`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 31/31.

Integrated result:

**STATIC RECTANGULAR TRIM IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / CLOSURE PENDING / NOT QUALIFIED.**

The initial failed head and its test-oracle diagnosis remain preserved in
Section 36.

No common surface-contract or existing production-surface source changed.

The implementation checkpoint is ready for documentation/continuity closure.
No next Surface Representation production work is authorized until that
closure is integrated and post-merge validated.


## 38. Implementation closure checkpoint

Implementation closure PR #166 used head:

`7ce52b5e880f4d7bd454c630aad00e89678eaf1e`.

Closure PR validation:

- FAST `35874987379`: PASS;
- INTEGRATION `35874987450`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #166 merged as:

`ad95dc048be988ad6f3fd9b29203f6e9d6d92b10`.

Closure post-merge validation:

- FAST `35875154581`: PASS;
- INTEGRATION `35875154595`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Terminal result:

**STATIC RECTANGULAR TRIM IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / CLOSED / NOT QUALIFIED.**

The initial failed validation and corrected acceptance lineage remain preserved
in Sections 36–37.

No rectangular-trim implementation work remains active.

After terminal documentation synchronization, the sole next admissible action
is one fresh literature-backed Surface Representation breadth decision
comparing the remaining candidates listed in Section 30. No candidate is
pre-authorized.
