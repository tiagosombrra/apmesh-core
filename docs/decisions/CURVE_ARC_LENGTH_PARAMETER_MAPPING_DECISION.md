# Arc Length and Parameter Mapping — Bounded Decision

Status: WORK UNIT 1 IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
WORK UNIT 2 DECISION PENDING / STAGE UNQUALIFIED  
Date: 2026-09-20  
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
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.

## 1. Question

What is the smallest scientifically defensible arc-length capability that can be
added to the fixed polynomial cubic Bézier representation while preserving
deterministic serial semantics and without prematurely introducing
parameter inversion, adaptive physical discretization, curvature, surfaces,
quadrilateral meshing, or parallel execution?

## 2. Decision

Open the investigation problem **Arc Length and Parameter Mapping**, but
authorize only one first implementation work unit:

**Certified Cubic Bézier Total Arc-Length Enclosure.**

The first work unit must return a conservative numeric enclosure

`[lower_length, upper_length]`

that contains the mathematical total arc length of the represented cubic
Bézier curve on `[0,1]`.

It must not return only a naked scalar approximation.

The enclosure must be obtained by deterministic dyadic de Casteljau
subdivision and the geometric fact that, for each polynomial Bézier segment,
the endpoint chord length is a lower bound on arc length and the control-polygon
length is an upper bound. Repeated subdivision must monotonically refine the
geometric enclosure in exact arithmetic; floating-point implementation must
preserve containment conservatively.

**Cumulative arc-length mapping and inverse parameter mapping are explicitly
deferred to the next bounded work unit.**

This decision defines the future mapping boundary now so that the total-length
API cannot accidentally become a discretization or reparameterization API.

## 3. Scientific sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| Jens Gravesen, “Adaptive subdivision and the length and energy of Bézier curves”, *Computational Geometry* 8(1), 1997, DOI 10.1016/0925-7721(95)00054-2 | For subdivided Bézier pieces, total chord length is below the true length and total control-polygon length is above it; repeated subdivision closes the gap. | That a particular stopping policy or floating-point implementation is automatically certified. | Use chord/control-polygon bounds as the primary scientific enclosure, not an opaque scalar quadrature estimate. |
| Gerald Farin, *Curves and Surfaces for CAGD*, 5th ed. | Standard Bézier/de Casteljau subdivision semantics, convex-hull/control-polygon geometry, derivative and parameterization theory. | That arbitrary-degree/rational/CAD support belongs in this work unit. | Keep the capability polynomial cubic and use de Casteljau subdivision already consistent with the curve representation. |
| Boost.Math Gauss–Kronrod documentation and the classical QUADPACK lineage | Adaptive Gauss–Kronrod provides efficient quadrature and an a-posteriori error estimate for smooth integrands. | A rigorous enclosure of the true integral without additional assumptions/validated arithmetic. | Do not use a Gauss–Kronrod difference as the scientific certificate in this work unit; it may later be used only as independent diagnostic/reference evidence. |
| Existing AP Mesh Numeric Contract and private curve interval-enclosure machinery | Non-finite behavior, explicit policy, outward/conservative arithmetic, deterministic evidence and no global epsilon are already repository authorities. | That existing regularity intervals directly provide arc length. | Extend only the private curve-local conservative machinery needed to enclose length; do not create a public generic interval library. |

External libraries and publications are semantic/reference sources only. No new
runtime dependency is admitted.

## 4. Mathematical contract

For a cubic Bézier segment with ordered control points
`P0,P1,P2,P3`, define:

- chord length  
  `Lc = ||P3 - P0||`;

- control-polygon length  
  `Lp = ||P1-P0|| + ||P2-P1|| + ||P3-P2||`.

For the exact mathematical segment:

`Lc <= L <= Lp`

where `L` is its total arc length.

After any complete de Casteljau subdivision partition into leaves
`C_i`:

`sum_i Lc_i <= L <= sum_i Lp_i`.

The implementation must retain floating-point lower and upper quantities that
conservatively enclose these mathematical sums.

No claim stronger than containment is authorized by this first work unit.

## 5. Floating-point containment requirement

A plain evaluation of Euclidean distances followed by ordinary summation is not
sufficient evidence for the word “certified”.

The implementation must construct bounds conservatively.

The intended approach is:

1. treat input binary64 control coordinates as exact represented values at the
   public boundary;
2. use private curve-local outward arithmetic for subdivision coordinates;
3. compute conservative lower/upper Euclidean-distance bounds for interval
   endpoint/control-point boxes;
4. accumulate global lower bounds downward and upper bounds upward;
5. return explicit failure if finite conservative enclosure cannot be
   maintained.

The existing private interval helper may be extended only with the minimal
operations needed by this algorithm. This does not authorize a public interval
arithmetic API.

A future implementation may choose an equivalent conservative construction if
the focused evidence proves the same containment semantics before integration.

## 6. Public scientific vocabulary

The first work unit may introduce an explicit policy equivalent to:

```text
CurveLengthPolicy
    absolute_tolerance
    relative_tolerance
    max_subdivision_depth
    max_processed_nodes
```

and an evidence/result object equivalent to:

```text
CurveLengthEvidence
    result: converged | indeterminate
    lower_length
    upper_length
    processed_nodes
    accepted_leaves
    max_depth_reached
```

Exact C++ spelling is not frozen by this decision.

The API must distinguish policy/numeric failure from a scientifically valid but
resource-limited `indeterminate` result.

At minimum, errors must distinguish:

- invalid policy;
- non-finite enclosure / inability to retain conservative finite bounds.

No exception-driven normal flow, hidden retry, hidden tolerance, or silent
scalar fallback is authorized.

## 7. Stopping semantics

Let the root segment provide one finite conservative upper bound `U0`.

Define an explicit global target width from the caller policy:

`target = max(abs_tol, rel_tol * U0)`.

No global repository epsilon participates.

A conforming deterministic subdivision algorithm must only report
`converged` when its final retained global enclosure satisfies:

`upper_length - lower_length <= target`.

Resource exhaustion before this condition is met must return
`indeterminate` together with the best valid enclosure accumulated so far.

The implementation may use leaf-local budget allocation or an equivalent
deterministic strategy, but the final global inequality above is mandatory.

Policies must reject:

- NaN or infinity in tolerance fields;
- negative tolerances;
- zero processed-node budget;
- subdivision depth outside the explicitly supported bound.

Both tolerances may be zero. In that case the caller requests an exact-width
enclosure; a nontrivial curve is expected to become `indeterminate` when the
resource budget is exhausted rather than silently relaxing the request.

## 8. Scope of the first implementation work unit

Authorized:

- total arc-length enclosure for `CubicBezier2` and `CubicBezier3`;
- deterministic dyadic midpoint subdivision;
- conservative chord lower bounds;
- conservative control-polygon upper bounds;
- explicit absolute/relative policy;
- resource-bounded `converged/indeterminate` evidence;
- exact zero length for an all-equal curve when conservatively provable;
- focused analytic/adversarial tests;
- preservation of existing Curve Representation and qualified prerequisite
  contracts;
- private curve-local arithmetic helpers needed solely for conservative
  enclosure.

## 9. Explicit exclusions

The first work unit does **not** authorize:

- cumulative length `S(t)` as a public API;
- inverse arc-length mapping `t(S)` or normalized fraction-to-parameter
  mapping;
- Newton, secant, or root-finding APIs;
- parameter lookup tables;
- physical curve sampling or adaptive discretization;
- equidistant point generation;
- public curve splitting/subdivision APIs;
- curvature, tangent frames, torsion, features, or inflections;
- rational Bézier, arbitrary degree, B-splines, NURBS, composite curves, or
  CAD-kernel integration;
- topological ownership or `EdgeId ↔ Curve` association;
- surface or patch geometry;
- sizing fields, boundary tracing, mesh generation, optimization or adaptation;
- quadrilateral generation;
- parallel evaluation, OpenMP, MPI, task systems, SIMD or GPU paths;
- a Curve Representation stage-level qualification campaign.

Quadrilateral work remains before parallelism in the global roadmap. Neither is
admitted by this decision.

## 10. Required invariants

### Containment

For every successful evidence object:

`0 <= lower_length <= true_length <= upper_length`.

### Result semantics

`converged` means the requested global enclosure-width criterion is proven.

`indeterminate` means a valid enclosure exists but the requested width was
not proven before the declared resource limit.

`indeterminate` is not a numeric failure and must retain its best valid
enclosure.

### Reversal invariance

A curve and its reversed control-point order represent the same geometric trace
with opposite parameter direction; total length enclosures must be equivalent
under the declared deterministic evidence semantics.

### Translation invariance

Translation of every control point must not change the mathematical length.
Focused safe fixtures must demonstrate equivalent enclosures within the exact
outward-arithmetic semantics.

### Power-of-two scale covariance

Scaling geometry by an admitted finite power of two and scaling absolute
tolerance by the same magnitude must scale the retained length enclosure by the
same magnitude for selected safe fixtures. Relative tolerance is dimensionless
and unchanged.

### Dimension parity

Embedding a 2D fixture in 3D with `z=0` must preserve the length enclosure
semantics.

### Determinism

The same curve and policy produce equivalent evidence fields in repeated serial
executions in the declared toolchain envelope.

### Representation independence

Total length depends only on continuous curve geometry. It has no topological
identity, mesh, filesystem, clock, random, locale, environment or thread
schedule dependency.

## 11. Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Axis-aligned straight cubic | Lower and upper bounds coincide with the exact segment length; result converges without unnecessary refinement. |
| Constant curve | Exact enclosure `[0,0]`; no false regularity prerequisite. |
| Degree-elevated parabola | Enclosure contains the independent analytic value for `B(t)=(t,t^2)`, namely `sqrt(5)/2 + asinh(2)/4`. |
| 2D/3D embedding parity | The same planar curve embedded at `z=0` yields equivalent evidence. |
| Reversal | Length evidence remains equivalent under control-point reversal. |
| Translation | Safe exactly representable translation does not alter length evidence. |
| Power-of-two scale | Bounds and absolute tolerance scale consistently. |
| Zero-tolerance request | Nontrivial curve may become `indeterminate`; policy is not silently relaxed. |
| Coarse resource budget | Returns `indeterminate` with ordered finite bounds, not a fabricated converged result. |
| Increased resource budget | A selected fixture produces a no-wider enclosure; convergence evidence improves or remains equivalent. |
| Duplicate/repeated controls | Valid degenerate control polygons still produce a valid length enclosure. |
| Endpoint-coincident loop-like cubic | Zero endpoint chord does not imply zero total length; upper/lower refinement remains sound. |
| Near-stationary regular fixture | Length enclosure remains valid independent of regularity certification difficulty. |
| Interior stationary fixture | Total length remains definable; no regularity gate is imposed on length. |
| Large finite values | Safe cases remain finite; deliberate un-enclosable arithmetic fails explicitly. |
| Subnormal-scale values | No silent flush-to-zero semantic assumption; valid enclosure or explicit numeric failure only. |
| Invalid policy | NaN, infinity, negative tolerances, zero node budget and unsupported depth are rejected. |
| Repeatability | Repeated serial calls retain equivalent scientific evidence. |
| Header/dependency isolation | No topology/model/meshing/threading/third-party dependency is introduced. |

The analytic parabola fixture must be evaluated independently of the production
arc-length algorithm.

## 12. Relationship to global regularity

Total arc length exists for the admitted polynomial cubic representation even
when the curve is singular or constant.

Therefore the first work unit must **not** require a prior
`CurveRegularityResult::regular` result.

This is intentional.

Global regularity becomes a prerequisite only for the later inverse
parameter-mapping problem, where strict monotonicity of cumulative length is
needed to obtain a unique inverse parameter.

## 13. Future cumulative/inverse parameter mapping boundary

After total-length enclosure is integrated and closed, a separate decision may
authorize a second work unit.

That future work unit may investigate:

`S(t) = length(B|[0,t])`

and an inverse mapping from target length/fraction to a parameter bracket.

It must require a globally regular curve before claiming a unique inverse.

The default safe inversion strategy should be bracket-preserving and
deterministic. Newton or secant acceleration, if ever introduced, may only be
an optional refinement inside a proven bracket and may never be required for
correctness.

No mapping implementation is authorized by this decision.

## 14. Why Gauss–Kronrod is not the first certificate

Adaptive Gauss–Kronrod is a strong numerical integration technique and remains
useful as independent diagnostic/reference evidence.

However, the difference between nested Gauss and Kronrod rules is an error
estimate, not by itself a proof that the true integral lies inside a returned
interval for every admitted floating-point input.

The scientific requirement here is explicit containment.

Because polynomial Bézier geometry already supplies convergent geometric lower
and upper arc-length bounds under subdivision, the first AP Mesh arc-length
work unit uses that structure directly.

A later decision may admit quadrature for performance or independent
cross-checking if it preserves the certified enclosure contract.

## 15. Focused validation boundary

Before integration, the first implementation work unit must pass focused
validation at minimum in:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

The selected semantic set must include:

- Numeric Contract;
- Geometry Primitives;
- Minimal Small Linear Algebra and header isolation where applicable;
- Cartesian Frames;
- Topological Model;
- cubic curve representation/evaluation;
- cubic differential evaluation;
- global cubic regularity; and
- the new total arc-length enclosure contract.

Passing focused validation establishes only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

Stage qualification remains deferred to Continuous Curve Geometry Regression.

## 16. Admission and stop conditions

Implementation may proceed only if:

1. the public result is an enclosure/evidence object, not an unqualified scalar;
2. containment is conservative under floating arithmetic;
3. the final global width criterion is explicit and policy-driven;
4. resource exhaustion returns `indeterminate` with retained bounds;
5. regularity is not incorrectly required for total length;
6. no parameter inversion or discretization API is introduced;
7. no generic public interval library is created;
8. no third-party runtime or parallel mechanism is added;
9. all currently admitted prerequisite semantics remain passing.

Stop and request a new bounded decision if implementation requires:

- a non-conservative heuristic error estimate as the only evidence;
- hidden tolerance adjustment;
- public adaptive subdivision state;
- arbitrary-degree/rational curve architecture;
- global root solving;
- curve/topology ownership;
- physical point sampling/discretization;
- curvature;
- surface geometry;
- quadrilateral construction; or
- parallel execution.

## 17. Phase map

The Curve Representation phase is now explicitly mapped as:

1. **Cubic value representation/evaluation** — integrated;
2. **Differential evaluation and pointwise speed** — integrated;
3. **Global cubic regularity certification** — integrated;
4. **Certified total arc-length enclosure** — next implementation work unit;
5. **Cumulative arc-length mapping and inverse bracketing** — blocked until 4
   closes;
6. **Continuous Curve Geometry Regression** — stage-exit qualification work,
   only after the admitted continuous-curve work units close.

Only after Curve Representation qualifies may the roadmap advance to Curve
Differential Geometry and later Boundary Curve Discretization.

## 18. Effect on roadmap if integrated

Arc Length and Parameter Mapping becomes
`IN INVESTIGATION / DECISION APPROVED / TOTAL-LENGTH IMPLEMENTATION NOT STARTED`.

The next bounded executable work item is only:

**Certified Cubic Bézier Total Arc-Length Enclosure.**

No inverse mapping, discretization, quadrilateral or parallel work becomes
authorized by integrating this decision.


## 19. Integration checkpoint

PR #59 integrated this decision as
`65cd93818fa54eac00c6f63ebefa3615074a82cd`.

Validation:

- PR FAST `35551687273`: PASS;
- PR INTEGRATION `35551687243`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35551746429`: PASS;
- post-merge INTEGRATION `35551746412`: PASS.

The decision checkpoint is closed.

The sole next executable work item is **Certified Cubic Bézier Total
Arc-Length Enclosure**. Cumulative/inverse parameter mapping remains blocked
until that implementation is integrated and closed.


## 20. Certified total arc-length implementation result

The first authorized work unit is implemented on
`curve/cubic-bezier-total-arc-length-enclosure`.

Public API:

- `CurveLengthPolicy`;
- `CurveLengthResult::{converged, indeterminate}`;
- `CurveLengthEvidence`;
- `CurveLengthError`;
- `CubicBezier2::arc_length_enclosure(...)`;
- `CubicBezier3::arc_length_enclosure(...)`.

The implementation represents each cubic leaf by interval edge vectors and
uses the exact midpoint de Casteljau edge identities for deterministic
subdivision. Chord norms provide lower bounds and the three edge norms provide
control-polygon upper bounds.

A pre-merge scientific review rejected reliance on a one-`nextafter`
`std::hypot` enclosure because the C++ contract does not establish the
required formal one-ulp error bound for `hypot`. The final implementation
instead constructs scaled interval squared-norm bounds from basic arithmetic,
uses correctly-rounded `std::sqrt`, and rounds the resulting lower/upper
bounds outward.

The focused contract covers the analytic/adversarial table defined above,
including an independently evaluated degree-elevated parabola reference,
resource exhaustion, stationary curves, extreme finite input and subnormal
scale.

Final validation at head
`6ba8a1e52927c696e5c297c49943c1b25d8ec116`:

- FAST `35552642188`: PASS;
- INTEGRATION `35552642196`: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

This establishes:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / VALIDATED_UNMERGED / NOT QUALIFIED.**

It does not authorize cumulative/inverse parameter mapping or any later
discretization/meshing phase.


## 14. Work unit 1 implementation result

**Certified Cubic Bézier Total Arc-Length Enclosure — IMPLEMENTED / FOCUSED
CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

PR #61 integrated the bounded implementation as
`5d89edfd391dc5548245f35ccedc2ac4c6c6951a`.

Validation:

- final PR head:
  `42002586dfd61a80d07053d88982b301b1f1acde`;
- FAST `35552739398`: PASS;
- INTEGRATION `35552739395`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35580244685`: PASS;
- post-merge INTEGRATION `35580244722`: PASS.

The integrated implementation provides:

- explicit `CurveLengthPolicy`, `CurveLengthResult`,
  `CurveLengthEvidence`, and `CurveLengthError`;
- deterministic dyadic cubic subdivision in edge-vector form;
- conservative chord/control-polygon total-length enclosure;
- private outward interval arithmetic;
- scaled interval Euclidean norm bounds;
- explicit `converged` versus resource-limited `indeterminate`;
- focused 2D/3D analytic and adversarial evidence.

The implementation does not add cumulative `S(t)`, inverse/fraction mapping,
public subdivision, curvature, physical discretization, topology ownership,
surfaces, quadrilateral generation or parallel execution.

The next admissible scientific action is a **separate bounded decision** for
Cumulative Arc-Length Mapping and Certified Inverse Bracketing. This document's
earlier future-boundary text remains informative, but it does not itself
authorize Work unit 2 implementation.
