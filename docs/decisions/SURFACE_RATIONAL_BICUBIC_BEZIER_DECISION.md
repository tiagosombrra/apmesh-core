# Rational Bicubic Bézier Surface — Bounded Breadth Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Surface Representation — Continuous Patch Geometry

## 1. Question

After integrating and closing the first tensor-product bicubic polynomial
Bézier patch, which Surface Representation family should be introduced next?

Required candidates:

1. rational Bézier patch;
2. B-spline/NURBS surface;
3. Coons/transfinite patch;
4. analytic elementary surfaces;
5. ruled/extrusion/revolution surfaces;
6. rectangular/general trimmed-surface semantics.

The next work unit must add one independent semantic dimension only and must
not imply that the full Surface Representation stage is qualified.

## 2. Fresh repository authority

Decision-entry main:

`bf218b31eb2b71d7ed651183b0cfc649c96b8c7c`.

Terminal first-surface lineage:

- bicubic polynomial implementation PR #140:
  `2d6d01e4202367d62db6017939cde0f2b8e83c65`;
- implementation post-merge FAST `35766651557`: PASS, 26/26;
- implementation post-merge INTEGRATION `35766651497`: PASS, 26/26;
- closure PR #141:
  `e964465fd7f7ffcf9d2403752cf9f791b8666433`;
- closure post-merge FAST `35767269783`: PASS;
- closure post-merge INTEGRATION `35767269772`: PASS;
- terminal reconciliation PR #142:
  `bf218b31eb2b71d7ed651183b0cfc649c96b8c7c`;
- terminal reconciliation FAST `35768039368`: PASS;
- terminal reconciliation INTEGRATION `35768039528`: PASS.

Ordinary semantic baseline: **26 tests**.

No production work item or open PR exists at decision entry.

## 3. External evidence

### 3.1 Open CASCADE Bézier surface rationality

Open CASCADE `Geom_BezierSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___bezier_surface.html

Relevant evidence:

- a mature CAD Bézier surface may be polynomial or rational;
- rationality is expressed through a two-dimensional weight array associated
  with the pole/control array;
- the U/V control-net structure remains the same when weights are introduced.

Decision impact:

- rational weights can be isolated without introducing U/V knots,
  multiplicities, periodicity, trimming or runtime degree;
- all-one/equal weights provide a direct reduction to the already integrated
  polynomial bicubic patch.

### 3.2 Rational and polynomial free-form surfaces remain distinct from NURBS

Open CASCADE `BSplSLib`:

https://dev.opencascade.org/doc/refman/html/class_b_spl_s_lib.html

Open CASCADE / STEP rational B-spline surface data models:

https://dev.opencascade.org/doc/refman/html/class_step_geom___b_spline_surface_with_knots_and_rational_b_spline_surface.html

Relevant evidence:

- a NURBS surface combines a 2D control net and weights with independent U/V
  degrees, knots, multiplicities and periodicity state;
- rationality is therefore only one part of full NURBS surface semantics.

Decision impact:

- introducing a rational Bézier patch first isolates homogeneous denominator
  semantics before the independent U/V knot/continuity problem;
- the later NURBS surface decision can reuse a separately validated rational
  surface layer and the already integrated curve knot/multiplicity layer.

### 3.3 NURBS surface continuity is a separate two-direction problem

Open CASCADE B-spline surface knot splitting:

https://dev.opencascade.org/doc/refman/html/class_geom_convert___b_spline_surface_knot_splitting.html

Relevant evidence:

- U/V knot multiplicities localize continuity changes independently in each
  parametric direction;
- a full NURBS surface must decide first/second/mixed partial behavior on knot
  lines.

Decision impact:

- adding rationality and U/V continuity in the same work unit would combine
  two independently testable scientific mechanisms;
- NURBS surfaces remain the preferred next spline-family candidate after the
  rational bicubic patch closes.

### 3.4 Coons, analytic, swept and trimmed surfaces are distinct families

Open CASCADE Coons/boundary filling:

https://dev.opencascade.org/doc/refman/html/_geom_fill___b_spline_curves_8hxx.html

Open CASCADE elementary surface evaluation:

https://dev.opencascade.org/doc/refman/html/class_el_s_lib.html

Open CASCADE geometry taxonomy and trimmed-surface references already recorded
in `docs/research/REFERENCE_REGISTER.md`.

Decision impact:

- Coons/transfinite filling introduces boundary compatibility/construction
  semantics;
- analytic elementary surfaces introduce placement, periodic/angular domains
  and singular parameter lines;
- swept surfaces depend on base-curve and construction semantics;
- trimming introduces supporting-surface/trim/topology separation;
- none of those mechanisms should be bundled with the first rational surface
  extension.

External references are design/scientific evidence only. No external runtime
dependency or numerical oracle is admitted.

## 4. Candidate comparison

| Candidate | New semantics now | Reuse | Downstream value | Main extra burden | Decision |
| --- | --- | --- | --- | --- | --- |
| Rational bicubic Bézier patch | Positive 2D weight net and homogeneous quotient derivatives | Existing bicubic patch + rational curve experience | High; isolates rational surface semantics and prepares NURBS | Denominator/weight scaling only | **SELECTED** |
| B-spline/NURBS surface | U/V dynamic nets, knots, multiplicities, continuity lines, weights | Curve NURBS + bicubic patch | Very high | Adds rationality and two-direction spline continuity at once | DEFER one work unit |
| Coons/transfinite | Boundary compatibility and transfinite construction | Curve boundaries + surface contract | High for AP Mesh construction | Construction semantics distinct from stored tensor patch | DEFER |
| Analytic elementary | Placement, angular/periodic domains, singularities | Primitive geometry | High CAD/reference value | Arbitrary orientation and family-specific parameter domains | DEFER |
| Ruled/extrusion/revolution | Generator/directrix and construction semantics | Curve families + surface contract | Medium-high | Depends on curve breadth/orientation policy | DEFER |
| Trimmed surface | Supporting surface + parameter-space trim + topology/orientation | Surface + curve + topology | Very high CAD value | Requires mature support/curve-on-surface semantics | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Positive-Weight Rational Tensor-Product Bicubic Bézier Patch in 3D.**

Proposed public type:

- `RationalBicubicBezierPatch3`.

The future implementation must keep frozen:

- degree exactly three in U;
- degree exactly three in V;
- exactly 4x4 finite `Point3` controls;
- exactly 4x4 finite strictly positive weights;
- exact concrete domain `[0,1] x [0,1]`;
- non-periodic U/V semantics;
- no knots or multiplicities;
- no trimming;
- no regularity rejection.

## 6. Public value and construction semantics

The patch owns:

- `BicubicBezierPatch3::ControlNet`-equivalent 4x4 controls;
- a 4x4 `double` weight net.

Preferred aliases:

- `WeightRow = std::array<double,4>`;
- `WeightNet = std::array<WeightRow,4>`.

Construction must be validated because raw `double` weights may be invalid.

At minimum reject:

- any NaN/infinite weight;
- any weight `<= 0`.

No epsilon defines positivity.

Control finiteness remains guaranteed by `Point3`.

Suggested construction error vocabulary:

- `non_finite_weight`;
- `non_positive_weight`.

No existing `SurfaceError` meaning should change.

## 7. Mathematical representation

Let `B_i^3` denote cubic Bernstein basis functions.

Define:

`A(u,v) = sum_i sum_j B_i^3(u) B_j^3(v) w_ij P_ij`

`W(u,v) = sum_i sum_j B_i^3(u) B_j^3(v) w_ij`

and

`S(u,v) = A(u,v) / W(u,v)`.

With all admitted weights strictly positive and Bernstein basis nonnegative on
`[0,1]`, `W(u,v) > 0` in exact arithmetic.

The production algorithm should use homogeneous tensor-product de Casteljau,
preserving the established deterministic **V-then-U** reduction order.

## 8. Scale-aware homogeneous policy

Raw coordinates must not be multiplied by arbitrarily large weights before a
common scaling step.

For each query, or through an equivalent bounded deterministic strategy:

1. choose a positive finite common weight scale;
2. divide all 16 weights by that common scale;
3. construct homogeneous controls from scaled weights;
4. evaluate homogeneous value/derivative data;
5. dehomogenize.

Common positive scaling of all weights must leave value and all partial
derivatives unchanged within the declared numerical contract.

Stored public weights remain the exact user inputs.

Because the patch is fixed 4x4, scanning 16 weights is bounded constant work and
does not introduce runtime-size semantics.

## 9. Exact domain and validation order

The concrete domain remains exactly `[0,1] x [0,1]`.

Reuse the existing bounded-surface query contract and validation order:

1. U finiteness;
2. V finiteness;
3. U domain;
4. V domain;
5. numerical result.

The common `BoundedParametricSurface3` concept should remain unchanged.

If the common concept must change, stop and require a new decision.

## 10. Exact corners

Required exact identities:

- `S(0,0)=P00`;
- `S(0,1)=P03`;
- `S(1,0)=P30`;
- `S(1,1)=P33`.

Production should preserve exact corner identity explicitly rather than rely on
floating homogeneous division to rediscover it.

## 11. First partial derivatives

Let homogeneous numerator derivatives be `A_u`, `A_v` and weight
derivatives `W_u`, `W_v`.

Required analytic identities:

`S_u = (A_u - S W_u) / W`

`S_v = (A_v - S W_v) / W`.

No finite differences are permitted.

## 12. Second partial derivatives

Required analytic identities:

`S_uu = (A_uu - 2 S_u W_u - S W_uu) / W`

`S_vv = (A_vv - 2 S_v W_v - S W_vv) / W`

`S_uv = (A_uv - S_u W_v - S_v W_u - S W_uv) / W`.

One deterministic mixed derivative `S_uv` is exposed, matching the existing
surface contract.

No regularity or normal-vector semantics are introduced.

## 13. Equal-weight polynomial parity

If all weights are equal positive values, the rational patch must reproduce the
integrated `BicubicBezierPatch3` in:

- parameter domain;
- value;
- `S_u`, `S_v`;
- `S_uu`, `S_uv`, `S_vv`;
- U reversal;
- V reversal;
- typed query failures.

All-one and non-unit equal-weight cases are both required.

The polynomial implementation remains frozen and is not replaced.

## 14. Independent rational Bernstein oracle

Focused tests must implement a direct rational Bernstein oracle independent of
production homogeneous de Casteljau helpers.

At minimum cover:

- asymmetric non-planar 4x4 control net;
- strongly nonuniform positive weight net;
- multiple interior parameter pairs;
- every edge;
- every corner;
- value;
- first partials;
- second partials.

Long-double test arithmetic may be used as reference computation but is not a
production dependency or qualification claim.

## 15. Boundary semantics

Every U/V edge is a rational cubic Bézier curve induced by the corresponding
four controls and weights.

Because AP Mesh does not yet expose a public rational cubic Bézier curve type,
focused evidence must use an independent rational cubic Bernstein oracle for
edge value and tangent semantics.

This decision does **not** silently introduce a rational cubic curve family.

A later curve-breadth decision may add that public family when required by
trimming or boundary discretization.

## 16. Cross-family rational fixture

Focused evidence should include at least one tensor-product construction whose
boundary or section is derived by homogeneous degree elevation from an already
integrated positive-weight rational quadratic Bézier curve.

This provides cross-family rational evidence without adding a new public curve
type.

Production degree-elevation algorithms are not authorized.

## 17. Weight-scale invariance

For any finite positive common factor `c`, replacing every `w_ij` by
`c w_ij` leaves geometry and partials unchanged.

Focused evidence must include:

- exact power-of-two scaling;
- strongly unbalanced finite positive weights;
- a case where naïve unscaled homogeneous products would be vulnerable to
  overflow/underflow.

No weight normalization becomes visible through the public stored weights.

## 18. U/V reversal

U reversal:

- reverses the outer U control order;
- reverses the outer U weight order.

V reversal:

- reverses each inner V control row;
- reverses each corresponding weight row.

Required value and derivative covariance matches the integrated polynomial
patch:

- reversed U value at `(1-u,v)` equals original;
- reversed U `S_u` changes sign;
- reversed U `S_v` preserves sign;
- corresponding second-partial sign rules;
- analogous V rules;
- double reversal recovers exact stored controls and weights.

## 19. Constant and degenerate geometry

A constant control net with arbitrary admitted positive weights is
representable.

Required behavior:

- exact common point where exact identity is preserved;
- exact zero first partials;
- exact zero second partials.

Rank-deficient patches remain valid representations.

No constructor rejects a patch because a future normal/metric calculation may
be singular.

## 20. Affine, embedding and deterministic evidence

Focused evidence must include:

- translation covariance;
- exact power-of-two coordinate scaling where representable;
- signed Cartesian axis permutation/reflection evidence within the currently
  qualified primitive/frame envelope;
- repeated identical success values;
- repeated identical typed failures.

No arbitrary-angle frame qualification is implied.

## 21. Extreme finite evidence

At least one fixture must combine:

- extreme finite coordinate magnitude;
- strongly unbalanced finite positive weights;
- parameters close to a domain edge.

Required behavior:

- succeed where the final value/partial is representable and scale-aware
  homogeneous arithmetic preserves it;
- otherwise return `SurfaceError::non_finite_result`.

No silent fallback, clamping or universal epsilon is permitted.

## 22. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Rational patch satisfies existing bounded 3D surface concept. |
| Construction | Non-finite/zero/negative weights rejected explicitly. |
| Exact domain | [0,1]² and existing deterministic U/V failure order. |
| Exact corners | Four control-point identities. |
| Independent oracle | Rational Bernstein value and all first/second partials match. |
| Equal-weight reduction | All-one and equal non-unit cases match polynomial bicubic patch. |
| Rational edge semantics | Four edges match independent rational cubic oracle. |
| Cross-family rational fixture | Test-only homogeneous degree-elevation relation to integrated rational quadratic curve. |
| Weight scale | Common positive weight scaling preserves value/partials. |
| U/V reversal | Stored data, involution and derivative covariance. |
| Constant patch | Exact point and zero partials. |
| Degenerate patch | Remains valid representation. |
| Translation/scale | Declared affine covariance. |
| Extreme finite | No avoidable overflow; explicit unrepresentable-result failure. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology, trimming, meshing, I/O, threading or external dependency. |
| Prerequisite preservation | Existing 26 ordinary tests remain PASS. |

If exactly one focused contract is added, ordinary FAST/INTEGRATION inventory
becomes **27 tests**.

## 23. Why rational Bézier is selected now

The polynomial bicubic patch already establishes:

- two-parameter bounded surface contract;
- 4x4 control-net convention;
- deterministic tensor evaluation;
- analytic first/second partials;
- U/V reversal;
- boundary consistency.

The next smallest independent mechanism is positive rational weighting.

Selecting rational Bézier now tests homogeneous 2D surface evaluation and
quotient partials without introducing U/V knots, multiplicities, dynamic
control-net dimensions or continuity lines.

This mirrors the project's accepted discipline of isolating one scientific
mechanism before combining it with spline structure.

## 24. Why NURBS surface is deferred one work unit

NURBS is high priority and is expected to be reconsidered immediately after
this work unit.

However a NURBS surface simultaneously introduces:

- runtime U/V control counts;
- independent U/V knot arrays;
- independent U/V multiplicities;
- local span search in two directions;
- continuity/failure lines;
- rational weights.

The curve layer already validates many of those components separately.
Validating the rational **surface** quotient seam first reduces the next NURBS
surface decision to primarily tensor-product knot/storage/continuity behavior.

## 25. Why Coons/transfinite is deferred

Coons/transfinite patches are boundary-driven constructions.

They require:

- compatible oriented boundary curves;
- corner agreement;
- chosen transfinite blend semantics.

Those are separate construction questions from a stored rational tensor patch.
They remain explicit future AP Mesh obligations.

## 26. Why analytic elementary surfaces are deferred

Plane/cylinder/cone/sphere/torus production types require family-specific
parameter domains, periodicity and 3D placement/orientation.

The current first surface abstraction intentionally does not yet admit those
family-specific semantics.

Analytic fixtures may still be used as independent references without claiming
production representation.

## 27. Why swept surfaces are deferred

Ruled, extrusion and revolution surfaces introduce generator/directrix/base
curve ownership and parameterization policy.

They remain distinct future representations and are not prerequisites for
rational tensor-product weighting.

## 28. Why trimming is deferred

Trimmed surfaces require explicit separation between:

- supporting surface;
- parameter-space trimming curves/loops;
- orientation;
- physical boundary correspondence;
- topology identity.

Those concerns should build on a broader supporting-surface family set rather
than being coupled to the first rational patch.

## 29. Repository mapping for future implementation

Authorized future mapping is limited to:

- public rational patch extension:
  `include/apmesh/geometry/surface.hpp`;
- new production source:
  `src/geometry/rational_surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_rational_bicubic_bezier.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

The common
`include/apmesh/geometry/parametric_surface.hpp`
contract should remain unchanged.

The polynomial `src/geometry/surface.cpp` scientific behavior remains frozen.

## 30. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST PASS;
- GCC 13 Debug / INTEGRATION PASS;
- Clang 18/libc++ Debug / INTEGRATION PASS;
- existing 26 ordinary semantic tests remain PASS;
- one new rational-bicubic focused contract PASS.

Passing yields only:

**SURFACE REPRESENTATION STAGE OPEN /
RATIONAL BICUBIC BÉZIER PATCH IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Representation or authorize NURBS, Coons,
analytic, swept or trimmed surfaces.

## 31. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricSurface3`;
- U/V knots or multiplicities;
- runtime-variable control-net dimensions;
- arbitrary degree;
- zero/negative weights;
- periodicity;
- Coons/transfinite construction;
- analytic elementary surface production types;
- swept surfaces;
- trimming/topology;
- normal/metric/curvature semantics;
- surface discretization/meshing;
- a universal tolerance;
- runtime virtual hierarchy;
- third-party runtime dependency.

## 32. Planned sequence after this work unit

Planning only, not authorization.

After rational bicubic implementation/closure, a fresh Surface Representation
breadth decision should compare:

1. cubic B-spline/NURBS surface, with U/V knot/multiplicity scope frozen;
2. Coons/transfinite patch;
3. analytic elementary surfaces;
4. ruled/extrusion/revolution;
5. rectangular/general trimmed-surface semantics.

NURBS surface is expected to be a strong candidate but is not pre-authorized.

## 33. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item becomes:

**Positive-Weight Rational Tensor-Product Bicubic Bézier Patch in 3D.**

No implementation begins on this decision branch.

Surface Differential Geometry, Boundary Curve Discretization and all meshing
stages remain blocked.

Remaining curve breadth remains retained.


## 34. Decision integration checkpoint

PR #143 integrated this bounded Surface Representation breadth decision.

Final decision head:

`af09c54f6c3f6c963fac140d02ae56d0bf40a886`.

Final decision-head validation:

- FAST `35780844901`: PASS;
- INTEGRATION `35780844932`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #143 merged as:

`281626d6ec02763be57b15fff15a82b0daa9129d`.

Post-merge validation:

- FAST `35780994515`: PASS;
- INTEGRATION `35780994410`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the rational bicubic Bézier surface implementation bounded by
Sections 5–31.

No NURBS, Coons, analytic, swept, trimmed or downstream surface capability is
authorized by this checkpoint.


## 35. Decision closure checkpoint

Decision closure PR #144 used head:

`423bf857661dc94e17f46a73a25369e08ec92811`.

Closure PR validation:

- FAST `35781257218`: PASS;
- INTEGRATION `35781257255`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #144 merged as:

`0e2e9620052f2bee3237eb8428065b68facff0fd`.

Closure post-merge validation:

- FAST `35781441396`: PASS;
- INTEGRATION `35781441405`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / RATIONAL BICUBIC BÉZIER IMPLEMENTATION AUTHORIZED /
NOT QUALIFIED.**

The sole active production work item is the rational bicubic patch bounded by
Sections 5–31.

No NURBS, Coons, analytic, swept, trimmed or downstream surface capability is
authorized.


## 36. Active implementation mapping

The sole authorized implementation is active on:

`surface/rational-bicubic-bezier`.

Candidate mapping:

- public value/API extension:
  `include/apmesh/geometry/surface.hpp`;
- production:
  `src/geometry/rational_surface.cpp`;
- focused semantic/reference contract:
  `tests/surface_rational_bicubic_bezier.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

Frozen prerequisites:

- `include/apmesh/geometry/parametric_surface.hpp`: unchanged;
- `src/geometry/surface.cpp`: unchanged;
- `BicubicBezierPatch3`: prerequisite polynomial oracle/production family.

Candidate production semantics:

- exact stored 4x4 controls and input weights;
- explicit finite/positive weight construction validation;
- common internal weight normalization;
- deterministic V-then-U homogeneous de Casteljau;
- homogeneous derivative nets through degree reduction;
- analytic rational Su/Sv/Suu/Suv/Svv dehomogenization;
- exact corners;
- exact constant-geometry shortcut;
- U/V reversal of controls and weights.

Focused contract includes:

- independent rational Bernstein value/partial oracle;
- all-equal non-unit weight parity with `BicubicBezierPatch3`;
- common weight-scale invariance;
- U/V reversal and derivative covariance;
- homogeneous degree elevation from integrated
  `RationalQuadraticBezier3` as cross-family evidence;
- translation and coordinate-scale covariance;
- extreme finite positive weights;
- explicit unrepresentable-result failure;
- deterministic repeated success/failure evidence.

Expected ordinary semantic inventory: **27 tests**.

Validation history:

Initial PR head:

`83d1aedd68cbd4457e36021c704604b3a616fd8a`.

Initial validation:

- FAST `35782696019`: FAIL during focused-test compilation;
- INTEGRATION `35782696016`: FAIL during the same focused-test compilation
  in GCC 13 Debug and Clang 18/libc++ Debug;
- diagnosis: two test-only `ControlNet` instances used empty aggregate
  initialization even though `Point3` is deliberately non-default-
  constructible;
- `src/geometry/rational_surface.cpp` compiled successfully in all initial
  jobs;
- the correction initialized the two test nets explicitly and changed no
  production, API or scientific semantics.

Corrected candidate head:

`3ac7b36db5a2a94f77a81fd441d9d871233653a3`.

Corrected validation:

- FAST `35782907623`: PASS, 27/27 ordinary semantic tests;
- INTEGRATION `35782907574`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 27/27 tests per cell;
- `apmesh_core.surface_rational_bicubic_bezier`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**

No U/V knot, NURBS, Coons, analytic, swept, trimmed, differential-geometry or
meshing capability is implied.


## 37. Implementation integration checkpoint

Implementation PR #145 used final head:

`9677c98882ce32569e537a9b5d91f23cffabec69`.

Final PR validation:

- FAST `35783120161`: PASS, 27/27 ordinary semantic tests;
- INTEGRATION `35783120208`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 27/27 tests per cell;
- `apmesh_core.surface_rational_bicubic_bezier`: PASS in all three jobs.

PR #145 merged as:

`8ac1abd913bf15ff1dc4d60595f809491902c055`.

Post-merge validation:

- FAST `35783312495`: PASS, 27/27;
- INTEGRATION `35783312402`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 27/27 tests per cell.

The earlier failed heads remain preserved as mechanical focused-test evidence;
no production/API/scientific semantics were changed by that correction.

Integrated result:

**SURFACE REPRESENTATION STAGE OPEN /
RATIONAL BICUBIC BÉZIER PATCH IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

The implementation is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is one fresh literature-backed Surface Representation breadth decision.
NURBS, Coons, analytic, swept and trimmed families remain unselected.


## 38. Implementation closure checkpoint

Implementation closure PR #146 used head:

`2e1e514216db89d5a1ef5507a2f0f8a2473fa76c`.

Closure PR validation:

- FAST `35783557983`: PASS;
- INTEGRATION `35783557757`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #146 merged as:

`c7f7b32b180082421cadc74c39d2919f713ec775`.

Closure post-merge validation:

- FAST `35783715705`: PASS;
- INTEGRATION `35783715671`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Terminal result:

**SURFACE REPRESENTATION STAGE OPEN /
RATIONAL BICUBIC BÉZIER PATCH IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED / NOT QUALIFIED.**

No production work item remains active.

The next admissible work is one fresh literature-backed Surface Representation
breadth decision comparing cubic B-spline/NURBS, Coons/transfinite, analytic
elementary, swept and trimmed surface families. No candidate is
pre-authorized.
