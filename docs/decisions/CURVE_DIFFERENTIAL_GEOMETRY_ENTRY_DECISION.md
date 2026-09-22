# Curve Differential Geometry — Bounded Entry Decision

Status: ENTRY DECISION APPROVED / IMPLEMENTATION NOT STARTED / STAGE UNQUALIFIED
Date: 2026-09-21
Stage: Curve Differential Geometry — Curvature, Regularity, and Features

Prerequisites:

- Foundation: QUALIFIED;
- Geometry Primitives: QUALIFIED;
- Topological Model: QUALIFIED in the admitted GitHub-hosted Ubuntu 24.04
  x86_64 cloud envelope;
- Curve Representation — Continuous Geometry Before Discretization:
  QUALIFIED by CGR0–CGR7 PASS in the same admitted cloud envelope.

## Question

What is the smallest differential-geometric capability that can be admitted
after qualified continuous cubic Bézier representation while remaining
independent of global feature classification, boundary discretization, sizing,
surfaces, meshing, and parallel execution?

## Decision

Authorize exactly one first bounded implementation work unit:

**Pointwise Curvature Magnitude on Regular Cubic Bézier Curves.**

The work unit may evaluate nonnegative scalar curvature magnitude for
`CubicBezier2` and `CubicBezier3` at one finite parameter
`t∈[0,1]`, provided the first derivative at that parameter is nonzero.

The implementation must reuse the already qualified continuous-curve
authorities for:

- first derivative;
- second derivative;
- pointwise speed;
- 2D/3D vector geometry;
- explicit parameter-domain failures.

It must not rederive or fork those semantics.

This entry decision fixes the mathematical contract, invariance laws, failure
semantics, focused evidence and exclusions. It does not itself implement
production code and does not qualify the Curve Differential Geometry stage.

## Literature and reference basis

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| Manfredo do Carmo, *Differential Geometry of Curves and Surfaces*; also listed by MIT OpenCourseWare differential-geometry materials | Regular curves, curvature as a local differential quantity, and the requirement that tangent speed be nonzero for the ordinary Frenet construction. | A particular floating-point implementation, Bézier-specific feature classifier, or AP Mesh API. | Curvature is admitted only at regular parameters; singular parameters must fail explicitly. |
| MIT OpenCourseWare 18.950, Chapter 1 / Frenet-curve notes | Curvature is intrinsic to the regular curve and invariant under admissible reparameterization; ordinary plane curvature is the first Frenet curvature. | Certified floating-point evaluation or global feature detection. | Require reversal/reparameterization invariance relations and separate local curvature from global feature claims. |
| Wolfram MathWorld, *Curvature* | Standard parametric plane-curve curvature formula and its dependence on first/second derivatives. | Numerical robustness, certification, or a dependency choice. | Use an independent analytic formula for selected 2D fixtures and evidence. |
| Miura and Salvi, *On the curvature extrema of special cubic Bézier curves*, 2021 | Even restricted cubic Bézier curvature-extremum structure requires a separate analysis. | A general robust extrema classifier for arbitrary cubic Bézier curves. | Defer extrema/monotonicity and global feature classification to later bounded decisions. |
| Existing qualified AP Mesh curve contracts | Deterministic cubic evaluation, first/second derivatives, speed, global regularity certification, reversal, scale/translation relations, explicit failures. | Pointwise curvature semantics. | Reuse these qualified prerequisites rather than introducing a new representation or derivative authority. |

External sources are mathematical/reference evidence only. No new runtime
dependency is admitted.

Reference URLs:

- https://ocw.mit.edu/courses/18-950-differential-geometry-fall-2008/
- https://ocw.mit.edu/courses/18-950-differential-geometry-fall-2008/pages/lecture-notes/
- https://mathworld.wolfram.com/Curvature.html
- https://arxiv.org/abs/2101.08138

## Mathematical contract

Let `B(t)` be a twice-differentiable regular parametric curve and define:

- `v = B'(t)`;
- `a = B''(t)`;
- `s = ||v||`.

The work unit is defined only when `s > 0`.

For a planar curve:

`κ(t) = |det(v,a)| / s^3`.

For a spatial curve:

`κ(t) = ||v × a|| / s^3`.

The API exposes only the nonnegative curvature magnitude `κ`.

A separate signed planar curvature is intentionally not admitted in this first
work unit because its sign depends on an orientation convention that has no
direct 3D scalar analogue. Global inflection classification is therefore also
deferred.

## Authorized production scope

The first implementation work unit may add only:

- one pointwise curvature-magnitude query for `CubicBezier2`;
- one pointwise curvature-magnitude query for `CubicBezier3`;
- a narrow curve-differential error vocabulary if existing `CurveError`
  cannot express singularity without semantic ambiguity;
- deterministic finite arithmetic required to evaluate the formulas above;
- focused analytic/metamorphic tests;
- header/dependency and prerequisite-preservation tests.

No new public curve representation, topology identity, sampling container,
feature object, interval type, root solver, or meshing type is authorized.

## Pointwise domain and failure semantics

A conforming query must distinguish at minimum:

- non-finite parameter;
- finite parameter outside `[0,1]`;
- singular parameter, meaning the evaluated first derivative is exactly the
  zero vector under the qualified curve/vector semantics;
- non-finite/unrepresentable result.

The implementation must not:

- clamp or extrapolate parameters;
- classify a near-zero speed as singular using a hidden epsilon;
- substitute zero curvature for a singular point;
- return infinity as a successful curvature value;
- require global regularity certification merely to evaluate a locally regular
  parameter;
- silently retry with a different mathematical definition.

An exact zero first derivative is a domain failure for curvature, even though
the underlying curve representation remains valid.

A regular inflection point is different: if `B'(t) != 0` and the curvature
numerator is zero, the correct curvature magnitude is exactly zero where the
declared arithmetic permits it. It is not a singularity error.

## Required invariants

### Nonnegativity

Every successful result satisfies `κ >= 0`.

### Straight-line zero curvature

Any regular affine-line cubic fixture has `κ = 0` throughout the valid
parameter interval.

### Reversal invariance

For `R(t)=B(1-t)`:

`κ_R(t) = κ_B(1-t)`.

This follows because reversal negates the first derivative but preserves the
second-derivative sign relation needed by curvature magnitude.

### Translation invariance

Translating every control point by the same finite vector does not change
curvature magnitude.

### Orthogonal-frame invariance

The already admitted signed-permutation Cartesian frame transformations do not
change curvature magnitude.

### Uniform scale covariance

For a uniform nonzero scale `λ` admitted by the qualified Cartesian-frame
power-of-two scaling contract:

`κ_{λB}(t) = κ_B(t) / |λ|`.

The focused work unit must test this relation rather than expecting scale
invariance.

### 2D/3D embedding parity

Embedding a planar 2D cubic as `z=0` in 3D must produce the same curvature
magnitude under the declared comparison policy.

### Locality

Pointwise curvature depends only on the qualified first and second derivatives
at the requested parameter. It must not depend on:

- global regularity policy;
- arc-length policy;
- filesystem or environment;
- topology identity;
- sampling density;
- thread schedule;
- unordered iteration;
- downstream meshing state.

## Numerical robustness requirements

The mathematical formulas contain products and a cubic power of speed. A
production implementation must avoid avoidable overflow/underflow caused only
by naïvely forming unscaled determinant/cross-product and `s^3`
intermediates when a representable curvature result could otherwise be
obtained.

The implementation decision may choose a scale-aware algebraic rearrangement,
but it must preserve the exact mathematical quantity and must not introduce a
hidden acceptance tolerance.

If the final mathematical result is not representable as a finite `double`,
the query must fail explicitly as a non-finite/unrepresentable result.

No universal epsilon, long-double oracle, arbitrary precision runtime
dependency, or platform-specific fast-math path is admitted.

## Required analytic and adversarial fixtures

| Case | Required observation |
| --- | --- |
| Regular 2D line | Curvature is zero at representative endpoints/interior parameters. |
| Regular 3D line | Same zero-curvature requirement. |
| Degree-elevated planar parabola | Curvature agrees with an independent analytic reference at selected exactly representable parameters. |
| Spatial polynomial cubic `(t,t²,t³)` | Selected curvature values agree with independently derived first/second derivatives and 3D cross-product reference. |
| Regular inflection fixture | Curvature is zero without reporting singularity. |
| Singular endpoint fixture | Exact zero first derivative yields explicit singular-parameter failure. |
| Constant curve | Every queried parameter is singular for curvature; no successful zero-curvature result. |
| Reversal | `κ_reverse(t)` agrees with `κ(1-t)`. |
| Translation | Curvature unchanged. |
| Signed-permutation frame | Curvature unchanged. |
| Power-of-two uniform scale | Curvature changes by reciprocal absolute scale. |
| 2D/3D planar embedding | Curvature magnitudes agree. |
| Near-singular but nonzero derivative | No hidden epsilon may convert a finite nonzero derivative into singularity. |
| Extreme finite derivatives | Scale-aware evaluation avoids avoidable intermediate overflow where the final result is representable. |
| Non-finite parameter | Explicit rejection. |
| Below/above domain | Explicit rejection. |
| Repeatability | Repeated evaluation produces equivalent scientific fields. |
| Header/dependency isolation | No topology, discretization, surface, meshing, threading, I/O, or third-party dependency is introduced. |

Independent expected values must not call the production curvature method.

## Focused validation boundary

The work unit must add one focused CTest contract covering 2D and 3D pointwise
curvature semantics.

Before integration, focused validation must pass at minimum in:

- GCC 13 Debug / libstdc++;
- Clang 18 Debug / libc++.

The selected focused regression must preserve all currently qualified
prerequisites relevant to this capability, including:

- Numeric Contract;
- Geometry Primitives;
- Minimal Small Linear Algebra / header isolation;
- Cartesian Frames;
- Topological Model;
- qualified Curve Representation value/differential/regularity/arc-length
  contracts.

A passing focused work unit yields only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

Stage qualification remains a later cumulative Curve Differential Geometry
regression campaign.

## Explicit exclusions

This decision does not authorize:

- signed planar curvature as a public semantic contract;
- tangent/normal/Frenet frame APIs;
- binormal or torsion;
- curvature derivative;
- curvature extrema or monotonicity classification;
- global curvature bounds;
- inflection isolation/classification;
- cusp or corner classification;
- feature points or feature intervals;
- global regularity reimplementation;
- root finding for curvature numerators/derivatives;
- osculating circles/centers;
- radius-of-curvature API;
- arc-length reparameterized curvature tables;
- adaptive sampling or boundary discretization;
- curvature-driven sizing;
- surface geometry;
- mesh generation/optimization/adaptation;
- Quad-Dominant construction;
- SIMD/GPU/OpenMP/MPI/task parallelism;
- third-party runtime dependencies;
- native-Windows qualification;
- a formal Curve Differential Geometry qualification campaign.

In particular, the downstream **Boundary Curve Discretization** stage remains
blocked.

## Admission and stop conditions

Implementation may proceed only if:

1. all formulas reuse qualified first/second derivatives;
2. singularity is exact and explicit, with no epsilon threshold;
3. curvature magnitude remains nonnegative and finite on success;
4. reversal, frame, scale and embedding relations are tested;
5. expected values are independently derived;
6. no global feature claim is inferred from sampled pointwise curvature;
7. no new runtime dependency or parallel path is introduced; and
8. relevant qualified prerequisites remain passing.

Stop and require a new bounded decision if implementation requires:

- third derivatives;
- a global polynomial root solver;
- interval-wide curvature certification;
- orientation-dependent signed curvature;
- feature extraction/classification;
- physical sampling/discretization;
- an undeclared tolerance or fallback; or
- a dependency outside the standard library.

## Alternatives considered

- **Implement curvature extrema together with pointwise curvature:** rejected;
  extrema are a distinct global algebraic problem and literature on even
  special cubic Bézier families treats them separately.
- **Require whole-curve regularity certification before every pointwise
  curvature query:** rejected; local curvature only requires a nonzero
  derivative at the queried parameter. Global regularity remains useful for
  later interval-wide claims.
- **Expose signed curvature first:** deferred because the first stage contract
  should be dimension-consistent between 2D and 3D.
- **Expose Frenet frames first:** deferred because normal/binormal semantics add
  extra singular cases at zero curvature and torsion requires third-order
  information.
- **Begin boundary sampling now:** rejected; differential geometry must be
  independently validated before it can inform discretization or sizing.
- **Introduce a global curvature tolerance:** rejected by the Numeric Contract
  and project-wide explicit-policy rule.

## Stage plan after this entry

The stage remains deliberately incremental.

The first admitted investigation problem is only pointwise curvature magnitude.

After that work unit is integrated and closed, the next transition must be a
separate decision. Candidate later investigations include:

- certified interval/global curvature bounds;
- planar signed curvature and certified inflection isolation;
- curvature extrema/monotonicity evidence;
- differential feature classification.

Those are roadmap possibilities, not authorization from this document.

The stage exit will require a separate cumulative regression protocol after all
admitted Curve Differential Geometry work units are closed. That future
campaign must include every prerequisite regression and regenerate
curvature/reference evidence.

## Effect on roadmap if integrated

Curve Differential Geometry moves from `NOT STARTED` to:

`IN INVESTIGATION / ENTRY DECISION APPROVED / IMPLEMENTATION NOT STARTED`.

Curve Representation remains qualified and frozen as a prerequisite.

The next bounded work item after this decision is integrated, post-merge
validation passes, and its checkpoint is closed is implementation of:

**Pointwise Curvature Magnitude on Regular Cubic Bézier Curves**

only.

Boundary Curve Discretization, sizing, meshing, Quad-Dominant and parallel
execution remain blocked.

## Integration checkpoint

PR #86 integrated this entry decision as
`e728e89f08b23cd0720e502e3efe0c565198d376`.

Validation:

- PR FAST `35671674354`: PASS;
- PR INTEGRATION `35671674360`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35671824327`: PASS;
- post-merge INTEGRATION `35671824323`: PASS in both cells.

The entry-decision checkpoint is closed. The sole next bounded work item is
implementation of **Pointwise Curvature Magnitude on Regular Cubic Bézier
Curves** within the scope fixed above.

## First work-unit implementation result

Branch `curve/pointwise-curvature-magnitude` implements only the authorized
**Pointwise Curvature Magnitude on Regular Cubic Bézier Curves** work unit.

Production result:

- `CubicBezier2::curvature_magnitude(t)`;
- `CubicBezier3::curvature_magnitude(t)`;
- exact `CurveError::singular_parameter` for zero first derivative;
- existing parameter errors preserved;
- explicit `non_finite_result` when a positive curvature cannot be represented;
- normalized derivative evaluation plus binary exponent reconstruction using
  `frexp`/`scalbn`;
- no hidden tolerance and no global-regularity precondition.

Focused evidence:
`apmesh_core.curve_curvature`.

PR validation:

- FAST `35672497018`: PASS;
- INTEGRATION `35672497040`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The focused fixtures cover all required analytic/metamorphic/adversarial
classes, including tiny nonzero derivative, large-scale intermediate-overflow
avoidance and explicit unrepresentable-result failure.

Scientific status:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

PR #88 merged as
`b1a279fbe2592cd4fe7f5a688318ac50f0e60d35`; post-merge FAST
`35673041777` and INTEGRATION `35673041859` passed.

The first work-unit checkpoint is closed. No later Curve Differential Geometry
work unit or stage qualification is authorized by this result. The next
investigation requires a separate literature-backed decision.

That separate next decision is now proposed in
`docs/decisions/CURVE_SIGNED_PLANAR_CURVATURE_DECISION.md`. It remains
decision-only until independently integrated and closed; this entry document
does not itself authorize the signed-curvature implementation.
