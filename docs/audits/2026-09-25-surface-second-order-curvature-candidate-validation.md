# Surface Second-Order Curvature Candidate Validation Audit

Date: 2026-09-25  
Work item: Pointwise Surface Second Fundamental Form + Gaussian/Mean Curvature  
Branch: `surface/second-order-curvature`

## Scope

Record the first PR-head validation incident and the correction without
rewriting history.

## Candidate head and runs

Initial candidate head:

`c3cd327ebbd4add551c7d331f0149513d15324c7`.

Validation:

- FAST `36125805859`: PASS;
- INTEGRATION `36125805817`:
  - GCC 13 Debug: PASS, 38/38;
  - Clang 18/libc++ Debug: FAIL, 37/38;
  - failing contract:
    `apmesh_core.surface_second_order_curvature`;
  - observed diagnostic:
    `unrepresentable curvature did not fail explicitly`.

All other 37 ordinary contracts passed in the failing Clang cell.

## Classification

The failure was classified as a **focused test-fixture portability defect**,
not a production semantic defect.

The original extreme fixture combined:

- a nearly collinear first-derivative pair;
- a very small area density;
- ordinary unit second derivatives.

Its intended purpose was to force K/H outside representable `double` range.
That result was observed in GCC but was not stable in the Clang/libc++ cell.

The production API/source were unchanged by the correction.

## Correction

Correction commit:

`89326bb1ad70449f4ed75c7b00d45699e998ce09`.

Only
`tests/surface_second_order_curvature.cpp`
changed.

The corrected fixture uses:

- orthonormal first derivatives;
- finite second derivatives with magnitude
  `std::numeric_limits<double>::max()/4`;
- a guaranteed finite first-order metric;
- a Gaussian-curvature numerator whose requested scalar result necessarily
  exceeds representable `double` range.

This preserves the scientific contract:
explicit `SurfaceDifferentialError::non_representable_result` when the final
requested curvature is not representable, while removing dependence on
near-singular floating-point behavior.

## Production impact

None.

Unchanged:

- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`;
- all concrete surface representations;
- `BoundedParametricSurface3`;
- scientific formulas and failure vocabulary.

## Required next validation

The corrected/documented final head must pass:

- GCC 13 Debug / FAST;
- GCC 13 Debug / INTEGRATION;
- Clang 18/libc++ Debug / INTEGRATION;
- 38/38 ordinary semantic tests;
- focused `apmesh_core.surface_second_order_curvature` in all three cells.

Until then the work item remains:

**IMPLEMENTED CANDIDATE / CORRECTED VALIDATION PENDING / NOT QUALIFIED.**
