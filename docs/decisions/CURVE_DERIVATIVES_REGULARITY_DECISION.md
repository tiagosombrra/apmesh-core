# Curve Derivatives and Regularity — Bounded Decision

Status: IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATION PENDING / STAGE UNQUALIFIED
Date: 2026-09-20
Stage: Curve Representation — Continuous Geometry Before Discretization
Prerequisites:
- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud envelope;
- Polynomial Cubic Bézier Value Representation and Evaluation:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED, integrated as
  `bde874311d9960c5fab7ce03b26b6cd5fbd61b34`.

## Question

What is the smallest scientifically defensible differential capability that can
be added to the fixed cubic polynomial Bézier representation without
prematurely claiming global regularity, curvature, arc length, discretization,
or topology ownership?

## Decision

Authorize one bounded implementation work unit named:

**Cubic Bézier Differential Evaluation and Pointwise Speed.**

The work unit may evaluate the first and second derivatives of
`CubicBezier2` and `CubicBezier3` on the same closed normalized parameter
interval `[0,1]`, and may evaluate the pointwise Euclidean speed
`||B'(t)||`.

The first derivative must be evaluated as the quadratic Bézier hodograph of the
cubic curve. The second derivative must be evaluated as the linear derivative
of that hodograph. The implementation must remain in Bézier/Bernstein form and
must not convert the original curve to power-basis coefficients.

A zero first derivative is a valid differential result. Pointwise speed zero is
therefore a valid observation, not an error.

This work unit deliberately does **not** authorize a public or scientific
`is_regular()` result for the complete curve. In differential geometry a
parametrized curve is regular only when its derivative is nonzero at every
parameter in its domain. Evaluating finitely many parameters cannot establish
that interval-wide statement.

The decision therefore distinguishes:

1. derivative evaluation at a supplied parameter;
2. pointwise speed / zero-speed observation at that supplied parameter; and
3. global regularity certification on the whole interval.

Only the first two are authorized here.

## Sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| Gerald Farin, *Curves and Surfaces for CAGD*, 5th ed., Bézier derivative/hodograph treatment | Derivatives of polynomial Bézier curves can be represented through finite differences of the control polygon and evaluated in Bernstein form. | A specific AP Mesh API or a global floating-point regularity classifier. | Preserve Bézier form and derive differential control vectors from the existing cubic controls. |
| C.-K. Shene, Michigan Technological University, “The Hodograph (First Derivative) Window and Second Derivative Window”, https://pages.mtu.edu/~shene/COURSES/cs3621/LAB/curve/1st-2nd.html | The derivative of a degree-`p` Bézier is a degree-`p-1` Bézier; the second derivative follows by differentiating the hodograph again. | A runtime dependency, acceptance oracle, or robust interval regularity proof. | Evaluate the cubic first derivative as a quadratic Bézier and the second derivative as a linear Bézier. |
| Open CASCADE `Geom2d_BezierCurve` reference, https://dev.opencascade.org/doc/refman/html/class_geom2d___bezier_curve.html | Mature CAD geometry exposes first and second derivative evaluation separately and reverses Bézier parameter direction through `1-u`. | That OCCT error/tolerance policy or mutable curve API belongs in AP Mesh. | Keep derivative evaluation separate from curve ownership and verify the analytic reversal identities. |
| University of Colorado differential-geometry notes, “Velocity, regularity, and speed”, https://math.colorado.edu/~casa/teaching/26fall/4230/lecture_notes/index.html | A curve is regular at `t` when `γ'(t) != 0`; it is globally regular only when this holds throughout the domain; speed is `||γ'(t)||`. | That sampled floating-point speed values certify interval-wide regularity. | Permit pointwise speed evidence while explicitly deferring global regularity certification. |

These sources define mathematical vocabulary and independent verification
expectations only. No external geometry library becomes a dependency or an
acceptance oracle.

## Mathematical semantics

For the already admitted ordered cubic control points
`P0, P1, P2, P3`, define the first-derivative control vectors:

`D0 = 3(P1 - P0)`

`D1 = 3(P2 - P1)`

`D2 = 3(P3 - P2)`.

Then:

`B'(t) = (1-t)^2 D0 + 2(1-t)t D1 + t^2 D2`.

Production evaluation must treat this as a quadratic Bézier vector curve,
using recursive component-wise linear interpolation rather than converting to a
power basis.

Define the second-derivative control vectors from the hodograph:

`E0 = 2(D1 - D0)`

`E1 = 2(D2 - D1)`.

Then:

`B''(t) = (1-t) E0 + t E1`.

Equivalently,

`E0 = 6(P2 - 2P1 + P0)`

and

`E1 = 6(P3 - 2P2 + P1)`,

but production code should prefer the already formed first-derivative control
vectors so that failure handling and the mathematical construction remain
explicit.

Pointwise speed is:

`v(t) = ||B'(t)||`

using the already qualified stable Euclidean vector norm.

No unit tangent is authorized in this work unit because normalization is
undefined at zero speed and would introduce a regularity-dependent semantic
boundary.

## Parameter and error semantics

Derivative and speed queries use the same parameter contract as value
evaluation:

- parameter must be finite;
- parameter must lie in the closed interval `[0,1]`;
- `+0.0` and `-0.0` both denote the first endpoint parameter;
- no clamping;
- no extrapolation;
- no fallback algorithm.

Existing parameter errors remain applicable:

- `CurveError::non_finite_parameter`;
- `CurveError::parameter_out_of_domain`.

Construction of derivative control vectors and their evaluation may fail if
otherwise finite point coordinates produce a non-finite vector arithmetic
result. Such a failure must remain explicit and distinct from parameter
failure. The existing `CurveError::non_finite_result` may be reused if it
preserves that distinction; no new error category is required merely for API
symmetry.

A mathematically zero first derivative is not a failure. It produces a zero
vector and pointwise speed zero.

## Required invariants and hypotheses

### First derivative agreement

For every admitted cubic and valid parameter, the returned first derivative
must agree with the independently derived quadratic hodograph expression under
the declared exact/proximity policy.

### Second derivative agreement

For every admitted cubic and valid parameter, the returned second derivative
must agree with the independently derived linear hodograph derivative under the
declared exact/proximity policy.

### Endpoint derivatives

Where the qualified arithmetic yields finite results:

- `B'(0) = D0`;
- `B'(1) = D2`;
- `B''(0) = E0`;
- `B''(1) = E1`.

Exactly representable fixtures require exact equality.

### Reversal

For `R = reverse(B)`:

- `R'(t) = -B'(1-t)`;
- `R''(t) = B''(1-t)`;
- `speed_R(t) = speed_B(1-t)`.

Exact fixtures use exact equality where the qualified operations permit it;
general floating-point fixtures use explicit `ProximityPolicy`.

### Translation invariance

Translating every control point by one vector does not change `B'(t)`,
`B''(t)`, or pointwise speed, subject only to ordinary finite-result
requirements.

### Admitted-frame covariance

For an already qualified Cartesian signed-permutation / power-of-two similarity
frame, mapping all control points and then differentiating must agree with
mapping the derivative vectors using the frame's vector transformation.

No new general transform abstraction is authorized.

### Zero-speed observation

If the evaluated first derivative is exactly the zero vector for an analytic
fixture, pointwise speed must be exactly zero and no error may be emitted.

A nonzero representable derivative, including a selected subnormal-magnitude
fixture when supported by the qualified vector/norm semantics, must not be
silently converted to zero by an arbitrary tolerance.

This is a pointwise computational observation only.

### Determinism and isolation

For identical control points and parameter, derivative and speed results are
deterministic within the declared compiler/library envelope and depend on no
filesystem, clock, locale, random state, topology identity, thread schedule,
unordered traversal, or global tolerance.

## Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| 2D endpoint first derivative | Exact `3(P1-P0)` and `3(P3-P2)` for exactly representable controls. |
| 3D endpoint first derivative | Same endpoint hodograph requirement in 3D. |
| Second-derivative endpoints | Exact `E0` and `E1` on exactly representable fixtures. |
| Constant cubic | First derivative, second derivative and speed are exactly zero for representative parameters. |
| Linear-equivalent cubic | Controls on an arithmetic progression produce constant first derivative, zero second derivative and constant positive speed. |
| Quadratic-equivalent fixture | Independent analytic expectations verify varying first derivative and constant second derivative. |
| Genuine cubic fixture | First and second derivatives agree with independent long-double Bernstein/hodograph references at interior parameters. |
| Interior stationary point | A fixture such as scalar controls `0,1,1,0` embedded in 2D/3D has exact zero first derivative at `t=1/2`; speed is zero there without a global-regularity claim. |
| Endpoint stationary point | Repeated initial or terminal controls produce the expected zero endpoint derivative. |
| Reversal first derivative | `R'(t) = -B'(1-t)`. |
| Reversal second derivative | `R''(t) = B''(1-t)`. |
| Reversal speed | Pointwise speed is preserved under `t ↔ 1-t`. |
| Translation invariance | Translating every control point leaves derivative vectors and speed unchanged. |
| 2D admitted-frame covariance | Frame-mapped derivative vectors agree with differentiating frame-mapped controls. |
| 3D admitted-frame covariance | Same covariance in 3D. |
| Extreme finite controls | Deliberate derivative overflow is reported explicitly; no infinity/NaN escapes as a valid vector. |
| Small nonzero derivative | Selected representable small derivative remains nonzero; no tolerance-based zero classification is introduced. |
| Invalid parameter | NaN, ±infinity, finite below-domain and finite above-domain inputs retain the exact curve parameter failures. |
| Signed-zero parameter | `+0.0` and `-0.0` produce the same endpoint derivatives/speed semantics. |
| Repeatability | Repeated derivative/speed queries reproduce equivalent scientific results. |
| Header/dependency isolation | Differential API introduces no topology/model/meshing/I/O/threading/third-party dependency. |

Expected values must be derived independently from the production derivative
helpers. The legacy AP Mesh implementation is not an oracle.

## Pointwise versus global regularity boundary

For the mathematical curve, regularity at one parameter means
`B'(t) != 0`. Global regularity on `[0,1]` means this is true for every
parameter.

This work unit permits tests to establish pointwise zero or nonzero derivative
behavior for analytic fixtures. It does not permit:

- inferring global regularity from a parameter grid;
- using a fixed epsilon to declare a derivative zero or nonzero;
- exposing `is_regular()` or equivalent interval-wide classification;
- silently assuming that endpoint or representative interior checks cover the
  interval.

A future **Global Cubic Regularity Certification** decision may investigate a
mathematically complete interval method, including simultaneous roots of the
quadratic vector hodograph, endpoint cases, finite arithmetic/error
classification, and independent certification evidence.

That future problem is separate from derivative evaluation.

## Focused validation boundary

The work unit must provide a focused CTest contract for differential evaluation
and pointwise speed in both 2D and 3D.

Before integration it must pass at minimum:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

It must preserve the current accepted semantic prerequisite set including:

- Numeric Contract;
- Geometry Primitives;
- Minimal Small Linear Algebra / header isolation where selected by the current
  profiles;
- Cartesian Frames;
- Topological Model;
- Cubic Bézier value/evaluation;
- Curve public-header isolation.

Passing this focused evidence permits only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

It does not qualify Curve Representation and does not establish global
regularity.

## Explicit exclusions

This decision does not authorize:

- global interval regularity certification or `is_regular()`;
- root finding/isolation for `B'(t)=0`;
- unit tangent normalization as a public curve result;
- curvature, torsion, Frenet frames, inflection or feature classification;
- arc length, quadrature, integration diagnostics, inverse arc-length mapping
  or reparameterization;
- subdivision/splitting APIs, degree conversion, fitting or interpolation;
- rational Bézier curves, arbitrary degree, B-splines, NURBS or conics;
- topology-owned curves, `CurveId`, edge/curve association, trimming or
  model serialization;
- closest-point, intersection or self-intersection classification;
- boundary discretization, sizing, metrics, surfaces, meshing, optimization or
  adaptation;
- quadrilateral construction;
- SIMD, GPU, OpenMP, MPI, task systems or any parallel execution;
- third-party runtime dependencies;
- native-Windows qualification;
- a formal Curve Representation qualification campaign.

## Admission and stop conditions

Implementation may proceed only if:

1. first/second derivative evaluation remains tied to the fixed cubic
   polynomial Bézier representation;
2. derivative construction/evaluation remains in Bernstein/Bézier form;
3. zero derivative remains valid data rather than an exception;
4. pointwise speed uses the qualified stable norm and introduces no hidden
   tolerance;
5. global regularity is not inferred from sampling;
6. reversal, translation and admitted-frame relations are covered;
7. the full focused case table is represented by tests;
8. no new dependency or threading path is introduced; and
9. all selected prerequisite semantics remain passing.

Stop and request a separate decision if implementation requires interval root
certification, a tolerance-based regularity rule, tangent normalization,
curvature, integration, rational/arbitrary-degree representation, topology
ownership, or a new numeric acceptance policy.

## Alternatives considered

- **Implement derivatives plus global regularity in one work unit:** rejected
  because derivative evaluation is local and direct, while proving
  `B'(t) != 0` for every `t∈[0,1]` is an interval certification problem.
- **Classify regularity from dense sampling:** rejected because absence of an
  observed zero on finitely many samples does not establish absence of a zero
  between samples.
- **Use a global speed epsilon:** rejected because it would reintroduce a
  context-free tolerance explicitly forbidden by the Numeric Contract.
- **Introduce unit tangents immediately:** deferred because normalization
  requires a regular/nonzero derivative at the queried point and is not needed
  to validate the derivative formulas themselves.
- **Compute derivatives by finite differences of evaluated curve points:**
  rejected because the exact polynomial derivative is available analytically,
  while numerical differencing adds a step-size policy and avoidable error.
- **Convert the cubic to power-basis coefficients and differentiate:** rejected
  because the existing representation and evidence are Bézier/Bernstein based,
  and no downstream need justifies basis conversion.
- **Proceed directly to arc length:** rejected because speed semantics must
  first be independently implemented and validated.

## Effect on the roadmap if integrated

Curve Representation remains:

**IN INVESTIGATION / CUBIC VALUE-EVALUATION IMPLEMENTED /
FOCUSED CONTRACTS PASS / STAGE UNQUALIFIED.**

The next bounded work item after this decision is integrated, validated and
closed is implementation of **Cubic Bézier Differential Evaluation and
Pointwise Speed** only.

A global regularity certificate remains a later decision within the Curve
Representation stage. Arc length cannot open until derivative/speed semantics
are integrated and closed.

No curvature, discretization, quadrilateral, or parallel work is authorized.


## Integration checkpoint

PR #50 integrated this bounded decision as
`9c3caa35b580402fa0d7ce71412f3def7bbd8aa4`.

Validation:

- PR FAST `35542378763`: PASS;
- PR INTEGRATION `35542378797`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35542416703`: PASS;
- post-merge INTEGRATION `35542416695`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug.

No production implementation was included in the decision work item.

The decision checkpoint is closed. The sole next bounded work item is
implementation of **Cubic Bézier Differential Evaluation and Pointwise Speed**
within the scope fixed above.


## Implementation candidate

Branch:
`curve/cubic-bezier-differential-evaluation`.

The bounded candidate adds only:

- first-derivative evaluation as the quadratic cubic-Bézier hodograph;
- second-derivative evaluation as the linear derivative of that hodograph;
- pointwise speed through the qualified stable vector norm;
- focused 2D/3D differential evidence and public-header isolation.

Production does not convert to the power basis and does not use finite
differences.

Focused validation on PR #52:

- FAST `35544242913`: PASS;
- INTEGRATION `35544242911`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The complete required focused case table is represented in
`tests/curve_differential.cpp`, while the original
`tests/curve_representation.cpp` remains a separate regression authority for
value/evaluation semantics.

This evidence supports only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

No global regularity result, arc-length result or Curve Representation
qualification follows from this implementation candidate.
