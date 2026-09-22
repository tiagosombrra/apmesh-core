# Certified Simple Planar Inflection Isolation — Bounded Scientific Decision

Status: **DECISION APPROVED / IMPLEMENTATION NOT STARTED / STAGE UNQUALIFIED**  
Date: 2026-09-21  
Stage: Curve Differential Geometry — Curvature, Regularity, and Features

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation — Continuous Geometry Before Discretization:
  QUALIFIED by CGR0–CGR7 PASS;
- Pointwise Curvature Magnitude on Regular Cubic Bézier Curves:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Pointwise Signed Curvature on Regular Planar Cubic Bézier Curves:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED;
- Global Cubic Regularity Certification:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / qualified prerequisite
  through the Curve Representation stage.

## 1. Question

What is the smallest next Curve Differential Geometry capability that can turn
the already integrated planar signed-curvature mathematics into **global,
interval-wide certified evidence** without incorrectly treating samples,
floating signed-curvature values, or an unresolved multiple zero as a proven
inflection?

## 2. Decision

Authorize exactly one bounded implementation work unit:

**Certified Simple Planar Inflection Isolation on Globally Regular Cubic Bézier
Curves.**

The work unit may operate only on `CubicBezier2`.

It may certify and isolate **interior simple roots** of the exact mathematical
planar curvature numerator

`N(t) = det(B'(t), B''(t))`

on `t∈(0,1)`, provided global regularity of the curve has first been
certified by the already integrated regularity authority.

A certified simple root of `N` on a regular planar curve is an admitted
inflection because a simple real root changes the sign of `N`, and the speed
is nonzero on the whole interval.

The work unit must **not** claim completeness for unresolved multiple roots,
near-multiple roots, tangential zero-curvature contacts, or intervals whose
coefficient signs cannot be certified under the explicit resource policy.
Those cases return `indeterminate`.

The work unit is therefore a **sound simple-inflection isolator**, not a general
feature classifier and not a promise that every cubic receives a complete
binary answer.

## 3. Literature and reference basis

External sources are mathematical/reference evidence only. No runtime
dependency is admitted.

### Patrikalakis, Maekawa and Cho — zero-curvature points

MIT Hyperbook, *Shape Interrogation for Computer Aided Design and
Manufacturing*, section 8.1.3.2.

Reference:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node153.html

The reference states that, under the regularity assumption, zero signed
curvature supplies a necessary condition for determining planar inflection
points.

Decision impact:

- regularity is an explicit prerequisite;
- solving the curvature numerator is the global algebraic problem;
- a zero alone is not promoted to an inflection unless the root structure
  establishes a sign-changing simple root.

### Farouki and Rajan — Bernstein root conditioning

R. T. Farouki and V. T. Rajan,
*On the numerical condition of polynomials in Bernstein form*,
Computer Aided Geometric Design 4(3), 1987.

Reference:

https://www.sciencedirect.com/science/article/pii/0167839687900124

The paper establishes favorable real-root conditioning in Bernstein form and
shows that standard Bernstein subdivision improves root conditioning.

Decision impact:

- retain the curvature-numerator polynomial in Bernstein form;
- refine through de Casteljau subdivision rather than convert globally to the
  monomial basis.

### Mourrain and Pavone — subdivision root solving

B. Mourrain and J.-P. Pavone,
*Subdivision methods for solving polynomial equations*,
Journal of Symbolic Computation 44(3), 2009.

Reference:

https://www.sciencedirect.com/science/article/pii/S0747717108001168

The paper uses Bernstein-basis subdivision and Descartes-style root-count
reasoning as a principled root-isolation method on bounded domains.

Decision impact:

- use sign-variation/root-count evidence on subintervals;
- subdivision is certification machinery, not a public curve-mutation API.

### Eigenwillig, Sharma and Yap — Descartes rule and multiple roots

A. Eigenwillig, V. Sharma and C. K. Yap,
*On multiple roots in Descartes’ Rule and their distance to roots of higher
derivatives*, Journal of Computational and Applied Mathematics 200(1), 2007.

Reference:

https://www.sciencedirect.com/science/article/pii/S0377042705008083

The paper records the Bernstein-basis form of Descartes root counting and the
special difficulty posed by multiple roots.

Decision impact:

- zero sign variations can certify absence of an interior root;
- one sign variation certifies exactly one interior root;
- multiple/unresolved sign variations must be subdivided or reported
  `indeterminate`;
- a multiple-root case is not silently classified as a simple inflection.

## 4. Mathematical reduction for a planar cubic Bézier curve

For control points `P0,P1,P2,P3`, define the exact mathematical quadratic
hodograph controls

`D0 = 3(P1-P0)`

`D1 = 3(P2-P1)`

`D2 = 3(P3-P2)`.

Then

`B'(t) = D0 B_0^2(t) + D1 B_1^2(t) + D2 B_2^2(t)`.

For the planar determinant numerator

`N(t) = det(B'(t),B''(t))`,

the cubic leading term cancels identically. `N` is therefore a scalar
quadratic polynomial.

In Bernstein degree two on `[0,1]`:

`N(t) = N0 B_0^2(t) + N1 B_1^2(t) + N2 B_2^2(t)`

with exact mathematical coefficients

`N0 = 2 det(D0,D1)`

`N1 = det(D0,D2)`

`N2 = 2 det(D1,D2)`.

The implementation must derive this polynomial from the exact mathematical
control-point values through conservative floating enclosures. It must not use
sampled `signed_curvature(t)` values as proof of the polynomial sign or root
count.

## 5. Why simple roots only

For a globally regular planar curve,

`signed_curvature(t) = N(t) / ||B'(t)||^3`.

The denominator is strictly positive everywhere.

Therefore the sign of signed curvature is exactly the sign of `N(t)`.

A **simple** root of `N` changes sign and is therefore a certified planar
inflection under the declared orientation convention.

A double root may have zero curvature without a sign change. The current work
unit must not call such a tangential zero a certified inflection.

Because `N` is quadratic, at most two interior simple roots can exist.

The public evidence may therefore use a fixed-capacity representation for at
most two certified brackets; no general dynamic polynomial-root container is
required.

## 6. Certification precondition: global regularity

The isolator must invoke or reuse the existing global regularity certifier with
an explicit caller-supplied `CurveRegularityPolicy`.

The isolation result may become scientifically `complete` only if global
regularity is certified `regular`.

If regularity is:

- `degenerate`: the inflection-isolation query is outside its admitted
  domain and must not return a successful complete result;
- `indeterminate`: inflection isolation must remain `indeterminate`;
- numerically unavailable: return an explicit failure.

The isolator must not replace global regularity with sampled speed checks or a
speed epsilon.

## 7. Certified floating representation

The represented finite binary64 control coordinates are interpreted as exact
real input values, consistent with the existing regularity certifier.

The implementation may reuse the existing private
`src/geometry/detail/curve_regularity_interval.hpp` enclosure machinery or
extract only a shared **private curve-local** interval primitive where that
reduces duplication.

No public general-purpose interval API is admitted.

Required conservative operations are limited to:

- point intervals;
- addition/subtraction;
- multiplication;
- multiplication by exact small integers;
- the 2D determinant;
- midpoint averaging for quadratic Bernstein subdivision.

Every retained interval must enclose the corresponding exact-real value.

Overflow, non-finite bounds, or inability to prove enclosure containment must
return explicit numeric failure or `indeterminate`; it must never continue
with a non-conservative bound.

No process-wide rounding-mode mutation or global scientific state is admitted.

## 8. Root-count certificate on one subinterval

For a subinterval `I=[a,b]`, retain interval enclosures of the three
quadratic Bernstein coefficients.

A coefficient sign is certified only when its entire enclosure is:

- strictly positive; or
- strictly negative.

An exact interval `[0,0]` is an exact zero coefficient; otherwise an interval
containing zero has unknown sign.

After exact-zero coefficients are removed for sign-variation counting:

- zero certified sign variations proves no interior root in `(a,b)`;
- one certified sign variation proves exactly one interior real root, counted
  with multiplicity; because total multiplicity is one, that root is simple;
- two sign variations or any unresolved coefficient sign requires subdivision
  or `indeterminate`.

The implementation must not infer a root count from midpoint samples or
ordinary rounded coefficient signs.

## 9. Isolation algorithm

For one globally regular `CubicBezier2`:

1. validate the explicit isolation policy;
2. certify global regularity;
3. construct conservative enclosures for `D0,D1,D2`;
4. construct conservative enclosures for `N0,N1,N2`;
5. initialize one root-analysis node for `[0,1]`;
6. analyze sign variation using certified coefficient signs;
7. if variation is zero, mark the interval root-free;
8. if variation is one:
   - it contains exactly one simple interior root;
   - if interval width is within the explicit parameter-bracket tolerance,
     retain it as one certified inflection bracket;
   - otherwise subdivide;
9. if variation is two or coefficient signs remain unresolved, subdivide if
   policy resources permit;
10. if subdivision/resources are exhausted while unresolved evidence remains,
    return `indeterminate`;
11. return `complete` only when the full open interval is covered by
    root-free leaves and pairwise-disjoint certified simple-root brackets.

Quadratic midpoint subdivision must use Bernstein/de Casteljau arithmetic and
retain exact child interval coverage.

No Newton iteration, quadratic formula, companion matrix, sampled
signed-curvature sign scan, or third-party solver is part of the proof.

## 10. Endpoint semantics

This work unit isolates **interior** inflections on `t∈(0,1)`.

A zero of `N` exactly at `t=0` or `t=1` is a boundary zero-curvature
event, not an interior inflection under this contract.

Endpoint zero coefficients are therefore excluded from the interior root count
according to the Bernstein/Descartes open-interval semantics.

The work unit does not introduce a separate boundary-feature classification.

## 11. Public result vocabulary

The public scientific result must distinguish at minimum:

- `complete`: every interior simple inflection is isolated in a retained
  bracket, and no unresolved interior curvature-numerator root remains;
- `indeterminate`: the bounded certifier cannot establish a complete result
  under the supplied policy.

Domain/numeric errors remain separately diagnosable.

A complete result may contain zero, one, or two inflection brackets.

Every bracket must be:

- finite;
- contained in `[0,1]`;
- positive width unless an exact representable interior root is separately and
  rigorously witnessed;
- no wider than the explicit parameter tolerance;
- pairwise disjoint;
- sorted in increasing parameter order;
- certified to contain exactly one simple interior root.

The evidence should retain at minimum:

- scientific result;
- global regularity evidence;
- inflection count;
- up to two brackets;
- processed node count;
- root-free leaf count;
- isolated-root leaf count;
- maximum depth reached.

## 12. Explicit policy

The public policy may contain only bounded resource/representation controls,
for example:

- `CurveRegularityPolicy regularity_policy`;
- positive finite parameter-bracket tolerance;
- maximum subdivision depth;
- maximum processed nodes.

The parameter tolerance controls only the maximum width of a returned bracket.
It is **not**:

- a curvature-zero threshold;
- a determinant sign epsilon;
- a multiplicity threshold;
- a regularity threshold.

No hidden default scientific epsilon is admitted.

Increasing the resource budget may refine a prior `indeterminate` result into
`complete`, but it may not invalidate a previously certified bracket or
root-free interval.

## 13. Required invariants

### Completeness implication

A `complete` result means no unresolved interior root of the curvature
numerator remains.

### Bracket soundness

Every retained bracket contains exactly one simple interior root of `N`.

### Regularity

Every successful complete result includes a global `regular` certificate.

### Reversal covariance

For `R(t)=B(1-t)`, bracket `[a,b]` maps to
`[1-b,1-a]`; bracket order reverses and is then canonicalized into increasing
parameter order.

Inflection count is unchanged.

### Translation invariance

Translation leaves all inflection parameters unchanged.

### Orientation-preserving and orientation-reversing frames

Both admitted signed-permutation frame classes leave inflection parameters and
count unchanged. An orientation-reversing frame flips the global sign of the
curvature numerator but does not change its roots.

### Uniform nonzero scale

Admitted finite uniform power-of-two scaling leaves inflection parameters and
count unchanged.

### Determinism

Identical curve and policy yield identical scientific fields and deterministic
diagnostics.

### No sampled proof

Pointwise `signed_curvature(t)` evaluations may appear only as independent
test diagnostics; they may not influence certification.

## 14. Required focused evidence

A dedicated focused contract must cover at minimum:

| Case | Required observation |
| --- | --- |
| Regular straight-line cubic | `complete`, zero simple inflections; identically zero numerator must not become infinitely many feature points. |
| Regular planar cubic with no inflection | `complete`, zero brackets. |
| Analytic one-inflection cubic | `complete`, one certified bracket containing the independent exact/reference parameter. |
| Analytic two-inflection cubic | `complete`, two disjoint ordered certified brackets. |
| Reversal | Brackets map by `[a,b]→[1-b,1-a]`; count preserved. |
| Reflection | Numerator sign reverses globally; brackets/count unchanged. |
| Translation | Brackets/count unchanged. |
| Power-of-two uniform scale | Brackets/count unchanged. |
| Endpoint zero-curvature numerator | Boundary zero is not reported as an interior inflection. |
| Exact double interior zero | Must not be reported as a certified simple inflection; result may be `indeterminate`. |
| Near-double / ill-conditioned fixture | No guessed simple-root classification; complete only if coefficient evidence proves it. |
| Globally degenerate curve | No successful complete inflection result. |
| Regularity-indeterminate curve/policy | Inflection result remains `indeterminate`. |
| Coarse isolation policy | May return `indeterminate`, never a guessed bracket. |
| Increased resource policy | May convert a prior indeterminate simple-root fixture to complete without changing sound prior results. |
| Extreme finite coordinates | Sound complete/indeterminate/numeric failure only; no overflow-derived sign. |
| Determinism | Repeated evidence is identical. |
| Header/dependency isolation | No topology ownership, discretization, meshing, threading, I/O, or third-party dependency. |

Independent fixtures must compute expected root structure without calling the
production isolation method.

At least one analytic fixture should derive the curvature-numerator polynomial
independently and verify the retained bracket against an exact or
high-precision reference root.

## 15. Focused regression boundary

Before integration, the implementation must pass at minimum:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

Selected regression must preserve:

- Numeric Contract;
- Geometry Primitives;
- Minimal Small Linear Algebra/header isolation where selected;
- Cartesian Frames;
- Topological Model;
- qualified Curve Representation contracts;
- global cubic regularity;
- pointwise curvature magnitude;
- planar signed curvature.

A passing work unit yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

Curve Differential Geometry stage qualification remains a later cumulative
regression campaign.

## 16. Explicit exclusions

This decision does not authorize:

- classification of multiple/tangential zero-curvature roots as inflections;
- a promise that every cubic receives a complete answer;
- endpoint feature classification;
- arbitrary-degree root isolation;
- rational Bézier, B-spline, or NURBS inflection analysis;
- a public polynomial solver;
- a public interval arithmetic library;
- quadratic-formula root solving as the certification authority;
- sampled sign-change detection as proof;
- curvature derivative;
- curvature extrema or monotonicity;
- interval/global curvature magnitude bounds;
- cusp/corner classification;
- feature-point persistence or IDs;
- feature intervals;
- osculating-circle/radius APIs;
- boundary discretization;
- curvature-driven sizing;
- surface geometry;
- mesh generation/optimization/adaptation;
- Quad-Dominant construction;
- SIMD/GPU/OpenMP/MPI/task parallelism;
- third-party runtime dependencies;
- stage-level Curve Differential Geometry qualification infrastructure.

Boundary Curve Discretization remains blocked.

## 17. Stop conditions

Stop and require a new bounded decision if implementation needs:

- classification of double/multiple numerator roots;
- exact/adaptive predicates beyond the declared private enclosure arithmetic;
- a general-purpose polynomial solver;
- a public interval type;
- third derivatives;
- curvature derivative;
- interval/global curvature bounds;
- feature persistence/identity;
- physical sampling/discretization;
- a hidden tolerance or fallback; or
- a new third-party dependency.

## 18. Alternatives considered

### Sample `signed_curvature(t)` and look for sign changes

Rejected.

Finite samples cannot prove that no root was missed, and the pointwise signed
curvature implementation explicitly disclaims certified near-zero determinant
signs.

### Use the ordinary quadratic formula

Deferred as the certification authority.

Although `N` is quadratic, a robust global contract still needs certified
coefficient signs, discriminant/boundary decisions, near-multiple handling and
root-rounding containment. Bernstein subdivision already matches the qualified
curve representation and the existing conservative-enclosure architecture.

### Classify all zero-curvature roots, including double roots

Deferred.

A double root is not automatically a sign-changing inflection. Multiplicity and
tangential-zero semantics are a distinct feature-classification decision.

### Global curvature bound next

Deferred.

That is a quantitative enclosure problem requiring lower speed evidence and
curvature-numerator/acceleration bounds, not root isolation.

### Curvature extrema next

Deferred.

Extrema require higher-order/global analysis and a separate zero problem for a
curvature derivative or equivalent algebraic numerator.

### Certified simple-inflection isolation

Chosen.

It is the smallest global capability that meaningfully consumes the integrated
signed-curvature mathematics while maintaining sound interval-wide evidence and
an explicit `indeterminate` outcome.

## 19. Repository mapping

Current authorities reused without semantic fork:

| Repository element | Role in this work unit |
| --- | --- |
| `include/apmesh/geometry/curve.hpp` | Existing cubic value/differential/regularity/curvature public authority; future API additions remain here if admitted. |
| `src/geometry/curve.cpp` | Existing derivative, regularity, curvature and signed-curvature implementation; future orchestration remains here. |
| `src/geometry/detail/curve_regularity_interval.hpp` | Existing private conservative interval primitives; may be reused or minimally factored without becoming public API. |
| `tests/curve_regularity.cpp` | Qualified-prerequisite evidence for global nonzero speed. |
| `tests/curve_curvature.cpp` | Pointwise curvature-magnitude prerequisite evidence. |
| `tests/curve_signed_curvature.cpp` | Local orientation-sensitive prerequisite evidence; not a root-certification authority. |
| `docs/decisions/CURVE_GLOBAL_REGULARITY_CERTIFICATION_DECISION.md` | Scientific regularity precondition. |
| `docs/decisions/CURVE_SIGNED_PLANAR_CURVATURE_DECISION.md` | Pointwise signed-curvature convention and explicit global-inflection exclusion. |

Expected implementation mapping, if this decision is separately integrated and
closed:

- public types/method declaration:
  `include/apmesh/geometry/curve.hpp`;
- production implementation:
  `src/geometry/curve.cpp`;
- private quadratic Bernstein/enclosure helper:
  either a narrow addition to
  `src/geometry/detail/curve_regularity_interval.hpp` or a new
  `src/geometry/detail/curve_inflection_interval.hpp`;
- focused contract:
  `tests/curve_inflection_isolation.cpp`;
- test registration/labels:
  `CMakeLists.txt`;
- decision/history:
  this document;
- stage state/mapping:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`.

No topology, surface, discretization, mesh, parallel, filesystem, network, or
third-party library path should become a dependency.

## 20. Effect on the stage plan

If this decision is integrated and its checkpoint is closed, the sole next
bounded implementation work item is:

**Certified Simple Planar Inflection Isolation on Globally Regular Cubic Bézier
Curves**

within the exact scope above.

Curve Differential Geometry remains:

**IN INVESTIGATION / POINTWISE CURVATURE INTEGRATED / SIGNED PLANAR CURVATURE
INTEGRATED / SIMPLE-INFLECTION DECISION APPROVED / STAGE UNQUALIFIED.**

After the work unit is implemented and closed, another literature-backed
decision is required before any global curvature bound, curvature extrema, or
feature-classification work.

Boundary Curve Discretization, sizing, surfaces, meshing, Quad-Dominant and
parallel execution remain blocked.
