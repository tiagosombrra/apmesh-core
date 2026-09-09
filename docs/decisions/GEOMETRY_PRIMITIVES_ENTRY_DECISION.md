# Geometry Primitives — Bounded Entry Decision

Status: IMPLEMENTED / FOCUSED CONTRACT PASS / QUALIFICATION INFRASTRUCTURE READY
Date: 2026-09-08
Stage: Geometry Primitives — Exact Semantics Before Curves
Prerequisite: Foundation `QUALIFIED` at candidate `b333755`

## Question

What is the smallest scientifically defensible geometry capability that may be
implemented after Foundation without importing topology, curve, surface, mesh,
or future linear-algebra abstractions prematurely?

## Decision

Authorize one implementation work unit for distinct two- and three-dimensional
point and vector value types and their immediately required affine and Euclidean
operations. The work unit must preserve the qualified Architecture, Numeric,
and Reproducible Experiment contracts.

This decision specifies semantics and evidence obligations. It does not select
the final spelling of every C++ function and does not itself implement geometry.

## Sources and decision impact

| Question | Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- | --- |
| Point/vector semantics | CGAL `Point_3` and `Vector_3` references | A point and a vector are distinct concepts; point subtraction yields a vector and point translation consumes a vector. | That CGAL must be a dependency or that its implementation is an oracle. | Use separate `Point2`/`Point3` and `Vector2`/`Vector3` types. |
| Affine transformations | CGAL `Aff_transformation_3` reference | Translation acts differently on points and displacement vectors; affine composition can be represented explicitly. | That a transformation type belongs in the first work unit. | Preserve the semantic distinction now; defer transformation objects. |
| Floating-point behavior | IEEE 754-2019, Goldberg 1991, and Higham 2002 | Finite values, exceptional values, rounding, scaling, conditioning, and error classification must be explicit. | That approximate equality is transitive or that one global epsilon is valid. | Reuse the qualified Numeric Contract and reject silent non-finite propagation. |
| Robust predicate signs | Shewchuk 1997 | Near-degenerate orientation decisions require certified sign handling rather than an arbitrary epsilon. | That raw dot/cross products qualify as topological predicates. | Defer orientation/incircle predicates and all topology-changing decisions. |

These sources are references for semantics and verification design. No external
geometry or linear-algebra dependency is admitted by this decision.

## Authorized scope

The first implementation work unit may add only:

- `Point2`, `Point3`, `Vector2`, and `Vector3` as distinct fixed-dimension value
  types using the already qualified scalar type `double`;
- finite-coordinate construction and component access;
- vector addition, subtraction, negation, finite scalar multiplication and
  division;
- point translation by a vector and subtraction of two points to obtain a
  vector;
- dot product in 2D/3D and cross product in 3D;
- stable Euclidean norm and vector normalization;
- exact coordinate equality as a numeric relation, explicitly not geometric
  coincidence or topological identity;
- explicit result/error reporting for invalid numeric input, non-finite result,
  zero-length normalization, and an uncertifiable operation;
- focused compile-time, analytic, adversarial, and deterministic tests.

A zero vector is a valid vector. Normalizing it is a `degenerate` operation.
NaN and infinities are not valid geometry. Positive and negative zero are
numerically equal; byte canonicalization remains an experiment concern.

Every operation that can overflow, divide by zero, or produce a non-finite
result must fail explicitly. It must not clamp, substitute zero, retry, or use a
hidden tolerance. Exact API names may be selected in the implementation, but
the distinctions `invalid_numeric`, `degenerate`, `ill_conditioned`, and
`indeterminate` must not be collapsed.

## Explicitly excluded

This entry decision does not authorize:

- topological IDs, incidence, adjacency, merging, or coordinate-based identity;
- orientation, incircle, intersection, sidedness, or other certified predicates;
- matrices, eigensystems, coordinate-frame objects, or general transforms;
- segments, lines, planes, bounding boxes, curves, surfaces, patches, or CAD;
- length integration, curvature, sizing, discretization, meshing, or adaptation;
- dimensional-analysis/unit libraries, dynamic dimensions, SIMD, or GPU paths;
- OpenMP/MPI behavior or native-Windows qualification;
- a third-party runtime dependency;
- comparison with the legacy implementation as an acceptance oracle.

Small linear algebra and transformations remain separate investigation problems.
They may be opened only after the point/vector semantics are qualified and an
actual downstream need is demonstrated.

## Hypotheses

### Semantic separation

Invalid affine expressions are unrepresentable through the public API: point
minus point yields vector; point plus vector yields point; vector plus vector
yields vector; point plus point and scalar multiplication of a point are not
admitted operations.

### Finite-state preservation

Finite valid inputs either produce a finite result satisfying the operation's
postcondition or an explicit classified failure. Non-finite input cannot enter a
valid geometry value through the public construction boundary.

### Analytic agreement

For exactly representable fixtures, component, affine, dot, cross, and selected
norm results agree exactly where IEEE binary arithmetic makes exact agreement a
declared invariant. Derived results requiring rounding are assessed only through
an explicit `ProximityPolicy` with a justified reference scale.

### Scale behavior

Exact power-of-two rescaling preserves the declared affine and directional
relations wherever no overflow or underflow occurs. A numerically nonzero vector
must not be classified as the zero vector merely because a naive intermediate
underflowed.

### Determinism and isolation

Claim fields are deterministic across the qualified GCC/Clang Debug/Release
envelope. The implementation has no filesystem, environment, logging, topology,
or meshing dependency.

## Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Origin and basis | Zero points/vectors and Cartesian basis vectors expose the declared components exactly. |
| Affine closure | `p - p` is the zero vector; `(p + v) - p` recovers `v`; `(p + v) - v` recovers `p` for exactly representable fixtures. |
| Type separation | Compile-time checks reject point-plus-point and scalar-times-point while accepting the authorized result types. |
| Dot basis | `e_i · e_j` equals the Kronecker pattern for basis vectors. |
| Cross basis | `e1 × e2 = e3`, antisymmetry holds, and parallel basis multiples give the zero vector. No predicate-sign claim is made. |
| Euclidean norm | Axis vectors and the `(3,4)` fixture produce exact declared lengths; normalization of an axis produces the same axis. |
| Zero normalization | Positive-zero, negative-zero, and mixed signed-zero vectors produce explicit `degenerate`, not a fabricated unit vector. |
| Finite envelope | `lowest()`, `min()`, `denorm_min()`, ordinary normals, and safe powers of two remain classified according to the Numeric Contract. |
| Invalid input | Every NaN and positive/negative infinity placement is rejected at construction. |
| Non-finite intermediate | Deliberate overflow in addition, scaling, dot, or cross produces explicit failure. |
| Stable small magnitude | A representable nonzero subnormal-direction fixture is not silently reclassified as zero by norm evaluation. |
| Repeatability | Repeated executions produce equivalent claim fields; permitted provenance differences remain outside scientific equality. |

Analytic fixtures must be generated independently in the tests rather than copied
from the legacy AP Mesh implementation. Approximate checks must record their
policy and residual; a default epsilon is forbidden.

## Admission and stop conditions

The implementation candidate may proceed to focused validation only when:

1. its public API preserves the point/vector and geometry/topology distinctions;
2. all fallible operations expose classified failure;
3. the complete case table above is represented by tests;
4. no new runtime dependency, global tolerance, fallback, or future abstraction
   is introduced; and
5. Foundation contracts remain passing.

Stop the work unit if a case requires a topological predicate, an undeclared
numeric acceptance rule, a general matrix abstraction, or reinterpretation of
the Numeric Contract. Such a finding requires a separate bounded decision.

## Effect on the roadmap

Geometry Primitives moves from `NOT STARTED` to `IN INVESTIGATION / ENTRY
DECISION APPROVED`. Scientific implementation and qualification remain pending.
Foundation stays qualified and is not reopened by this documentation-only
decision.

## Implementation result

The bounded candidate adds only `include/apmesh/core/geometry.hpp`,
`src/core/geometry.cpp`, and `tests/geometry_primitives.cpp`, with the focused
CTest label `geometry;contract`. It implements the authorized point/vector
construction, affine operations, dot/cross, stable norm, normalization, and
classified failure paths. No matrix, predicate, topology, curve, surface, or
meshing code was introduced.

The focused contract passed on the qualified GCC 13 and Clang 18/libc++
toolchains in both Debug and Release. It covers type separation, analytic basis
fixtures, signed zero, finite extrema, subnormal normalization, invalid input,
and each fallible operation's deliberate overflow or division failure.

This is implementation evidence only. It does not yet establish a
revision-bound work-unit certificate, cross-compiler claim-field equivalence,
native Windows behavior, or Point/Vector qualification. The bounded
qualification protocol is pre-registered in
`docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md`; it explicitly
does not qualify the complete Geometry Primitives stage.

## Next bounded action

Review and publish the Point/Vector qualification infrastructure package. The
focused contracts pass, but do not prepare or execute the formal qualification
regression and do not implement matrices, transforms, predicates, topology,
curves, or meshing.
