# Curve Representation — Bounded Entry Decision

Status: ENTRY DECISION PROPOSED / IN REVIEW / NO PRODUCTION IMPLEMENTATION
Date: 2026-09-21
Stage: Curve Representation — Continuous Geometry Before Discretization
Prerequisites: Foundation QUALIFIED; Geometry Primitives QUALIFIED;
Topological Model QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04 x86_64
cloud envelope

## 1. Question

What is the smallest scientifically defensible continuous-curve capability that
can be introduced after Topological Model qualification without importing
curve differential geometry, arc-length integration, topology/geometry
ownership, discretization, surfaces, meshing, generic CAD hierarchy, or
third-party runtime dependencies?

## 2. Decision

Authorize one future implementation work unit named:

**Cubic Bézier 3D Immutable Representation and Evaluation on [0,1].**

The first work unit may represent exactly one non-rational cubic Bézier curve
in three-dimensional physical space using four already-valid `Point3`
control points and may evaluate that curve only on the exact closed parameter
domain `[0,1]`.

The production evaluation algorithm is the cubic de Casteljau construction
using coordinate-wise `std::lerp`. Reversal is represented by exact control-
point order reversal and the mathematical parameter map `t -> 1-t`.

This decision defines scientific semantics and evidence obligations. It does
not itself implement curve code and does not authorize derivatives, regularity,
arc length, discretization, or stage qualification.

## 3. Literature and reference basis

### Farin — Curves and Surfaces for CAGD

Farin's treatment of linear interpolation, de Casteljau evaluation, Bernstein
form, and Bézier properties supplies the mathematical reference for the first
work unit.

Decision impact:

- use the degree-three Bézier definition with four control points;
- use de Casteljau as the production evaluator;
- retain Bernstein form as an independent verification formulation;
- require endpoint interpolation, affine invariance, convex interpolation
  behavior, and reversal consistency;
- defer derivatives and all later curve interrogation to separate investigation
  problems.

### C++ `std::lerp`

The C++ working draft specifies that, for finite interpolation endpoints,
`std::lerp(a,b,0)` returns `a`, `std::lerp(a,b,1)` returns `b`, and
for `t` in `[0,1]` the result is finite.

Decision impact:

- use `std::lerp` for every coordinate-level de Casteljau interpolation;
- do not implement interpolation as an unchecked `a + t*(b-a)` expression;
- preserve exact endpoint behavior;
- treat any later contradiction of the finite-result assumptions as an
  explicit internal/domain failure rather than silently propagate NaN/Inf.

### Open CASCADE `Geom_BezierCurve`

OCCT is inspected as a mature CAD semantic comparison, not as an acceptance
oracle. Its Bézier API exposes canonical endpoint parameters `0` and `1`
and reversal through `u -> 1-u`, while also demonstrating that rational
weights, arbitrary degree, derivatives, mutation, segmentation, and
extrapolation are separate additional capabilities.

Decision impact:

- AP Mesh Core deliberately restricts the first work unit to degree three,
  non-rational, immutable data and `[0,1]`;
- OCCT is not a dependency and its numeric output is not used as expected
  scientific truth.

Full references are registered in
`docs/research/REFERENCE_REGISTER.md`.

## 4. Mathematical definition

For finite control points
`P0, P1, P2, P3 in R^3` and parameter `t in [0,1]`, define

`L(A,B,t) = (1-t)A + tB`.

The production evaluation is the degree-three de Casteljau recursion:

`Q0 = L(P0,P1,t)`
`Q1 = L(P1,P2,t)`
`Q2 = L(P2,P3,t)`

`R0 = L(Q0,Q1,t)`
`R1 = L(Q1,Q2,t)`

`C(t) = L(R0,R1,t)`.

The independent mathematical reference is the cubic Bernstein form:

`C(t) = (1-t)^3 P0
       + 3(1-t)^2 t P1
       + 3(1-t)t^2 P2
       + t^3 P3`.

Production and reference paths must not share an implementation routine.

## 5. Representation semantics

The first curve value is immutable after construction.

Required semantic state:

- exactly four `Point3` values;
- declared order `P0, P1, P2, P3`;
- no hidden tolerance;
- no curve ID;
- no edge ID;
- no ownership relation to topology;
- no mutable control-point editing;
- no rational weights;
- no cached discretization;
- no cached derivative or length state.

Because `Point3` construction already rejects non-finite coordinates, a curve
constructed only from `Point3` values has finite control points by type
precondition.

Representation equality, if exposed, means exact equality of the four stored
control points in declared order. It is not geometric equivalence of different
parameterizations.

## 6. Parameter and failure semantics

The admitted parameter domain is exactly the closed interval `[0,1]`.

Required classifications:

- finite `t < 0`: `parameter_out_of_domain`;
- finite `t > 1`: `parameter_out_of_domain`;
- NaN or ±infinity: `non_finite_parameter`;
- `-0.0` and `+0.0`: admitted endpoint zero;
- `1.0`: admitted endpoint one;
- impossible/non-finite final point under otherwise admitted preconditions:
  explicit `non_finite_result` defensive failure.

No clamping, modulo mapping, extrapolation, retry, fallback, global epsilon, or
implicit parameter normalization is admitted.

The exact error-enum spelling may be selected by the implementation, but the
three distinctions above must remain observable and must not be collapsed.

## 7. Reversal semantics

For control points `[P0,P1,P2,P3]`, the reversed curve is exactly

`[P3,P2,P1,P0]`.

Required invariant:

`reverse(C).evaluate(t) == C.evaluate(1-t)`

mathematically for all admitted `t`.

For exact dyadic fixtures whose arithmetic is exactly representable, the
focused test requires exact coordinate equality.

For non-exact fixtures, comparison uses an explicit test-owned Numeric Contract
policy and records residual and bound. No approximate comparison is part of the
production curve API.

Double reversal must recover the exact original representation.

## 8. Affine-consistency semantics

A polynomial Bézier curve commutes mathematically with affine maps.

The first work unit must verify this property without adding a production
general-affine-transform abstraction.

Focused tests may define independent test-only affine maps with exactly
representable signed permutations, power-of-two scalings, shears/translations,
or other bounded fixtures and compare:

`A(C(t))`

against

`C_A(t)`

where `C_A` is constructed from `A(P0)...A(P3)`.

Exactly representable fixtures require exact equality. Rounded fixtures require
an explicit test-owned error bound.

The qualified `CartesianFrame3` API may also be used for prerequisite
preservation but is not promoted into a generic affine-transform API by this
decision.

## 9. Independent reference evaluation

The focused evidence must include an independent high-precision reference that
does not call production de Casteljau code.

Required reference path:

- Python standard-library `decimal`;
- at least 80 decimal digits of working precision;
- Bernstein-form evaluation;
- exact conversion of each binary64 input through `Decimal.from_float`;
- explicit output of reference coordinates and residuals.

No Python package dependency is admitted.

For non-dyadic fixtures, comparison must use a predeclared test-only
scale-aware bound. The characteristic length scale must be translation
invariant and derived from the control polygon, not from absolute world
coordinates. A zero-extent constant curve uses exact endpoint/constant
invariants instead of inventing a positive scale.

The comparison bound is evidence policy only. It is not a geometric coincidence
tolerance and must never affect production curve evaluation or topology.

## 10. Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Endpoint zero | `C(0) == P0` exactly. |
| Endpoint one | `C(1) == P3` exactly. |
| Signed zero parameter | `C(-0.0) == C(+0.0) == P0`. |
| Constant curve | Four equal control points evaluate to the same point for every tested admitted parameter. |
| Exact linear fixture | Collinear equally spaced binary-exact controls reproduce the expected linear parameterization at declared dyadic parameters. |
| General spatial cubic | A non-planar four-point fixture agrees with independent high-precision Bernstein evaluation. |
| Midpoint/dyadic set | `t = 1/4, 1/2, 3/4` satisfies independently derived exact or bounded expectations. |
| Reversal | Reversed-control evaluation at `t` agrees with original evaluation at `1-t`; double reversal restores representation exactly. |
| Affine consistency | Test-only affine transformation of the evaluated point agrees with evaluation of transformed controls. |
| Translation stress | Large exactly representable translation does not change the declared affine-consistency result or reference scale. |
| Finite extremes | Finite `lowest/max/min/denorm` control-coordinate fixtures remain finite for admitted parameters when the standard interpolation contract applies. |
| Opposite extremes | Interpolation between opposite finite extremes at interior admitted parameters remains finite. |
| NaN parameter | Explicit `non_finite_parameter`. |
| Infinite parameter | Explicit `non_finite_parameter`. |
| Below domain | `nextafter(0,-inf)` and representative negative values are rejected without clamp. |
| Above domain | `nextafter(1,+inf)` and representative values above one are rejected without extrapolation. |
| Repeatability | Repeated evaluation of the same representation/parameter produces identical structured results inside one declared environment. |
| Isolation | Curve evaluation has no topology, filesystem, environment, logging, randomness, discretization, or meshing dependency. |

## 11. Focused contract required to close the first work unit

The first implementation work unit may close only when:

1. the immutable 3D cubic representation exists;
2. evaluation is restricted to `[0,1]`;
3. the production path is de Casteljau using `std::lerp`;
4. the complete case table above is represented by executable evidence;
5. the independent Decimal/Bernstein reference path passes;
6. reversal and affine-consistency evidence pass;
7. no default/global tolerance enters production;
8. public-header isolation passes;
9. FAST passes;
10. INTEGRATION passes in the currently required GCC/Clang cells;
11. all qualified prerequisite semantic tests remain passing.

Passing the focused work-unit contract does not qualify the Curve
Representation stage.

## 12. Explicit exclusions

This entry decision does not authorize:

- first, second, or higher derivatives;
- tangent, speed, regularity, zero-speed classification, inflection, curvature,
  torsion, Frenet frames, or any differential geometry;
- arc length, numerical integration, inverse arc-length mapping, closest-point
  or parameter inversion;
- subdivision, clipping, root finding, intersection, bounding boxes, or
  distance queries;
- adaptive or uniform sampling;
- polyline or mesh discretization;
- topological `CurveId`, edge-to-curve association, ownership, trimming, or
  shared-boundary semantics;
- rational Bézier curves, weights, arbitrary degree, B-splines, NURBS,
  composite curves, degree elevation/reduction, or knot vectors;
- 2D curve API in the first work unit;
- general affine-transform production types;
- surfaces, patches, CAD import/export, meshing, optimization, adaptation, or
  parallel execution;
- third-party runtime dependencies;
- legacy AP Mesh output as an oracle.

## 13. Stop conditions

Stop the first work unit and open a new bounded decision if implementation
requires any of the following:

- derivative or regularity semantics;
- an operation-specific geometric tolerance in production;
- extrapolation outside `[0,1]`;
- rational weights or arbitrary degree;
- topological curve identity or edge association;
- generic runtime polymorphism for curve families;
- a third-party numeric/geometry dependency;
- hidden non-deterministic state;
- reinterpretation of qualified Point3 or Numeric Contract semantics.

A failure of any previously qualified prerequisite is a regression and blocks
curve progression until resolved.

## 14. Stage progression after the first work unit

The Curve Representation stage remains decomposed into at most three levels.

Scientific Stage:

**Curve Representation — Continuous Geometry Before Discretization**

Investigation problems, in order:

1. **Cubic Bézier Evaluation**
   - first executable work unit authorized by this decision;
2. **Curve Derivatives and Regularity**
   - opened only after evaluation focused closure;
3. **Arc Length and Parameter Mapping**
   - opened only after derivative/regularity semantics are closed;
4. **Continuous Curve Geometry Regression**
   - stage-exit regression after all preceding investigation problems.

No discretization work may begin before Curve Representation stage
qualification.

## 15. Later stage-exit regression boundary

The eventual Continuous Curve Geometry Regression must be pre-registered
separately. At minimum it must:

- rerun all curve evaluation fixtures;
- rerun derivative/regularity fixtures once those capabilities exist;
- rerun arc-length/mapping fixtures once those capabilities exist;
- rerun all relevant qualified Foundation, Geometry, and Topological Model
  prerequisite semantic tests on the current candidate;
- execute deterministic repeated runs in the admitted matrix;
- compare machine-readable curve certificates;
- independently regenerate declared curve/reference/error tables and any
  required figures;
- verify reversal and admitted reparameterization invariants;
- retain exact source/toolchain/input hashes;
- classify all differences;
- produce a human terminal audit before qualification.

The stage exit gate remains:

**continuous curve geometry is qualified before any adaptive sampling or
boundary discretization is introduced.**

## 16. Effect on roadmap and next action

On merge of this decision, Curve Representation moves from `NOT STARTED` to
`IN INVESTIGATION / ENTRY DECISION APPROVED`.

The next bounded work item is exactly:

**Cubic Bézier 3D Immutable Representation and Evaluation on [0,1].**

No other curve capability is admitted by this entry decision.
