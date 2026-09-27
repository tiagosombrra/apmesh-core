# Surface Metric Conditioning Diagnostics — Scientific Decision

Status: DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-27  
Parent stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Question

After terminal closure of the bounded analytic spherical-surface work unit and
the portable-spline maintenance checkpoint, which single bounded scientific
work unit should be admitted next?

Required comparison:

1. bounded analytic cone;
2. bounded analytic torus;
3. principal directions / line-field semantics;
4. explicit conditioning diagnostics;
5. general trimming / p-curves / topological faces;
6. Surface Representation qualification readiness;
7. Surface Differential Geometry qualification readiness;
8. Boundary Curve Discretization readiness.

The next work unit must close a real prerequisite gap while avoiding a broader
topology, periodicity, meshing or qualification seam.

## 2. Fresh repository authority

Decision-entry protected `main`:

`fa04f8bdd7d1359bafd62a02999e0921a835c0ef`.

Decision-entry validation:

- FAST #596: PASS;
- INTEGRATION #587: PASS in GCC 13 Debug and Clang 18/libc++ Debug;
- ordinary semantic inventory: **41 tests**;
- open PRs: none;
- open issues: none;
- active production/maintenance work item: none.

The active decision branch is:

`surface/metric-conditioning-decision`.

The execution environment for this decision session has no mounted local
checkout. No local scientific regression is claimed or repeated. The exact
protected-main FAST/INTEGRATION runs above are the entry regression authority;
the decision branch itself is documentation-only and must receive fresh
required PR checks before any integration.

## 3. Relevant integrated prerequisites

Surface Representation already provides focused production coverage for:

- tensor-product bicubic polynomial Bézier;
- positive-weight rational bicubic Bézier;
- bicubic NURBS with simple/double-knot continuity;
- Coons/transfinite patch;
- rectangular trimming;
- linear extrusion;
- revolution;
- arbitrary right-handed `AxisPlacement3`;
- bounded analytic plane;
- bounded analytic circular cylinder sector;
- bounded analytic spherical sector.

Surface Differential Geometry already provides:

- exact singular-parameterization classification;
- first fundamental form `E,F,G`;
- area density;
- oriented unit normal;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state.

Still absent in production:

- explicit pointwise parameterization-conditioning diagnostic;
- principal directions / line fields;
- analytic cone;
- analytic torus;
- general p-curve / topological-face trimming;
- formal Surface Representation qualification;
- formal Surface Differential Geometry qualification;
- Boundary Curve Discretization.

## 4. External scientific and CAD evidence

External sources are used only as scientific/design evidence. They introduce no
runtime dependency and are not numerical or behavioral oracles.

### 4.1 Conditioning is already encoded by the first fundamental form

A surface parameterization has local Jacobian

`J = [S_u S_v]`.

The first fundamental form is

`I = J^T J = [[E,F],[F,G]]`.

The singular values of `J` are the square roots of the eigenvalues of `I`;
they quantify local stretching of the parameterization. The Stanford geometry
processing course notes make this relation explicit and use those singular
values for distortion analysis:

https://graphics.stanford.edu/courses/cs348a-21-winter/Papers/2007_Meshes_Pauly_course_c23.pdf

The Caltech parameterization thesis likewise treats distortion as change in the
first fundamental form and reports Jacobian condition number as a standard
parameterization-distortion measure:

https://multires.caltech.edu/pubs/NathanPhDthesis.pdf

Pilgerstorfer and Jüttler show that domain parameterization strongly influences
numerical conditioning in isogeometric analysis and use condition-number bounds
as parameterization-quality measures:

https://doi.org/10.1016/j.cma.2013.09.019

Decision impact:

- AP Mesh already computes the exact first fundamental form for every admitted
  regular surface query;
- no new surface family, topology object or second-order differential quantity
  is required to expose a scale-free conditioning diagnostic;
- conditioning is already named as a required Surface Differential Geometry
  stage concern in the repository roadmap;
- the missing seam is therefore a bounded diagnostic over already-integrated
  metric data.

### 4.2 Principal directions are a larger line-field problem

Patrikalakis, Maekawa and Cho describe principal directions as the tangent
directions of extremal normal curvature and show that integrating lines of
curvature needs equation switching, orientation control and special handling
around indeterminate cases:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node186.html

At umbilics the principal-direction choice is non-unique, while the repository
already models exact represented-data umbilic state.

Decision impact:

- pointwise eigenvector mechanics alone would not close the line-field problem;
- sign/orientation continuity and undefined/non-unique semantics would need a
  separate scientific contract;
- principal directions are therefore broader than the missing scalar
  conditioning diagnostic.

### 4.3 Cone and torus remain representation-breadth work

Open CASCADE's conical-surface construction uses placement, reference radius
and semi-angle, has angular U parameterization and an unbounded natural V
coordinate, and exposes an apex-specific family seam:

https://occt3d.com/dev/doc/refman/html/class_g_c___make_conical_surface.html

Open CASCADE's toroidal-surface model uses major/minor radii and two angular
parameters; both natural U and V directions are periodic:

https://occt3d.com/dev/doc/refman/html/class_geom___toroidal_surface.html

Decision impact:

- cone still needs radius/semi-angle/apex policy;
- torus still needs two-radius and double-periodicity policy;
- each is useful Surface Representation breadth, but neither closes the
  conditioning prerequisite already named by Surface Differential Geometry.

### 4.4 General trimming remains a cross-layer p-curve/topology seam

Open CASCADE explicitly models a point/curve on a surface through a 2D
parameter-space curve bound to a supporting surface, and its CAD data model
contains dedicated curve-on-surface representations:

https://occt3d.com/dev/doc/refman/html/class_b_rep___point_on_curve_on_surface.html

Decision impact:

- general trimming requires parameter-space curves, physical realization,
  loop orientation and explicit face/topology identity;
- this remains important for CAD face coverage and later shared-boundary
  certification;
- it is materially larger than a pointwise metric diagnostic and should not be
  entered implicitly through the differential layer.

## 5. Candidate comparison

| Candidate | Missing semantics now | Prerequisite reuse | Direct prerequisite value | Decision |
| --- | --- | --- | --- | --- |
| Pointwise metric conditioning diagnostics | scale-free local stretch/condition measure over existing metric | **Very high** | **Directly closes an explicit Surface Differential Geometry gap** | **SELECTED** |
| Principal directions / line fields | eigenvectors, sign/orientation continuity, umbilic non-uniqueness | high | valuable later, but larger line-field contract | DEFER |
| Bounded analytic cone | semi-angle, reference radius, apex/singularity policy | high | Surface Representation breadth only | DEFER |
| Bounded analytic torus | major/minor radii, two periodic angular directions | high | Surface Representation breadth only | DEFER |
| General trimming / p-curves / faces | curve-on-surface representation, loops, orientation, topology binding | moderate | very high downstream, but cross-layer | DEFER |
| Surface Representation qualification readiness | cumulative stage protocol over still-open breadth decisions | incomplete | premature while cone/torus/general trimming policy remains open | DEFER |
| Surface Differential Geometry qualification readiness | cumulative protocol over current differential stack | **blocked by missing conditioning diagnostic** | high after conditioning closure | BLOCKED/DEFER |
| Boundary Curve Discretization readiness | shared physical trace, face/p-curve ownership and orientation | incomplete | downstream-critical | BLOCKED/DEFER |

## 6. Decision

Select exactly one future bounded implementation work unit:

**Pointwise Surface Metric Conditioning Diagnostics in 3D.**

This work unit belongs to **Surface Differential Geometry**.

It does not qualify Surface Differential Geometry or Surface Representation.

No production implementation begins on this decision branch.

## 7. Scientific definition

At an admitted regular surface parameter `(u,v)`, let

`J = [S_u S_v]`

and let the already-computed first fundamental form be

`I = J^T J = [[E,F],[F,G]]`.

Because the point is regular, `I` is symmetric positive definite. Let

`lambda_min <= lambda_max`

be its positive eigenvalues and

`sigma_min = sqrt(lambda_min)`,
`sigma_max = sqrt(lambda_max)`

be the singular values of the surface Jacobian.

The selected diagnostic is the 2-norm parameterization condition number

`kappa = sigma_max / sigma_min = sqrt(lambda_max / lambda_min)`.

Required semantics:

- `kappa >= 1`;
- isotropic local stretch gives `kappa == 1` where exactly representable;
- uniform spatial scaling leaves `kappa` unchanged;
- translation and rigid-frame changes leave `kappa` unchanged;
- U/V reversal leaves `kappa` unchanged;
- swapping the two parameter axes leaves `kappa` unchanged;
- non-uniform reparameterization may change `kappa` and that change is
  intentional;
- exact singular parameterizations remain the existing
  `SurfaceDifferentialError::singular_parameterization`;
- regular but strongly anisotropic points are not converted to failures by an
  arbitrary tolerance;
- no global "ill-conditioned" threshold is introduced;
- if the final condition number is not representable in the public scalar,
  existing `non_representable_result` semantics apply.

The implementation must not use a naïve eigenvalue formula if an algebraically
equivalent scale-aware computation avoids intermediate overflow/underflow.

## 8. Bounded public/API scope

A future implementation may add one small value type and one query in the
existing Surface Differential Geometry API, for example conceptually:

- a pointwise metric-conditioning result containing the dimensionless
  condition number;
- an overload accepting already-computed first-order metric data;
- a bounded-surface convenience overload.

Exact naming is an implementation detail, but semantics in Section 7 are
authoritative.

The work unit must not add:

- principal directions or curvature-line integration;
- a user-tunable conditioning tolerance;
- a new `ill_conditioned` failure class;
- cone, torus or any new surface representation;
- p-curves, topology/face ownership or general trimming;
- boundary discretization, sizing or meshing;
- a formal qualification campaign;
- arbitrary precision or third-party dependencies.

## 9. Focused evidence plan

The future focused contract must include at least:

1. identity/isotropic metric with exact `kappa=1`;
2. diagonal anisotropic metric with independent analytic condition number;
3. non-orthogonal metric with independent eigenvalue reference;
4. exact singular metric preserving `singular_parameterization`;
5. regular near-singular metric with large but representable condition number;
6. extreme finite metric coefficients exercising scale-aware arithmetic;
7. uniform spatial scaling invariance;
8. translation and signed-frame invariance;
9. U reversal, V reversal and both-reversal invariance;
10. parameter-axis swap invariance;
11. a non-uniform parameter-scaling fixture that changes `kappa` as expected;
12. plane and cylinder analytic fixtures;
13. sphere regular-point evidence plus exact pole singularity preservation;
14. conformance across every admitted existing bounded surface family;
15. deterministic repeated successes and failures;
16. full ordinary prerequisite regression.

The ordinary profile remains development/integration evidence only.

## 10. Qualification boundary

Passing focused FAST/INTEGRATION tests for this work unit may establish only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

It does not:

- qualify Surface Differential Geometry;
- qualify Surface Representation;
- qualify native Windows;
- authorize a formal campaign;
- establish acceptable conditioning thresholds for downstream meshing;
- authorize principal directions or Boundary Curve Discretization.

A later qualification decision must separately define cumulative fixtures,
cross-cell evidence, retained artifacts, acceptance criteria and exact stage
claims.

## 11. Stop conditions

Stop and require a new scientific decision if implementation needs any of:

- a numerical threshold separating "well" and "ill" conditioned points;
- a new error category;
- principal eigenvectors/directions or line-field continuity;
- third derivatives;
- a new surface representation;
- periodic-seam topology;
- p-curves or face/topology binding;
- boundary discretization, sizing or meshing;
- a formal qualification/preparation/execution campaign;
- arbitrary precision or a third-party numerical package;
- changed singular-parameterization semantics.

## 12. Work-class plan

Planned ordinary decision/implementation distribution:

- Implementation: 60%;
- Tests/validation: 30%;
- Evidence/experiments: 0%;
- Documentation/governance: 10%.

These percentages are process accounting, not scientific completion scores.

The current decision branch itself is documentation/governance only.

## 13. Effect if this decision is integrated and separately closed

After this decision receives required PR checks, merges, receives protected-main
validation, and a separate documentation/decision closure also merges and is
post-merge validated, the sole next production work item becomes:

**Pointwise Surface Metric Conditioning Diagnostics in 3D.**

Until that separate decision closure:

- implementation is not authorized;
- no other scientific candidate is active;
- Surface Representation remains IN INVESTIGATION / NOT QUALIFIED;
- Surface Differential Geometry remains IN INVESTIGATION / NOT QUALIFIED;
- Boundary Curve Discretization remains NOT STARTED.

After the conditioning implementation itself is later integrated and closed, a
fresh decision must recompare principal directions, cone/torus breadth, general
trimming/p-curves/topological faces, Surface Representation qualification
readiness, Surface Differential Geometry qualification readiness and Boundary
Curve Discretization readiness. No later candidate is pre-authorized.


## 14. Decision integration checkpoint

Decision PR #221 used final head:

`30359e7f392f40db9074b68762f6d659a0ef3661`.

Required PR validation on that exact head:

- FAST #597: PASS;
- INTEGRATION #588: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

PR #221 squash-merged as:

`a9cdfb57dd263d6ed3ca1f85ad1285811bb20c41`.

Protected-main validation on that exact decision revision:

- FAST #598: PASS;
- INTEGRATION #589: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

Decision integration result:

**POINTWISE SURFACE METRIC CONDITIONING DIAGNOSTICS SELECTED /
DECISION INTEGRATED / CLOSURE PENDING / IMPLEMENTATION NOT AUTHORIZED /
NOT QUALIFIED.**

This documentation-only closure checkpoint must itself receive required PR
checks, merge and pass protected-main validation before the selected
implementation becomes authorized.

No threshold, new error class, principal-direction semantics, new surface
family, topology, boundary discretization, formal campaign or qualification
authority is introduced by this checkpoint.


## 15. Decision closure checkpoint

Decision-closure PR #222 used final head:

`4e0f33a0a8cb8795a8a088840a2e6ca008abd93f`.

Required closure-PR validation:

- FAST #599: PASS;
- INTEGRATION #590: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

PR #222 squash-merged as:

`b4df654794bea9cc88e33d0c048fdc298179a4a8`.

Protected-main validation on that exact closure revision:

- FAST #600: PASS;
- INTEGRATION #591: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

Decision checkpoint result:

**DECISION CLOSED / POINTWISE SURFACE METRIC CONDITIONING DIAGNOSTICS
IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole authorized production work item is now the bounded implementation
defined by Sections 7--11. No principal directions, conditioning threshold,
cone, torus, general trimming/topology, Boundary Curve Discretization,
qualification preparation/execution or other scientific candidate is authorized.


## 16. Implementation activation checkpoint

Terminal decision-state synchronization PR #223 used final head:

`94b5f00c5636497206b1fae7d86d04d4cae6e7ec`.

PR validation:

- FAST #601: PASS;
- INTEGRATION #592: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

PR #223 squash-merged as:

`49efeaca5ac4fee5ec8fcda9859caa646fbce1a4`.

Protected-main validation:

- FAST #602: PASS;
- INTEGRATION #593: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

The bounded production work item is active on:

`surface/metric-conditioning`.

Authorized implementation mapping:

- public differential API:
  `include/apmesh/geometry/surface_differential.hpp`;
- production:
  `src/geometry/surface_differential.cpp`;
- focused contract:
  `tests/surface_metric_conditioning.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / WORKLOG / ROADMAP / this decision.

No code has yet been integrated. The implementation remains bounded by
Sections 7--11 and does not authorize conditioning thresholds, principal
directions, new surface representations, topology, discretization or a formal
qualification campaign.


## 17. Active implementation candidate

The first bounded implementation candidate is:

`ae1d4e0f59ae1e5eb017a58030c69f5bea7e8bc6`.

Candidate scope:

- `SurfaceMetricConditioning` with one dimensionless
  `condition_number`;
- `surface_metric_conditioning(const SurfaceMetricNormal3&)`;
- bounded-surface convenience overload using existing
  `surface_metric_normal` and exact error propagation;
- scale-aware metric eigenvalue evaluation with
  `lambda_max / area_density = sigma_max / sigma_min`;
- exact singular semantics inherited from the existing metric/normal query;
- no user threshold and no new error class;
- focused contract in
  `tests/surface_metric_conditioning.cpp`;
- ordinary-profile registration in `CMakeLists.txt`.

Focused evidence in the candidate covers:

- exact isotropic `kappa=1`;
- diagonal and non-orthogonal analytic references;
- parameter-axis swap;
- U/V/both reversal;
- uniform spatial-scale invariance;
- deliberate non-uniform reparameterization sensitivity;
- exact singularity;
- representable near-singular conditioning;
- extreme finite scale separation;
- explicit unrepresentable result;
- translation and signed-frame invariance;
- cylinder and sphere analytic fixtures;
- exact sphere-pole singularity;
- conformance across every currently admitted bounded surface family;
- deterministic repeated success/failure behavior.

Expected ordinary semantic inventory: **42 tests**.

Validation status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-SYNC REVALIDATION PENDING / NOT QUALIFIED.**

Initial documented PR head:

`3302a6209b54d63ddde6ecbd8ba05ead9aef1284`.

Validation on that exact head:

- FAST #603: PASS;
- INTEGRATION #594: PASS in GCC 13 Debug and Clang 18/libc++ Debug;
- ordinary static registration inventory: **42 tests**;
- `apmesh_core.surface_metric_conditioning` is registered by `add_test`,
  carries both `fast` and `integration` labels, and belongs to both green
  CTest preset selections;
- no prior ordinary test registration was removed.

The current execution environment cannot resolve the public GitHub host for a
throwaway local clone, so no local-build evidence is claimed. Required remote
PR validation is authoritative.


## 18. Implementation integration checkpoint

Technical implementation candidate:

`ae1d4e0f59ae1e5eb017a58030c69f5bea7e8bc6`.

Final immutable implementation head:

`c9176da223e6568861803743d992b91371aa50fb`.

Initial candidate validation:

- FAST #603: PASS;
- INTEGRATION #594: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

Final validation history:

- PR #224: final-head INTEGRATION #599 PASS; closed unmerged after a
  mechanical GitHub Actions concurrency incident blocked final FAST scheduling;
- replacement PR #225 used the exact same immutable head;
- PR #225 FAST #610: PASS;
- PR #225 INTEGRATION #601: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

PR #225 squash-merged as:

`7b3c833273dba042b7f6c055dd736510573664a6`.

Protected-main validation on that exact revision:

- FAST #611: PASS;
- INTEGRATION #602: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

Integrated ordinary semantic registration inventory: **42 tests**.

Implementation audit:

`docs/audits/2026-09-27-surface-metric-conditioning-implementation-audit.md`.

Integrated result:

**POINTWISE SURFACE METRIC CONDITIONING DIAGNOSTICS IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / CLOSURE PENDING /
NOT QUALIFIED.**

The sole active work item is now documentation-only implementation closure.
No next scientific capability is selected or authorized by this checkpoint.
