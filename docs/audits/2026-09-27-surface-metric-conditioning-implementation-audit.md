# Surface Metric Conditioning — Implementation Audit

Date: 2026-09-27  
Status: IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED / NOT QUALIFIED

## 1. Authority

Implementation-entry protected `main`:

`49efeaca5ac4fee5ec8fcda9859caa646fbce1a4`.

Entry validation:

- FAST #602: PASS;
- INTEGRATION #593: PASS in GCC 13 Debug and Clang 18/libc++ Debug;
- ordinary semantic registration inventory: **41 tests**.

Scientific authority:

`docs/decisions/SURFACE_METRIC_CONDITIONING_DIAGNOSTICS_DECISION.md`.

The closed decision authorized exactly one bounded production work item:

**Pointwise Surface Metric Conditioning Diagnostics in 3D.**

## 2. Implemented scope

Technical candidate:

`ae1d4e0f59ae1e5eb017a58030c69f5bea7e8bc6`.

Final immutable implementation head:

`c9176da223e6568861803743d992b91371aa50fb`.

Production/API changes:

- `include/apmesh/geometry/surface_differential.hpp`
  - `SurfaceMetricConditioning`;
  - direct metric-normal query;
  - bounded-surface convenience overload.
- `src/geometry/surface_differential.cpp`
  - scale-aware pointwise Jacobian condition-number evaluation.
- `tests/surface_metric_conditioning.cpp`
  - focused semantic contract.
- `CMakeLists.txt`
  - focused target and ordinary FAST/INTEGRATION registration.

No unrelated production file changed.

## 3. Numeric contract

For the surface Jacobian

`J=[S_u S_v]`

the existing first fundamental form is

`I=J^T J=[[E,F],[F,G]]`.

Let `lambda_max` be the largest eigenvalue of `I`. The already-integrated
area density is

`A=sigma_max*sigma_min`.

Therefore the candidate evaluates

`kappa_2(J)=lambda_max/A=sigma_max/sigma_min`.

Implementation details:

- `E,F,G` are normalized by their maximum magnitude before the symmetric
  2x2 spectral calculation;
- the spectral gap uses `hypot`;
- the result reuses the existing robust area-density contract instead of
  forming a cancellation-prone determinant ratio;
- exact singular parameterizations retain
  `SurfaceDifferentialError::singular_parameterization`;
- regular but strongly anisotropic points are accepted without an arbitrary
  threshold;
- an unrepresentable final condition number uses the existing
  `non_representable_result` error;
- no `ill_conditioned` class or user-tunable conditioning threshold exists.

## 4. Focused evidence

The focused contract covers:

- exact isotropic `kappa=1`;
- diagonal anisotropic analytic value;
- non-orthogonal analytic value;
- parameter-axis swap invariance;
- U, V and both reversal invariance;
- uniform spatial-scale invariance;
- intentional sensitivity to non-uniform reparameterization;
- exact singular classification;
- representable near-singular regular conditioning;
- extreme finite scale separation;
- explicit non-representable result;
- translation and signed-frame invariance;
- bounded cylinder analytic evidence and reversal invariance;
- bounded sphere analytic evidence, equator identity and exact pole singularity;
- conformance across every currently admitted bounded surface family;
- deterministic repeated successes and failures.

Ordinary semantic registration inventory after the work item: **42 tests**.

The focused target is registered by `add_test` and carries both `fast` and
`integration` labels.

## 5. Validation history

### 5.1 Initial documented candidate

Head:

`3302a6209b54d63ddde6ecbd8ba05ead9aef1284`.

Validation:

- FAST #603: PASS;
- INTEGRATION #594 / GCC 13 Debug: PASS;
- INTEGRATION #594 / Clang 18/libc++ Debug: PASS.

A static registration audit confirmed 42 ordinary `add_test` entries and the
new focused target in both FAST and INTEGRATION label selections.

### 5.2 Documentation synchronization

The candidate audit and continuity documents were synchronized in subsequent
documentation-only commits. Intermediate Actions runs were cancelled
mechanically by the workflow concurrency policy as newer heads appeared.

The final immutable head became:

`c9176da223e6568861803743d992b91371aa50fb`.

### 5.3 PR #224 Actions concurrency incident

PR #224 used the final immutable head above.

INTEGRATION #599 passed in both required GCC and Clang cells on that exact
head. Its final FAST run was prevented from starting normally because
intermediate FAST #607 on obsolete head
`f095c039d2845f58ebd1801d1db8279104a70f9e` remained orphaned
`in_progress` instead of being cancelled by the workflow's declared
`cancel-in-progress: true` policy.

The incident was classified as:

**MECHANICAL GITHUB ACTIONS SCHEDULING/CANCELLATION INCIDENT /
NO PRODUCTION OR TEST FAILURE OBSERVED.**

PR #224 was closed **without merge**. No repository content was changed to
work around the incident.

### 5.4 Replacement PR #225

PR #225 was opened from the same branch and exact same final head:

`c9176da223e6568861803743d992b91371aa50fb`.

Required validation:

- FAST #610: PASS;
- INTEGRATION #601 / GCC 13 Debug: PASS;
- INTEGRATION #601 / Clang 18/libc++ Debug: PASS;
- no review threads or conflicting reviews;
- branch was ahead of `main` with behind count zero;
- ordinary registration remained 42 tests;
- focused target retained FAST and INTEGRATION labels.

PR #225 squash-merged as:

`7b3c833273dba042b7f6c055dd736510573664a6`.

### 5.5 Protected-main validation

Protected-main validation on exact merge revision
`7b3c833273dba042b7f6c055dd736510573664a6`:

- FAST #611: PASS;
- INTEGRATION #602 / GCC 13 Debug: PASS;
- INTEGRATION #602 / Clang 18/libc++ Debug: PASS.

No post-merge regression was observed.

## 6. Local validation limitation

A throwaway local clone was attempted during candidate preparation, but the
execution environment could not resolve `github.com`. No local-build evidence
is claimed. Remote GitHub Actions on exact repository heads are the validation
authority recorded above.

## 7. Scientific boundary

This implementation does **not**:

- classify a point as well/ill conditioned;
- establish a downstream meshing acceptance threshold;
- implement principal directions or curvature line fields;
- add cone, torus or another surface representation;
- add p-curves, general trimming or topological faces;
- start Boundary Curve Discretization;
- start sizing or meshing;
- qualify Surface Differential Geometry;
- qualify Surface Representation;
- authorize a formal qualification campaign.

## 8. Result

Integrated implementation result:

**POINTWISE SURFACE METRIC CONDITIONING DIAGNOSTICS IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / CLOSURE PENDING /
SURFACE DIFFERENTIAL GEOMETRY NOT QUALIFIED.**

The current work item is documentation-only implementation closure. No next
scientific capability is selected by this audit.


## 9. Implementation closure

Documentation-only implementation closure used PR #226 with final head:

`2955e80bf0f61afe971ab0eaa40661b1b0923cdf`.

Closure PR validation:

- FAST #612: PASS;
- INTEGRATION #603 / GCC 13 Debug: PASS;
- INTEGRATION #603 / Clang 18/libc++ Debug: PASS;
- no reviews or unresolved review threads;
- branch relation at merge gate: ahead=6, behind=0;
- diff restricted to documentation/audit files.

PR #226 squash-merged as:

`0c1c4c2365257549122fa4bb9b8720c4e83bddc9`.

Protected-main validation on that exact closure revision:

- FAST #613: PASS;
- INTEGRATION #604 / GCC 13 Debug: PASS;
- INTEGRATION #604 / Clang 18/libc++ Debug: PASS.

Ordinary semantic registration inventory remains **42 tests**.

Closure result:

**POINTWISE SURFACE METRIC CONDITIONING DIAGNOSTICS IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED / NOT QUALIFIED.**

The implementation work item is terminally closed. The current
`docs/surface-metric-conditioning-implementation-terminal-sync` branch only
publishes this terminal continuity state so another conversation can recover
the authoritative status directly from the remote repository.
