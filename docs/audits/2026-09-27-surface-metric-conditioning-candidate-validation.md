# Surface Metric Conditioning — Candidate Validation Audit

Date: 2026-09-27  
Status: INITIAL CANDIDATE VALIDATED / FINAL HEAD REVALIDATION PENDING / NOT QUALIFIED

## Authority

Protected-main baseline:

`49efeaca5ac4fee5ec8fcda9859caa646fbce1a4`.

Baseline protected-main validation:

- FAST #602: PASS;
- INTEGRATION #593: PASS in GCC 13 Debug and Clang 18/libc++ Debug;
- ordinary semantic inventory before this work item: **41 tests**.

Active branch:

`surface/metric-conditioning`.

Technical candidate:

`ae1d4e0f59ae1e5eb017a58030c69f5bea7e8bc6`.

## Scope audit

Changed production/API/test files:

- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`;
- `tests/surface_metric_conditioning.cpp`;
- `CMakeLists.txt`.

No new surface representation, topology, discretization, sizing, meshing,
principal-direction semantics, conditioning threshold, formal campaign or
qualification claim is present.

## Numeric design audit

For the surface Jacobian `J=[S_u S_v]`, the existing first fundamental form
is `J^T J`. If `lambda_max` is the largest eigenvalue of that metric and
`A=sigma_max*sigma_min` is the existing positive area density, then:

`lambda_max / A = sigma_max / sigma_min = kappa_2(J)`.

The candidate scales `E,F,G` by their maximum magnitude before the symmetric
2x2 eigenvalue computation and uses `hypot` for the spectral gap. This avoids
forming the cancellation-prone metric determinant for the final ratio and
reuses the independently robust area density already admitted by the
first-order differential contract.

Exact singular points remain the existing
`SurfaceDifferentialError::singular_parameterization`. Regular anisotropic
points are never converted to failures by a tolerance. A final condition number
outside the public scalar range uses the existing
`non_representable_result` contract.

## Focused evidence

The new focused target is intended to cover:

- exact isotropic value;
- diagonal and non-orthogonal analytic values;
- axis swap and reversal invariance;
- uniform spatial scale invariance;
- non-uniform reparameterization sensitivity;
- exact singular and representable near-singular cases;
- extreme finite scale separation;
- non-representable condition result;
- translation and signed-frame invariance;
- cylinder and sphere analytic evidence;
- exact sphere-pole singularity;
- every admitted bounded surface family;
- deterministic repeated success/failure.

Expected ordinary inventory after registration: **42 tests**.

## Local validation note

A throwaway local clone was attempted from the execution environment, but DNS
resolution for `github.com` was unavailable. No local compile/test evidence is
therefore claimed. This is recorded as an execution-environment limitation, not
as a repository failure.

## Initial remote validation

Documented PR head:

`3302a6209b54d63ddde6ecbd8ba05ead9aef1284`.

Results:

- FAST #603: PASS;
- INTEGRATION #594 / GCC 13 Debug: PASS;
- INTEGRATION #594 / Clang 18/libc++ Debug: PASS;
- no check annotations;
- no review threads or conflicting reviews at validation time.

Registration audit on this exact head:

- ordinary `add_test` count before qualification-only tests: **42**;
- `apmesh_core.surface_metric_conditioning`: registered by `add_test`;
- focused target labels include `fast` and `integration`;
- FAST workflow executes `ctest --preset fast`;
- INTEGRATION workflows execute
  `ctest --preset integration-gcc-debug` and
  `ctest --preset integration-clang-debug`;
- those presets select the corresponding `fast` / `integration` labels.

Therefore the new focused target is part of the green ordinary profile rather
than merely compiled.

No prerequisite test registration was removed.

## Final-head requirement

This audit update changes the PR head. Before integration, the new exact final
head must again show:

- FAST: PASS;
- INTEGRATION GCC 13 Debug: PASS;
- INTEGRATION Clang 18/libc++ Debug: PASS;
- unchanged **42-test** ordinary registration;
- no prerequisite regression or review/concurrency conflict.

Any failure must be preserved here with its diagnosis and correction before a
fresh final-head validation.
