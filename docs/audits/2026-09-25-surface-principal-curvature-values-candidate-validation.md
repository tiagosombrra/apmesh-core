# Surface Principal Curvature Values — Candidate Validation Audit

Date: 2026-09-25  
Status: PASS / CANDIDATE VALIDATED / FINAL DOCUMENTATION-SYNC REVALIDATION REQUIRED  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Audited candidate

Candidate head:

`a59f52933a829410e7b24f505caa25092b0671fe`.

Decision authority:

`docs/decisions/SURFACE_PRINCIPAL_CURVATURE_VALUES_DECISION.md`.

Bounded capability:

**Pointwise Ordered Principal Curvature Values plus Exact Represented-Data
Umbilic State for Regular C2 Bounded Parametric Surfaces in 3D.**

## 2. Authorized repository delta

Audited implementation mapping:

- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`;
- `tests/surface_principal_curvatures.cpp`;
- `CMakeLists.txt`;
- synchronized continuity documents.

No concrete surface representation source is modified.

No topology, trimming, discretization, sizing, meshing, anisotropic metric,
principal-direction or third-party runtime dependency is introduced.

## 3. Numerical strategy audit

Production does not use
`H +/- sqrt(H*H-K)` as the principal-value algorithm.

The candidate:

1. independently normalizes the first and second fundamental forms;
2. uses the already validated positive area density to construct the
   Cholesky-equivalent metric whitening without subtracting `EG-F^2`;
3. forms an equivalent real symmetric 2x2 curvature operator;
4. scales that operator before eigenvalue extraction;
5. uses `hypot` for the eigenvalue gap;
6. uses the scaled determinant to recover the cancellation-prone eigenvalue
   when possible;
7. restores physical curvature scale only after the normalized solve;
8. converts final values only when finite `double` representation exists.

No universal epsilon, discriminant clamp, saturation, NaN repair or hidden
fallback is used.

Exact represented-data umbilicity is true only when the whitened represented
operator has exactly equal diagonal entries and exactly zero off-diagonal
entry.

## 4. Focused evidence audit

The focused contract
`apmesh_core.surface_principal_curvatures` covers:

- exact plane 0/0 principal values and exact umbilic state;
- bounded cylinder 0 and signed `-1/R`;
- elliptic distinct same-sign synthetic values;
- hyperbolic opposite-sign values;
- parabolic exactly-one-zero case;
- exact nonzero umbilic;
- exactly distinct near-umbilic represented values;
- generic non-orthogonal first fundamental form;
- `k_max+k_min=2H` and `k_max*k_min=K` cross-checks;
- single-U and single-V reversal sign/swap laws;
- double reversal invariance;
- positive coordinate scaling;
- translation covariance;
- admitted signed Cartesian frame covariance;
- extreme finite success;
- explicit non-representable-result failure;
- generic parameter/domain/continuity error propagation;
- deterministic repeated successes and failures.

Principal directions/eigenvectors are absent by design.

## 5. Validation evidence

### FAST

Run:
`36209864983`.

Cell:
GCC 13 Debug / FAST.

Result:

- 39/39 ordinary semantic tests PASS;
- `apmesh_core.surface_principal_curvatures`: PASS;
- 100% tests passed.

### INTEGRATION — GCC

Run:
`36209865005`.

Cell:
GCC 13 Debug / INTEGRATION.

Result:

- 39/39 ordinary semantic tests PASS;
- `apmesh_core.surface_principal_curvatures`: PASS;
- 100% tests passed.

### INTEGRATION — Clang/libc++

Run:
`36209865005`.

Cell:
Clang 18 libc++ Debug / INTEGRATION.

Result:

- 39/39 ordinary semantic tests PASS;
- `apmesh_core.surface_principal_curvatures`: PASS;
- 100% tests passed.

## 6. Regression result

All 38 prerequisite ordinary semantic contracts remain PASS in all required
cells.

No prior Curve Representation or Surface Representation qualification claim is
widened.

Surface Differential Geometry remains NOT QUALIFIED.

## 7. Candidate conclusion

Candidate result:

**PASS / IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
NOT QUALIFIED.**

The candidate may proceed only after the documentation synchronization commit
itself receives fresh FAST and INTEGRATION validation on its immutable final
PR head.

A passing final-head validation authorizes normal PR integration, followed by
protected-main post-merge FAST/INTEGRATION and a separate implementation
closure checkpoint.
