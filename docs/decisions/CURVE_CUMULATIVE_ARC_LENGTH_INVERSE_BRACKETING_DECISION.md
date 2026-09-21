# Cumulative Arc-Length Mapping and Certified Inverse Bracketing — Bounded Decision

Status: DECISION APPROVED / INTEGRATED / WORK UNIT 2A AUTHORIZED /
WORK UNIT 2B BLOCKED / STAGE UNQUALIFIED  
Date: 2026-09-21  
Stage: Curve Representation — Continuous Geometry Before Discretization

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud envelope;
- Polynomial Cubic Bézier Value Representation and Evaluation:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Cubic Bézier Differential Evaluation and Pointwise Speed:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Certified Global Cubic Regularity by Bernstein Speed-Squared Enclosure:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Certified Cubic Bézier Total Arc-Length Enclosure:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.

## 1. Question

What is the smallest scientifically defensible parameter-mapping capability
that can be added after certified total arc length, while preserving
conservative numeric evidence and deterministic serial semantics and without
collapsing cumulative length or inverse parameterization to an uncertified
lookup table or scalar approximation?

## 2. Decision

Open the investigation problem **Cumulative Arc-Length Mapping and Certified
Inverse Bracketing**, but split it into two sequential executable work units.

### Work Unit 2A — Certified Cumulative Arc-Length Enclosure

Authorize first only a conservative enclosure of

`S(t) = length(B|[0,t]) = integral_0^t ||B'(u)|| du`

for `t in [0,1]`.

The public scientific result must retain a finite lower/upper enclosure of the
mathematical prefix length together with explicit resource/convergence evidence.

This forward cumulative map does **not** require global regularity. For every
admitted cubic, including constant or singular curves, `S(t)` is defined and
nondecreasing.

### Work Unit 2B — Certified Inverse Arc-Length Bracketing

Define the future boundary now, but keep implementation blocked until Work Unit
2A is separately implemented, validated, integrated and closed.

A unique inverse parameter may be claimed only when the same curve is globally
certified regular, so that

`S'(t) = ||B'(t)|| > 0`

throughout the complete parameter interval and `S` is strictly increasing.

The inverse result must be a proven parameter bracket, not merely one floating
estimate.

## 3. Scientific sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| HKUST MATH 4223, “Arc-Length Parametrization”, Theorem 1.3, https://www.math.hkust.edu.hk/~mafong/math4223/p_1_1_02.html | For a regular curve, `S(t)=integral ||gamma'||` has positive derivative and therefore a strictly increasing invertible arc-length map. | A certified binary64 implementation or resource policy. | Require global regularity before any unique inverse claim. |
| Y. Gil and D. Keren, “New Approach to the Arc Length Parameterization Problem”, Spring Conference on Computer Graphics, 1997; author publication index: https://www.cs.haifa.ac.il/~dkeren/mypapers/index.html | Arc-length parameterization is a distinct mapping problem and common spline classes generally require numerical treatment. | That the paper's alternate curve family should replace AP Mesh cubic Béziers. | Keep parameter mapping explicit and separate from geometric curve representation. |
| M. Walter and A. Fournier, “Approximate Arc Length Parametrization”, SIBGRAPI 1996, https://www.visgraf.impa.br/sibgrapi96/trabs/abst/a14.html | Lookup/fitted approximations can provide practical length-vs-parameter maps, but retain approximation error. | A conservative certificate or a unique inverse under AP Mesh contracts. | Approximate tables/fitted curves may be diagnostic references only, never correctness authorities. |
| Existing `CURVE_ARC_LENGTH_PARAMETER_MAPPING_DECISION.md` and integrated total-length implementation | Conservative dyadic Bézier length enclosure, explicit resource evidence and private outward arithmetic are already accepted repository semantics. | Certified prefix-length construction or inversion. | Reuse/refactor the certified length machinery instead of introducing an unrelated integration method. |
| Existing global regularity certifier | The same cubic can be classified `regular`, exact `degenerate`, or `indeterminate` using complete Bernstein interval evidence. | Arc-length inversion by itself. | Future inverse work must bind regularity to the same curve rather than accept an unbound boolean assertion. |

No third-party runtime dependency is admitted.

## 4. Mathematical contract for Work Unit 2A

For one represented cubic Bézier curve `B:[0,1] -> R^d`, define

`S(t) = integral_0^t ||B'(u)|| du`.

Required exact mathematical properties:

1. `S(0)=0`;
2. `S(1)=L`, where `L` is total arc length;
3. `S(t)>=0`;
4. `S` is nondecreasing for every admitted curve;
5. for `0<=a<=b<=1`, `S(a)<=S(b)`;
6. under reversal `R(t)=B(1-t)`,
   `S_R(t)=L-S_B(1-t)`;
7. if `B` is globally regular, `S` is strictly increasing.

The implementation must return a conservative enclosure

`[S_lower(t), S_upper(t)]`

containing the true `S(t)`.

A point estimate alone is not an authorized scientific result.

## 5. Certified prefix construction

The production implementation may not obtain a “certified” prefix by simply
evaluating a rounded de Casteljau point and treating it as exact control data.

The intended conforming construction is:

1. treat original binary64 control coordinates as exact represented inputs;
2. construct the prefix cubic corresponding to `[0,t]` using private
   curve-local outward arithmetic, or an algebraically equivalent certified
   edge-vector construction;
3. retain interval/control data that encloses the exact mathematical prefix;
4. feed that representation through the same conservative
   chord/control-polygon length semantics used by certified total length;
5. accumulate lower bounds downward and upper bounds upward;
6. fail explicitly if a finite conservative enclosure cannot be retained.

The implementation may refactor the existing total-length core to expose a
private common certified-segment engine. It must not create a public generic
interval or public subdivision API.

## 6. Parameter and error semantics

Work Unit 2A must reject explicitly:

- NaN parameter;
- positive/negative infinity parameter;
- parameter below 0;
- parameter above 1;
- invalid length policy;
- inability to retain a finite conservative enclosure.

Endpoint semantics:

- `t=0` must return exact `[0,0]` with converged evidence;
- `t=1` must be scientifically equivalent to the integrated total-length
  enclosure under the same policy.

The implementation should reuse existing parameter/error vocabulary when doing
so does not collapse distinct scientific failure classes. A new public error
type requires demonstrated semantic need.

## 7. Cumulative convergence semantics

The existing total-length policy remains the default model:

- caller-controlled absolute tolerance;
- caller-controlled relative tolerance;
- maximum subdivision depth;
- maximum processed-node budget.

`converged` means the returned prefix enclosure proves the declared global
width condition.

`indeterminate` means a valid finite prefix enclosure exists but the width
condition was not proved before the declared resource limit.

Resource exhaustion must not fabricate a converged scalar.

## 8. Work Unit 2A required evidence

Focused contracts must include at least:

| Case | Required observation |
| --- | --- |
| `t=0` | Exact cumulative enclosure `[0,0]`. |
| `t=1` | Scientific equivalence with total-length enclosure under the same policy. |
| Axis-aligned straight cubic | Exact analytic `S(t)` for selected binary-exact parameters. |
| Constant curve | `S(t)=0` for every tested parameter. |
| Degree-elevated parabola | Prefix enclosure contains an independently evaluated analytic integral. |
| Monotone parameter sequence | Safe analytic fixtures do not contradict exact nondecreasing cumulative length. |
| Reversal | Forward/reversed cumulative enclosures satisfy the exact reversal relation conservatively. |
| 2D/3D embedding parity | Planar `z=0` embedding retains equivalent cumulative semantics. |
| Translation | Safe exactly representable translation does not change cumulative length evidence. |
| Power-of-two scale | Prefix enclosures and absolute tolerance scale consistently. |
| Interior stationary curve | Prefix length remains defined without a regularity prerequisite. |
| Endpoint-coincident curve | Prefix length is not inferred from endpoint displacement alone. |
| Zero-tolerance request | Nontrivial prefix may return `indeterminate`; policy is not relaxed. |
| Coarse resource budget | Returns a valid enclosure with `indeterminate`. |
| Invalid parameter | NaN, infinities and out-of-domain values fail explicitly. |
| Invalid policy | Existing invalid-policy cases remain rejected. |
| Finite extremes/subnormal scale | Conservative finite evidence or explicit numeric failure only. |
| Repeatability | Repeated serial calls produce equivalent evidence fields. |
| Header/dependency isolation | No topology/model/mesh/threading/third-party dependency is introduced. |

The analytic reference must be independent of the production cumulative
algorithm.

## 9. Work Unit 2A explicit exclusions

Work Unit 2A does not authorize:

- inverse arc-length mapping;
- normalized fraction-to-parameter mapping;
- target-length-to-parameter APIs;
- lookup tables or cached sampling tables;
- Newton, secant, Brent or other root-solving APIs;
- physical point sampling;
- equal-length point generation;
- public subdivision;
- curvature or tangent-frame features;
- topology ownership;
- surfaces;
- boundary discretization;
- quadrilateral generation;
- parallel evaluation;
- stage-level Curve Representation qualification.

## 10. Future Work Unit 2B inverse contract

Work Unit 2B remains blocked until Work Unit 2A closes.

Its future correctness contract must satisfy all of the following.

### Same-curve regularity authority

A unique inverse may be claimed only after the implementation establishes
`CurveRegularityResult::regular` for the **same curve**.

An unbound caller-provided boolean or detached “regular” evidence object is not
sufficient authority.

The future implementation should either:

- internally invoke the integrated regularity certifier with an explicit
  caller policy; or
- introduce a stronger curve-bound evidence mechanism through a separate
  decision.

### Absolute target length

For target `s`, a certified inverse bracket `[t_low,t_high]` must prove an
ordering equivalent to:

`S_upper(t_low) <= s <= S_lower(t_high)`.

That relation proves the true inverse lies in the retained parameter bracket.

If target membership in the total-length domain cannot be proved from
conservative evidence, the result is `indeterminate` or an explicit domain
failure; it is never silently clamped.

### Normalized target fraction

For a requested fraction `f in [0,1]`, total length is itself retained as an
enclosure. Therefore the exact target length is also uncertain:

`target in [f*L_lower, f*L_upper]`

with conservative multiplication.

A future fraction-to-parameter result must preserve a bracket valid for the
entire target interval. It must not choose a midpoint total length and treat it
as exact.

### Bracket-preserving inversion

Deterministic bisection or equivalent bracket refinement is the correctness
mechanism.

Newton or secant acceleration may be investigated later only as an optional
step inside an already proven bracket. Such acceleration must never be
required for correctness or convergence claims.

## 11. Future Work Unit 2B evidence boundary

A later inverse decision/implementation must cover at least:

- exact endpoints `s=0 -> t=0` and `s=L -> t=1`;
- exact fractions 0 and 1;
- straight cubic analytic inverses;
- nonuniform regular cubic fixtures;
- reversal;
- 2D/3D parity;
- translation;
- power-of-two scale;
- target just inside/outside the length domain;
- regularity `degenerate` and `indeterminate` rejection;
- prefix enclosure uncertainty preventing a split decision;
- resource-limited bracket result;
- deterministic repeatability.

These requirements are prospective and do not authorize Work Unit 2B yet.

## 12. Relationship to physical discretization

A certified mapping from arc length to parameter is still continuous-geometry
infrastructure.

It does not define:

- how many boundary samples to create;
- physical approximation-error tolerances;
- mesh sizing;
- gradation;
- shared trace identity;
- endpoint ownership;
- patch compatibility;
- meshing.

Those remain later scientific stages.

## 13. Admission and stop conditions for Work Unit 2A

Implementation of Work Unit 2A may begin only after this decision is merged,
ordinary post-merge validation passes, and the decision checkpoint is closed.

Stop the work unit and require a new decision if implementation requires:

- a public generic interval library;
- a new global tolerance;
- an uncertified floating prefix construction;
- a sampling/lookup approximation as authority;
- inverse mapping;
- global regularity as a prerequisite for forward cumulative length;
- topology/model ownership;
- physical discretization;
- parallel execution; or
- reinterpretation of the integrated total-length contract.

## 14. Stage effect

Approval of this decision does not qualify Curve Representation.

After Work Unit 2A and then Work Unit 2B are separately integrated and closed,
the stage still requires a separately pre-registered **Continuous Curve
Geometry Regression** before any stage-level qualification claim.

No Curve Differential Geometry, Boundary Curve Discretization, surface,
quadrilateral or parallel work is opened by this decision.

## 15. Next transition

If this decision is integrated and its checkpoint closes successfully, the
sole next executable work item is:

**Work Unit 2A — Certified Cumulative Arc-Length Enclosure.**

Work Unit 2B remains blocked.


## 16. Decision integration result

PR #63 integrated this decision as
`32428d29407949f058d44bf2dfdcab59600714c1`.

Validation:

- PR FAST `35581198802`: PASS;
- PR INTEGRATION `35581198681`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35581291836`: PASS;
- post-merge INTEGRATION `35581291965`: PASS.

Decision effect:

- Work Unit 2A — Certified Cumulative Arc-Length Enclosure: **AUTHORIZED**;
- Work Unit 2B — Certified Inverse Arc-Length Bracketing: **BLOCKED**;
- Curve Representation: **STAGE UNQUALIFIED**.

No production mapping behavior was introduced by this decision.


## 17. Work Unit 2A implementation result

Implementation branch:
`curve/cumulative-arc-length-enclosure`.

PR: #66.

The bounded candidate implements only Work Unit 2A.

Public behavior added:

- cumulative prefix enclosure for `CubicBezier2`;
- cumulative prefix enclosure for `CubicBezier3`;
- explicit non-finite/out-of-domain cumulative parameter errors.

The certified prefix is not constructed from rounded public curve points.
Instead, the implementation starts from interval-enclosed original Bézier edge
vectors, applies an outward interval de Casteljau construction for the exact
represented parameter, and passes the resulting three prefix edge enclosures to
the same conservative chord/control-polygon engine used by certified total
length.

The total-length engine was refactored only enough to accept already-certified
edge-vector input. Its existing total-length public semantics are retained.

Focused validation:

- FAST `35591468322`: PASS;
- INTEGRATION `35591468337`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The cumulative contract covers the required endpoint, analytic, reversal,
embedding, translation, scale, singular, resource, invalid-input, finite
extreme, subnormal and repeatability fixtures.

Decision effect at this point:

- Work Unit 2A:
  **IMPLEMENTED / FOCUSED CONTRACTS PASS / VALIDATED_UNMERGED**;
- Work Unit 2B: **BLOCKED**;
- Curve Representation: **STAGE UNQUALIFIED**.

Integration and post-merge closure remain required before Work Unit 2B may be
opened.


## 18. Work Unit 2A integration and closure

PR #66 integrated Certified Cumulative Arc-Length Enclosure as:

`3cfb580cae2e2d26e87e9dfcfeab0aade0a3a3be`.

Validation:

- final PR FAST `35591661827`: PASS;
- final PR INTEGRATION `35591661836`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug;
- post-merge FAST `35591774007`: PASS;
- post-merge INTEGRATION `35591773982`: PASS.

Closed result:

- Work Unit 2A:
  **IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
  NOT QUALIFIED**;
- Work Unit 2B:
  **UNBLOCKED FOR A SEPARATE BOUNDED WORK ITEM**;
- Curve Representation:
  **STAGE UNQUALIFIED**.

The 2A integration introduces no inverse mapping, physical sampling,
discretization, surface, quadrilateral or parallel capability.


## 19. Work Unit 2B implementation-decision activation

Work Unit 2A is integrated and closed.

A separate executable 2B contract is now under review in:

`docs/decisions/CURVE_INVERSE_ARC_LENGTH_BRACKETING_IMPLEMENTATION_DECISION.md`.

That child decision preserves the prospective requirements of Sections 10–12
and makes them explicit as a public policy/evidence/error contract before any
inverse production implementation begins.

Until the child decision is integrated and closed:

- Work Unit 2B production code is **NOT AUTHORIZED**;
- physical discretization remains blocked;
- Curve Representation remains stage-unqualified.
