# Bicubic Positive-Weight NURBS Surface Double-Knot C1 Continuity — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Surface Representation — Continuous Patch Geometry

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- polynomial cubic Bézier Curve Representation baseline: QUALIFIED by
  CGR0–CGR7 in the admitted cloud envelope;
- bounded parametric-surface contract: integrated;
- tensor-product bicubic polynomial Bézier patch: integrated focused work unit;
- positive-weight rational bicubic Bézier patch: integrated focused work unit;
- clamped positive-weight bicubic NURBS surface with runtime-variable simple
  U/V knots: integrated and closed focused work unit;
- cubic NURBS curve multiplicity-one/two C1 semantics:
  integrated and closed focused work unit.

## 1. Question

Which Surface Representation breadth item should follow the closed simple-knot
bicubic NURBS surface work unit?

Required candidates:

1. surface knot multiplicity-two / C1 continuity semantics;
2. Coons/transfinite patch construction;
3. analytic elementary surfaces;
4. ruled/extrusion/revolution surfaces;
5. rectangular/general trimmed-surface semantics;
6. arbitrary degree/periodicity only if the admitted input class requires it.

The selected work unit must isolate one new scientific seam without opening
Surface Differential Geometry or meshing.

## 2. Fresh repository entry authority

Decision-entry main:

`0d43b54aaec971c6887e481c9282b9fc4bff0049`.

Closed bicubic NURBS implementation lineage:

- candidate head:
  `86d794c58dc2eae3323f47de276ff637e2e2c3ec`;
- candidate FAST `35802648932`: PASS, 28/28;
- candidate INTEGRATION `35802648875`: PASS, 28/28 in GCC and Clang;
- final PR head:
  `7fc82143e8fae6a2ee164b5ebae7a57bdd9b55a2`;
- final PR FAST `35802788385`: PASS, 28/28;
- final PR INTEGRATION `35802788429`: PASS, 28/28;
- implementation PR #151:
  `3042f0a2eb1c4df20207248c1b16c7b023e5c525`;
- implementation post-merge FAST `35802888299`: PASS, 28/28;
- implementation post-merge INTEGRATION `35802888266`: PASS, 28/28;
- implementation closure PR #152:
  `0d43b54aaec971c6887e481c9282b9fc4bff0049`;
- closure PR FAST `35803111019`: PASS;
- closure PR INTEGRATION `35803111107`: PASS;
- closure post-merge FAST `35803190543`: PASS;
- closure post-merge INTEGRATION `35803190533`: PASS.

No production work item or open PR exists at decision entry.

## 3. Literature and mature-kernel basis

### 3.1 Surface continuity is directional and knot-multiplicity dependent

Open CASCADE `GeomConvert_BSplineSurfaceKnotSplitting`:

https://dev.opencascade.org/doc/occt-7.2.0/refman/html/class_geom_convert___b_spline_surface_knot_splitting.html

Relevant evidence:

- B-spline surface discontinuities are localized at knot values;
- continuity in a parameter direction at a knot is
  `degree - multiplicity`;
- therefore degree-three multiplicity one is C2 and multiplicity two is C1.

Decision impact:

- the first repeated-knot surface step can stay bicubic while introducing only
  C1 knot-line semantics;
- no arbitrary-degree or C0 policy is needed in the same work unit.

### 3.2 U and V multiplicities are independent surface data

STEP / Open CASCADE
`StepGeom_BSplineSurfaceWithKnots`:

https://dev.opencascade.org/doc/refman/html/class_step_geom___b_spline_surface_with_knots.html

Relevant evidence:

- U knots and U multiplicities are stored independently from V knots and
  V multiplicities;
- practical CAD surface interchange therefore requires directional
  multiplicity metadata.

Decision impact:

- AP Mesh must represent multiplicities independently in U and V;
- a U C1 knot line must not silently alter V continuity, and conversely.

### 3.3 Coons/filling is a different boundary-construction seam

Open CASCADE `GeomFill_BezierCurves`:

https://dev.opencascade.org/doc/occt-7.9.0/refman/html/class_geom_fill___bezier_curves.html

Relevant evidence:

- a filling surface is constructed from contiguous boundary curves;
- boundary compatibility and filling style are explicit construction inputs.

Decision impact:

- Coons/transfinite work introduces boundary ownership, compatibility and
  blending semantics rather than closing the current NURBS storage/continuity
  seam;
- it remains a later, separate Surface Representation decision.

### 3.4 Analytic elementary surfaces have placement/periodic semantics

Open CASCADE `Geom_ElementarySurface`:

https://dev.opencascade.org/doc/occt-7.0.0/refman/html/class_geom___elementary_surface.html

Open CASCADE `ElSLib`:

https://dev.opencascade.org/doc/refman/html/class_el_s_lib.html

Relevant evidence:

- plane, cylinder, cone, sphere and torus have family-specific analytic
  parameterizations;
- elementary surfaces are placed/oriented in 3D through a coordinate system;
- several families carry periodic and singular parameter behavior.

Decision impact:

- analytic-surface admission is a distinct placement/periodicity decision and
  should not be smuggled into a NURBS continuity extension.

### 3.5 Trimming is a supporting-surface plus boundary-domain seam

Open CASCADE `IGESGeom_TrimmedSurface`:

https://dev.opencascade.org/doc/refman/html/class_i_g_e_s_geom___trimmed_surface.html

Relevant evidence:

- a trimmed surface retains a supporting surface plus outer/inner boundary
  information;
- trimming introduces parameter-space domain and curve-on-surface/topology
  concerns.

Decision impact:

- trimming remains blocked until supporting-surface and boundary semantics are
  mature;
- it is not the smallest next change to the existing NURBS surface value type.

External references are scientific/design evidence only. No external runtime
dependency or numerical oracle is admitted.

## 4. Candidate comparison

| Candidate | New semantics | Reuse of current NURBS surface | Downstream value | Main unresolved burden | Decision |
| --- | --- | --- | --- | --- | --- |
| Multiplicity-two / C1 knot lines | Directional C1 continuity and second-jet failure | **Very high** | Direct CAD NURBS breadth and prerequisite clarity for surface differential geometry | Exact failure semantics at U/V double lines | **SELECTED** |
| Coons/transfinite | Boundary-driven construction and blending | Moderate | High for AP Mesh patch construction | Boundary compatibility/ownership/corners | DEFER |
| Analytic elementary | Placement, periodicity, singular parameterization | Low-moderate | High for validation/input breadth | Arbitrary placement and family-specific domains | DEFER |
| Ruled/extrusion/revolution | Generator ownership and sweep semantics | Moderate | Useful CAD breadth | Composition/orientation/domain rules | DEFER |
| Trimmed surface | Parameter-space loops and topology binding | Depends on mature boundaries | Very high for CAD faces | Curve-on-surface, loop identity/orientation | DEFER |
| Arbitrary degree/periodicity | Runtime degree and periodic seams | High | Broad CAD import | Larger algorithm/API expansion | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Bicubic Positive-Weight NURBS Surface with Interior U/V Knot Multiplicity One
or Two and Explicit C1/Second-Jet Failure Semantics.**

The existing public value type remains:

`BicubicNURBSSurface3`.

No new surface family is introduced.

## 6. Frozen representation envelope

The future implementation remains:

- degree exactly 3 in U and V;
- clamped endpoint multiplicity exactly 4;
- arbitrary finite bounded U/V domains;
- runtime-variable U/V span counts;
- positive finite weights;
- non-periodic;
- U-major flattened control/weight storage;
- unique interior U/V knot vectors;
- each interior multiplicity exactly 1 or 2.

No multiplicity three is admitted.

## 7. Size relations

Let unique U interior knots be `u_i` with multiplicities `m_i in {1,2}`.

Then:

`Nu = 4 + sum_i m_i`.

Likewise for V:

`Nv = 4 + sum_j n_j`.

The flattened control and weight net size remains:

`Nu * Nv`.

Distinct U span count remains:

`number_of_unique_u_interior_knots + 1`.

Distinct V span count is analogous.

The flat knot sequence repeats each unique interior knot according to its
multiplicity.

## 8. Public multiplicity API

The existing simple-knot factory must remain source-compatible and continue to
synthesize multiplicity one in both directions.

A new validated overload may accept:

- `std::vector<std::uint8_t> u_interior_multiplicities`;
- `std::vector<std::uint8_t> v_interior_multiplicities`.

Read-only accessors should expose immutable spans.

Every multiplicity must be exactly 1 or 2.

Construction errors may add explicit U/V multiplicity-count mismatch and
unsupported-multiplicity values. Existing construction-error meanings may not
be weakened.

## 9. Common surface error extension

The sole common query-error extension authorized is:

`SurfaceError::insufficient_continuity`.

No common concept signature changes are authorized.

The error means that the requested aggregate differential object is not
guaranteed by the represented continuity at that exact parameter pair.

## 10. Continuity semantics

For degree three:

- simple interior knot: C2 in that direction;
- double interior knot: C1 in that direction.

At a U double knot line:

- `evaluate(u,v)`: succeeds if finite/representable;
- `first_derivatives(u,v)`: succeeds;
- `second_derivatives(u,v)`: returns
  `SurfaceError::insufficient_continuity`.

At a V double knot line the same aggregate second-jet failure applies.

At an intersection of U and V double knot lines:

- value and first derivatives remain admitted;
- aggregate second derivatives fail with
  `SurfaceError::insufficient_continuity`.

At simple knot lines and on open spans, second derivatives retain current
semantics.

## 11. Why the whole second-derivative object fails

`SurfaceSecondDerivatives3` is an aggregate containing:

- `S_uu`;
- `S_uv`;
- `S_vv`.

At a U C1 knot line, `S_uu` is not representation-guaranteed.

At a V C1 knot line, `S_vv` is not representation-guaranteed.

Some individual components can still exist mathematically, but returning the
aggregate as if the entire second jet were guaranteed would be false.

This work unit therefore uses one explicit aggregate failure instead of adding
partial-component or one-sided APIs.

A later decision may add component-specific/one-sided derivative queries if
downstream algorithms prove they are required.

## 12. Exact failure ordering

For `second_derivatives(u,v)`:

1. validate U finiteness;
2. validate V finiteness;
3. validate U domain;
4. validate V domain;
5. if U is exactly a multiplicity-two interior knot, return
   `insufficient_continuity`;
6. if V is exactly a multiplicity-two interior knot, return
   `insufficient_continuity`;
7. evaluate the finite second jet;
8. return `non_finite_result` only for representability failure.

No epsilon participates in knot equality or continuity classification.

## 13. Accidental smoothness does not weaken representation semantics

A repeated knot may be inserted without changing the physical surface.

Such a particular surface can remain geometrically smoother than C1.

Nevertheless, the represented class with a multiplicity-two knot only
**guarantees C1**.

Therefore ordinary `second_derivatives()` must still return
`insufficient_continuity` exactly on the double knot line, even for:

- knot-insertion parity fixtures;
- constant surfaces;
- other accidentally smoother control nets.

This is a representation contract, not a numerical smoothness heuristic.

## 14. Production evaluation strategy

Production should preserve the current local homogeneous strategy.

Required changes are limited to:

- store unique multiplicities in U/V;
- construct U/V flat-knot caches once;
- locate spans on those flat sequences;
- keep local active 4x4 evaluation;
- keep analytic first/second rational derivatives off C1 lines.

No per-query allocation or global control/weight scan is introduced.

## 15. First-derivative semantics at C1 lines

A C1 cubic knot line guarantees the surface value and both first partials.

Focused evidence must verify at a U double knot:

- value from left/right agrees;
- `S_u` from left/right agrees;
- `S_v` from left/right agrees.

Analogous evidence is required at a V double knot.

The ordinary `first_derivatives()` query at the exact line must match the
independent reference.

## 16. Generic true-C1 fixture

At least one U-double-knot fixture must be selected where independent
one-sided `S_uu` values differ materially.

At least one V-double-knot fixture must similarly expose distinct one-sided
`S_vv`.

The tests must not merely assume that multiplicity two caused a visible second
derivative jump.

The one-sided evidence is test/reference evidence only. No one-sided
production API is authorized.

## 17. Independent repeated-knot rational tensor oracle

Focused tests must extend the existing independent Cox-de Boor tensor oracle
to flat knot vectors containing duplicate interior knots.

At minimum verify:

- value;
- `S_u`, `S_v`;
- off-line `S_uu`, `S_uv`, `S_vv`;
- exact double-line value/first derivatives;
- exact aggregate second-jet failure;
- U double line;
- V double line;
- U/V double-line intersection.

The oracle must not reuse production span location or local de Boor helpers.

## 18. Test-only repeated-knot insertion parity

Focused tests must create multiplicity-two surface data by inserting an
already-simple U knot and an already-simple V knot in homogeneous form.

Required physical parity away from the inserted line:

- value;
- first partials;
- second partials.

At the exact inserted double line:

- value and first partials remain parity-compatible;
- production aggregate second derivatives fail explicitly by representation
  policy.

Production knot insertion remains unauthorized.

## 19. Boundary-curve parity

Surface boundaries must continue to match integrated
`MultiSpanCubicNURBS3` curve semantics.

When a boundary's running parameter direction contains a multiplicity-two knot:

- boundary curve value/first derivative remain available;
- boundary curve second derivative returns
  `CurveError::insufficient_continuity`;
- surface value/first tangential partial must agree;
- surface aggregate second-jet query fails with
  `SurfaceError::insufficient_continuity`.

No new curve semantics are introduced.

## 20. U/V reversal

U reversal must:

- reverse U control rows and corresponding weights;
- reflect/reverse unique U knots;
- reverse U multiplicities in the same order;
- preserve V knots/multiplicities.

V reversal is analogous.

Required evidence:

- exact stored multiplicity reflection;
- involution;
- value/first-derivative covariance;
- aggregate second-jet failure maps to the reflected double knot line.

## 21. Simple-knot regression

The legacy simple-knot factory and current 28-test baseline remain frozen.

Explicit multiplicity vectors containing only ones must reproduce the existing
simple-knot representation semantics.

No existing passing simple-knot query may change error category or result.

## 22. Constant/degenerate surface semantics

Constant and rank-deficient surfaces remain valid representations.

At a double knot line:

- value succeeds;
- first derivatives succeed (zero for a constant surface);
- aggregate second derivatives still return
  `SurfaceError::insufficient_continuity`.

Continuity policy is checked before any constant-surface second-derivative
shortcut.

## 23. Determinism and finite behavior

All existing scale-aware finite behavior remains mandatory.

Focused evidence must retain:

- deterministic repeated success;
- deterministic repeated continuity failure;
- extreme finite values off a C1 line;
- explicit `non_finite_result` where final represented results cannot be
  expressed.

No universal tolerance or fallback is admitted.

## 24. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept preservation | Existing bounded-surface concept signatures unchanged. |
| Legacy factory | Simple-knot factory remains source/semantic compatible. |
| Multiplicity storage | U/V unique multiplicities explicitly preserved. |
| Construction failures | U/V multiplicity count and unsupported values typed. |
| Size relation | Nu/Nv follow sums of multiplicities plus four. |
| Simple-knot regression | Existing simple behavior preserved. |
| U C1 line | Value/first partials succeed; aggregate second jet fails. |
| V C1 line | Value/first partials succeed; aggregate second jet fails. |
| U/V intersection | Value/first partials succeed; aggregate second jet fails. |
| True C1 evidence | Independent one-sided Suu/Svv differ materially. |
| Repeated-knot oracle | Independent rational tensor basis validates value/partials. |
| Knot insertion | Test-only repeated insertion preserves physical surface. |
| Boundary parity | Curve C1 semantics and surface tangent semantics agree. |
| Reversal | Multiplicity reflection, involution and failure covariance. |
| Constant surface | Zero first partials but representation-level second-jet failure. |
| Determinism | Repeated success/failure identical. |
| Prerequisite preservation | Existing 28 ordinary tests remain PASS. |

If exactly one focused semantic contract is added, ordinary FAST/INTEGRATION
inventory becomes **29 tests**.

## 25. Why surface C1 continuity is selected now

The simple-knot NURBS surface already resolves:

- dynamic rectangular control/weight storage;
- independent U/V span location;
- local rational tensor evaluation;
- analytic first/second partials;
- NURBS boundary-curve parity.

The next smallest unresolved NURBS representation seam is directional
continuity when knots repeat.

Closing that seam before Surface Differential Geometry prevents normals,
metrics or curvature work from silently assuming a globally C2 NURBS surface.

## 26. Why Coons/transfinite is deferred

Coons/filling requires:

- boundary-curve ownership;
- contiguity/corner compatibility;
- blending/filling policy;
- possibly derivative compatibility at boundaries.

Those are substantial new construction semantics independent of the current
NURBS continuity question.

## 27. Why analytic elementary surfaces are deferred

Plane/cylinder/cone/sphere/torus introduce:

- family-specific parameter domains;
- arbitrary 3D placement/orientation;
- periodic directions;
- singular parameter locations for some families.

Those semantics should be admitted together under a dedicated analytic-surface
decision.

## 28. Why swept surfaces are deferred

Ruled/extruded/revolved surfaces require generator ownership, sweep transforms,
orientation and domain composition.

They are not prerequisites for resolving NURBS knot-line continuity.

## 29. Why trimming is deferred

General trimming introduces parameter-space boundary loops, curve-on-surface
correspondence, orientation and topology identity.

It depends on mature supporting-surface semantics and should not be merged into
a local NURBS continuity change.

## 30. Why arbitrary degree/periodicity is deferred

The doctoral baseline remains bicubic and non-periodic.

Variable degree and periodic seams alter active-control counts, knot
relationships and endpoint/domain semantics simultaneously.

They remain later only if the admissible input class demonstrates need.

## 31. Repository mapping for future implementation

Authorized future mapping is limited to:

- common error vocabulary only:
  `include/apmesh/geometry/parametric_surface.hpp`;
- existing NURBS surface API/storage:
  `include/apmesh/geometry/nurbs_surface.hpp`;
- existing production:
  `src/geometry/nurbs_surface.cpp`;
- focused contract:
  `tests/surface_bicubic_nurbs_double_knot_continuity.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

Frozen prerequisites:

- `include/apmesh/geometry/surface.hpp`;
- `src/geometry/surface.cpp`;
- `src/geometry/rational_surface.cpp`;
- curve production semantics.

If a common surface concept signature must change, stop and require a new
decision.

## 32. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 28 ordinary semantic tests must remain PASS;
- one new double-knot surface continuity contract must pass.

Passing yields only:

**SURFACE REPRESENTATION STAGE OPEN /
BICUBIC NURBS C1 CONTINUITY IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or authorize any next family.

## 33. Stop conditions

Stop and require a new decision if implementation needs:

- multiplicity three / C0;
- one-sided or component-specific partial-derivative public APIs;
- a common surface concept signature change;
- arbitrary degree;
- periodicity;
- Coons/transfinite construction;
- analytic elementary surfaces;
- ruled/extrusion/revolution;
- trimming/curve-on-surface/topology;
- Surface Differential Geometry;
- a universal tolerance;
- third-party dependency;
- discretization/meshing.

## 34. Planned sequence after this work unit

Planning only, not authorization.

After double-knot C1 surface integration/closure, recompare:

1. Coons/transfinite patch;
2. analytic elementary surfaces;
3. ruled/extrusion/revolution;
4. rectangular/general trimmed-surface semantics;
5. multiplicity-three/C0 only if required;
6. arbitrary degree/periodicity only if required.

No option is pre-authorized.

## 35. Effect if integrated and closed

After decision integration, post-merge validation and a separate decision
closure, the sole next production work item becomes:

**Bicubic Positive-Weight NURBS Surface with Interior U/V Knot Multiplicity
One or Two and Explicit C1/Second-Jet Failure Semantics.**

No implementation begins on this decision branch.

Surface Differential Geometry, Boundary Curve Discretization and meshing remain
blocked.
