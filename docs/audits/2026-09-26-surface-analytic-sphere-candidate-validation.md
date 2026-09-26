# Bounded Analytic Spherical Surface — Candidate Validation Audit

Date: 2026-09-26  
Status: PASS / CANDIDATE VALIDATED / FINAL DOCUMENTATION-SYNC REVALIDATION REQUIRED  
Stage: Surface Representation — Continuous Patch Geometry

## 1. Audited candidate

Candidate head:

`363ad75f50d51886dc8cc9c48647fc3cf94e57d0`.

Decision authority:

`docs/decisions/SURFACE_ANALYTIC_SPHERE_DECISION.md`.

Bounded capability:

**Bounded Analytic Spherical Surface Sector in 3D.**

The candidate is a bounded, non-periodic representation work unit. It does not
qualify Surface Representation or Surface Differential Geometry.

## 2. Authorized repository delta

Audited implementation mapping:

- `include/apmesh/geometry/elementary_surface.hpp`;
- `src/geometry/elementary_surface.cpp`;
- `tests/surface_sphere.cpp`;
- `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / decision.

The candidate does **not** modify:

- `include/apmesh/geometry/parametric_surface.hpp`;
- `include/apmesh/core/geometry.hpp`;
- `src/core/geometry.cpp`;
- `include/apmesh/geometry/surface_differential.hpp`;
- `src/geometry/surface_differential.cpp`.

Therefore the common surface error/contract, `AxisPlacement3`, and generic
Surface Differential Geometry algorithms remain frozen prerequisites.

## 3. Representation and numerical strategy audit

The candidate stores:

- an existing right-handed `AxisPlacement3`;
- finite strictly positive radius;
- strict finite U/V domains;
- U/V reversal flags.

Construction rejects:

- non-finite radius;
- non-positive radius;
- U width at least the represented `2*pi`;
- latitude outside the represented `[-pi/2,+pi/2]`.

No modulo normalization, periodic wrapping or universal tolerance is used.

Production evaluates

`S(u,v) = O + R cos(v)(cos(u)X + sin(u)Y) + R sin(v)Z`

and analytic first/second partials.

At the exact stored canonical latitudes `+/-pi/2`, production uses exact
represented trigonometric states:

- north: `sin(v)=+1`, `cos(v)=0`;
- south: `sin(v)=-1`, `cos(v)=0`.

This makes the polar value independent of U and `S_u` exactly zero without an
epsilon or approximate pole neighborhood.

Finite conversion is checked before public point/vector construction. A final
unrepresentable result returns the existing
`SurfaceError::non_finite_result`.

## 4. Focused evidence audit

The focused contract `apmesh_core.surface_sphere` covers:

- `BoundedParametricSurface3` concept satisfaction;
- stored radius, placement, domains and reversal state;
- radius, full-revolution and latitude construction failures;
- established U/V query error ordering;
- independent long-double value/Su/Sv/Suu/Suv/Svv oracle;
- identity placement;
- genuinely non-axis-aligned `AxisPlacement3`;
- U, V and double reversal covariance/involution;
- canonical polar V-reversal;
- translation covariance;
- exact power-of-two radius/coordinate scaling;
- exact north/south pole values;
- exact zero `S_u` at a canonical pole;
- generic metric-normal singularity at a canonical pole;
- generic II/K/H singularity propagation at a canonical pole;
- generic principal-value singularity propagation at a canonical pole;
- exact unit-equator E/F/G/area-density/outward-normal evidence;
- exact unit-equator `K=1`, `H=-1`, ordered principal values
  `-1,-1`, exact represented-data umbilic state;
- representable near-pole regularity without tolerance coercion;
- radius-scale covariance of K/H/principal values;
- extreme finite value/jet success;
- explicit non-representable world-point failure;
- deterministic repeated successes and failures.

No principal-direction, conditioning, cone, torus, periodic seam,
general-trimming, boundary-discretization or meshing evidence is claimed.

## 5. Validation evidence

### FAST

Run:
`36263014748`.

Job:
GCC 13 Debug / FAST, job `108462291414`.

Result:

- 40/40 ordinary semantic tests PASS;
- `apmesh_core.surface_sphere`: PASS;
- 100% tests passed.

### INTEGRATION — GCC

Run:
`36263014773`.

Job:
GCC 13 Debug / INTEGRATION, job `108462291939`.

Result:

- 40/40 ordinary semantic tests PASS;
- `apmesh_core.surface_sphere`: PASS;
- 100% tests passed.

### INTEGRATION — Clang/libc++

Run:
`36263014773`.

Job:
Clang 18 libc++ Debug / INTEGRATION, job `108462291804`.

Result:

- 40/40 ordinary semantic tests PASS;
- `apmesh_core.surface_sphere`: PASS;
- 100% tests passed.

## 6. Regression result

All 39 prerequisite ordinary semantic contracts remain PASS in all required
cells.

The audited diff contains only the files authorized by the decision and
continuity documentation.

No existing qualification claim is widened.

Surface Representation remains **IN INVESTIGATION / NOT QUALIFIED**.

Surface Differential Geometry remains **IN INVESTIGATION / NOT QUALIFIED**.

## 7. Candidate conclusion

Candidate result:

**PASS / BOUNDED ANALYTIC SPHERE IMPLEMENTED CANDIDATE /
FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

The candidate may proceed only after this audit/documentation synchronization
itself receives fresh FAST and INTEGRATION validation on the immutable final PR
head.

A passing final-head validation authorizes normal PR integration, followed by
protected-main post-merge FAST/INTEGRATION and a separate implementation
closure checkpoint.
