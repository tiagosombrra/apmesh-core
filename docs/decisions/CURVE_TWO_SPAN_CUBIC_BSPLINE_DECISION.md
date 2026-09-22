# Two-Span Clamped Cubic Polynomial B-Spline — Bounded Decision

Status: DECISION APPROVED / IMPLEMENTATION NOT STARTED / NOT QUALIFIED  
Date: 2026-09-22  
Stage: Curve Representation Breadth Gate

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- polynomial cubic Bézier Curve Representation baseline:
  QUALIFIED by CGR0–CGR7 in the admitted cloud envelope;
- bounded parametric curve contract: integrated;
- directed line segment 2D/3D: integrated focused work unit;
- positive-weight rational quadratic Bézier 2D/3D:
  integrated focused work unit;
- oriented trimmed parametric subcurve 2D/3D:
  integrated focused work unit and closed.

## 1. Question

Which remaining representation-breadth concept should be introduced next, and
what is the smallest work unit that isolates that concept without combining
multiple unresolved CAD semantics?

Candidates required by the preceding closure are:

1. bounded non-periodic B-spline;
2. NURBS after B-spline semantics;
3. arbitrary-degree polynomial/rational Bézier;
4. analytic conic plus arbitrary 3D supporting-plane/orientation;
5. heterogeneous composition/polycurve.

The decision must account for the fact that the repository already isolates:

- polynomial cubic Bernstein/Bézier evaluation and derivatives;
- positive rational-weight semantics;
- non-normalized bounded parameter domains;
- orientation reversal;
- bounded trim/subcurve semantics.

The missing representation concept with the highest downstream value is now
**knot-partitioned local support**.

## 2. Fresh repository evidence

Canonical decision-entry `main`:
`e29a08b07c175a91410123f99867eef4190b983b`.

Trim implementation evidence:

- implementation PR #114:
  `133a98ea056b12d86049e36abc0370208776106b`;
- candidate FAST `35730750101`: PASS, 21/21;
- candidate INTEGRATION `35730749747`: PASS, 21/21 in GCC and Clang;
- final PR-head FAST `35730921629`: PASS;
- final PR-head INTEGRATION `35730921744`: PASS;
- implementation post-merge FAST `35731127728`: PASS;
- implementation post-merge INTEGRATION `35731127685`: PASS;
- implementation closure PR #115:
  `e29a08b07c175a91410123f99867eef4190b983b`;
- closure post-merge FAST `35731529518`: PASS;
- closure post-merge INTEGRATION `35731529546`: PASS.

Production representation values/semantics at entry:

- `CubicBezier2/3`;
- `LineSegment2/3`;
- `RationalQuadraticBezier2/3`;
- `TrimmedCurve2/3<Curve>` over bounded conforming bases.

Absent at entry:

- any B-spline representation;
- NURBS;
- arbitrary-degree polynomial/rational Bézier;
- dedicated analytic conics;
- heterogeneous polycurve storage/dispatch;
- production surfaces.

## 3. Literature basis

### B-spline basis definition and local support

Michigan Technological University notes:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/bspline-basis.html

Relevant evidence:

- a B-spline basis is defined by a nondecreasing knot vector and a degree;
- knots subdivide the domain into knot spans;
- B-spline basis functions have local support rather than global support;
- the Cox–de Boor recursion supplies an independent basis-summation reference.

### B-spline continuity and multiplicity

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/bspline-property.html

Relevant evidence:

- a degree-`p` basis with knot multiplicity `k` is
  `C^(p-k)` at that knot;
- at most `p+1` basis functions are active in one span;
- simple interior knots on a cubic (`p=3`) therefore provide `C2`
  continuity.

Decision impact:

- the first work unit uses exactly one **simple** interior knot;
- repeated interior-knot continuity/failure semantics are deferred.

### de Boor evaluation

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/de-Boor.html

Relevant evidence:

- de Boor is the standard local evaluation algorithm for B-spline curves;
- it generalizes the de Casteljau construction;
- only locally active control points participate in one span.

Decision impact:

- production evaluation should use a fixed-degree de Boor construction;
- independent tests must not validate production only against itself.

### Bézier as a special B-spline case

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/de-boor-special-case.html

Relevant evidence:

- a Bézier curve is a special clamped B-spline;
- de Boor reduces to de Casteljau for the corresponding endpoint-only knot
  vector.

Decision impact:

- one-knot insertion into an existing cubic Bézier supplies a strong
  independent parity fixture for the selected two-span representation.

### B-spline derivatives

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/bspline-derv.html

Relevant evidence:

- the derivative of a degree-`p` B-spline is a degree-`p-1` B-spline with
  derived control points;
- higher derivatives follow recursively;
- a clamped B-spline passes through its first/last control points and has
  endpoint tangent relations tied to the first/last control-polyline legs.

Decision impact:

- D1/D2 are part of the first work unit;
- tests can use independently derived derivative control polygons.

### NURBS relation

Reference:
https://pages.mtu.edu/~shene/PUBLICATIONS/2004/NURBS.pdf

Relevant evidence:

- B-spline requires control points, knot vector and degree;
- NURBS adds weights and rational basis normalization;
- setting all NURBS weights to one reduces it to a B-spline.

Decision impact:

- B-spline semantics should be isolated before combining knots and rational
  weights into NURBS;
- the repository already has separate rational-weight evidence from the
  rational-quadratic work unit.

### Mature CAD B-spline semantics

Open CASCADE reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Relevant evidence:

- production CAD B-splines expose degree, knots and multiplicities;
- general B-splines may be rational/non-rational and
  periodic/non-periodic;
- continuity depends on degree/multiplicity.

Decision impact:

- a general B-spline class would combine too many new semantics for the next
  bounded work unit;
- fixed degree, non-rational, non-periodic, clamped and simple-interior-knot
  scope is intentionally narrower.

External sources are design/reference evidence only. No external runtime
dependency is admitted.

## 4. Candidate comparison

| Candidate | New concept added now | Semantic burden | Reuses current work | Downstream value | Decision |
| --- | --- | --- | --- | --- | --- |
| Two-span clamped cubic polynomial B-spline | Knots, multiple spans, local support, C2 knot continuity, de Boor | **Bounded**: fixed degree/control count/one simple knot | Cubic Bézier parity + trimming | High bridge to B-spline surfaces/NURBS | **SELECTED** |
| General bounded B-spline | Same plus arbitrary degree/count/multiplicity | High | Yes | High | DEFER |
| NURBS | Knots + local support + general rational weights | Very high if B-spline not isolated first | Rational layer reusable later | Very high | DEFER |
| Arbitrary-degree Bézier | Degree/storage generalization | Moderate | Extends cubic/rational work | Moderate; still no local support | DEFER |
| Analytic conic | Native CAD analytic family | 3D supporting-plane/orientation + periodic/angular semantics | Rational conic already available | High | BLOCKED/DEFER |
| Heterogeneous composition | Runtime chain/storage/dispatch | Type erasure/variant/ownership/join semantics | Trim reusable | High | DEFER |

## 5. Decision

Authorize exactly one future implementation work unit:

**Two-Span Clamped Cubic Polynomial B-Spline Representation in 2D and 3D.**

The family is deliberately fixed:

- polynomial, not rational;
- degree exactly three;
- exactly five control points;
- exactly one finite simple interior knot;
- clamped endpoint knots with multiplicity four;
- exactly two nonzero knot spans;
- non-periodic;
- bounded;
- D1 and D2 required;
- no dynamic-degree or dynamic-control-point container.

This is the smallest non-Bézier B-spline that introduces a genuine interior
knot and local support.

## 6. Mathematical representation

Let five finite control points be

`P0,P1,P2,P3,P4`.

Let finite knot values satisfy

`a < k < b`.

The full degree-three knot vector is exactly

`U = [a,a,a,a,k,b,b,b,b]`.

Thus:

- degree `p=3`;
- control-point last index `n=4`;
- knot last index `m=8`;
- `m=n+p+1`;
- curve domain is
  `[U_p,U_(n+1)] = [a,b]`;
- the only nonzero knot spans are
  `[a,k)` and `[k,b]`.

The curve is

`C(u) = sum_(i=0)^4 N_(i,3)(u) P_i`

for `u in [a,b]`.

## 7. Continuity scope

The single interior knot `k` has multiplicity one.

For cubic degree `p=3`, the representation is therefore `C2` at `k`.

The first work unit does not admit:

- repeated interior knots;
- C1/C0 knot joins;
- discontinuous joins;
- per-query derivative-continuity failure semantics.

Because the only interior knot is simple, value, D1 and D2 are expected to be
defined over the complete closed domain.

## 8. Construction semantics

The proposed production family may be named:

- `TwoSpanCubicBSpline2`;
- `TwoSpanCubicBSpline3`.

Construction must be validated.

At minimum reject:

- non-finite `a`;
- non-finite `k`;
- non-finite `b`;
- `a >= k`;
- `k >= b`.

No epsilon defines order.

Control-point finiteness remains guaranteed by existing `Point2/Point3`
construction.

A bounded construction-error vocabulary is authorized.

The implementation may store either:

- five control points plus `a,k,b`; or
- five control points plus the exact nine-entry full knot vector.

The public API must make the actual knot semantics inspectable and must not
silently normalize the knot domain.

## 9. Parameter domain

`parameter_domain()` returns exactly `[a,b]`.

Required query failures:

- NaN/infinity parameter:
  `CurveError::non_finite_parameter`;
- finite parameter below `a` or above `b`:
  `CurveError::parameter_out_of_domain`.

No clamp and no periodic wrap are allowed.

Required endpoint values:

- `C(a)=P0`;
- `C(b)=P4`.

These endpoint values should be returned exactly.

## 10. Production evaluation algorithm

Production point evaluation should use a fixed cubic de Boor construction.

The implementation must:

- identify the left/right nonzero span deterministically;
- define the interior-knot query `u==k` deterministically;
- use only the locally active four controls for one span;
- avoid converting to a global power basis;
- avoid avoidable intermediate overflow in interpolation ratios;
- return `CurveError::non_finite_result` if the final point is not
  representable.

A private scale-aware parameter-ratio helper is authorized.

No common numeric or parametric-curve API change is expected.

## 11. D1 and D2

D1 and D2 must be derived from the standard derivative B-spline relations or an
algebraically equivalent fixed-degree algorithm.

For the first derivative, derived degree-two controls satisfy the standard
relation

`Q_i = 3 * (P_(i+1)-P_i) / (U_(i+4)-U_(i+1))`

for `i=0..3`.

The derivative knot sequence is the original knot sequence with one endpoint
knot removed at each side.

D2 may be obtained by applying the derivative relation again.

Required semantics:

- finite representable D1/D2 succeed over the full closed domain;
- an unrepresentable final derivative returns
  `CurveError::non_finite_result`;
- no derivative is clamped, normalized or replaced by zero after failure;
- no regularity certification is introduced.

The implementation must avoid a known anti-pattern: forming an overflowing
point difference before division when an equivalent scale-aware computation
could produce a finite representable derivative.

## 12. Endpoint tangent relations

For the clamped representation:

`C'(a) = 3/(k-a) * (P1-P0)`

and

`C'(b) = 3/(b-k) * (P4-P3)`.

These relations are mandatory focused evidence when representable.

They provide an independent endpoint oracle separate from de Boor point
evaluation.

## 13. Local support

The first span `[a,k)` depends on

`P0,P1,P2,P3`

and not on `P4`.

The second span `[k,b]` depends on

`P1,P2,P3,P4`

and not on `P0`.

Focused evidence must demonstrate that modifying:

- only `P4` does not change an interior query in the first span;
- only `P0` does not change an interior query in the second span.

This is the first production curve-family contract in AP Mesh that explicitly
tests B-spline local support.

## 14. Bézier parity through one-knot insertion

A cubic Bézier over `[a,b]` is a clamped cubic B-spline with only endpoint
knots.

Insert one simple knot `k` with normalized position

`t = (k-a)/(b-a)`

using an overflow-safe ratio.

For original cubic controls `B0,B1,B2,B3`, the one-insertion controls are:

- `P0=B0`;
- `P1=lerp(B0,B1,t)`;
- `P2=lerp(B1,B2,t)`;
- `P3=lerp(B2,B3,t)`;
- `P4=B3`.

The resulting two-span cubic B-spline represents the same physical polynomial
cubic as the original Bézier, with parameter `u` mapped to Bézier parameter
`(u-a)/(b-a)`.

Focused tests must use this relation as a qualified-prerequisite parity oracle
for value, D1 and D2, including the required parameter derivative scale factors.

This avoids validating the B-spline implementation only against another
B-spline formula.

## 15. Independent basis-summation oracle

Focused tests must additionally implement an independent reference based on
Cox–de Boor basis recursion or explicit two-span basis polynomials.

This reference must be separate from production de Boor code.

It must cover at minimum:

- one asymmetric non-Bézier control fixture;
- a non-midpoint interior knot;
- values on both spans;
- value at the interior knot;
- D1/D2 reference evidence.

This second oracle is required because Bézier-parity fixtures cover only the
subclass produced by knot insertion into one global cubic polynomial.

## 16. Interior-knot evidence

At the simple interior knot `k`, focused evidence must show:

- one unambiguous value;
- finite D1;
- finite D2;
- independent agreement with the reference basis/derivative calculation.

Near-knot probes from each side may supplement this evidence, but sampled
near-equality alone is not a proof of C2 continuity.

The C2 claim derives from the fixed representation invariant
degree-three + multiplicity-one, while tests verify implementation consistency
with that representation.

## 17. Reversal

Reversal preserves the physical locus and domain `[a,b]`.

Required reversed representation:

- control points:
  `P4,P3,P2,P1,P0`;
- lower/upper knots remain `a,b`;
- reversed interior knot:
  `k_r = reversed_parameter([a,b],k)`.

The implementation must reuse the existing overflow-aware reversal primitive
rather than naïvely compute `a+b-k`.

For `r(u)=reversed_parameter([a,b],u)`:

- `reverse(C)(r(u))=C(u)`;
- `reverse(C)'(r(u))=-C'(u)`;
- `reverse(C)''(r(u))=C''(u)`.

Double reversal must recover the exact stored representation.

## 18. Affine/embedding invariants

Focused evidence must include:

- translation covariance;
- exact power-of-two uniform scaling where representable;
- 2D/3D embedding parity for a planar `z=0` fixture;
- deterministic repeated evaluation/failure evidence.

No arbitrary-angle frame qualification is implied.

## 19. Extreme finite numeric evidence

At least one fixture must use finite extreme-scale knot values or control
coordinates to expose naïve subtraction/interpolation overflow risks.

The required outcome is:

- succeed when the final value/D1/D2 is representable and an equivalent
  overflow-aware computation exists;
- otherwise fail explicitly with `CurveError::non_finite_result`.

No universal epsilon or hidden coordinate rescaling is introduced.

## 20. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | 2D/3D types satisfy existing bounded-parametric concepts. |
| Knot validation | Non-finite or non-strict `a<k<b` rejected. |
| Exact domain | Domain is exactly `[a,b]`, not normalized. |
| Endpoint values | Exact P0/P4. |
| Endpoint D1 | Independent clamped endpoint tangent formulas. |
| Left/right span value | Independent basis-summation reference. |
| Interior knot | Value/D1/D2 agree with independent reference. |
| Local support | P4 isolation on left span and P0 isolation on right span. |
| Bézier knot-insertion parity | Value/D1/D2 agree with existing cubic Bézier after parameter scaling. |
| Reversal | Knot/control reversal, involution and value/D1/D2 covariance. |
| Query failures | Existing typed non-finite/out-of-domain failures. |
| Translation/scale | Declared affine covariance. |
| 2D/3D embedding | x/y parity and zero z. |
| Extreme finite | No avoidable overflow; explicit unrepresentable-result failure. |
| Determinism | Repeated fields/failures identical. |
| Header isolation | No topology, surface, meshing, I/O, threading or third-party dependency. |
| Prerequisite preservation | Existing 21 ordinary tests remain passing plus one B-spline contract. |

If exactly one focused test is added, ordinary FAST/INTEGRATION inventory
becomes **22 tests**.

## 21. Why not general B-spline yet

A general B-spline implementation would require several independent policies
simultaneously:

- dynamic degree;
- dynamic control count;
- arbitrary knot count;
- repeated interior knots;
- continuity classification;
- possible non-clamped endpoints;
- periodicity;
- allocation/resource bounds.

Those are scientifically separable from the first question:

**Can AP Mesh correctly represent/evaluate a knot-partitioned, locally
supported, C2 cubic curve while preserving all current contracts?**

The two-span work unit answers that question without committing the general
container/API design prematurely.

## 22. Why not NURBS yet

NURBS combine:

- B-spline knots/local support; and
- rational weights/denominator semantics.

The repository has isolated rational semantics, but it has not yet isolated
B-spline semantics.

Therefore B-spline must precede NURBS in the current evidence chain.

This is decomposition, not a claim that polynomial B-spline is more general
than NURBS.

## 23. Why not arbitrary-degree Bézier yet

Arbitrary-degree Bézier would generalize degree/storage but retain global
Bernstein support.

It would not exercise:

- an interior knot;
- multiple parameter spans;
- local control support;
- continuity across a knot.

The selected B-spline work unit adds those missing concepts directly.

Arbitrary-degree Bézier remains a valid later decision and may become useful
for degree-elevation/compatibility tooling.

## 24. Why not analytic conic yet

The repository already represents conic segments through rational quadratic
Bézier.

A dedicated analytic 3D conic still requires a supporting-plane orientation
semantics beyond the currently qualified Cartesian-frame claim.

That prerequisite must not be bypassed by pretending the current frame is an
arbitrary rotation frame.

## 25. Why not heterogeneous composition yet

Heterogeneous polycurve storage still requires an explicit runtime
representation choice:

- type erasure;
- variant-like closed family set;
- virtual base;
- another bounded alternative.

The newly integrated trim wrapper is statically typed and deliberately does
not answer that question.

Composition remains deferred until its storage/identity/join semantics receive
their own decision.

## 26. Explicit exclusions

This decision does not authorize:

- more than two B-spline spans;
- more or fewer than five controls;
- degree other than three;
- repeated interior knots;
- non-clamped endpoint multiplicities;
- periodic/closed B-spline semantics;
- rational B-spline/NURBS weights;
- arbitrary-degree Bézier;
- knot insertion/removal as production mutation;
- degree elevation/reduction as production mutation;
- interpolation/fitting;
- heterogeneous composition;
- analytic conic;
- arbitrary 3D frame/plane expansion;
- regularity/arc-length/curvature generic refactors;
- surface B-splines/NURBS;
- boundary discretization;
- sizing/meshing;
- Quad-Dominant work;
- parallel execution;
- third-party runtime dependencies;
- a representation-breadth qualification campaign.

## 27. Repository mapping for future implementation

Authorized future mapping is limited to:

- public family:
  `include/apmesh/geometry/bspline.hpp`;
- production implementation:
  `src/geometry/bspline.cpp`;
- focused semantic/header contract:
  `tests/two_span_cubic_bspline.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision.

No change to `parametric_curve.hpp` is expected.

If common concept semantics must change, stop and require a new decision.

## 28. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 21 ordinary semantic tests must remain passing;
- the new B-spline focused contract must pass.

Passing yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify general B-spline, NURBS or spline surfaces.

## 29. Stop conditions

Stop and require a new decision if implementation needs:

- a change to `BoundedParametricCurve2/3`;
- dynamic degree or dynamic control/knot containers;
- repeated interior knots;
- periodicity;
- rational weights;
- general continuity-error vocabulary;
- topology/surface/meshing code;
- a second new representation family;
- a universal tolerance;
- a third-party dependency.

## 30. Planned sequence after this work unit

Planning only, not authorization.

After two-span cubic B-spline implementation/closure, a fresh decision should
compare:

1. general bounded clamped B-spline expansion;
2. NURBS;
3. arbitrary-degree polynomial/rational Bézier;
4. analytic conic after orientation prerequisites;
5. heterogeneous composition.

The next winner must be selected from fresh repository/literature evidence.

## 31. Effect if integrated and closed

After decision integration, post-merge validation and checkpoint closure, the
sole next implementation work item is:

**Two-Span Clamped Cubic Polynomial B-Spline Representation in 2D and 3D.**

No implementation begins on this decision branch.

Curve Differential Geometry remains paused/unqualified.

Boundary Curve Discretization and Surface Representation remain blocked until
their prerequisite representation-breadth decisions are explicitly closed.


## 32. Decision integration checkpoint

PR #116 integrated this bounded decision as
`0978256b53d8eba7f974229da06cd74b21d3ee53`.

Final decision-head validation:

- FAST `35732529892`: PASS;
- INTEGRATION `35732529957`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35734755167`: PASS;
- INTEGRATION `35734755301`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The integrated decision selects only:

**Two-Span Clamped Cubic Polynomial B-Spline Representation in 2D and 3D.**

This decision checkpoint is ready for documentation/continuity closure.

After closure integration and its post-merge validation, the sole next work
item is the implementation bounded by Sections 5–29.

General B-spline, NURBS, arbitrary degree/count, repeated interior knots,
periodicity, analytic conics, heterogeneous composition, surfaces and
downstream meshing remain unauthorized.


## 33. Active implementation mapping

Decision closure PR #117 merged as
`5abcc8bd512097e1ae5881e1643f67b89420e1dc`.

Closure post-merge validation:

- FAST `35735198199`: PASS;
- INTEGRATION `35735198173`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The sole authorized implementation is active on:

`curve/two-span-cubic-bspline`.

Candidate repository mapping:

- `include/apmesh/geometry/bspline.hpp`;
- `src/geometry/bspline.cpp`;
- `tests/two_span_cubic_bspline.cpp`;
- `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

Candidate semantics:

- fixed degree-three, five-control, two-span polynomial B-spline;
- full inspectable knot vector `[a,a,a,a,k,b,b,b,b]`;
- validated finite strict `a<k<b`;
- exact bounded parameter domain `[a,b]`;
- de Boor point evaluation;
- fixed derivative control polygons for D1/D2;
- typed non-finite/out-of-domain/unrepresentable-result failures;
- reflected-knot reversal using the existing common primitive;
- unchanged common bounded-parametric concepts.

Focused evidence covers every Section 20 obligation using an independent
Cox–de Boor/basis-derivative oracle plus qualified cubic-Bézier knot-insertion
parity.

Expected ordinary FAST/INTEGRATION inventory after registration: **22 tests**.

Initial PR validation on head
`25ca05e122ab3961d70702fd6322b68b39f108e9` did not reach semantic test
execution:

- FAST `35736203787`: FAIL during focused-test compilation;
- INTEGRATION `35736203805`: FAIL in both GCC 13 Debug and Clang 18/libc++
  Debug during the same focused-test compilation.

The failure is mechanical and isolated to
`tests/two_span_cubic_bspline.cpp`: a temporary
`std::array<Point2,5>{}` required default construction of `Point2`, which
the validated geometry value type intentionally does not provide.

The production B-spline source compiled before the focused test failed.

Correction commit
`e42484c6c81163b13bd01761603421dcfff34ff1` changes only fixture
initialization by copying an already valid control array before replacement.
No production code, mathematical semantics, reference oracle, expected result
or acceptance criterion changed.

Corrected candidate validation:

- candidate head:
  `ee733a1fbd779cfb4256a19d9e39d1adbf5e9cc0`;
- FAST `35736410584`: PASS, 22/22 tests;
- INTEGRATION `35736410585`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug, 22/22 tests in each cell;
- `apmesh_core.two_span_cubic_bspline`: PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS / FINAL DOCUMENTATION-SYNC
REVALIDATION PENDING / NOT QUALIFIED.**

The initial failed validation remains retained above as part of the work-unit
evidence and was not overwritten or reinterpreted.

No general B-spline, NURBS or downstream capability is implied.


## 34. Implementation integration checkpoint

PR #118 integrated the authorized work unit as
`c336460b751fa600c893aa6a96f9d594cdcd9a9e`.

Validation lineage:

- initial PR head:
  `25ca05e122ab3961d70702fd6322b68b39f108e9`;
- initial FAST `35736203787`: FAIL during focused-test compilation;
- initial INTEGRATION `35736203805`: FAIL in GCC 13 and Clang 18 during
  the same focused-test compilation;
- production `src/geometry/bspline.cpp` compiled before the focused test
  failed;
- correction commit
  `e42484c6c81163b13bd01761603421dcfff34ff1` changed only fixture
  initialization;
- corrected candidate:
  `ee733a1fbd779cfb4256a19d9e39d1adbf5e9cc0`;
- corrected FAST `35736410584`: PASS, 22/22 tests;
- corrected INTEGRATION `35736410585`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug, 22/22 tests per cell;
- final documentation-synchronized head:
  `01556ff836ea9f49e907c45c40f5a824bba3622e`;
- final PR FAST `35736642982`: PASS;
- final PR INTEGRATION `35736642765`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug;
- post-merge FAST `35736839516`: PASS;
- post-merge INTEGRATION `35736839526`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug.

Integrated production scope:

- `TwoSpanCubicBSpline2`;
- `TwoSpanCubicBSpline3`;
- degree exactly three;
- five control points;
- full knots `[a,a,a,a,k,b,b,b,b]`;
- strict finite `a<k<b`;
- two nonzero spans and one simple interior knot;
- de Boor value evaluation;
- fixed derivative B-spline D1/D2;
- endpoint, local-support, knot, reversal, knot-insertion parity,
  affine/embedding, extreme-finite and determinism evidence.

The common bounded-parametric concepts were not changed.

Work-unit result:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

This result does not qualify general B-spline, NURBS, arbitrary-degree Bézier,
analytic conics, heterogeneous composition, spline surfaces or downstream
meshing.

After implementation closure integration/post-merge validation, a fresh
literature-backed decision is mandatory before another representation-breadth
work item.
