# Parametric Curve Family Abstraction and Representation Breadth — Bounded Transition Decision

Status: DECISION APPROVED / IMPLEMENTATION NOT STARTED / REPRESENTATION-BREADTH EXTENSION UNQUALIFIED  
Date: 2026-09-22  
Stage transition: Curve Differential Geometry investigation -> Curve Representation Breadth Gate

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation — Polynomial Cubic Bézier baseline:
  QUALIFIED by CGR0–CGR7 PASS in the same admitted cloud envelope;
- Curve Differential Geometry:
  pointwise curvature, signed planar curvature and certified simple planar
  inflection work units integrated, but the stage remains NOT QUALIFIED.

## 1. Question

What is the smallest architectural/scientific transition that allows the
qualified cubic-Bézier curve baseline to grow toward the required boundary
curve family set without duplicating parameter-domain, evaluation, derivative
and reversal semantics separately for line/arc, rational Bézier, B-spline and
NURBS implementations?

The decision must also determine whether the project should:

1. continue adding Cubic-Bézier-specific differential features first;
2. implement a second concrete curve family directly and generalize later; or
3. define a minimal static parametric-curve semantic contract first, make the
   qualified cubic Bézier types satisfy it without changing their scientific
   outputs, and admit concrete families only in later bounded work units.

## 2. Repository evidence motivating the decision

The fresh repository audit on canonical `main`
`13ec3ac80a88434d73c09ae25c9d542182109c51` establishes:

- the only production continuous curve value types are `CubicBezier2` and
  `CubicBezier3`;
- `include/apmesh/geometry/curve.hpp` exposes value evaluation, first and
  second derivatives, speed, regularity, arc length, inverse arc length and
  curvature through those concrete types;
- `src/geometry/curve.cpp` contains the current curve-family production
  implementation and substantial reusable private templated machinery;
- no production `surface.hpp`, analytic arc/conic curve, rational Bézier,
  arbitrary-degree Bézier, B-spline or NURBS type exists;
- Boundary Curve Discretization already declares a future
  `line/arc/Bezier` regression envelope;
- the qualified CGR0–CGR7 claim is explicitly limited to the polynomial cubic
  Bézier baseline and must not be generalized to families that have not been
  implemented or qualified.

This is an architectural breadth problem, not evidence of a defect in the
qualified cubic-Bézier baseline.

## 3. Decision

Choose option 3.

Authorize exactly one future bounded implementation work unit:

**Bounded Parametric Curve Contract and Cubic Bézier Conformance.**

The work unit will define a minimal compile-time semantic contract for finite
closed-interval 2D and 3D parametric curves used as boundary geometry and make
`CubicBezier2` and `CubicBezier3` satisfy that contract without changing
their existing mathematical results, accepted domains, failure semantics,
determinism, or CGR-qualified behavior.

The first work unit will not implement a second curve family.

Concrete line/segment, circular/conic arc, rational Bézier, arbitrary-degree
Bézier, B-spline, NURBS and composite/trimmed curve representations remain
separate later scientific decisions/work units.

The qualified cubic-Bézier baseline remains frozen as a prerequisite. This
decision is a breadth extension and does not reopen or weaken CGR0–CGR7.

## 4. Literature and best-practice basis

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| Gerald Farin, *Curves and Surfaces for CAGD*, 5th ed. | Bézier and B-spline representations as parametric CAGD constructions and the role of derivatives/evaluation in geometric algorithms. | A required AP Mesh class hierarchy, error vocabulary or runtime polymorphism mechanism. | Preserve representation-specific mathematics while extracting only the semantic operations that genuinely cross families. |
| Piegl and Tiller, *The NURBS Book*, 2nd ed. | B-spline/NURBS curve definitions, knot-domain structure, rational/non-rational relationships and derivative algorithms; NURBS generalize non-rational B-splines and rational/non-rational Bézier curves/surfaces. | That AP Mesh should implement all spline/rational families in one change or use a third-party NURBS kernel. | Do not encode cubic degree, Bernstein control count or normalized `[0,1]` as universal curve semantics. |
| Open CASCADE `Geom_Curve` reference | Mature-kernel common curve behavior includes parameter bounds, evaluation, derivatives, reversal, closed/periodic information and concrete line/conic/Bézier/B-spline implementations. | That AP Mesh should copy OCCT inheritance, exception behavior, fixed tolerances or ownership model. | Confirms a family-independent parametric semantic layer is meaningful, while AP Mesh remains value-oriented and dependency-free. |
| Open CASCADE `Geom_BSplineCurve` reference | B-spline first/last parameters are knot values; knot multiplicity affects continuity; curves may be periodic and rational. | A particular knot container, tolerance policy or AP Mesh spline API. | A universal normalized domain is invalid; continuity/periodicity must not be silently assumed from cubic Bézier behavior. |
| Open CASCADE `Geom_Circle` reference | A circle uses parameter interval `[0,2π]`, is periodic, and reversal maps parameters as `2π-u`. | That all AP Mesh circles must use OCCT conventions or that full-circle periodicity belongs in the first abstraction work unit. | Demonstrates concretely that `[0,1]` is not a universal curve domain and motivates explicit reversal-parameter semantics. |
| C++23 constraints/concepts semantics | Named concepts express compile-time requirements for generic algorithms; constraints reject syntactically nonconforming types during template substitution. | Runtime scientific semantics or mathematical correctness; semantic requirements still require tests/contracts. | Prefer a static C++23 semantic concept over a mandatory virtual base hierarchy for the first abstraction layer. |
| C++ Core Guidelines template/concept guidance | Concepts should model meaningful semantic categories rather than incidental syntax. | Scientific correctness of a geometry abstraction. | The concept must be defined by parametric-curve meaning, not by arbitrary member-name presence alone. |

Reference URLs:

- https://www.sciencedirect.com/book/9781558607378/curves-and-surfaces-for-cagd
- https://home.zcu.cz/~bastl/GM1/the-nurbs-book.pdf
- https://dev.opencascade.org/doc/refman/html/class_geom___curve.html
- https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html
- https://dev.opencascade.org/doc/refman/html/class_geom___circle.html
- https://en.cppreference.com/w/cpp/language/constraints
- https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

External sources are design/reference evidence only. No new runtime dependency is admitted.

## 5. Why the abstraction precedes more Cubic-Bézier differential work

The remaining Curve Differential Geometry candidates include global curvature
bounds, curvature extrema/monotonicity and richer feature classification.

Those are scientifically meaningful, but implementing them first directly on
`CubicBezier2/3` would deepen a public API in which family identity and
generic parametric behavior are still fused.

That creates two later risks:

1. each new curve family may receive a separate copy of domain/evaluation/
   derivative orchestration and error handling; or
2. the project may need a disruptive abstraction retrofit after more
   differential algorithms have already been qualified against concrete-only
   APIs.

The smallest safe intervention is therefore not a general NURBS framework and
not a rewrite of existing curve algorithms. It is a semantic seam that future
algorithms and families can share.

Curve Differential Geometry is paused, not invalidated. Its integrated work
units remain valid for the cubic-Bézier baseline.

## 6. First-work-unit mathematical scope

The first abstraction applies only to a **bounded parametric curve** with one
finite closed parameter interval

`D = [u_min, u_max]`

such that:

- `u_min` and `u_max` are finite;
- `u_min < u_max`;
- every successful evaluation is associated with one finite
  `u ∈ [u_min,u_max]`;
- finite parameters outside the interval fail explicitly;
- non-finite parameters fail explicitly.

This contract deliberately does not model unbounded base lines, piecewise
continuity partitions, periodic wraparound or trimming composition in the first
work unit.

A line **segment** or bounded analytic arc can satisfy the future contract.
An unbounded line cannot until a separate unbounded-curve decision exists.

## 7. Required common semantic operations

A conforming 2D/3D bounded parametric curve must provide semantic equivalents
of:

1. a finite closed parameter domain;
2. point evaluation at a finite in-domain parameter;
3. first derivative with respect to the represented parameter;
4. second derivative with respect to the represented parameter;
5. geometric reversal producing the same physical locus with opposite
   orientation;
6. mapping from an original valid parameter to the parameter of the reversed
   representation for the same physical point.

For a bounded interval `[a,b]`, the admitted first-work-unit reversal map is

`u_reverse = a + b - u`

mathematically, with implementation required to avoid avoidable intermediate
overflow where an equivalent finite result is representable.

The semantic relation is

`reverse(C)(u_reverse) = C(u)`.

For cubic Bézier on `[0,1]`, this reduces to the already qualified relation
`reverse(C)(t) = C(1-t)`.

## 8. Proposed C++23 semantic boundary

The implementation decision may introduce a narrow public header such as:

`include/apmesh/geometry/parametric_curve.hpp`

containing only the common value vocabulary and static concepts.

The preferred mechanism is a C++23 concept/constraint layer rather than a
mandatory virtual base class.

Concept names are implementation details, but the scientific roles are:

- bounded parametric curve in 2D;
- bounded parametric curve in 3D.

The constraints must require the common return/error types already used by the
core where applicable and must not erase failures into booleans or exceptions.

A concept's compile-time satisfaction is only syntactic evidence. Focused
runtime contracts must verify the semantic relations.

## 9. Parameter-domain vocabulary

The first implementation work unit may add a small immutable
`CurveParameterDomain`-like value containing the finite lower and upper
parameters.

Its construction/validation must reject at minimum:

- non-finite lower parameter;
- non-finite upper parameter;
- reversed interval;
- zero-width interval.

No epsilon may turn a positive-width interval into zero width.

The type must support exact read-only access and deterministic containment for
a supplied finite parameter.

It must not:

- normalize arbitrary domains to `[0,1]`;
- silently reorder endpoints;
- clamp parameters;
- wrap periodic parameters;
- store topology identity;
- encode knots, weights or control points.

## 10. Cubic Bézier conformance requirement

`CubicBezier2` and `CubicBezier3` must conform without changing their
qualified scientific semantics.

At minimum:

- their parameter domain remains exactly `[0,1]`;
- existing `evaluate`, `first_derivative` and `second_derivative`
  results remain unchanged;
- existing error behavior for NaN, infinities and out-of-domain parameters
  remains unchanged;
- `reversed()` retains exact control-point reversal;
- the common reversed-parameter relation reduces to `1-t`;
- existing regularity, arc-length, inverse-length, curvature and inflection
  outputs are unaffected;
- all qualified Curve Representation and integrated Curve Differential
  Geometry contracts remain passing.

No compatibility wrapper may become a second mathematical implementation of
Bézier evaluation or derivatives.

## 11. Error/failure semantics

The abstraction must preserve typed failure.

The first work unit may introduce a domain-construction error vocabulary if
needed, but must not replace existing `CurveError` query failures.

A successful common operation must never:

- clamp;
- extrapolate;
- convert NaN/infinity into an endpoint;
- retry with an alternate parameterization;
- insert a hidden tolerance;
- silently normalize the parameter interval;
- convert a derivative failure into a zero vector.

Future B-spline/NURBS continuity failures may require a later extension of the
curve error vocabulary. They are not pre-authorized here.

## 12. Required invariants

### Domain preservation under reversal

Reversal preserves `[a,b]`.

### Reversal involution

`reverse(reverse(C))` represents the original curve exactly where the
concrete representation admits exact value equality.

### Reversed parameter involution

For finite `u∈[a,b]`:

`r(r(u)) = u`

under the declared arithmetic relation, with exact endpoint exchange:

- `r(a)=b`;
- `r(b)=a`.

### Evaluation covariance under reversal

`reverse(C).evaluate(r(u))` agrees with `C.evaluate(u)`.

### First-derivative reversal covariance

For the affine reversal map above:

`reverse(C)'(r(u)) = -C'(u)`.

### Second-derivative reversal covariance

`reverse(C)''(r(u)) = C''(u)`.

### No normalized-domain assumption

Generic code admitted by this contract must obtain the parameter domain from
the curve. It may not hard-code `0`, `1`, or `1-u` except inside a
representation-specific implementation whose domain is itself fixed that way.

### Determinism

The same curve and parameter produce identical scientific fields under the
declared compiler/library envelope, independent of filesystem, environment,
topology identity, thread schedule and unordered iteration.

## 13. Focused evidence required for the first implementation

A dedicated focused contract must cover at minimum:

| Case | Required observation |
| --- | --- |
| Concept satisfaction | `CubicBezier2` satisfies the 2D bounded-parametric-curve contract and `CubicBezier3` satisfies the 3D contract. |
| Negative compile-time fixture | An intentionally incomplete fake type does not satisfy the concept. |
| Domain | Cubic Bézier reports exactly `[0,1]`. |
| Endpoint containment | `0` and `1` are contained exactly. |
| Out-of-domain containment | finite values below/above are rejected. |
| Non-finite domain construction | rejected explicitly. |
| Reversed parameter endpoints | `0 -> 1` and `1 -> 0`. |
| Reversed parameter interior | representative exact values map by `1-t`. |
| Reversal involution | double reversal recovers the same cubic value representation. |
| Evaluation covariance | reversed evaluation agrees with original evaluation at mapped parameter. |
| D1 covariance | mapped first derivative negates. |
| D2 covariance | mapped second derivative agrees. |
| Existing query preservation | all existing cubic value/differential failures remain unchanged. |
| Full curve regression | representation, differential, regularity, arc-length, inverse-length, curvature, signed-curvature and inflection tests remain passing. |
| Header isolation | common abstraction introduces no topology, surface, meshing, I/O, threading or third-party dependency. |

## 14. Focused validation boundary

Before integration of the future implementation:

- FAST must pass with GCC 13 Debug / libstdc++;
- INTEGRATION must pass with GCC 13 Debug / libstdc++;
- INTEGRATION must pass with Clang 18 Debug / libc++;
- all currently selected curve contracts must remain in the test inventory.

The work unit may add compile-time/static assertions and one focused runtime
contract.

Passing the work unit yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify any new concrete curve family.

## 15. Explicit exclusions

This decision does not authorize:

- `LineSegment2` or `LineSegment3`;
- circle or conic arc production types;
- arbitrary-degree Bézier;
- rational Bézier;
- B-spline;
- NURBS;
- offset curves;
- composite curves;
- trimming composition;
- unbounded curve domains;
- periodic wrapping semantics;
- continuity partitions/knot-span iteration;
- control-point/weight/knot storage abstractions;
- a general curve type-erasure container;
- a `std::variant` of all future curve families;
- a virtual base-class hierarchy;
- refactoring every existing cubic algorithm into a generic algorithm;
- global curvature bounds;
- curvature extrema/monotonicity;
- new differential feature classification;
- boundary discretization;
- surface representation;
- sizing;
- mesh generation/optimization/adaptation;
- Quad-Dominant construction;
- parallel execution;
- third-party runtime dependencies;
- a new formal qualification campaign.

## 16. Alternatives considered

### Continue with global curvature bounds/extrema on CubicBezier first

Deferred.

Those investigations remain scientifically valid but deepen family-specific
public coupling before the representation breadth seam exists.

### Implement LineSegment directly, generalize later

Rejected for the next work unit.

It would prove a second family is possible but would still leave generic
algorithms without a declared shared semantic contract.

### Implement NURBS first and use it as the universal representation

Rejected.

NURBS can represent a very broad family, but forcing analytic/low-degree
geometry through one heavy representation would conflate exact native
semantics with one storage form and dramatically widen the first work unit.

### Use a mandatory virtual `Curve` base class

Rejected for the first abstraction.

The project currently uses value semantics and has no demonstrated need for
runtime-subtype ownership, heap allocation or virtual dispatch at this layer.
A static semantic concept is sufficient to express compile-time generic
requirements while retaining concrete value types.

### Use `std::variant` over every supported curve family

Rejected.

The supported family set is intentionally evolving. A closed central variant
would require broad edits for every new family and would mix runtime
dispatch/storage policy with the mathematical contract.

### Static C++23 bounded parametric-curve concept

Chosen.

It is the smallest mechanism that exposes the genuine common semantics while
leaving storage, family-specific mathematics and runtime polymorphism
uncommitted.

## 17. Repository mapping for the future implementation

Authorized future mapping is limited to:

- new common public semantic vocabulary:
  `include/apmesh/geometry/parametric_curve.hpp`;
- cubic declarations/conformance only where necessary:
  `include/apmesh/geometry/curve.hpp`;
- minimal cubic/domain implementation only where necessary:
  `src/geometry/curve.cpp`;
- focused contract:
  `tests/parametric_curve_contract.cpp`;
- cubic public-header/conformance preservation:
  `tests/curve_header_isolation.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- decision/state authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`.

No topology, surface, meshing or qualification tooling file is expected to
change.

If implementation requires a larger mapping, stop and require a new decision.

## 18. Concrete-family sequence after the first work unit

The following is a planning sequence, not blanket implementation
authorization:

1. bounded line/segment representation;
2. bounded circular/conic arc representation;
3. rational Bézier and/or arbitrary-degree polynomial Bézier after comparing
   exact conic and free-form needs;
4. B-spline;
5. NURBS;
6. composite/trimmed boundary-curve semantics.

Each family must receive its own bounded decision defining representation,
domain, derivative/continuity semantics, degeneracies, invariants, independent
fixtures and focused regression.

The sequence may be changed only by a later literature-backed decision.

## 19. Surface relationship

No surface implementation is authorized.

The purpose of the curve abstraction is also to avoid carrying a
Cubic-Bézier-only assumption into future trimming and boundary consistency.

Surface Representation will still require a separate entry decision covering
at minimum:

- tensor-product polynomial Bézier patches;
- Coons/transfinite patches where scientifically required;
- rational Bézier patches;
- B-spline/NURBS surfaces;
- admitted analytic surfaces;
- trimmed-surface semantics;
- explicit topology/surface/boundary identity separation.

## 20. Admission and stop conditions

The future implementation may proceed only if:

1. the existing cubic scientific results remain unchanged;
2. the common domain is explicit and not normalized implicitly;
3. the first abstraction is bounded to a finite closed interval;
4. concepts model semantic operations rather than incidental syntax;
5. no runtime polymorphism is introduced without a separate need;
6. no new curve family is smuggled into the same work unit;
7. no global tolerance is introduced;
8. all current curve tests remain passing in GCC and Clang integration;
9. no third-party runtime dependency is added.

Stop and require a new decision if implementation requires:

- runtime type erasure;
- ownership/lifetime polymorphism;
- periodic parameter wrapping;
- unbounded domains;
- continuity partition APIs;
- generic knot/weight/control-net storage;
- a new domain tolerance;
- changes to existing cubic mathematical results;
- a second concrete curve family;
- surface code;
- downstream discretization or meshing.

## 21. Effect on roadmap if integrated

The current scientific work focus becomes:

**Curve Representation Breadth Gate — Parametric Curve Family Abstraction
Decision Integrated / Cubic Baseline Qualification Preserved / No New Family
Qualified.**

Curve Differential Geometry remains:

**IN INVESTIGATION / POINTWISE CURVATURE INTEGRATED / SIGNED PLANAR CURVATURE
INTEGRATED / SIMPLE-INFLECTION INTEGRATED / NOT QUALIFIED / PAUSED FOR
REPRESENTATION-BREADTH SEAM.**

After decision integration, post-merge validation and a separate checkpoint
closure, the sole next bounded work item is:

**Bounded Parametric Curve Contract and Cubic Bézier Conformance.**

No concrete line/arc/rational/B-spline/NURBS implementation begins
automatically.

Boundary Curve Discretization and Surface Representation remain blocked.


## 22. Decision integration checkpoint

PR #98 integrated this decision as
`12ecbf584751dadb0dd142c485b1cd4f220736d8`.

Final PR-head validation:

- FAST `35711481469`: PASS;
- INTEGRATION `35711481473`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Post-merge validation:

- FAST `35711563476`: PASS;
- INTEGRATION `35711563585`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision checkpoint is closed.

The qualified polynomial cubic-Bézier CGR0–CGR7 baseline remains unchanged.
No concrete new curve family or surface capability was added by the decision.

The sole next bounded implementation work item is:

**Bounded Parametric Curve Contract and Cubic Bézier Conformance**

within Sections 6–17 of this decision.

No line/segment, circle/conic arc, arbitrary-degree/rational Bézier, B-spline,
NURBS, composite/trimmed curve, surface, discretization, sizing, meshing,
Quad-Dominant or parallel work is authorized by this checkpoint.


## 23. Closure synchronization

PR #99 merged the decision-closure checkpoint as
`60e7677323300d4263d53c616b3081dd2fa03d0f`.

Validation:

- final closure PR FAST `35711824443`: PASS;
- final closure PR INTEGRATION `35711824521`: PASS in GCC 13 Debug and
  Clang 18/libc++ Debug;
- post-merge FAST `35711924946`: PASS;
- post-merge INTEGRATION `35711924910`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug.

The decision lifecycle is therefore **CLOSED / SYNCHRONIZED**.

The sole next admissible work item is implementation of **Bounded Parametric
Curve Contract and Cubic Bézier Conformance** within Sections 6–17.

No second concrete curve family, surface, discretization, sizing, meshing,
Quad-Dominant or parallel capability is authorized by this synchronization.
