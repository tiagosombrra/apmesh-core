# Curve Representation — Bounded Entry Decision

Status: ENTRY DECISION / IMPLEMENTATION NOT STARTED
Date: 2026-09-20
Stage: Curve Representation — Continuous Geometry Before Discretization
Prerequisites: Foundation QUALIFIED; Geometry Primitives QUALIFIED; Topological
Model QUALIFIED in the exact admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud
envelope

## Question

What is the smallest continuous-curve capability that can be implemented after
Topological Model qualification while preserving explicit numeric failure,
determinism, geometry/topology separation, and a clean boundary before
differential geometry, integration, discretization, surfaces, and meshing?

## Decision

Authorize one bounded implementation work unit named:

**Cubic Bézier Representation and Point Evaluation.**

The work unit may represent non-rational cubic Bézier curves in two and three
dimensions from exactly four already-valid control points, expose those control
points immutably, evaluate the curve for a finite normalized parameter
`t ∈ [0,1]`, and construct the exact reversed representation by reversing the
control-point order.

No derivative, tangent, curvature, arc-length, subdivision, intersection,
projection, closest-point, discretization, curve identity, topology ownership,
surface relation, rational weight, B-spline/NURBS, arbitrary degree, or
extrapolation behavior is authorized by this decision.

This decision fixes scientific semantics and evidence obligations. Exact C++
spelling and final file partitioning remain implementation choices consistent
with the Architecture Contract.

## Sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| CGAL Bézier curve documentation | A Bézier curve is defined by an ordered control-point sequence; the first and last control points are the curve endpoints; point evaluation is parameterized by `t`. | That CGAL is a dependency, acceptance oracle, or that arbitrary degree belongs in the first AP Mesh work unit. | Preserve ordered control points and endpoint interpolation, but restrict the first work unit to degree three. |
| Open CASCADE `Geom_BezierCurve` documentation | Bézier parameter range is `[0,1]`; first and last poles are endpoints; polynomial/non-rational Bézier is a valid special case. | That AP Mesh should inherit OCCT tolerance, weight, degree-limit, closure, or mutation policies. | Use normalized closed parameter domain and non-rational curves only; do not import OCCT tolerance semantics. |
| Michigan Tech de Casteljau notes | De Casteljau evaluates a Bézier point by repeated affine interpolation and avoids direct expanded Bernstein-polynomial evaluation; it applies on `u ∈ [0,1]`. | That the teaching implementation is an external runtime dependency or a formal floating-point proof for every AP Mesh scale. | The first implementation must use a de Casteljau-style affine evaluation path and independently test its numeric behavior. |
| Qualified AP Mesh Geometry Primitives | `Point2`/`Point3`, vectors, classified numeric failures, and exact Cartesian-frame transforms are already qualified prerequisites. | Curve semantics, parameter semantics, derivatives, or integration. | Reuse existing finite point values and failure vocabulary; do not duplicate point/vector mathematics. |

Reference links:

- https://doc.cgal.org/6.1.1/Arrangement_on_surface_2/classCGAL_1_1Arr__Bezier__curve__traits__2_1_1Curve__2.html
- https://dev.opencascade.org/doc/refman/html/class_geom___bezier_curve.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/de-casteljau.html

No third-party runtime dependency is admitted.

## Authorized representation

The implementation work unit may add only the scientific capability equivalent
to immutable `CubicBezier2` and `CubicBezier3` values with:

1. exactly four ordered control points `P0, P1, P2, P3`;
2. control-point access in declared order;
3. exact source `P0` and target `P3`;
4. point evaluation on the closed normalized parameter domain `[0,1]`;
5. exact curve reversal obtained by control-point order
   `P3, P2, P1, P0`; and
6. deterministic value semantics sufficient for focused tests.

The representation itself is valid whenever its four control points are valid
finite AP Mesh point values. Coincident control points are permitted. A curve
whose four control points are identical is permitted. Neither condition implies
topological identity, regularity, positive length, or a later meshing decision.

The first work unit must not invent a `CurveId`. Geometric curve values and
topological identity remain separate.

## Parameter contract

Evaluation accepts only a finite scalar parameter `t` satisfying exactly:

`0 <= t <= 1`.

Required behavior:

- `t = 0` returns `P0` exactly;
- `t = 1` returns `P3` exactly;
- NaN and positive/negative infinity are explicit invalid-parameter failures;
- finite `t < 0` or `t > 1` is an explicit out-of-domain failure;
- no clamping, periodic wrapping, extrapolation, retry, or hidden tolerance is
  allowed;
- signed zero is accepted as the endpoint parameter zero;
- evaluation must return a finite valid point or an explicit classified
  failure if the arithmetic cannot produce a finite result.

The implementation may introduce a curve-specific error vocabulary or a
carefully justified reuse/extension of existing geometry error vocabulary, but
it must distinguish invalid/non-finite parameter, out-of-domain parameter, and
non-finite arithmetic result. These failures must not be collapsed into a
boolean or exception-only contract.

## Evaluation contract

For control points `P0..P3`, the mathematical reference is the cubic Bézier
curve

`B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t)t^2 P2 + t^3 P3`.

Production evaluation must use the de Casteljau affine construction rather than
forming and evaluating an expanded cubic polynomial.

The mathematical Bernstein form is the independent reference used to derive
analytic fixtures. Tests must not call the production evaluator to compute
their expected values.

The implementation must preserve the existing rule that approximate comparison
is explicit and scale-aware. It may not introduce a universal epsilon.

## Required invariants

1. The curve stores exactly four control points in declared order.
2. Source equals `P0`; target equals `P3`.
3. `B(0) = P0` exactly.
4. `B(1) = P3` exactly.
5. Reversing twice exactly restores the original representation.
6. If `R` is the reversed curve, `R(t)` agrees with `B(1-t)` under the
   declared numeric comparison policy.
7. Equal control-point sequences produce equal representations.
8. Repeated evaluation of the same valid curve/parameter is deterministic.
9. Evaluation depends only on the curve's control points and the supplied
   parameter; no topology, filesystem, process state, environment variable,
   random source, global tolerance, or mutable scientific state participates.
10. Finite valid inputs never silently produce a non-finite valid point.
11. Coincident control points remain accepted representation data and are not
    silently regularized or collapsed.
12. Curve reversal changes geometric orientation only; it does not create or
    imply topological identity.

## Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Endpoint zero | `B(0)` equals `P0` exactly. |
| Endpoint one | `B(1)` equals `P3` exactly. |
| Constant curve | Four identical points evaluate to that point throughout the tested parameter set; no regularity or length claim is made. |
| Linear reproduction | Four collinear control points sampled at affine parameters `0, 1/3, 2/3, 1` reproduce the corresponding line parameterization within an explicit reference policy. |
| Symmetric midpoint | An exactly constructed symmetric fixture at `t=1/2` matches its independently derived expected point. |
| Generic cubic | At least three interior dyadic parameters are compared against independently computed Bernstein-form reference values. |
| Reversal endpoints | Reversed source/target are exactly swapped. |
| Reversal involution | Double reversal restores all four control points exactly. |
| Reversal evaluation | For declared dyadic parameters, reversed evaluation agrees with original evaluation at `1-t`. |
| 2D/3D parity | Equivalent planar fixtures embedded in 3D with zero third coordinate agree in the shared coordinates. |
| Signed zero parameter | `+0` and `-0` both select the source endpoint without changing representation. |
| Non-finite parameter | Every NaN and ±infinity parameter is rejected explicitly. |
| Out-of-domain parameter | Representative finite values below zero and above one are rejected; no clamp/extrapolation occurs. |
| Extreme finite control points | Valid finite points near the admitted numeric envelope either produce a finite result or explicit non-finite-result failure. |
| Qualified-frame covariance | In tests only, a fixture transformed through an already qualified Cartesian frame gives an evaluation consistent with transforming the original evaluated point, within an explicit policy. |
| Repeatability | Repeated executions produce equivalent scientific fields and results. |

Dyadic parameters should be preferred for exact or especially transparent
binary fixtures. Any approximate assertion must declare its reference scale,
policy, residual, and reason exact comparison is not applicable.

## Explicit exclusions

This entry decision does not authorize:

- first, second, or higher derivatives;
- tangent, normal, speed, regularity classification, curvature, torsion, or
  Frenet frames;
- arc length, quadrature, inverse arc-length mapping, or parameter
  reparameterization;
- subdivision/splitting, degree elevation/reduction, fitting, interpolation, or
  approximation;
- bounding boxes, convex-hull algorithms, flatness estimators, or distance
  bounds;
- closest-point, projection, root solving, intersection, self-intersection, or
  curve-curve predicates;
- rational Bézier curves, weights, conics, B-splines, NURBS, arbitrary degree,
  knots, or periodic curves;
- `CurveId`, ownership by topological edges, patch/surface trims, model
  integration, or serialization;
- boundary discretization, sizing, meshing, adaptation, or acceptance;
- coordinate welding, snapping, repair, or topology inference;
- filesystem I/O, logging, randomness, global state, global epsilon, hidden
  fallback, or third-party runtime dependency;
- OpenMP/MPI/GPU execution or performance claims;
- native-Windows qualification;
- a stage-level qualification campaign.

Derivatives and regularity are the next planned investigation problem only
after this representation/evaluation work unit is implemented and its focused
contract passes.

## Architecture boundary

The new capability belongs to continuous geometry, not topology. It may depend
on the already qualified point/vector/numeric primitives but must not introduce
a dependency from topology to geometry or from curve mathematics to model,
boundary, meshing, I/O, or adapters.

The implementation should preserve the single `apmesh::core` library target.
Any new public header/private source paths must follow the Architecture
Contract's logical-module rules. This decision does not require a new linker
target.

## Focused contract required before continuation

The implementation work unit must add one focused CTest contract for cubic
Bézier representation/evaluation. The contract must cover the complete case
table above in both 2D and 3D where applicable and must preserve the already
qualified prerequisite test set.

Focused validation must run at minimum in the repository's admitted GCC 13
Debug and Clang 18/libc++ Debug environments before the work unit can be
considered IMPLEMENTED / FOCUSED CONTRACT PASS.

That focused result is implementation evidence only. It does not qualify Curve
Representation as a stage.

## Admission and stop conditions

Implementation may proceed only if it can:

1. reuse qualified finite point values without introducing duplicate geometry
   primitives;
2. keep the curve representation immutable and topology-free;
3. enforce the exact closed parameter domain with explicit failure;
4. implement evaluation without global tolerance or hidden fallback;
5. cover the complete required case table independently of legacy AP Mesh
   output;
6. preserve Foundation, Geometry Primitives, and Topological Model behavior.

Stop and require a separate scientific decision if implementation needs:

- derivative/regularity semantics;
- rational weights or arbitrary degree;
- adaptive subdivision or integration;
- curve identity or topology ownership;
- a new third-party dependency;
- a new global numeric policy;
- any reinterpretation of a qualified prerequisite contract.

## Alternatives considered

### Arbitrary-degree Bézier immediately

Rejected for the first work unit. It adds dynamic representation and degree
policy before the doctoral reconstruction demonstrates a need beyond the cubic
curve used as the first continuous reference capability.

### Rational Bézier / NURBS immediately

Rejected. Weight semantics, knots, conic exactness, parameter domains, and
additional failure modes are separate scientific decisions.

### Abstract polymorphic Curve base class first

Rejected. No second concrete curve representation exists yet, so a runtime
hierarchy would freeze an interface before the common semantics are known.

### Derivatives in the first work unit

Deferred. Evaluation must be independently verified before derivative and
regularity semantics depend on it.

### Cubic polynomial coefficients instead of control points

Rejected as the public scientific representation. Ordered control points make
endpoint/reversal semantics direct and preserve the geometric construction used
by independent references.

### Chosen entry

Exactly four finite control points plus normalized point evaluation and exact
representation reversal are sufficient to establish the first continuous curve
claim while leaving differential geometry, integration, topology ownership and
discretization scientifically independent.

## Stage effect

At entry-decision time, Curve Representation moves from `NOT STARTED` to:

`IN INVESTIGATION / ENTRY DECISION APPROVED / IMPLEMENTATION NOT STARTED`.

Topological Model remains qualified and closed. No prerequisite qualification
is reopened by this documentation-only decision.

## Next bounded action after entry integration

Implement only **Cubic Bézier Representation and Point Evaluation** and its
focused contract. Do not implement derivatives, arc length, subdivision,
discretization, surfaces, or stage-level qualification infrastructure in that
work item.
