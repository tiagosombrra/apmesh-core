# AP Mesh Core — Minimal Small Linear Algebra Contract

Status: IMPLEMENTED / FOCUSED CONTRACT PASS / FORMAL QUALIFICATION NOT STARTED
Date: 2026-09-10
Stage: Geometry Primitives — Exact Semantics Before Curves
Prerequisites: Foundation `QUALIFIED`; Point and Vector Semantics `QUALIFIED`
Amendment: 1 — implementation-admission ambiguities resolved on 2026-09-10

## 1. Question and boundary

What is the smallest fixed-size linear-algebra capability required to support
later coordinate-frame and differential-geometry work without introducing
dynamic algebra, robust predicates, transformations, or curve/surface
semantics prematurely?

This contract specifies one bounded work unit. It does not implement or qualify
`Mat2` or `Mat3`, and it does not qualify the complete Geometry Primitives
stage.

## 2. Candidate hypothesis

Concrete fixed-size `Mat2` and `Mat3` value types, restricted to finite values
and a minimal closed set of algebraic operations, are sufficient for the next
Geometry Primitives dependency. They can preserve the qualified Numeric and
Point/Vector semantics without a dynamic matrix abstraction or third-party
dependency.

The hypothesis is rejected or narrowed if the bounded implementation requires
an undeclared tolerance, a topology-changing sign decision, an inverse or
decomposition algorithm, or a general-purpose matrix framework.

## 3. Authorized capability

One bounded implementation work unit may add only:

- concrete `Mat2` and `Mat3` fixed-dimension value types over `double` in the
  logical `math` layer;
- construction that accepts only finite entries;
- dimension-preserving checked component access;
- zero and identity values;
- transpose;
- same-dimension matrix composition;
- a determinant value as an algebraic scalar result; and
- matrix-vector application for the matching qualified vector type, exposed
  only by the logical `geometry` layer that already owns `Vector2` and
  `Vector3`.

The dependency direction is normative:

```text
numeric/base -> math -> geometry
```

The `math` representation and matrix-only operations must not include the
Point/Vector header or otherwise depend on `geometry`. The Geometry-side
matrix-vector adapter may depend on both layers. This preserves the qualified
Architecture Contract without moving or redefining Point/Vector types and does
not require a new linker target.

Every operation whose arithmetic produces a non-finite result must return an
explicit classified failure. Internal storage order is an implementation detail
and must not become a scientific claim.

The determinant operation reports a scalar only. Its sign or magnitude must not
be used to infer orientation, rank, degeneracy, singularity, incidence, or any
other geometric or topological decision.

## 4. Explicit exclusions

This contract does not authorize:

- dynamically sized matrices or dimension-generic public templates;
- inverses, linear solves, condition estimators, LU, QR, SVD, or other
  decompositions;
- eigenvalues, eigenvectors, principal directions, or tensor metrics;
- determinant-sign predicates, rank tests, or a singularity tolerance;
- transformation or coordinate-frame objects;
- points as matrix operands;
- affine translation, homogeneous coordinates, quaternions, or rotations as a
  domain abstraction;
- topology, incidence, curves, surfaces, patches, meshes, sizing, or adaptation;
- SIMD, GPU, OpenMP, MPI, or native-Windows qualification;
- a global/default tolerance, hidden retry, fallback, clamp, or rescue rule;
- a third-party runtime dependency; or
- legacy implementation behavior as an acceptance oracle.

Any demonstrated need for an excluded capability requires its own bounded
decision after this work unit.

## 5. Numeric and API obligations

The qualified Numeric Contract remains authoritative. The bounded matrix API
uses the following exact error vocabulary:

- valid matrix entries and successful results are finite;
- matrix construction from a non-finite entry returns
  `LinearAlgebraError::non_finite_input`;
- checked access outside `[0, dimension)` for either row or column returns
  `LinearAlgebraError::index_out_of_range`;
- a non-finite matrix-only arithmetic result returns
  `LinearAlgebraError::non_finite_result`;
- the Geometry-side matrix-vector adapter returns the already qualified
  `GeometryError::non_finite_result` if its finite operands produce a
  non-finite component;
- exact binary fixtures use exact comparison only where exactness is a declared
  invariant;
- rounded observations use an explicit `ProximityPolicy` and independently
  computed residual and limit; and
- no matrix result creates topological identity or scientific acceptance.

These names are implementation obligations, not aliases for the conceptual
Numeric Contract categories. No existing `NumericError` or `GeometryError`
enumerator is renamed or reinterpreted. Zero, identity, and transpose are
infallible because a valid matrix remains valid under those operations.

The public API must preserve matrix dimension at compile time. Public
construction and access use mathematical `(row, column)` indices; an array
factory, if selected, documents row-major input order without exposing internal
storage. Invalid indices must fail explicitly rather than access memory
unchecked. The implementation must use value semantics, immutable inputs, and
no mutable global state, I/O, logging, environment discovery, or allocation
justified only by matrix size.

## 6. Fixed analytic and adversarial cases

The bounded implementation and its focused contracts must cover at least:

| Case | Required observation |
| --- | --- |
| Zero and identity | Zero entries are valid; identity preserves every admitted basis vector and matrix. |
| Checked access | Every valid component is observed exactly; every out-of-range row or column fails explicitly. |
| Diagonal scale | Signed and exact power-of-two diagonal entries act component-wise without dimension conversion. |
| Permutation | A fixed 2D swap and 3D coordinate permutation produce the independently declared vector and determinant results. |
| Quarter turn | The 2D matrix with entries in `{0, 1, -1}` maps basis vectors exactly and composes to the declared half turn. |
| Transpose | Double transpose recovers the original matrix and the composition law is exactly `(A B)^T = B^T A^T` for the fixed exact fixtures. |
| Composition | Matrix-vector and matrix-matrix results match independent component formulas for non-commuting fixtures. |
| Determinant | Exact diagonal, permutation, and zero-row fixtures produce their declared algebraic scalar; no predicate conclusion is emitted. |
| Invalid input | Every NaN and positive/negative infinity placement is rejected for both dimensions. |
| Non-finite result | Deliberate overflow in application, composition, or determinant returns explicit failure. |
| Scale envelope | For independently constructed `sA`, with `s = 2^k`, verify `(sA)v = s(Av)`, `(sA)B = s(AB)`, `A(sB) = s(AB)`, `(sA)^T = sA^T`, and `det(sA) = s^n det(A)` for dimension `n`, only where all declared intermediates and results remain representable. These fixtures do not authorize scalar-matrix operators. |
| Isolation | Foundation and Point/Vector preservation contracts remain passing and no excluded module dependency appears. |

Expected results must be derived independently in tests rather than copied from
production code or the legacy repository.

## 7. Bounded qualification gates

The short identifiers are secondary to the descriptive gate names.

| Gate | Required evidence |
| --- | --- |
| `LA0` — Scope and revision identity | One clean revision contains only the admitted fixed-size capability and its evidence infrastructure. |
| `LA1` — Dimension, dependency, and access semantics | `Mat2`/`Mat3` remain distinct in `math`; `math` does not depend on `geometry`; matching vector operations are exposed from `geometry`; mismatched dimensions and point operands are rejected; checked access and its error classification are complete. |
| `LA2` — Finite construction and failure | All non-finite inputs and non-finite intermediates produce the declared explicit failure. |
| `LA3` — Algebraic agreement | Identity, transpose, application, composition, and exact analytic fixtures agree with independent component formulas. |
| `LA4` — Determinant boundary | Determinant fixtures agree algebraically and neither implementation nor evidence assigns predicate, rank, or topology semantics. |
| `LA5` — Scale and deterministic equivalence | The operation-specific power-of-two laws above and repeated GCC/Clang Debug/Release observations satisfy the pre-registered exact or explicit-proximity rules. |
| `LA6` — Prerequisite preservation | Qualified Foundation and Point/Vector contracts pass on the same candidate without semantic change. |
| `LA7` — Evidence integrity and bounded closure | Manifest, inputs, command records, certificates, comparisons, failures, hashes, limitations, and retention are revision-bound and independently verifiable. |

Passing focused tests is implementation evidence only. `LA0`–`LA7` may become
`PASS` only after one separately authorized revision-bound qualification and an
independent scientific audit.

## 8. Stop conditions

Stop before expanding the implementation if:

- an admitted operation needs an inverse, decomposition, eigenproblem, robust
  predicate, or undeclared acceptance rule;
- a point must be multiplied directly by a matrix;
- determinant output is proposed as a certified sign or rank decision;
- Foundation or Point/Vector behavior changes;
- cross-cell differences remain unexplained; or
- a third-party dependency appears necessary.

The finding must be recorded as a new bounded decision; it must not be resolved
by silently broadening this contract.

## 9. Basis and non-claims

IEEE 754-2019, Goldberg 1991, and Higham 2002 provide the floating-point and
stability basis already registered in `docs/research/REFERENCE_REGISTER.md`.
The C++ Core Guidelines support value semantics and narrow typed interfaces.
The qualified Numeric Contract controls finite values, explicit failure, and
proximity; the Point/Vector qualification controls operand semantics.

These sources and contracts do not prove universal numerical stability,
conditioning, robust determinant signs, or suitability for arbitrary future
geometry algorithms. No new external dependency is admitted.

## 10. Amendment 1 — implementation admission

The scientific review resolved four admission ambiguities without expanding the
candidate capability:

1. matrix representation and matrix-only algebra belong to `math`, while
   Point/Vector interoperability belongs to the depending `geometry` layer;
2. `LinearAlgebraError` owns matrix input, index, and result failures, while the
   Geometry-side adapter preserves the qualified `GeometryError` vocabulary;
3. transpose composition means exactly `(A B)^T = B^T A^T`; and
4. scale behavior is specified separately for application, composition,
   transpose, and the dimension-dependent determinant law.

The determinant remains admitted only as an algebraic scalar needed by later
Jacobian and metric investigations. It has no predicate, rank, degeneracy, or
singularity semantics. No inverse, solve, eigensystem, transformation, or new
dependency was admitted.

## 11. Decision effect

The contract is accepted as the next bounded implementation work unit. It
authorizes only the capability and dependency split in sections 3–8. This
decision itself makes no behavior change and adds no scientific qualification.
Implementation, focused validation, and the later revision-bound `LA0`–`LA7`
qualification remain separate states and must not be reported as complete until
their respective evidence exists.
