# Surface Second-Order Curvature Implementation Audit

Date: 2026-09-25  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures  
Work unit: Pointwise Second Fundamental Form + Gaussian/Mean Curvature

## Integrated scope

Implemented only the bounded decision in
`docs/decisions/SURFACE_SECOND_ORDER_CURVATURE_DECISION.md`:

- oriented second fundamental coefficients L/M/N;
- Gaussian curvature K;
- oriented mean curvature H;
- direct first+second derivative API;
- generic bounded-surface API;
- exact singularity / insufficient-continuity propagation;
- explicit non-representable-result failure;
- no regularity epsilon;
- no principal curvature/direction API;
- no conditioning policy;
- no new surface family.

Production mapping:

- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`;
- `tests/surface_second_order_curvature.cpp`;
- `CMakeLists.txt`.

The common `BoundedParametricSurface3` contract and all concrete surface
representation sources were unchanged.

## Validation history retained

### Initial candidate incident

Initial candidate head:

`c3cd327ebbd4add551c7d331f0149513d15324c7`.

- FAST `36125805859`: PASS;
- INTEGRATION `36125805817`:
  - GCC 13 Debug: PASS, 38/38;
  - Clang 18/libc++ Debug: FAIL, 37/38;
  - sole failing contract:
    `apmesh_core.surface_second_order_curvature`;
  - diagnostic:
    `unrepresentable curvature did not fail explicitly`.

Classification:

**focused cross-compiler test-fixture portability defect**.

No production/API semantic change was required.

Correction:

`89326bb1ad70449f4ed75c7b00d45699e998ce09`.

Only the extreme test fixture changed. The corrected fixture uses orthonormal
first derivatives and finite very-large second derivatives so the requested
Gaussian curvature necessarily exceeds representable `double` range without
depending on near-singular floating behavior.

Detailed incident audit:

`docs/audits/2026-09-25-surface-second-order-curvature-candidate-validation.md`.

### Corrected candidate

Corrected/documented head:

`32e05ddec7b81f099f0384c95f44f0df27ed24e2`.

- FAST `36126869683`: PASS, 38/38;
- INTEGRATION `36126869675`: PASS, 38/38 in GCC 13 Debug and
  Clang 18/libc++ Debug.

### Final PR head

Final PR head:

`ffc81253bad6096741352ced28999e7626735bfc`.

- FAST `36127069279`: PASS, 38/38;
- INTEGRATION `36127069324`: PASS, 38/38 in GCC 13 Debug and
  Clang 18/libc++ Debug.

PR #201 merged as:

`ed91d70d11446925148cc0ee5f0efab879022270`.

### Protected-main validation

- FAST `36127252797`: PASS, 38/38;
- INTEGRATION `36127252837`: PASS, 38/38 in GCC 13 Debug and
  Clang 18/libc++ Debug;
- focused `apmesh_core.surface_second_order_curvature`: PASS in all three
  jobs.

## Scientific evidence

Focused evidence includes:

- general non-orthogonal E/F/G + II reference;
- positive and negative Gaussian-curvature synthetic references;
- exact plane K=H=0;
- bounded outward-oriented cylinder K=0 and H=-1/(2r);
- U/V/double-reversal covariance;
- power-of-two scale covariance;
- exact singular rejection and near-singular nonzero-area acceptance;
- explicit non-representable curvature failure;
- actual multiplicity-two bicubic NURBS insufficient-continuity propagation;
- regular cross-family conformance across integrated polynomial/rational/NURBS,
  Coons, rectangular trim, extrusion and revolution families;
- deterministic success/failure evidence.

## Result

**SECOND FUNDAMENTAL FORM + GAUSSIAN/MEAN CURVATURE IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / READY FOR CLOSURE /
NOT QUALIFIED.**

Surface Differential Geometry is not qualified by this work unit.

## Retained limitations

Still unauthorized:

- principal curvatures/directions;
- umbilic classification;
- conditioning diagnostics or near-singular thresholds;
- sphere/cone/torus representation breadth;
- general trimming/p-curves/topological faces;
- boundary discretization;
- sizing/meshing;
- Quad-Dominant;
- parallel execution.

## Next admissible action after closure

After closure integration and post-merge validation, open one fresh
literature-backed scientific decision comparing:

1. principal curvatures/directions;
2. explicit conditioning diagnostics;
3. sphere/cone/torus Surface Representation breadth;
4. general trimming/p-curves/topological faces;
5. whether existing pointwise differential geometry is sufficient to resume
   Boundary Curve Discretization preparation.

No option is pre-authorized.


## Closure evidence

Implementation closure PR #202 head:

`36d2d3da72e4c0da9175f2f50331df181ccec3eb`.

Closure PR validation:

- FAST `36127669983`: PASS;
- INTEGRATION `36127670012`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #202 merged as:

`2ddd991bb9246bb8e6330f0cdf9a87afde93283f`.

Closure post-merge validation:

- FAST `36127811804`: PASS, 38/38;
- INTEGRATION `36127811852`: PASS, 38/38 in GCC 13 Debug and
  Clang 18/libc++ Debug.

Final audit result:

**PASS — IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
NOT QUALIFIED.**

The initial Clang failure remains part of the retained audit history and is not
reclassified as a production defect.
