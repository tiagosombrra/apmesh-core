# Bounded Analytic Spherical Surface — Implementation Audit

Date: 2026-09-26  
Status: PASS / IMPLEMENTED / INTEGRATED / CLOSED / NOT QUALIFIED  
Stage: Surface Representation — Continuous Patch Geometry

## 1. Integrated capability

Integrated bounded capability:

**Bounded Analytic Spherical Surface Sector in 3D.**

Decision authority:

`docs/decisions/SURFACE_ANALYTIC_SPHERE_DECISION.md`.

Candidate-validation audit:

`docs/audits/2026-09-26-surface-analytic-sphere-candidate-validation.md`.

The implementation does not widen the scope to principal directions,
conditioning diagnostics, cone, torus, full-periodic seams, general
trimming/p-curves/topological faces, Boundary Curve Discretization, Physical
Sizing, meshing or qualification.

## 2. Candidate and final implementation evidence

Initial validated candidate head:

`363ad75f50d51886dc8cc9c48647fc3cf94e57d0`.

Initial candidate validation:

- FAST `36263014748`: PASS, 40/40;
- INTEGRATION `36263014773`: PASS, 40/40 in GCC 13 Debug and Clang
  18/libc++ Debug;
- `apmesh_core.surface_sphere`: PASS in all three jobs.

Final immutable PR head after candidate audit/documentation synchronization:

`0fb729054b0d3fa97045c0238ed4de9ce2e94d3d`.

Final PR validation:

- FAST `36263188269`: PASS, 40/40;
- INTEGRATION `36263188266`: PASS, 40/40 in GCC 13 Debug and Clang
  18/libc++ Debug;
- `apmesh_core.surface_sphere`: PASS in all three jobs.

Implementation PR #214 merged as:

`461c3f04454cc4f8aa789d9baba5085362aa6254`.

Protected-main implementation validation:

- FAST `36263360375`: PASS, 40/40;
- INTEGRATION `36263360805`: PASS, 40/40 in GCC 13 Debug and Clang
  18/libc++ Debug;
- `apmesh_core.surface_sphere`: PASS in all three jobs.

## 3. Repository delta audit

Integrated production/test mapping:

- `include/apmesh/geometry/elementary_surface.hpp`;
- `src/geometry/elementary_surface.cpp`;
- `tests/surface_sphere.cpp`;
- `CMakeLists.txt`;
- synchronized continuity documents;
- candidate-validation audit.

Frozen prerequisite files remained unchanged:

- `include/apmesh/geometry/parametric_surface.hpp`;
- `include/apmesh/core/geometry.hpp`;
- `src/core/geometry.cpp`;
- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`.

No topology, general trimming, discretization, sizing, meshing or third-party
runtime dependency was introduced.

## 4. Scientific contract audit

The integrated family:

- stores an existing `AxisPlacement3`;
- stores a finite strictly positive radius;
- stores finite strict U/V domains;
- rejects U width at least the represented `2*pi`;
- restricts V to represented `[-pi/2,+pi/2]`;
- performs no modulo normalization or periodic wrapping;
- evaluates the standard spherical parameterization analytically;
- supplies analytic Su, Sv, Suu, Suv, Svv;
- reuses existing U/V reversal semantics.

At exact represented canonical latitudes `+/-pi/2`, production enforces:

- `sin(v)=+/-1` exactly;
- `cos(v)=0` exactly;
- polar value independent of U;
- exact zero `S_u`;
- representation success without an epsilon.

The generic differential layer is unchanged and identifies those canonical
poles as exact singular parameterizations.

## 5. Focused evidence audit

The focused contract `apmesh_core.surface_sphere` validates:

- bounded-surface concept satisfaction;
- construction failures and query error order;
- independent long-double value/partial oracle;
- identity and arbitrary right-handed placement;
- U/V/both reversal and polar V reversal;
- translation covariance;
- exact power-of-two radius/coordinate scale covariance;
- exact north/south pole value semantics;
- exact zero polar Su;
- exact generic metric/II/K/H/principal singularity propagation at poles;
- exact canonical unit-equator E/F/G/J and outward normal;
- exact `K=1`, `H=-1`, principal values `-1,-1`, exact umbilic state;
- representable near-pole regularity;
- radius curvature-scale covariance;
- extreme finite success;
- explicit non-representable world-point failure;
- deterministic repeated successes/failures.

## 6. Regression result

- ordinary semantic inventory increased from 39 to **40 tests**;
- all 39 prerequisite ordinary semantic contracts remain PASS in every
  required compiler/profile cell;
- no common surface contract was changed;
- no generic Surface Differential Geometry algorithm was changed;
- no prior qualification claim is widened;
- Curve Representation remains the most recently qualified
  continuous-geometry stage in its admitted CGR0–CGR7 cloud envelope;
- Surface Representation remains **IN INVESTIGATION / NOT QUALIFIED**;
- Surface Differential Geometry remains **IN INVESTIGATION / NOT QUALIFIED**.

## 7. Implementation conclusion

Implementation result:

**PASS / BOUNDED ANALYTIC SPHERE IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

Implementation closure evidence:

- closure PR #215 head:
  `3947547b24b43a0058cd3686c8739ba6f60c626c`;
- closure PR FAST `36264003640`: PASS;
- closure PR INTEGRATION `36264003550`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug;
- closure merge:
  `87ff882606f92259da6a6e788af8a8e6a220a3bf`;
- closure post-merge FAST `36264093555`: PASS;
- closure post-merge INTEGRATION `36264093548`: PASS in GCC 13 Debug and
  Clang 18/libc++ Debug.

Terminal result:

**PASS / BOUNDED ANALYTIC SPHERE IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED / NOT QUALIFIED.**

No next scientific capability is selected by this audit.
