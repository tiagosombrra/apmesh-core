# Curve Representation — Bounded Entry Decision

Status: ENTRY DECISION PROPOSED / NO PRODUCTION IMPLEMENTATION YET
Date: 2026-09-21
Stage: Curve Representation — Continuous Geometry Before Discretization
Prerequisites:
- Foundation — QUALIFIED;
- Geometry Primitives — QUALIFIED;
- Topological Model — QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope.

## 1. Question

What is the smallest continuous-curve capability that AP Mesh Core can
implement and verify after qualified geometry and topology without prematurely
introducing derivatives, curvature, arc-length integration, adaptive
discretization, surface geometry, meshing, Quad-Dominant research, or parallel
execution?

## 2. Decision

Authorize one bounded implementation work unit:

**Polynomial Cubic Bézier Value Representation and Evaluation.**

The work unit may introduce only immutable 2D and 3D polynomial cubic Bézier
value types with exactly four ordered finite control points, normalized
parameter domain `[0,1]`, deterministic evaluation by the cubic de Casteljau
recurrence, exact endpoint handling, control-point reversal, and focused
analytic/reference evidence.

This decision fixes the scientific boundary and evidence obligations. It does
not itself implement curve code and does not authorize stage qualification.

## 3. Literature and best-practice basis

The focused review is retained in
`docs/research/REFERENCE_REGISTER.md`.

### Bernstein/Bézier representation

Farouki and Rajan, *On the numerical condition of polynomials in Bernstein
form*, Computer Aided Geometric Design 4(3), 1987,
DOI 10.1016/0167-8396(87)90012-4, supports retaining the curve directly in
Bernstein/Bézier form and treating numerical stability as an explicit concern.
It does not make legacy output an oracle and does not supply an AP Mesh
acceptance tolerance.

### de Casteljau evaluation

The standard recursive de Casteljau construction evaluates a Bézier polynomial
through repeated affine interpolation. Published floating-point analyses of
Bernstein/Bézier evaluation, including Delgado Gracia 2020
(DOI 10.3390/math8122219), support using de Casteljau as the first reference
algorithm while still requiring an explicit residual/error policy in tests.

No compensated algorithm is admitted by this first work unit.

### Normalized domain and reversal semantics

Open CASCADE Bézier references use the normalized parameter range `[0,1]`,
identify the first/last poles with curve endpoints, and map reversal by
`u -> 1-u`. These semantics are used only as an external cross-check; Open
CASCADE is not a dependency or acceptance oracle.

### C++ interpolation primitive

For finite endpoints, C++ `std::lerp` guarantees exact endpoint return at
`t == 0` and `t == 1`, and a finite result for `t in [0,1]`.
Component-wise `std::lerp` is therefore admitted for each de Casteljau
interpolation step.

## 4. Mathematical object

For control points `P0, P1, P2, P3` in 2D or 3D, the admitted curve is

```text
B(t) =
    (1-t)^3 P0
  + 3(1-t)^2 t P1
  + 3(1-t) t^2 P2
  + t^3 P3,
t in [0,1].
```

The production evaluator must use the equivalent cubic de Casteljau recurrence
rather than a power-basis conversion:

```text
Q0 = lerp(P0, P1, t)
Q1 = lerp(P1, P2, t)
Q2 = lerp(P2, P3, t)

R0 = lerp(Q0, Q1, t)
R1 = lerp(Q1, Q2, t)

B(t) = lerp(R0, R1, t)
```

where each point interpolation is performed component-wise.

## 5. Authorized production capability

The first implementation work unit may add only:

- immutable `CubicBezier2` and `CubicBezier3` value types;
- exactly four ordered matching-dimension control points;
- read-only control-point access;
- exact `start_point()` and `end_point()` semantics;
- evaluation for one scalar parameter `t`;
- explicit parameter validation;
- a pure value-returning `reversed()` operation that reverses the control
  point order;
- deterministic focused tests;
- the minimum build integration required for those files/tests.

Exact file names and API spelling may be refined during implementation, but no
additional scientific capability may be added.

## 6. Parameter and failure semantics

The admitted parameter domain is exactly the closed normalized interval
`[0,1]`.

Evaluation rules:

1. NaN or infinity as `t` is `invalid_numeric`.
2. Finite `t < 0` or `t > 1` is `parameter_out_of_range`.
3. `t == 0` returns the first control point exactly.
4. `t == 1` returns the last control point exactly.
5. Interior evaluation must return a finite point or an explicit classified
   failure.
6. No parameter clamping is permitted.
7. No extrapolation is admitted.
8. No hidden epsilon or global tolerance is permitted.

The curve control points are existing validated finite `Point2`/`Point3`
values. The first curve type therefore does not need a second coordinate
validation policy.

A curve-specific error vocabulary may be introduced only for
curve-owned failures such as invalid parameter or out-of-range parameter. It
must not collapse existing numeric/geometry failures into success.

## 7. Determinism and representation semantics

The control-point order is part of curve value semantics.

For one curve `C = [P0,P1,P2,P3]`:

- repeated evaluation with the same finite inputs in one declared arithmetic
  environment must produce the same scientific result;
- `start_point(C) == P0`;
- `end_point(C) == P3`;
- `reversed(C) == [P3,P2,P1,P0]`;
- `reversed(reversed(C)) == C` exactly as a value;
- the reversed curve represents the same geometric locus with opposite
  parameter direction;
- for admitted comparison parameters, evaluation must verify
  `reversed(C)(t)` against `C(1-t)` under a declared exact or
  operation-owned proximity rule.

No topological identity is inferred from curve equality or point proximity.
No `CurveId` is introduced in this work unit.

## 8. Analytic and adversarial evidence

The focused implementation contract must include at least the following cases.

| Case | Required observation |
| --- | --- |
| Control-point order | All four control points are returned in declared order and remain immutable. |
| Endpoint zero | `evaluate(0)` equals `P0` exactly. |
| Endpoint one | `evaluate(1)` equals `P3` exactly. |
| Constant curve | Four identical control points evaluate to that point for representative parameters. |
| Linear cubic | Equally spaced collinear control points reproduce the corresponding linear parameterization at exact dyadic parameters. |
| General planar cubic | 2D interior evaluations agree with an independently implemented high-precision/reference Bernstein evaluator within an explicit case-owned residual policy. |
| General spatial cubic | 3D interior evaluations agree with an independently implemented high-precision/reference Bernstein evaluator within an explicit case-owned residual policy. |
| Reversal endpoints | Reversal swaps endpoints exactly. |
| Reversal involution | Double reversal exactly recovers the original four-point value. |
| Reversal interior | At selected dyadic parameters, reversed evaluation agrees with original evaluation at `1-t` under the declared case policy. |
| Frame equivariance | For exact signed-permutation/power-of-two Cartesian-frame fixtures, transforming all control points then evaluating agrees with transforming the evaluated point, without introducing a general transform type. |
| Signed zero | Signed-zero control-point components do not alter geometric acceptance through byte identity assumptions. |
| Invalid parameter | NaN, ±infinity, values below zero and above one are rejected with the declared error class. |
| Extreme finite points | Finite control points near safe extrema produce finite interior evaluations or an explicit failure; no NaN/Inf silently enters a valid point. |
| Repeatability | Repeated focused executions preserve declared semantic fields. |

## 9. Independent reference path

Production de Casteljau output must not be accepted solely because the same
algorithm is re-run in the test.

At least one nontrivial 2D and one nontrivial 3D fixture must be checked against
an independently coded reference path using the cubic Bernstein expression in
higher precision available to the test environment, with expected values and
residuals generated independently of the production evaluator.

The acceptance policy must:

- declare one characteristic coordinate scale per fixture;
- use the qualified Numeric Contract proximity machinery where a rounded
  comparison is required;
- record residual and limit;
- use no default tolerance;
- never use legacy AP Mesh output as an oracle.

Exactly representable fixtures should prefer exact equality.

## 10. Prerequisite preservation

Focused validation must preserve the current semantic prerequisite set:

- `apmesh_core.bootstrap_smoke`;
- `apmesh_core.numeric_contract`;
- `apmesh_core.geometry_primitives`;
- `apmesh_core.minimal_small_linear_algebra`;
- `apmesh_core.math_header_isolation`;
- `apmesh_core.cartesian_frames`;
- `apmesh_core.topological_model`.

The new curve focused test must be added without weakening or relabeling those
contracts.

Curve code may depend on qualified numeric/math/geometry primitives but must
not make the topology module depend on curves.

## 11. Explicit exclusions

This entry decision does not authorize:

- first or second derivatives;
- tangent, speed, regularity or zero-speed classification;
- curvature, torsion, normals or Frenet frames;
- arc length, numerical integration or parameter-to-length mapping;
- subdivision as a public API;
- adaptive sampling or any discrete polyline/trace;
- arbitrary-degree Bézier curves;
- rational Bézier curves, B-splines, NURBS or knot vectors;
- circular/elliptic exact rational representation;
- intersections, closest-point queries, projection or root finding;
- topological `CurveId`, edge ownership or curve/topology binding;
- surface, patch or trimming semantics;
- boundary discretization;
- triangular or Quad-Dominant meshing;
- optimization/adaptation;
- parallel execution;
- GPU/SIMD specialization;
- third-party runtime dependencies;
- native-Windows qualification; or
- a Curve Representation stage-level qualification claim.

## 12. Stop conditions

Stop the work unit and open a new scientific decision if implementation
requires any of the following:

- derivatives or regularity classification;
- an undeclared tolerance or approximation policy;
- a general transform abstraction;
- arbitrary degree or rational weights;
- public subdivision;
- topology ownership;
- adaptive sampling;
- numerical integration;
- parameter clamping/extrapolation;
- a third-party dependency;
- a change to a qualified prerequisite semantic contract.

A performance concern alone does not justify replacing de Casteljau in this
first bounded work unit. A later optimization requires equivalent scientific
evidence and a separate decision if it changes the numerical mechanism.

## 13. Focused completion gate

The first work unit may close only when:

1. the implementation remains inside Sections 5–6;
2. all required analytic/adversarial cases exist;
3. the independent reference path exists and reports its policy/residual;
4. the exact prerequisite semantic set passes;
5. GCC 13 Debug and Clang 18/libc++ Debug focused validation pass;
6. no new dependency or hidden tolerance is introduced; and
7. documentation records the implementation revision and retained exclusions.

Passing this gate establishes only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

It does not qualify Curve Representation.

## 14. Later Curve Representation stage-exit boundary

The roadmap already separates later investigation problems:

1. Cubic Bézier Evaluation;
2. Curve Derivatives and Regularity;
3. Arc Length and Parameter Mapping; and
4. Continuous Curve Geometry Regression.

The stage may become `QUALIFIED` only after those admitted continuous
capabilities are separately bounded, implemented, and followed by a dedicated
cumulative regression that:

- reruns all curve analytic/reference fixtures;
- reruns relevant qualified prerequisite semantics;
- checks reversal and admitted parameterization invariants;
- regenerates required curve/derivative/speed/arc-length error evidence;
- verifies repeated deterministic execution; and
- retains a revision-bound machine-readable certificate/evidence package.

No adaptive curve sampling or boundary discretization belongs to the Curve
Representation qualification gate.

## 15. Alternatives considered

### Direct cubic Bernstein formula in production

Rejected as the primary first implementation path. It is mathematically
equivalent, but the project prefers the simple recursive Bernstein/Bézier
evaluation mechanism with explicit interpolation semantics. The direct
Bernstein expression remains useful as an independent test reference.

### Power-basis conversion plus Horner evaluation

Rejected for the first work unit. It adds a basis conversion that is not needed
for the declared capability and weakens the direct correspondence between the
stored control polygon and the evaluation mechanism.

### Generic arbitrary-degree Bézier class

Deferred. The roadmap requires a bounded first investigation, and fixed degree
three is enough to establish the representation/evaluation semantics needed by
later curve work.

### Rational Bézier/NURBS immediately

Deferred. Rational weights, homogeneous evaluation, knot semantics and exact
conic representation create materially different numerical and API contracts.

### Curve hierarchy/base-class abstraction

Deferred. One fixed value type does not yet justify inheritance, dynamic
polymorphism, type erasure or a generic curve interface.

## 16. Decision effect

On integration of this decision, Curve Representation moves from
`NOT STARTED` to:

**IN INVESTIGATION / ENTRY DECISION APPROVED / NO PRODUCTION CURVE CODE YET.**

The sole next bounded work item becomes implementation of:

**Polynomial Cubic Bézier Value Representation and Evaluation.**

Derivatives and every later curve capability remain blocked pending separate
decisions.
