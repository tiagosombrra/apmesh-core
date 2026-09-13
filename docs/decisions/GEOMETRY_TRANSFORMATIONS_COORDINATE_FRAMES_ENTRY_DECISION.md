# Geometry Primitives - Transformations and Coordinate Frames Entry Decision

Status: ACCEPTED FOR BOUNDED IMPLEMENTATION / NOT IMPLEMENTED
Date: 2026-09-13
Stage: Geometry Primitives - Exact Semantics Before Curves
Prerequisites: Foundation `QUALIFIED`; Point/Vector Semantics `QUALIFIED`;
Minimal Small Linear Algebra `QUALIFIED`

## 1. Question and downstream need

What is the smallest transformation and coordinate-frame capability needed to
test later curve and surface constructions under changes of origin, Cartesian
axis labeling, exact quarter turns, reflections, and scale without introducing
general affine algebra or numerical inversion prematurely?

The immediate downstream need is analytic verification. Later curve and
surface fixtures must be expressible in a local frame and mapped into world
coordinates while preserving the already qualified distinction between points
and displacement vectors. No curve or surface implementation is authorized by
this decision.

## 2. Basis and interpretation

The qualified Point/Vector and Minimal Small Linear Algebra contracts remain
normative. The Numeric Contract controls finite values, scale, explicit
failure, and any future proximity rule. CGAL's public kernel documentation is
used only as mature interface evidence that points, vectors, and affine
transformations are distinct concepts and that translation affects points and
vectors differently. CGAL is neither a dependency nor a numerical oracle.

The decision deliberately narrows the general affine model. A general affine
transformation supports arbitrary linear parts and inversion; neither is
required for the first downstream analytic fixtures. The admitted candidate is
an exactly invertible Cartesian similarity frame.

## 3. Candidate hypothesis

A 2D or 3D frame represented by:

- one finite world-space origin;
- one already qualified `Mat2` or `Mat3` whose entries form an exact signed
  permutation matrix; and
- one positive power-of-two scale represented by an integer exponent;

is sufficient to provide deterministic local-to-world and world-to-local maps
for the first curve/surface analytic fixtures. Because a signed permutation
matrix has inverse equal to its transpose and a power-of-two scale has an exact
reciprocal inside the admitted exponent envelope, this capability requires no
general inverse, solve, decomposition, tolerance, or predicate.

The hypothesis is rejected or narrowed if implementation needs arbitrary-angle
rotation, approximate orthonormality, a general matrix inverse, a determinant
sign decision, or a hidden acceptance tolerance.

## 4. Authorized capability

One bounded implementation work unit may add concrete 2D and 3D Cartesian
frame value types in the existing Geometry layer. The descriptive candidate
names are `CartesianFrame2` and `CartesianFrame3`; implementation review may
adjust spelling but not semantics.

Each frame may expose only:

- validated construction from a qualified point origin, matching matrix basis,
  and integer power-of-two scale exponent;
- identity construction;
- read-only access to origin, basis, and scale exponent;
- local-to-world mapping for the matching point type;
- local-to-world mapping for the matching vector type;
- world-to-local mapping for the matching point type; and
- world-to-local mapping for the matching vector type.

The maps are defined by

```text
point_to_world(p)  = origin + 2^k B (p - local_origin)
vector_to_world(v) =          2^k B v
point_to_local(q)  = local_origin + 2^-k B^T (q - origin)
vector_to_local(w) =                2^-k B^T w
```

where `B` is the signed permutation basis and `k` is the scale exponent. Point
and vector overloads remain statically distinct. `local_origin` is the
dimension-matched zero point: point coordinates enter matrix application only
after point-minus-point yields a vector, and the local result becomes a point
only through point-plus-vector. A matrix never accepts a point operand.
Translation must never be applied to a vector.

The work unit may add explicit Geometry error categories for an invalid frame
basis and an exponent outside the finite reciprocal-safe envelope. Arithmetic
that produces a non-finite result retains the qualified
`GeometryError::non_finite_result` meaning. No existing error is renamed or
reinterpreted.

## 5. Exact basis and scale rules

An admitted basis contains exactly one entry in `{ -1, 1 }` in each row and
column and zero elsewhere. Positive and negative zero are the same basis zero.
Validation is exact and does not use a tolerance. Both reflecting and
non-reflecting signed permutations are admitted, but no handedness,
orientation, or determinant-sign claim is made.

The scale is strictly positive and generated as `2^k`. Construction succeeds
only when both `2^k` and `2^-k` are finite and nonzero in `double`. This is an
operation-owned representability check, not a global exponent threshold or a
general unit system.

## 6. Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Identity | Identity frame preserves every admitted point and vector exactly. |
| Translation separation | Changing the origin translates points and leaves vector mapping independent of the origin. |
| 2D quarter turn | The exact signed-permutation quarter turn maps Cartesian basis points/vectors to independently declared results. |
| 3D axis cycle | A fixed cyclic basis maps all three axes to independently declared results. |
| Reflection without predicate | A reflecting signed permutation maps values correctly but emits no orientation or handedness conclusion. |
| Power-of-two scale | Exponents `{-8, -1, 0, 1, 8}` satisfy independently derived point/vector mappings where results remain representable. |
| Round trip | Local-to-world followed by world-to-local recovers exact fixtures, and the reverse round trip does likewise. |
| Affine compatibility | `point_to_world(p + v)` equals `point_to_world(p) + vector_to_world(v)` for exact fixtures. |
| Difference compatibility | Transforming `p - q` as a vector agrees with the difference of transformed points. |
| Metric scaling | Dot products scale by `2^(2k)` and norms by `2^k` for exact axis fixtures; no general conditioning claim is made. |
| Dimension separation | 2D frames reject 3D operands and 3D frames reject 2D operands at compile time. |
| Invalid basis | Missing, duplicate, non-unit, or non-finite basis entries fail with the exact declared category. |
| Invalid scale | Exponents producing zero, infinity, or a non-finite reciprocal fail explicitly. |
| Non-finite result | Finite operands whose mapped result overflows return explicit failure without clamp or fallback. |
| Determinism | Repeated GCC/Clang Debug/Release claim fields agree under preregistered exact rules. |

Expected results must be computed independently from the production mapping.
Exact hexadecimal comparison uses the already qualified signed-zero policy.
Any later rounded fixture requires an explicit `ProximityPolicy`, reference
scale, residual, and limit before execution; no rounded fixture is admitted in
this first work unit.

## 7. Explicit exclusions

This decision does not authorize:

- a general affine-transform class or homogeneous matrices;
- arbitrary-angle or trigonometric rotation;
- quaternions, Euler angles, axis-angle representations, or interpolation;
- shear, nonuniform scale, projective transforms, or perspective division;
- general matrix inverse, solve, decomposition, conditioning, or
  orthonormalization;
- approximate frame validation or a default/global tolerance;
- determinant-sign, handedness, orientation, rank, degeneracy, incidence, or
  topology decisions;
- normal/covector transformation semantics;
- transformation of curves, surfaces, patches, meshes, or sizing fields;
- allocation, I/O, logging, environment discovery, third-party dependencies,
  OpenMP, MPI, GPU, SIMD, or native-Windows qualification; or
- comparison with the legacy implementation as an acceptance oracle.

No standalone general `Transform2`/`Transform3` abstraction is admitted. The
frame itself is the bounded local/world map; a second representation would be
duplication without a demonstrated downstream need.

## 8. Dependency and behavior obligations

The dependency direction remains:

```text
numeric/base -> math -> geometry
```

The implementation belongs to the existing Geometry layer and may consume
qualified `Mat2`/`Mat3`; `math` must not depend on Geometry. Existing
Point/Vector and Mat behavior is unchanged. Inputs are immutable value types;
no shared mutable or global state is admitted.

Focused implementation tests are required before any qualification protocol.
Passing them establishes implementation evidence only. A later revision-bound
qualification and the cumulative Geometry Primitives regression remain
separate gates.

## 9. Stop conditions

Stop the bounded implementation before broadening scope if:

1. a signed-permutation basis cannot express the first downstream analytic
   fixtures;
2. round-trip behavior requires a general inverse or approximate
   orthonormality test;
3. an accepted Point/Vector or Mat result changes;
4. a non-finite result is hidden by clamping, fallback, or retry;
5. a determinant is used to classify orientation or validity;
6. a new dependency or general algebra abstraction appears necessary; or
7. exact cross-toolchain differences remain unexplained.

Such a finding requires a new scientific decision. It must not be absorbed by
silently expanding this entry decision.

## 10. Decision effect

**Decision: ACCEPTED FOR BOUNDED IMPLEMENTATION.** The next implementation may
add only the capability in sections 3-8 and its focused contracts. This
decision changes no code and creates no qualification claim. Geometry
Primitives remains `IN INVESTIGATION`.

## 11. Next bounded action

Implement the two concrete Cartesian frame value types and the exact focused
cases above in the existing Geometry layer. Do not add general transformations,
curves, surfaces, predicates, topology, meshes, or formal qualification
infrastructure in that work unit.
