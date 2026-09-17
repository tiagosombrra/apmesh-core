# Geometry Primitives — Cartesian Similarity Frames Entry Decision

Status: IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED
Date: 2026-09-17
Stage: Geometry Primitives — Exact Semantics Before Curves
Prerequisites: Foundation `QUALIFIED`; Point/Vector Semantics `QUALIFIED`;
Minimal Small Linear Algebra `QUALIFIED`

## Question

What is the smallest local/world coordinate capability needed by later analytic
curve and surface fixtures without introducing general affine algebra, numerical
inversion, a tolerance, or topology semantics?

## Bounded hypothesis

A 2D or 3D Cartesian frame formed by one finite world origin, an exact signed
permutation `Mat2` or `Mat3` basis, and a positive power-of-two scale `2^k` is
sufficient for deterministic point and vector maps in the first analytic
fixtures. Its inverse is exactly the basis transpose with reciprocal `2^-k`;
therefore no general inverse, solve, decomposition, or approximate
orthonormality test is required.

The hypothesis is rejected or narrowed if the admitted fixtures require an
arbitrary-angle rotation, a general matrix inverse, approximate validation, a
hidden acceptance tolerance, or an orientation/topology decision.

## Authorized capability

One implementation work unit may add immutable `CartesianFrame2` and
`CartesianFrame3` value types in the Geometry layer. Each type may provide only:

- validated construction from a matching finite origin, signed-permutation
  basis, and integer scale exponent;
- identity construction and read-only accessors;
- local-to-world and world-to-local maps for the matching point type; and
- local-to-world and world-to-local maps for the matching vector type.

For basis `B`, exponent `k`, and world origin `o`, the admitted maps are:

```text
point_to_world(p)  = o + 2^k B p
vector_to_world(v) =     2^k B v
point_to_local(q)  = 2^-k B^T (q - o)
vector_to_local(w) = 2^-k B^T w
```

Point/vector separation remains static: matrices consume vectors only, and
translation is never applied to a vector.

## Exact validity rules

- Every basis row and column has exactly one entry in `{ -1, 1 }`; all other
  entries are zero. Signed zero is the canonical basis zero.
- A scale is admitted only when both `2^k` and `2^-k` are finite and nonzero in
  `double`.
- Invalid basis and invalid scale receive explicit Geometry errors. A finite
  input whose mapped result is non-finite retains the existing explicit
  non-finite-result failure; no clamp, retry, or fallback is permitted.
- A reflecting basis is admissible for value mapping, but this work unit makes
  no determinant-sign, handedness, orientation, rank, incidence, or topology
  claim.

## Required focused evidence

The implementation package must include independently derived exact cases for:

- identity and point/vector translation separation;
- 2D quarter turn, 3D axis cycle, and reflection;
- scale exponents `{-8, -1, 0, 1, 8}` where results are representable;
- both local/world round trips;
- affine compatibility and difference compatibility;
- point/vector and dimension separation;
- invalid/missing/duplicate/non-unit/non-finite basis entries;
- unrepresentable scale and non-finite mapped results; and
- deterministic GCC/Clang Debug/Release focused results under the established
  signed-zero comparison policy.

Expected values must be independently derived. Rounded fixtures are excluded;
they require a later explicit proximity policy, residual, and limit.

## Explicit exclusions

This decision does not authorize general `Transform2`/`Transform3` classes,
general affine or homogeneous matrices, arbitrary-angle rotation, trigonometry,
quaternions, shear, nonuniform scale, projective transforms, general inverse or
solve, predicates, normal/covector semantics, transformations of curves,
surfaces, patches, meshes, or sizing fields, third-party dependencies, parallel
execution, I/O, logging, or native-Windows qualification.

## Dependency and stop conditions

The dependency direction remains `numeric/base -> math -> geometry`; `math`
must not depend on Geometry. Existing Point/Vector and Mat behavior is
unchanged, and no mutable global state is admitted.

Stop before broadening scope if an accepted primitive result changes, a general
inverse or approximate test becomes necessary, a non-finite result is hidden,
a determinant is used to classify validity, or exact cross-toolchain results
are unexplained. Such a finding requires a new scientific decision.

## Decision

The bounded Cartesian similarity-frame work unit was accepted after the
post-merge integration of Minimal Small Linear Algebra at `main` commit
`ca51c333b5fcae33f05b5e25f6b0780ec195ad77`. The bounded implementation and
its focused analytic contract now pass on GCC 13 and Clang 18 libc++, in Debug
and Release. This is not a qualification claim.

## Qualification admission

The post-merge audit of `main` at
`12afa394af24c8f0627b12e3697010735826a9a1` confirmed that the reviewed source
tree was preserved exactly and that the bounded implementation remains inside
this decision. No implementation correction or scope expansion is required.

The revision-bound qualification protocol and its admission infrastructure,
including a clean/published-candidate `PREPARED` creator,
are recorded in
`docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`. Their
focused contracts pass in the declared four-cell envelope. That evidence does
not qualify the implementation, authorize a manifest, or start a campaign.

## Next bounded action

Audit the completed CF0–CF7 admission infrastructure against the pre-registered
protocol before any separate decision about invoking manifest preparation. No excluded
capability, manifest preparation, or formal execution is authorized by this
decision.
