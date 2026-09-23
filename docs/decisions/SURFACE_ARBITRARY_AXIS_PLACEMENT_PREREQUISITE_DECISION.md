# Arbitrary Right-Handed 3D Axis Placement — Surface Prerequisite Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-23  
Parent stage: Surface Representation — Continuous Patch Geometry  
Implementation layer if later authorized: Geometry Primitives extension

## 1. Question

After terminally closing the bounded cubic Bézier linear-extrusion surface,
which single next Surface Representation breadth step should be admitted?

Required comparison:

1. explicit arbitrary-placement prerequisite for analytic elementary surfaces;
2. bounded revolution surface;
3. general trimmed-surface / p-curve / face-boundary semantics;
4. broader Coons/transfinite boundaries;
5. remaining NURBS degree/multiplicity/periodic breadth.

The next step must unlock downstream surface families without silently widening
the already qualified exact Cartesian-frame claim.

## 2. Fresh repository authority

Decision-entry `main`:

`cbc29da630e6eba3f757bfedc06419431907d6d7`.

Terminal linear-extrusion lineage:

- implementation PR #170:
  `63b4d963fbed25e6482d37ae1944a08799043b2c`;
- implementation post-merge FAST `35892078309`: PASS, 32/32;
- implementation post-merge INTEGRATION `35892078255`: PASS, 32/32;
- implementation closure PR #171:
  `fccb7330e9fb0e5a45b53d9e72efee689f30ea2f`;
- closure post-merge FAST `35892574623`: PASS;
- closure post-merge INTEGRATION `35892574146`: PASS;
- terminal sync PR #172:
  `cbc29da630e6eba3f757bfedc06419431907d6d7`;
- sync PR FAST `35892883045`: PASS;
- sync PR INTEGRATION `35892882815`: PASS;
- sync post-merge FAST `35893168075`: PASS;
- sync post-merge INTEGRATION `35893168175`: PASS;
- ordinary semantic inventory: 32 tests;
- no active production work item.

## 3. Existing frame constraint

Current `CartesianFrame3` is intentionally exact and narrow:

- basis must be a signed permutation matrix;
- optional scale is a power of two;
- the type participates in an already qualified Geometry Primitives envelope.

That contract is preserved unchanged.

This decision does **not** weaken, reinterpret, or widen the qualified
`CartesianFrame3` claim.

## 4. External design evidence

### Open CASCADE gp_Ax3

Official references:

- https://dev.opencascade.org/doc/refman/html/gp___ax3_8hxx.html
- https://dev.opencascade.org/doc/occt-7.7.0/refman/html/classgp___ax3.html

Relevant evidence:

- a 3D local coordinate system is represented by a location plus orientation
  directions;
- construction from a point, main direction and X direction is a mature CAD
  pattern;
- the placement may later be used by analytic curves/surfaces without encoding
  shape-specific semantics inside the frame.

Decision impact:

- introduce a separate general right-handed placement value rather than
  broadening `CartesianFrame3`;
- construct orientation from finite vectors rather than accepting an arbitrary
  floating matrix claimed to be orthonormal.

### Open CASCADE elementary surfaces

Reference:
https://dev.opencascade.org/doc/occt-7.0.0/refman/html/class_geom___elementary_surface.html

Relevant evidence:

- elementary surfaces use a local 3D coordinate system;
- position, axis and parameter orientation are explicit geometry state.

Additional concrete evidence:

- spherical surface:
  https://dev.opencascade.org/doc/refman/html/class_geom___spherical_surface.html
- surface of revolution:
  https://dev.opencascade.org/doc/occt-7.8.0/refman/html/Geom__SurfaceOfRevolution_8hxx.html

Decision impact:

- arbitrary placement is a common prerequisite for sphere/cylinder/cone/torus
  and for general-axis revolution;
- implementing one analytic surface first would duplicate or prematurely hide
  this shared placement seam.

External references are design/scientific evidence only. No external runtime
dependency is admitted.

## 5. Candidate comparison

| Candidate | New semantics now | Shared downstream value | Dependency burden | Decision |
| --- | --- | --- | --- | --- |
| Arbitrary right-handed 3D placement prerequisite | finite direction normalization, orthogonal frame construction, local/world transforms | **Very high**: analytic elementary + revolution + later conics | bounded Geometry Primitives extension | **SELECTED** |
| Bounded revolution surface | axis + angular domain + trig/periodicity + generatrix | high | still needs arbitrary axis/placement semantics | DEFER |
| General trimming / p-curve / face seam | loops, p-curves, topology binding, holes/orientation | high | much broader topology/representation seam | DEFER |
| Broader Coons/transfinite | generic boundary-family composition | moderate-high | heterogeneous curve-family seam unresolved | DEFER |
| Remaining NURBS breadth | arbitrary degree/multiplicity/periodicity | high CAD breadth | independent from elementary/revolution placement | DEFER |

## 6. Decision

Authorize exactly one future implementation work unit:

**Right-Handed Arbitrary 3D Axis Placement.**

Proposed public type:

`AxisPlacement3`.

This is a Geometry Primitives extension required by Surface Representation.

It is **not** a replacement for `CartesianFrame3`.

## 7. Construction inputs

`AxisPlacement3::make` accepts:

- finite `Point3 origin`;
- finite non-zero `Vector3 main_direction`;
- finite non-zero `Vector3 x_reference`.

The constructor derives an orthonormal right-handed triad.

Semantic intent:

- `z_direction` follows `main_direction`;
- `x_direction` is the normalized component of `x_reference` orthogonal to
  the main direction;
- `y_direction` completes a right-handed frame.

A cross-product-first construction is preferred to avoid subtractive
orthogonalization when possible:

1. normalize main direction -> Z;
2. normalize X reference -> R;
3. compute Y candidate = normalize(cross(Z,R));
4. compute X = normalize(cross(Y,Z));
5. store X,Y,Z.

The exact implementation may use an algebraically equivalent robust sequence.

## 8. Failure semantics

Existing `GeometryError` should be reused.

At minimum:

- zero main direction -> `invalid_frame`;
- zero X reference -> `invalid_frame`;
- exactly collinear main/reference directions after representable arithmetic ->
  `invalid_frame`;
- any non-finite derived result -> `non_finite_result`.

No universal epsilon defines collinearity.

Near-collinear but non-zero finite input is not silently rejected by a global
threshold. If the finite derived frame is representable, construction may
succeed; otherwise failure is explicit.

## 9. Stored representation

Store exactly:

- origin;
- X direction;
- Y direction;
- Z direction.

No scale is stored.

This is a rigid placement/orientation value, not a similarity transform.

All stored directions are construction outputs and immutable.

## 10. Handedness and orientation

The first work unit admits only right-handed placements.

Required relation in mathematical intent:

`X cross Y = Z`.

No left-handed placement mode is introduced.

Surface parameter reversals remain surface-family semantics, not a frame
handedness switch.

## 11. Public operations

The bounded implementation may expose:

- `origin()`;
- `x_direction()`;
- `y_direction()`;
- `z_direction()`;
- `point_to_world(local)`;
- `vector_to_world(local)`;
- `point_to_local(world)`;
- `vector_to_local(world)`;
- `identity()`.

No generic 4x4 affine transform API is authorized.

No arbitrary scaling is authorized.

## 12. Local/world semantics

For local vector `v=(vx,vy,vz)`:

`world(v)=vx X + vy Y + vz Z`.

For local point `p`:

`world(p)=origin + world(vector(p))`.

Inverse vector coordinates are dot products with the stored axes.

Inverse point coordinates use the vector from origin followed by the inverse
vector map.

Intermediate overflow/underflow must be handled explicitly; final
non-representable results return `GeometryError::non_finite_result`.

## 13. Determinism

Construction and transforms are serial deterministic.

No random branch, hidden tolerance, architecture-dependent fallback or mutable
global state is admitted.

Repeated identical inputs must produce identical stored components and
success/failure outcomes within one admitted environment.

## 14. Qualified-subset parity

The new type must reproduce the existing scale-exponent-zero
`CartesianFrame3` transform semantics for every signed-permutation basis that
can be represented by an equivalent origin/main/X-reference construction.

This is mandatory prerequisite-preservation evidence.

The old type remains untouched and continues to define the previously qualified
exact subset.

## 15. Independent orientation evidence

Focused tests must include a genuinely arbitrary orientation not representable
by signed permutation, for example directions derived from non-axis-aligned
finite vectors.

Evidence must verify:

- stored directions are finite;
- unit-length relations within declared scale-aware comparison;
- pairwise orthogonality within declared comparison;
- right-handed cross-product orientation;
- local/world vector round-trip;
- local/world point round-trip.

The reference may be constructed independently in long double from normalized
input vectors.

Production helpers must not be reused by the independent oracle.

## 16. Input-scale invariance

Positive power-of-two scaling of either direction input must not change the
represented placement orientation.

Required evidence:

- main direction scaled independently;
- X reference scaled independently;
- both scaled;
- extreme finite but representable direction magnitudes.

Stored floating components need not be bit-identical across algebraically
different input scaling, but represented transforms must satisfy the declared
numeric comparison contract.

## 17. Exact identity fixture

For:

- origin=(0,0,0);
- main=(0,0,1);
- X reference=(1,0,0);

the placement should reproduce the identity orientation exactly where existing
primitive arithmetic permits exact identity.

## 18. Degenerate fixtures

Must reject:

- zero main direction;
- zero X reference;
- exactly parallel directions;
- exactly antiparallel directions.

A nearly parallel finite pair is a robustness fixture, not automatically a
domain failure.

No epsilon threshold is permitted.

## 19. Extreme finite evidence

At least one fixture must use extreme finite direction magnitudes and a finite
translated origin.

Required behavior:

- succeed if normalization and final transforms remain representable;
- otherwise fail explicitly with an existing `GeometryError`.

No silent rescaling of world coordinates changes the represented placement.

## 20. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Identity | Exact canonical XYZ orientation where representable. |
| Arbitrary orientation | Non-signed-permutation frame succeeds. |
| Right-handedness | X×Y agrees with Z under numeric contract. |
| Orthonormality | Pairwise dot≈0, norms≈1 under declared comparison. |
| Point round-trip | local→world→local returns input within contract. |
| Vector round-trip | local→world→local returns input within contract. |
| Signed-permutation parity | Matches `CartesianFrame3` with scale exponent 0. |
| Input scaling | Power-of-two scaling of direction inputs preserves transform. |
| Parallel inputs | Exact parallel/antiparallel rejected. |
| Zero inputs | Rejected explicitly. |
| Near parallel | Deterministic explicit success/failure; no epsilon. |
| Extreme finite | Scale-aware success or explicit failure. |
| Determinism | Repeated outputs/failures identical. |
| Header isolation | No surface/topology/mesh/I/O/threading/external dependency. |
| Prerequisites | Existing 32 ordinary tests remain PASS. |

If exactly one focused contract is added, ordinary semantic inventory becomes
**33 tests**.

## 21. Why not revolution next

A general surface of revolution requires an axis plus angular parameterization.
Mature CAD representations define revolution around an explicit axis and use an
angular parameter.

Implementing revolution before a reusable arbitrary placement/axis foundation
would either:

- hard-code a global coordinate axis; or
- duplicate placement semantics inside the surface class.

Both are rejected.

## 22. Why not elementary surfaces directly

Sphere, cylinder, cone and torus share local placement/orientation semantics.

Selecting one analytic surface first would not remove the common prerequisite.

The placement value should be established once, then reused by later
surface-family decisions.

## 23. Why not general trimming now

General trimming introduces:

- p-curves;
- loop ordering and holes;
- supporting-surface/physical-boundary correspondence;
- topology-face binding.

That is a larger cross-layer seam and is independent of the placement problem.

## 24. Why not broader Coons now

Broader Coons/transfinite support is mainly blocked by heterogeneous boundary
family/composition semantics, not by a missing surface value formula.

It remains later work.

## 25. Why not broader NURBS now

Current bicubic NURBS with multiplicity one/two already exercises rational
tensor-product knot semantics.

Arbitrary degree, multiplicity three and periodicity are important breadth but
do not unlock elementary/revolution placement.

They remain later CAD breadth decisions.

## 26. Qualification boundary

If later implemented and integrated, the result is only:

**ARBITRARY AXIS PLACEMENT IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / NOT QUALIFIED.**

It does not widen the already qualified Geometry Primitives claim.

It does not qualify arbitrary-angle frame equivalence.

It only provides an integrated prerequisite for future surface-family work.

## 27. Repository mapping for future implementation

Authorized future mapping is limited to:

- declaration:
  `include/apmesh/core/geometry.hpp`;
- production:
  `src/core/geometry.cpp`;
- focused contract:
  `tests/arbitrary_axis_placement.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized continuity authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  this decision.

Existing `CartesianFrame2/3` production behavior must remain frozen.

No surface production file is authorized by this work unit.

## 28. Stop conditions

Stop and require a new decision if implementation needs:

- tolerance-based orthogonality/parallel classification;
- left-handed placements;
- non-uniform scale;
- arbitrary affine/shear transforms;
- quaternion public API;
- surface-specific state;
- trigonometric/periodic semantics;
- topology/trim/mesh code;
- third-party runtime dependency.

## 29. Planned next comparison

Planning only, not authorization.

After arbitrary placement implementation/closure, recompare:

1. bounded revolution surface;
2. analytic elementary surfaces, likely beginning with plane/cylinder/sphere
   under the now-available placement contract;
3. general trimming/p-curve/topological-face semantics;
4. broader Coons/transfinite boundaries;
5. remaining NURBS breadth.

No option is pre-authorized.

## 30. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Right-Handed Arbitrary 3D Axis Placement.**

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

No analytic or revolution surface is implemented by this decision.


## 31. Decision integration checkpoint

PR #173 integrated this bounded prerequisite decision as:

`36381dec1f7af3a723fd386a3f55e0f109d804b1`.

Final decision-head validation:

- FAST `35894230229`: PASS;
- INTEGRATION `35894230134`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35894377748`: PASS;
- INTEGRATION `35894377875`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint is ready for documentation closure.

After closure integration and its post-merge validation, the sole next work
item is the bounded `AxisPlacement3` implementation defined by Sections
6–28.

No analytic surface, revolution, trimming, broader Coons/NURBS or downstream
capability is authorized.


## 32. Decision closure checkpoint

Decision closure PR #174 used head:

`56f9b85e95f5074b3f3b692b512472964735135e`.

Closure PR validation:

- FAST `35894725433`: PASS;
- INTEGRATION `35894725365`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #174 merged as:

`fbdfb98cfc5574c053f32298cab75d714ff31772`.

Closure post-merge validation:

- FAST `35900347873`: PASS;
- INTEGRATION `35900347965`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

After terminal documentation synchronization, the sole next production work
item is the bounded `AxisPlacement3` implementation defined by Sections
6–28.

The qualified `CartesianFrame3` claim remains unchanged.

No analytic elementary surface, revolution, trimming, broader Coons/NURBS or
downstream capability is authorized.


## 33. Active implementation mapping

Terminal decision synchronization PR #175 merged as:

`281c935578fc3fa9fb625178ce4473d54c684591`.

Sync validation:

- PR FAST `35900708572`: PASS;
- PR INTEGRATION `35900708552`: PASS;
- post-merge FAST `35900859164`: PASS;
- post-merge INTEGRATION `35900859257`: PASS.

The sole authorized implementation is active on:

`surface/arbitrary-axis-placement`.

Candidate repository mapping is restricted to:

- `include/apmesh/core/geometry.hpp`;
- `src/core/geometry.cpp`;
- `tests/arbitrary_axis_placement.cpp`;
- `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

The qualified exact `CartesianFrame3` implementation remains frozen.

Expected ordinary semantic inventory: **33 tests**.

Candidate implementation details:

- separate immutable `AxisPlacement3`;
- scale-aware maximum-component normalization;
- no universal tolerance for collinearity;
- long-double transform intermediates with explicit final representability
  checks;
- exhaustive parity across all 24 compatible right-handed signed-permutation
  bases;
- focused test excluded from qualification labels.

Candidate validation:

- candidate head:
  `bb19471c2307aec70427719baf3f8d9500605c42`;
- FAST `35901914044`: PASS, 33/33 ordinary tests;
- INTEGRATION `35901914002`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 33/33 tests per cell;
- `apmesh_core.arbitrary_axis_placement`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**


## 34. Implementation integration checkpoint

The bounded `AxisPlacement3` work unit was integrated by PR #176.

Initial candidate head:

`bb19471c2307aec70427719baf3f8d9500605c42`.

Initial candidate validation:

- FAST `35901914044`: PASS, 33/33;
- INTEGRATION `35901914002`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 33/33 per cell.

Final PR head:

`484b1b353f09e5e8e4b6b768ec0a5b9403960d72`.

Final-head validation:

- FAST `35902281707`: PASS, 33/33;
- INTEGRATION `35902281599`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 33/33 per cell.

PR #176 merged as:

`6d90036300671656c3bbde459dd2a783f8457cc1`.

Post-merge validation:

- FAST `35902586443`: PASS, 33/33;
- INTEGRATION `35902586537`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 33/33 per cell.

Integrated result:

**ARBITRARY AXIS PLACEMENT IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / CLOSURE PENDING / NOT QUALIFIED.**

The qualified exact `CartesianFrame3` claim remains unchanged.

After implementation closure, a fresh Surface Representation breadth decision
must select the next family. No option is pre-authorized.


## 35. Implementation closure checkpoint

Implementation closure PR #177 used head:

`cd1d5dea1da2146a581d309d0e90f0cc3e7d43b0`.

Closure PR validation:

- FAST `35903080976`: PASS;
- INTEGRATION `35903080966`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #177 merged as:

`fda3d1284ed500cdd97a7b6b153791f9d0884818`.

Closure post-merge validation:

- FAST `35903368279`: PASS;
- INTEGRATION `35903368467`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Terminal result:

**ARBITRARY AXIS PLACEMENT IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / CLOSED / NOT QUALIFIED.**

The qualified exact `CartesianFrame3` claim remains unchanged.

No implementation work item remains active.

After terminal documentation synchronization, the sole next admissible work is
one fresh literature-backed Surface Representation breadth decision comparing
revolution, analytic elementary surfaces, general trimming/p-curves,
broader Coons/transfinite boundaries and remaining NURBS breadth. No candidate
is pre-authorized.
