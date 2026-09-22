# Planar Signed Curvature — Bounded Scientific Decision

Status: **DECISION APPROVED / IMPLEMENTATION NOT STARTED / STAGE UNQUALIFIED**  
Date: 2026-09-21  
Stage: Curve Differential Geometry — Curvature, Regularity, and Features

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted cloud envelope;
- Curve Representation: QUALIFIED by CGR0–CGR7 PASS;
- Pointwise Curvature Magnitude on Regular Cubic Bézier Curves:
  IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.

## 1. Question

What is the smallest next Curve Differential Geometry capability that adds
orientation-sensitive planar differential information without incorrectly
promoting a pointwise floating evaluation into a certified global inflection or
feature classifier?

## 2. Decision

Authorize exactly one bounded implementation work unit:

**Pointwise Signed Curvature on Regular Planar Cubic Bézier Curves.**

The work unit may add one query to `CubicBezier2` only:

`signed_curvature(t)`

for one finite parameter `t∈[0,1]` at which the already qualified first
derivative is nonzero.

No 3D signed-curvature scalar is admitted.

This work unit is local and pointwise. It does not isolate, count, classify or
certify inflection points over an interval.

## 3. Literature and reference basis

The decision uses the following external sources only as mathematical/reference
evidence; none becomes a runtime dependency.

### Farin, Curves and Surfaces for CAGD

Gerald Farin, *Curves and Surfaces for CAGD: A Practical Guide*, 5th ed.,
Morgan Kaufmann / Academic Press, 2002.

Reference:
https://www.sciencedirect.com/book/9781558607378/curves-and-surfaces-for-cagd

Relevant material records that planar parametric curves may be assigned signed
curvature using the sign of the determinant of first and second derivatives.
It also distinguishes this orientation-sensitive planar quantity from the
ordinary nonnegative 3D curvature magnitude.

Decision impact:

- admit a 2D-only signed scalar;
- define its sign from an explicit orientation convention;
- do not invent a 3D scalar signed analogue.

### MIT Hyperbook — zero-curvature points

Patrikalakis, Maekawa and Cho, *Shape Interrogation for Computer Aided Design
and Manufacturing*, MIT Hyperbook, section 8.1.3.2.

Reference:
https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node153.html

Relevant material records the signed-curvature formulation for planar
parametric curves and relates zero curvature, under regularity assumptions, to
the search for inflection candidates.

Decision impact:

- a pointwise signed-curvature zero is useful evidence;
- zero at one sampled parameter is not by itself a certified global inflection
  classifier;
- interval-wide zero isolation remains a separate investigation.

### MIT Hyperbook — planar differential-geometry convention

Reference:
https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node219.html

Relevant material makes the planar curvature sign convention explicit rather
than treating sign as coordinate-free.

Decision impact:

- the AP Mesh API must state its orientation convention;
- orientation-reversing Cartesian transforms must flip signed curvature.

### Miura and Salvi 2021

Kenjiro T. Miura and Péter Salvi,
*On the curvature extrema of special cubic Bézier curves*,
arXiv:2101.08138, 2021.

Reference:
https://arxiv.org/abs/2101.08138

Decision impact:

- curvature extrema/monotonicity remain a distinct global analytical problem;
- this pointwise signed-curvature work unit does not authorize extrema
  classification.

## 4. Mathematical contract

For a regular planar parametric curve `B(t)`, let:

- `v = B'(t)`;
- `a = B''(t)`;
- `s = ||v||`.

The work unit is defined only when `s > 0`.

Using the repository's standard ordered `(x,y)` Cartesian orientation, define:

`κ_s(t) = det(v,a) / s^3`

where:

`det(v,a) = v_x a_y - v_y a_x`.

The successful result is a finite `double`.

If the determinant is exactly zero under the declared production arithmetic,
the public result must be canonical positive zero `+0.0`.

## 5. Relation to existing curvature magnitude

For every successful regular planar query:

`abs(signed_curvature(t)) = curvature_magnitude(t)`

under the same declared floating comparison policy.

The implementation must not create an independent derivative, speed or
parameter-domain authority.

The preferred implementation structure is a shared internal scale-aware planar
curvature core that produces:

- curvature magnitude;
- determinant sign; and
- exact zero state

from the same normalized first/second derivatives.

A second independent raw determinant path is not admitted merely to recover
sign.

## 6. Orientation and transformation laws

### Reversal

For `R(t)=B(1-t)`:

`κ_s,R(t) = -κ_s,B(1-t)`.

### Translation

Translation of every control point by the same finite vector does not change
signed curvature.

### Orientation-preserving orthogonal frame

For an admitted orientation-preserving signed-permutation Cartesian frame,
signed curvature is preserved.

### Orientation-reversing orthogonal frame

For an admitted reflection / orientation-reversing signed-permutation frame,
signed curvature changes sign while curvature magnitude is preserved.

### Uniform scale

For an admitted nonzero uniform scale `λ`:

`κ_s,λB(t) = κ_s,B(t) / |λ|`.

The sign is unchanged by a uniform scalar scale; the dimensional magnitude
changes inversely with physical length.

## 7. Failure semantics

The query reuses existing `CurveError` semantics.

At minimum:

- non-finite parameter → existing non-finite-parameter failure;
- parameter outside `[0,1]` → existing domain failure;
- exact zero first derivative → `CurveError::singular_parameter`;
- finite mathematical value that cannot be represented as a successful finite
  `double` → `CurveError::non_finite_result`.

The implementation must not:

- clamp parameters;
- use a hidden speed epsilon;
- convert singularity to zero curvature;
- return infinity successfully;
- introduce a global tolerance;
- silently replace an underflowed nonzero curvature with zero;
- infer global regularity or inflection classification.

## 8. Numerical robustness boundary

The existing pointwise curvature-magnitude implementation already avoids
avoidable overflow/underflow by independently scaling first and second
derivatives and reconstructing the physical factor with binary exponent
arithmetic.

The signed implementation must preserve that strategy.

The determinant sign must be obtained from the same normalized planar
determinant used by the magnitude computation.

This bounded work unit does **not** claim an exact robust-predicate sign for
arbitrarily ill-conditioned near-zero determinants.

Consequently:

- signed curvature is a pointwise floating differential value;
- future certified inflection isolation must not use sampled
  `signed_curvature(t)` signs as its proof mechanism;
- if a future global sign/root claim requires certified determinant signs or
  polynomial root isolation, that requires a new decision.

## 9. Required focused evidence

A dedicated focused contract must cover at minimum:

| Case | Required observation |
| --- | --- |
| Counter-clockwise planar parabola fixture | Positive signed curvature at selected regular parameters. |
| Reflected parabola | Same magnitude, opposite sign. |
| Curve reversal | Sign flips with `t↦1-t`. |
| Regular straight line | Canonical `+0.0`. |
| Regular inflection parameter | Canonical `+0.0`, not singular. |
| Singular endpoint | Explicit `singular_parameter`. |
| Constant curve | Explicit singularity. |
| Translation | Signed curvature unchanged. |
| Orientation-preserving signed-permutation frame | Signed curvature unchanged. |
| Orientation-reversing signed-permutation frame | Signed curvature sign flips. |
| Power-of-two uniform scale | Reciprocal magnitude scaling, sign preserved. |
| Magnitude parity | `abs(signed)==curvature_magnitude` under declared comparison policy. |
| Tiny nonzero derivative | No hidden singularity epsilon. |
| Extreme representable curvature | Scale-aware result remains finite/correct. |
| Unrepresentable nonzero curvature | Explicit `non_finite_result`. |
| NaN / out-of-domain parameter | Existing explicit parameter failures. |
| Signed-zero parameter | `-0.0` parameter is semantically identical to `+0.0`. |
| Repeatability | Repeated identical query is deterministic. |
| Header/dependency isolation | No topology, discretization, surface, meshing, threading, I/O or third-party dependency. |

Independent expected values must not call the production signed-curvature
method.

## 10. Focused regression boundary

Before integration, the work unit must pass at minimum:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

The selected regression must preserve all qualified prerequisites and all
integrated Curve Differential Geometry contracts, including:

- numeric contract;
- geometry primitives;
- minimal small linear algebra/header isolation;
- Cartesian frames;
- Topological Model;
- qualified Curve Representation contracts;
- pointwise curvature-magnitude contract.

A passing focused work unit yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

## 11. Explicit exclusions

This decision does not authorize:

- any 3D signed-curvature scalar;
- unit tangent/normal/binormal/Frenet-frame API;
- torsion;
- curvature derivative;
- interval/global curvature bounds;
- curvature extrema or monotonicity classification;
- root solving for the curvature numerator;
- inflection isolation, counting or classification;
- cusp/corner classification;
- feature points or feature intervals;
- radius-of-curvature/osculating-circle API;
- sampling/discretization;
- curvature-driven sizing;
- surfaces;
- meshing;
- Quad-Dominant construction;
- parallel execution;
- third-party runtime dependencies;
- stage-level qualification infrastructure.

Boundary Curve Discretization remains blocked.

## 12. Admission and stop conditions

Implementation may proceed only if:

1. the public API is 2D-only;
2. the standard `(x,y)` orientation convention is explicit;
3. sign comes from the same scale-aware planar curvature core used for
   magnitude;
4. exact zero is canonicalized to `+0.0`;
5. singularity remains exact and explicit;
6. orientation-preserving and orientation-reversing frame laws are tested;
7. reversal sign change is tested;
8. magnitude parity is tested;
9. no global feature claim is inferred; and
10. all qualified prerequisites remain passing.

Stop and require another bounded decision if implementation needs:

- certified sign under near-zero determinant cancellation;
- adaptive/exact geometric predicates;
- interval-wide sign certification;
- root isolation;
- global inflection/extrema classification;
- third derivatives;
- sampling/discretization; or
- a new third-party dependency.

## 13. Alternatives considered

### Certified inflection isolation immediately

Deferred.

A regular planar inflection is a global/root-structure question. Pointwise
signed curvature supplies orientation-sensitive local information, but
certified interval-wide root isolation requires its own representation,
enclosure, multiplicity and termination decisions.

### Curvature extrema next

Deferred.

Extrema require a higher-order/global analysis and are separately treated even
for special cubic Bézier families in the literature.

### Frenet frames next

Deferred.

Normal/binormal semantics add zero-curvature degeneracy and 3D torsion requires
additional differential information.

### Global curvature bound next

Deferred.

A useful certified bound requires a separate quantitative enclosure contract,
including lower speed evidence and conservative numerator/acceleration
enclosures.

## 14. Effect on the stage plan

If this decision is integrated and its checkpoint is closed, the sole next
bounded work item is implementation of:

**Pointwise Signed Curvature on Regular Planar Cubic Bézier Curves.**

After that implementation is integrated and closed, the next transition again
requires a separate literature-backed decision.

Likely later investigations remain:

- certified planar zero-curvature/inflection isolation;
- certified interval/global curvature bounds;
- curvature extrema/monotonicity;
- differential feature classification.

Those are roadmap candidates only and are not authorized by this document.

Curve Differential Geometry remains unqualified until a later cumulative
stage-exit campaign covers all admitted work units and every qualified
prerequisite.

## 15. Integration checkpoint

PR #90 integrated this decision as
`8da6ad656871c23f26f74f148298283970338583`.

Validation:

- PR FAST `35674524237`: PASS;
- PR INTEGRATION `35674524211`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35674581493`: PASS;
- post-merge INTEGRATION `35674581550`: PASS.

The decision checkpoint is closed. The sole next bounded work item is
implementation of **Pointwise Signed Curvature on Regular Planar Cubic Bézier
Curves** within the contract above. No later Curve Differential Geometry
capability is authorized by this checkpoint.

## 16. Implementation result

Branch `curve/signed-planar-curvature` implements exactly the bounded 2D
signed-curvature contract.

Production result:

- `CubicBezier2::signed_curvature(t)`;
- no corresponding `CubicBezier3` signed scalar;
- one shared planar scale-aware curvature core for magnitude and sign;
- canonical successful `+0.0` at exact zero determinant;
- exact singular-parameter failure;
- explicit non-finite-result failure for unrepresentable nonzero curvature.

Focused contract:
`apmesh_core.curve_signed_curvature`.

Validation:

- FAST `35675167119`: PASS;
- INTEGRATION `35675167196`: PASS in GCC 13 Debug and Clang 18/libc++ Debug.

Scientific status:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / VALIDATED_UNMERGED / NOT QUALIFIED.**

No certified inflection isolation, interval/global curvature bound, extrema,
feature classification or downstream discretization capability is authorized
by this result.

## 17. Work-unit closure

PR #92 integrated the bounded implementation as
`170c8c8a8db8676933e8107a1eb8abb2dedd6204`.

Validation:

- final PR FAST `35675261376`: PASS;
- final PR INTEGRATION `35675261409`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35675349460`: PASS;
- post-merge INTEGRATION `35675349468`: PASS.

Final work-unit status:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

This decision now serves as historical authority for the local 2D signed
curvature contract. The next Curve Differential Geometry investigation requires
a separate literature-backed decision and must not reinterpret pointwise signed
curvature as certified global inflection evidence.
