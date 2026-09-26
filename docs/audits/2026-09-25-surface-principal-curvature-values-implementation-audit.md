# Surface Principal Curvature Values — Implementation Audit

Date: 2026-09-25  
Status: PASS / IMPLEMENTED / INTEGRATED / CLOSURE PENDING / NOT QUALIFIED  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Scientific scope

Closed decision authority:

`docs/decisions/SURFACE_PRINCIPAL_CURVATURE_VALUES_DECISION.md`.

Integrated bounded capability:

**Pointwise Ordered Principal Curvature Values plus Exact Represented-Data
Umbilic State for Regular C2 Bounded Parametric Surfaces in 3D.**

Explicitly excluded:

- principal directions/eigenvectors;
- line-field/sign-continuity semantics;
- near-umbilic thresholds or conditioning classes;
- new Surface Representation families;
- general trimming/p-curves/topological faces;
- Boundary Curve Discretization;
- Physical Sizing;
- anisotropic/tensor metrics;
- meshing;
- Quad-Dominant and parallel work.

## 2. Implementation mapping

Integrated production/API mapping:

- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`;
- `tests/surface_principal_curvatures.cpp`;
- `CMakeLists.txt`.

No concrete surface representation source was modified.

## 3. Numerical implementation

The integrated production algorithm:

1. consumes the already validated first/second fundamental forms and positive
   area density;
2. normalizes I and II independently to reduce avoidable overflow/underflow;
3. forms a Cholesky-equivalent whitening of the positive-definite metric using
   area density rather than subtracting `EG-F^2`;
4. forms a real symmetric 2x2 curvature operator;
5. scales that operator before solving;
6. computes the eigenvalue gap with `hypot`;
7. recovers the cancellation-prone eigenvalue through the scaled determinant
   when possible;
8. restores physical curvature scale;
9. converts final values only when representable as finite `double`.

Production does not use `H +/- sqrt(H*H-K)` with an epsilon repair.

Exact represented-data umbilicity is successful only when the whitened
represented operator has exactly equal diagonal entries and exactly zero
off-diagonal entry.

## 4. Focused scientific evidence

`apmesh_core.surface_principal_curvatures` covers:

- plane: 0/0 and exact umbilic;
- bounded cylinder: 0 and signed `-1/R`;
- elliptic same-sign distinct principal values;
- hyperbolic opposite-sign values;
- parabolic exactly-one-zero value;
- exact nonzero umbilic;
- exactly distinct near-umbilic represented data;
- generic non-orthogonal metric;
- `k_max+k_min=2H`;
- `k_max*k_min=K`;
- U-only and V-only reversal sign/swap laws;
- U+V reversal invariance;
- positive coordinate scaling;
- translation covariance;
- admitted signed Cartesian frame covariance;
- extreme finite success;
- explicit non-representable-result failure;
- generic parameter/domain/continuity error propagation;
- deterministic repeated successes/failures.

## 5. Candidate validation

Candidate head:

`a59f52933a829410e7b24f505caa25092b0671fe`.

- FAST `36209864983`: PASS, 39/39;
- INTEGRATION `36209865005`: PASS, 39/39 in GCC 13 Debug and Clang
  18/libc++ Debug;
- focused principal-curvature contract: PASS in all three cells.

Candidate audit:

`docs/audits/2026-09-25-surface-principal-curvature-values-candidate-validation.md`.

## 6. Final PR-head validation

Final immutable PR head:

`6c04e1a370f3b7a8a20e082040e46efe5aadb90e`.

- FAST `36209969064`: PASS, 39/39;
- INTEGRATION `36209969075`: PASS, 39/39 in GCC 13 Debug and Clang
  18/libc++ Debug;
- focused principal-curvature contract: PASS in all three cells;
- every prior ordinary semantic contract remained PASS.

No rescue rerun or acceptance weakening was required.

## 7. Integration

PR #208:

`surface: add principal curvature values`.

Integrated merge commit:

`3d0635d0a9420d48a1820905709bea367c96cec0`.

The squash merge preserved the reviewed bounded scientific scope.

## 8. Protected-main post-merge validation

FAST:

- run `36210105115`;
- GCC 13 Debug;
- 39/39 PASS;
- focused principal-curvature contract PASS.

INTEGRATION:

- run `36210105079`;
- GCC 13 Debug: 39/39 PASS;
- Clang 18/libc++ Debug: 39/39 PASS;
- focused principal-curvature contract PASS in both cells.

## 9. Regression result

All 38 prerequisite ordinary semantic contracts remain PASS.

No existing qualified stage claim is widened.

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

Surface Differential Geometry remains IN INVESTIGATION / NOT QUALIFIED.

## 10. Implementation conclusion

Implementation result:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSURE READY / NOT QUALIFIED.**

This audit authorizes only documentation/continuity closure of the integrated
work item.

No next scientific capability is pre-authorized by this audit.
