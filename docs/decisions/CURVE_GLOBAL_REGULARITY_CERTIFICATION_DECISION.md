# Global Cubic Regularity Certification — Bounded Decision

Status: DECISION INTEGRATED / IMPLEMENTATION NOT STARTED
Date: 2026-09-20
Stage: Curve Representation — Continuous Geometry Before Discretization
Prerequisites:
- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud envelope;
- Polynomial Cubic Bézier Value Representation and Evaluation:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Cubic Bézier Differential Evaluation and Pointwise Speed:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.

## Question

What is the smallest scientifically defensible method that can certify that a
fixed polynomial cubic Bézier curve satisfies

`B'(t) != 0`

for every `t ∈ [0,1]`, without dense sampling, a context-free speed epsilon,
or an unbounded/exact-arithmetic dependency?

## Decision

Authorize one bounded implementation work unit named:

**Certified Global Cubic Regularity by Bernstein Speed-Squared Enclosure.**

The work unit may construct a conservative interval enclosure of the quartic
scalar polynomial

`s(t) = B'(t) · B'(t) = ||B'(t)||^2`

in Bernstein form, and may recursively subdivide that quartic over `[0,1]`.

A curve may be reported **regular** only when the retained enclosure proves

`s(t) > 0`

for every parameter in the complete interval.

The implementation may report **degenerate** only when it has an exact
mathematical witness already admitted by this decision, such as an exactly zero
endpoint derivative implied by identical adjacent endpoint control points.

Every other unresolved case must return **indeterminate**. In particular,
failure to prove positivity is not evidence of degeneracy, and observing only
positive sampled speeds is not evidence of regularity.

The work unit is therefore a sound global regularity **certifier**, not a
heuristic classifier and not a promise that every admissible curve will receive
a binary answer within finite floating-point resources.

## Sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| Farouki and Rajan, *On the numerical condition of polynomials in Bernstein form*, CAGD 4(3), 1987 | Bernstein form has favorable conditioning for polynomial/root computations, and subdivision improves conditioning. | A complete AP Mesh regularity API or a floating implementation proof by itself. | Keep the certification polynomial in Bernstein form and refine by subdivision rather than convert to a global power basis. |
| Meng and Liu, *Regularity of Bézier Curves*, Applied Mechanics and Materials 48–49, 2011 | Bézier regularity can be reduced to existence/nonexistence of zeros of polynomial equations derived from the derivative. | That their complete algebraic method or external implementation is required by AP Mesh. | Treat the global question as a zero-exclusion problem for the derivative, not a sampling problem. |
| Mourrain and Pavone, *Subdivision methods for solving polynomial equations*, Journal of Symbolic Computation 44(3), 2009 | Bernstein-basis subdivision is a principled method for isolating/excluding polynomial roots on bounded domains. | A direct implementation dependency or a guarantee that ordinary floating arithmetic without enclosures is certified. | Use hierarchical interval subdivision and explicit inconclusive results when floating bounds cannot certify. |
| AP Mesh Numeric Contract | Robust decisions require certified bounds or explicit `indeterminate`; a global epsilon is forbidden. | A ready-made interval arithmetic implementation. | Implement only the private bounded enclosure operations required by this curve certifier; do not introduce a general numeric framework implicitly. |

No third-party runtime dependency is admitted by this decision.

## Mathematical reduction

For the already integrated cubic Bézier control points
`P0, P1, P2, P3`, define the exact mathematical derivative control vectors:

`D0 = 3(P1 - P0)`

`D1 = 3(P2 - P1)`

`D2 = 3(P3 - P2)`.

The derivative is the quadratic Bézier vector curve

`B'(t) = D0 B_0^2(t) + D1 B_1^2(t) + D2 B_2^2(t)`.

Define the scalar squared-speed polynomial

`s(t) = B'(t) · B'(t)`.

It is a quartic polynomial. In Bernstein degree four its exact mathematical
coefficients are:

`S0 = D0 · D0`

`S1 = D0 · D1`

`S2 = (D0 · D2 + 2(D1 · D1)) / 3`

`S3 = D1 · D2`

`S4 = D2 · D2`.

Because Bernstein basis functions on `[0,1]` are nonnegative and sum to one,

`min_i S_i <= s(t) <= max_i S_i`.

Therefore, if all exact Bernstein coefficients of a subinterval are strictly
positive, then `s(t) > 0` everywhere on that subinterval.

Repeated de Casteljau subdivision of a strictly positive polynomial produces
progressively tighter Bernstein range enclosures. The certifier uses this
property to cover the complete parameter interval.

## Certified floating enclosure

The represented AP Mesh control coordinates are finite IEEE-754 binary64
values. For certification they are interpreted as their exact real values, not
as uncertain measurements.

The implementation must not treat ordinary rounded arithmetic as exact.
Instead it may introduce one **private, curve-local interval-enclosure helper**
sufficient only for:

- subtraction;
- addition;
- multiplication;
- multiplication by the exact small integers 2 and 3;
- division by the exact integer 3;
- midpoint averaging required by Bernstein subdivision.

Each operation must return a closed binary64 interval proven to contain the
corresponding exact-real result.

The reference environment is the already qualified round-to-nearest IEEE-754
binary64 envelope. The implementation must use conservative outward expansion
without mutating the process rounding mode or introducing global state.
A suitable implementation strategy is to compute the round-to-nearest result
and widen finite bounds with `std::nextafter` toward ±infinity as required by
the operation proof.

If an enclosure operation overflows, produces a non-finite bound, or cannot
establish its containment obligation, certification returns an explicit
numeric failure or `indeterminate`; it must never silently continue with a
non-conservative interval.

This helper is not a new public general-purpose interval arithmetic API.

## Certification algorithm

For one cubic curve:

1. validate the explicit certification policy;
2. check exact endpoint singular witnesses:
   - if `P1 == P0`, then `B'(0) = 0`: return `degenerate`;
   - if `P3 == P2`, then `B'(1) = 0`: return `degenerate`;
3. construct interval enclosures for `D0,D1,D2` from the exact input
   coordinates;
4. construct interval enclosures for the five exact quartic Bernstein
   coefficients `S0..S4`;
5. initialize one work item covering `[0,1]`;
6. for each work item:
   - if every coefficient interval has lower bound strictly greater than zero,
     certify that entire subinterval as regular;
   - otherwise, if the explicit subdivision/resource policy permits, subdivide
     the quartic coefficient intervals at the exact parameter midpoint using
     interval de Casteljau and process both children;
   - otherwise return `indeterminate`;
7. return `regular` only after every leaf covering `[0,1]` has a strictly
   positive lower bound.

No parameter grid is part of the proof. Samples may be retained only as
diagnostics and cannot affect the classification.

## Result vocabulary

The public scientific result must distinguish at least:

- `regular`: complete interval-wide positivity of squared speed is certified;
- `degenerate`: an exact admitted zero-derivative witness is established;
- `indeterminate`: the bounded certifier cannot prove either admitted
  conclusion under the supplied resource policy.

Numeric construction failures remain separately diagnosable.

A `regular` result is scientific evidence. An `indeterminate` result is not
a failed curve and must not be converted to `degenerate`.

## Explicit certification policy

Subdivision/resource limits affect only whether the algorithm may return
`indeterminate`; they must not change the truth conditions for `regular` or
`degenerate`.

The policy must be immutable, explicit and validated. It may bound quantities
such as:

- maximum subdivision depth;
- maximum processed nodes.

There is no hidden default scientific epsilon, speed threshold, minimum
accepted speed or coordinate-scale tolerance.

Two policies may differ in whether a difficult curve is certified before
resources are exhausted, but no policy may make a false `regular` result
valid.

## Required invariants

1. `regular` implies `B'(t) != 0` for every `t∈[0,1]`.
2. `degenerate` requires a retained exact mathematical witness.
3. `indeterminate` is distinct from `degenerate`.
4. All retained coefficient intervals contain the exact mathematical
   coefficients they claim to enclose.
5. Subdivision children conservatively enclose the exact parent polynomial on
   their corresponding half intervals.
6. The union of active/certified leaves covers exactly `[0,1]` with no gap.
7. A leaf is certified regular only if every coefficient lower bound is
   strictly positive.
8. Reversal preserves the regularity result for the same certification policy,
   except that deterministic diagnostics may appear in reversed traversal
   order only if explicitly excluded from scientific equality.
9. Translation preserves the regularity result.
10. Admitted Cartesian-frame transformations preserve the regularity result
    when their finite transformations succeed.
11. Exact power-of-two rescaling that remains within the admitted finite
    envelope preserves the classification.
12. No topology, filesystem, environment, random source, thread schedule,
    global tolerance or mutable scientific state participates.
13. Repeated runs with identical curve/policy produce identical scientific
    result and deterministic diagnostics.

## Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Regular straight-line cubic | Certifies `regular`; derivative is constant nonzero. |
| Regular genuine cubic | Certifies `regular` with complete interval coverage. |
| Endpoint singular at zero | `P1 == P0` returns exact `degenerate`. |
| Endpoint singular at one | `P3 == P2` returns exact `degenerate`. |
| Constant curve | Returns exact `degenerate` through endpoint witness; never `regular`. |
| Interior stationary analytic fixture | Must never return `regular`; may return `indeterminate` unless a separately admitted exact witness is implemented. |
| Near-stationary but regular | Must not be classified degenerate by magnitude; either certifies `regular` or returns `indeterminate`. |
| Coarse resource policy | A curve requiring more subdivision returns `indeterminate`, not a guessed result. |
| Increased resource policy | May turn the same prior `indeterminate` regular fixture into certified `regular`; may not change a prior certified result. |
| Reversal | Scientific classification is invariant. |
| Translation | Scientific classification is invariant. |
| Qualified frames | Scientific classification is invariant under admitted finite frame maps. |
| Power-of-two scale | Classification is invariant where all required arithmetic remains finite/certifiable. |
| Extreme finite coordinates | Returns a sound certificate or explicit numeric/indeterminate result; never relies on overflowed bounds. |
| 2D/3D parity | Planar fixture embedded in 3D preserves the scientific classification. |
| Determinism | Repeated executions retain identical scientific result and declared diagnostics. |

Focused tests must independently validate the interval helper on exact and
adversarial arithmetic fixtures rather than assuming `std::nextafter`
usage is automatically correct.

## Explicit exclusions

This decision does not authorize:

- a claim that all cubic curves receive a binary regular/degenerate answer;
- dense sampling or adaptive sampling as proof of regularity;
- a fixed or scale-aware speed threshold as a substitute for a mathematical
  zero decision;
- general polynomial root solving;
- public general-purpose interval arithmetic;
- arbitrary-degree regularity;
- rational Bézier, B-spline or NURBS regularity;
- unit tangent, curvature, torsion, Frenet frames or feature classification;
- arc-length quadrature or inverse arc-length mapping;
- subdivision as a public curve-geometry mutation API;
- closest point, projection, intersections or self-intersections;
- topology-owned curves or `CurveId`;
- boundary discretization, sizing, surfaces, meshing, optimization or
  adaptation;
- quadrilateral construction;
- SIMD, GPU, OpenMP, MPI, task systems or other parallel execution;
- third-party runtime dependencies;
- native-Windows qualification;
- stage-level Curve Representation qualification tooling.

The internal Bernstein subdivision used for certification is evidence machinery
for one mathematical decision. It does not authorize public curve subdivision.

## Admission and stop conditions

Implementation may proceed only if:

1. the certifier reasons over the complete `[0,1]` interval;
2. every `regular` result is justified by conservative positive lower bounds;
3. interval operations are proven conservative in the admitted floating
   envelope;
4. resource exhaustion returns `indeterminate`;
5. exact endpoint singularity is separated from near-zero magnitude;
6. the implementation adds no hidden tolerance or sampling acceptance;
7. the full adversarial table is represented in focused tests;
8. all selected Foundation/Geometry/Topology/curve prerequisite contracts
   remain passing.

Stop and require a new decision if implementation needs:

- a general exact-arithmetic package;
- public interval arithmetic;
- a third-party root solver;
- a new rounding-mode policy;
- a magnitude threshold to force a result;
- arbitrary-degree/rational support;
- curvature, arc length or discretization semantics.

## Alternatives considered

### Dense or adaptive speed sampling

Rejected. Finite samples cannot prove absence of an unsampled zero.

### Global speed epsilon

Rejected. Small nonzero speed is not degeneracy, and a context-free threshold
violates the Numeric Contract.

### Solve vector quadratic roots in ordinary floating point

Rejected as the certification basis. Approximate roots plus approximate
component tests require another tolerance policy and are weakest precisely near
multiple or near-common roots.

### Power-basis quartic root solver for `s(t)`

Deferred. A general robust root solver is a substantially larger numeric
capability than required to certify positive squared speed.

### Exact rational/GCD or Gröbner machinery

Not admitted for this bounded work unit. IEEE binary64 inputs are dyadic
rationals, but a complete exact algebraic stack would create a new foundational
numeric subsystem disproportionate to the current requirement.

### Bernstein squared-speed enclosure

Chosen. It keeps the problem in the existing Bézier/Bernstein representation,
covers the complete interval, can soundly certify positive separation, permits
explicit `indeterminate` when finite resources cannot prove the claim, and
requires only a private bounded enclosure helper consistent with the Numeric
Contract.

## Focused validation boundary

The implementation work unit must add a focused CTest contract for global cubic
regularity certification in both 2D and 3D.

Before integration it must pass at minimum:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

It must preserve the current accepted semantic prerequisite set, including:

- Numeric Contract;
- Geometry Primitives;
- Minimal Small Linear Algebra / header isolation where selected;
- Cartesian Frames;
- Topological Model;
- Cubic Bézier value/evaluation;
- Curve public-header isolation;
- Cubic Bézier differential evaluation and pointwise speed.

Passing the focused evidence permits only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

It does not qualify Curve Representation or authorize Arc Length and Parameter
Mapping automatically.

## Effect on the roadmap if integrated

Curve Representation remains:

**IN INVESTIGATION / CUBIC VALUE + DIFFERENTIAL EVALUATION IMPLEMENTED /
GLOBAL REGULARITY DECISION APPROVED / STAGE UNQUALIFIED.**

## Next bounded action after decision integration

Implement only:

**Certified Global Cubic Regularity by Bernstein Speed-Squared Enclosure**

and its focused interval-enclosure/regularity contracts.

Arc Length and Parameter Mapping remains blocked until this regularity work
unit is separately integrated, validated and closed.


## Integration checkpoint

PR #55 integrated this bounded decision as
`224c8ab530f88475c2d8281cb60682a7c0db851a`.

Validation:

- PR FAST `35549162022`: PASS;
- PR INTEGRATION `35549161971`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35549240099`: PASS;
- post-merge INTEGRATION `35549240093`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug.

The decision checkpoint is closed. The sole permitted continuation is the
bounded implementation of **Certified Global Cubic Regularity by Bernstein
Speed-Squared Enclosure** and its focused contracts.
