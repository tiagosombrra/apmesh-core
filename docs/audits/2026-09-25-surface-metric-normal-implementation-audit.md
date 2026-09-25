# Surface Metric / Oriented Normal — Implementation Closure Audit

Date: 2026-09-25  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures  
Work unit: Pointwise Surface Regularity, First Fundamental Form, Area Density,
and Oriented Unit Normal in 3D

## 1. Scope

This audit closes the first bounded Surface Differential Geometry production
work unit.

Authorized scientific outputs:

- first fundamental form coefficients E/F/G;
- scale-aware area density;
- oriented unit normal;
- exact singular-parameterization failure when the tangent-plane cross product
  is exactly zero in the declared floating representation;
- typed propagation of surface parameter/domain/continuity failures;
- explicit non-representable-result failure.

Explicitly not authorized:

- second fundamental form;
- Gaussian, mean, principal curvature or principal directions;
- new surface representation families;
- general trimming/p-curves/topological faces;
- boundary discretization, sizing or meshing;
- Quad-Dominant or parallel execution work.

## 2. Decision lineage

Surface Differential Geometry entry decision:

- decision PR #194 merged as
  `2a4b1df3732eaaa1a768c1d8d0b41bdd3310ffac`;
- decision post-merge FAST `36114962084`: PASS;
- decision post-merge INTEGRATION `36114962093`: PASS.

Decision closure:

- PR #195 head:
  `00212906a86df1e019919431ab14460becc4cc82`;
- PR FAST `36115295284`: PASS;
- PR INTEGRATION `36115295327`: PASS;
- merge:
  `10c2c7ab2231721a28858b9395aa6f0eb9b2d603`;
- closure post-merge FAST `36115414541`: PASS;
- closure post-merge INTEGRATION `36115414557`: PASS.

## 3. Implementation mapping

Integrated implementation:

- public API:
  `include/apmesh/geometry/surface_differential.hpp`;
- production:
  `src/geometry/surface_differential.cpp`;
- focused semantic contract:
  `tests/surface_metric_normal.cpp`;
- build/test registration:
  `CMakeLists.txt`.

The implementation is generic over existing
`BoundedParametricSurface3` first derivatives and does not modify concrete
surface representation semantics.

## 4. Numeric strategy

The production kernel:

- uses existing scale-aware vector norm operations;
- normalizes U/V tangent vectors before cross product;
- detects singularity only when the normalized tangent cross product has exact
  zero norm;
- reconstructs E/F/G and area density through exponent-aware products;
- returns `non_representable_result` when a required final metric quantity is
  not representable;
- introduces no universal geometric epsilon;
- accepts near-degenerate but nonzero parameterizations.

## 5. Initial validation defect

Initial candidate head:

`e57e80fcb33a63518475dd9ac3a8f34873e4c0ec`.

Runs:

- FAST `36116721390`: mechanically PASS;
- INTEGRATION `36116721741`: mechanically PASS in GCC 13 Debug and
  Clang 18/libc++ Debug.

Material diagnosis:

- each job executed only **36/36** ordinary tests;
- `apmesh_core.surface_metric_normal` was compiled and linked but absent from
  CTest execution;
- root cause: the new test was missing from the existing surface
  `set_tests_properties` block that assigns `fast;integration` labels.

Classification:

**VALIDATION INCOMPLETE / MECHANICAL TEST-REGISTRATION DEFECT /
NO PRODUCTION-SEMANTIC FAILURE OBSERVED.**

The initial green runs are retained as historical evidence and are not treated
as scientific validation of the new work unit.

## 6. Correction validation

The correction added only the missing ordinary-profile labels for
`apmesh_core.surface_metric_normal`.

Corrected candidate head:

`696fd384907fbe2200d25944385bedc6ecf912da`.

Runs:

- FAST `36118467156`: PASS, **37/37**;
- INTEGRATION `36118467165`: PASS, **37/37** in GCC 13 Debug and
  Clang 18/libc++ Debug;
- focused contract executed as test 26/37 and PASS in all three jobs.

No production scientific semantics changed in the correction.

## 7. Final PR validation

Final PR head:

`38da1f9eeefdff15173f421057580db72884e64c`.

Runs:

- FAST `36118684757`: PASS, **37/37**;
- INTEGRATION `36118684741`: PASS, **37/37** in GCC 13 Debug and
  Clang 18/libc++ Debug;
- focused contract: PASS in all three jobs;
- every prerequisite ordinary semantic contract: PASS.

PR #196 merged as:

`e476eaaaa56f59a5d66574f180083ecd621f8b95`.

## 8. Protected-main post-merge validation

Post-merge runs on
`e476eaaaa56f59a5d66574f180083ecd621f8b95`:

- FAST `36118945024`: PASS, **37/37**;
- INTEGRATION `36118944987`: PASS, **37/37** in GCC 13 Debug and
  Clang 18/libc++ Debug;
- `apmesh_core.surface_metric_normal`: PASS;
- all prerequisite ordinary contracts: PASS.

## 9. Focused evidence coverage

The focused contract verifies:

- analytic plane E/F/G, area density and normal;
- analytic cylinder E/F/G, area density and radial normal;
- non-orthogonal synthetic metric oracle;
- exact singular parameterization failure;
- near-degenerate but nonzero success;
- U/V/both reversal covariance;
- translation invariance;
- signed-frame covariance;
- power-of-two scale covariance;
- explicit non-representable metric failure;
- deterministic repeated success/failure;
- regular differential conformance across integrated polynomial/rational
  Bézier, NURBS, Coons, rectangular trim, extrusion, revolution, plane and
  cylinder surfaces.

## 10. Closure result

Current closure result before closure-PR integration:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSURE PENDING / NOT QUALIFIED.**

The Surface Differential Geometry stage remains **NOT QUALIFIED**.

The most recently qualified scientific stage remains the frozen Curve
Representation CGR0-CGR7 baseline in the admitted GitHub-hosted Ubuntu 24.04
x86_64 cloud envelope.

## 11. Next admissible action

Only after this closure is integrated and post-merge validated may one fresh
literature-backed Surface Differential Geometry decision be opened.

Candidates to compare:

1. second fundamental form plus Gaussian/mean curvature;
2. principal curvatures/directions;
3. explicit conditioning / near-singular diagnostics;
4. return to Surface Representation for sphere/cone/torus;
5. return to general trimming/p-curves/topological faces.

No candidate is pre-authorized.


## 12. Documentation closure

Closure PR #197 used head:

`8bdab0d43da87800add044a1ff1a9a3a4cd47e99`.

Closure PR validation:

- FAST `36119500567`: PASS, 37/37;
- INTEGRATION `36119500599`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 37/37 per cell.

PR #197 merged as:

`5fc561a2e0e927369c5dc5c4644ef18699602914`.

Closure post-merge validation:

- FAST `36119643984`: PASS, 37/37;
- INTEGRATION `36119643997`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 37/37 per cell.

## 13. Terminal audit result

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
NOT QUALIFIED.**

No active work item remains.

The ordinary protected-main inventory is **37 tests**.

The initial 36/36 green-but-incomplete runs remain preserved as a diagnosed
mechanical validation-registration defect and are not reinterpreted as complete
scientific evidence.

The next admissible action is one fresh literature-backed Surface Differential
Geometry decision. No second-order or representation candidate is
pre-authorized.
