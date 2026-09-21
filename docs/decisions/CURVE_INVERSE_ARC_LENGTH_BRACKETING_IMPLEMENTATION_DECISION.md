# Curve Representation — Work Unit 2B Certified Inverse Arc-Length Bracketing

Status: IMPLEMENTATION COMPLETE / FOCUSED CONTRACTS PASS / INTEGRATED /
CLOSED / NOT QUALIFIED
Date: 2026-09-21
Stage: Curve Representation — Continuous Geometry Before Discretization
Parent authority:
`docs/decisions/CURVE_CUMULATIVE_ARC_LENGTH_INVERSE_BRACKETING_DECISION.md`
Prerequisite work unit:
Work Unit 2A — Certified Cumulative Arc-Length Enclosure — CLOSED

## 1. Question

What is the smallest executable inverse arc-length capability that can convert
a certified cumulative-length relation into a parameter result without
collapsing enclosure uncertainty, bypassing same-curve regularity, or
introducing sampling/lookup approximations as correctness authorities?

## 2. Decision

Authorize one implementation work unit:

**Certified Inverse Arc-Length Bracketing.**

The implementation may return only a certified parameter interval
`[t_low,t_high]`. It must not return one unqualified scalar inverse as the
scientific result.

The implementation must internally establish global regularity for the same
curve, preserve total/cumulative length enclosures, and refine the bracket by
deterministic midpoint bisection.

This decision does not implement code and does not qualify Curve
Representation.

## 3. Public target modes

Two target modes are admitted.

### 3.1 Absolute arc length

Input:

`s >= 0`

For a regular curve with exact total length `L`, the mathematical target is
the unique `t` satisfying:

`S(t) = s`.

A returned non-endpoint bracket must retain the proof:

`S_upper(t_low) <= s <= S_lower(t_high)`.

### 3.2 Normalized arc-length fraction

Input:

`f in [0,1]`.

Because total length is certified as `[L_lower,L_upper]`, an interior
normalized target is retained as the conservative interval:

`T = f * [L_lower,L_upper]`.

For `0 < f < 1`, a returned bracket must be valid for the complete target
interval:

`S_upper(t_low) <= T_lower <= T_upper <= S_lower(t_high)`.

The implementation must not choose a midpoint or best estimate of total length
and treat it as exact.

Endpoint fractions are identities:

- `f=0 -> [0,0]`;
- `f=1 -> [1,1]`;

after same-curve regularity is certified.

## 4. Same-curve regularity authority

Every inverse call must internally invoke the integrated global regularity
certifier on the same curve using an explicit caller-provided
`CurveRegularityPolicy`.

A detached boolean, a caller-created evidence structure, or evidence from a
different curve is not accepted as authority.

Only:

`CurveRegularityResult::regular`

permits a unique inverse claim.

`degenerate` and `indeterminate` both fail closed as
`regularity_not_certified`.

Regularity-certifier numeric failure remains an explicit numeric/enclosure
failure.

## 5. Proposed policy

The bounded implementation may introduce:

```text
CurveInverseLengthPolicy {
    CurveLengthPolicy length_policy;
    CurveRegularityPolicy regularity_policy;
    double parameter_tolerance;
    size_t max_refinement_iterations;
}
```

Requirements:

- nested length and regularity policies retain their existing validation;
- `parameter_tolerance` must be finite and nonnegative;
- `max_refinement_iterations` must be positive;
- no default/global tolerance is introduced;
- `parameter_tolerance=0` is admitted but can produce `indeterminate`;
- the policy does not include Newton/secant settings, lookup-table size, or
  sampling density.

## 6. Proposed result and evidence

The implementation may introduce:

```text
CurveInverseLengthResult {
    converged,
    indeterminate
}

CurveInverseLengthEvidence {
    CurveInverseLengthResult result;
    double target_lower_length;
    double target_upper_length;
    double lower_parameter;
    double upper_parameter;
    CurveLengthEvidence lower_cumulative;
    CurveLengthEvidence upper_cumulative;
    CurveLengthEvidence total_length;
    CurveRegularityEvidence regularity;
    size_t refinement_iterations;
}
```

The retained evidence must be sufficient to independently inspect the claimed
bracket and the regularity authority.

For non-endpoint results:

- `0 <= lower_parameter <= upper_parameter <= 1`;
- lower/upper cumulative evidence comes from the same curve and same
  `CurveLengthPolicy`;
- total-length evidence comes from the same curve and same policy;
- the retained target interval is finite, ordered and nonnegative;
- the lower/upper proof inequalities remain valid.

An `indeterminate` resource result must still retain the last valid certified
bracket. It must never fabricate a midpoint result.

## 7. Error vocabulary

The bounded implementation may introduce a curve-inverse-specific error
vocabulary containing only the semantics required here:

- `invalid_policy`;
- `non_finite_target`;
- `target_out_of_domain`;
- `target_domain_indeterminate`;
- `regularity_not_certified`;
- `non_finite_enclosure`.

Rules:

- negative absolute target: `target_out_of_domain`;
- non-finite target/fraction: `non_finite_target`;
- fraction outside `[0,1]`: `target_out_of_domain`;
- absolute target greater than certified total upper bound:
  `target_out_of_domain`;
- absolute target above total lower bound but not proven above the true total:
  `target_domain_indeterminate`;
- interior fraction whose conservative target upper bound cannot be proven at
  or below the total lower bound: `target_domain_indeterminate`;
- invalid nested/refinement policy: `invalid_policy`;
- same-curve regularity not proven: `regularity_not_certified`;
- failed finite enclosure arithmetic: `non_finite_enclosure`.

No target is clamped.

## 8. Endpoint semantics

After regularity is certified:

### Absolute target zero

`s=0 -> [0,0]`

with exact cumulative zero evidence.

### Absolute target at a certified exact total

If total-length evidence is exact and
`s == L_lower == L_upper`, return:

`[1,1]`.

If total-length evidence is not exact, equality to the unknown true total may
not be inferred from an arbitrary scalar target.

### Normalized fraction zero

`f=0 -> [0,0]`.

### Normalized fraction one

`f=1 -> [1,1]` by target identity, while retaining the certified total
enclosure as the target-length enclosure.

## 9. Deterministic bracket refinement

For an admitted target interval
`T=[T_lower,T_upper]`, initialize:

- `t_low=0`;
- `t_high=1`;
- `S(t_low)=[0,0]`;
- `S(t_high)=[L_lower,L_upper]`.

The initial bracket is admitted only when its target-domain proof is valid.

Each refinement iteration uses:

`t_mid = midpoint(t_low,t_high)`

with a deterministic binary midpoint.

Compute the certified cumulative enclosure
`S(t_mid)=[S_lower,S_upper]`.

Exactly three outcomes are admitted:

1. if `S_upper <= T_lower`, set `t_low=t_mid`;
2. else if `T_upper <= S_lower`, set `t_high=t_mid`;
3. otherwise, the split is not certifiable under the current length policy;
   stop with `indeterminate` and retain the current valid bracket.

If a midpoint enclosure and target are both exact and equal, an exact
`[t_mid,t_mid]` bracket may be returned.

A non-endpoint result is `converged` only when:

`t_high - t_low <= parameter_tolerance`.

If `max_refinement_iterations` is exhausted first, return
`indeterminate` with the last valid bracket.

## 10. Absolute-target domain proof

Let total evidence be `[L_lower,L_upper]`.

For finite `s >= 0`:

- `s > L_upper`: definitely outside -> `target_out_of_domain`;
- `s <= L_lower`: membership is proven;
- `L_lower < s <= L_upper`: membership cannot be certified ->
  `target_domain_indeterminate`.

This is intentionally conservative.

## 11. Fraction-target domain proof

For `0 < f < 1`, conservatively compute:

`T = f * [L_lower,L_upper]`

with outward arithmetic.

The initial high endpoint proof requires:

`T_upper <= L_lower`.

If that cannot be proven under the current total-length enclosure, return
`target_domain_indeterminate`.

The caller may request a tighter `CurveLengthPolicy`; the implementation must
not guess the true total.

## 12. Proposed public operations

Exact C++ spelling may be refined during implementation, but the semantic
surface is limited to operations equivalent to:

```text
inverse_arc_length_bracket(
    target_length,
    CurveInverseLengthPolicy)

inverse_arc_length_fraction_bracket(
    normalized_fraction,
    CurveInverseLengthPolicy)
```

for both `CubicBezier2` and `CubicBezier3`.

No generic root-solver API is authorized.

## 13. Required focused evidence

The implementation contract must include at least:

| Case | Required observation |
| --- | --- |
| Absolute zero | regular curve maps `s=0` exactly to `[0,0]`. |
| Fraction zero | `f=0` exactly maps to `[0,0]`. |
| Fraction one | `f=1` exactly maps to `[1,1]`. |
| Exact-total endpoint | exact total target maps to `[1,1]`. |
| Straight cubic | analytic absolute targets retain brackets containing the exact inverse. |
| Straight fractions | representative fractions retain brackets containing the exact fraction parameter for uniform linear cubic fixtures. |
| Nonuniform regular cubic | returned bracket satisfies retained lower/upper cumulative proof inequalities. |
| Parabola reference | target generated from the analytic prefix at a known parameter remains inside the returned bracket. |
| Reversal | corresponding forward/reversed targets produce compatible reversed parameter brackets. |
| 2D/3D parity | planar embedding preserves inverse evidence. |
| Translation | exact translation preserves inverse evidence. |
| Power-of-two scale | scaled absolute target and scaled length tolerance preserve parameter bracket semantics. |
| Negative target | explicit domain failure. |
| Above-total target | explicit domain failure. |
| Uncertain total membership | explicit `target_domain_indeterminate`. |
| Invalid fraction | non-finite/out-of-range fraction fails explicitly. |
| Degenerate regularity | no inverse claim. |
| Indeterminate regularity | no inverse claim. |
| Ambiguous midpoint | retain valid current bracket and return `indeterminate`. |
| Iteration exhaustion | retain valid current bracket and return `indeterminate`. |
| Zero parameter tolerance | no fabricated exact solution. |
| Repeatability | repeated calls retain identical scientific evidence. |
| Header isolation | public curve header exposes exactly the bounded inverse contract. |

## 14. Preservation requirements

FAST and INTEGRATION must preserve all current prerequisite and curve contracts,
including:

- bootstrap;
- numeric;
- Geometry Primitives;
- Minimal Small Linear Algebra;
- math-header isolation;
- Cartesian frames;
- Topological Model;
- cubic curve representation;
- curve differential evaluation;
- global regularity;
- total arc length;
- cumulative arc length;
- curve header isolation.

No existing acceptance criterion may be weakened to admit 2B.

## 15. Explicit exclusions

This decision does not authorize:

- scalar-only inverse results as scientific evidence;
- caller-supplied unbound regularity booleans/evidence;
- Newton, secant, Brent or other acceleration as correctness authority;
- lookup tables or cached sampled mappings;
- fitted inverse parameterizations;
- equal-arc-length physical sample generation;
- adaptive boundary discretization;
- topology ownership;
- surfaces or trimming;
- curvature/feature work;
- meshing;
- Quad-Dominant work;
- parallel execution;
- GPU/SIMD specialization;
- stage qualification.

## 16. Stop conditions

Stop implementation and require a new decision if 2B requires:

- a public generic root solver;
- a public generic interval library;
- target clamping;
- a global tolerance;
- a detached regularity authority;
- an uncertified midpoint decision;
- treating total-length midpoint as exact for a fraction target;
- lookup/sampling approximation as authority;
- physical discretization;
- a surface/topology ownership model;
- parallel execution; or
- reinterpretation of Work Unit 2A evidence.

## 17. Completion gate

Work Unit 2B may close only when:

1. the public API remains inside Sections 5–12;
2. all required focused cases are present;
3. same-curve regularity is internally proven before every unique inverse
   claim;
4. every returned non-endpoint bracket retains inspectable lower/upper
   cumulative proof evidence;
5. both target modes preserve total-length uncertainty;
6. resource/ambiguity paths retain a valid bracket and return
   `indeterminate`;
7. FAST passes;
8. INTEGRATION passes in GCC 13 Debug and Clang 18/libc++ Debug;
9. no excluded downstream capability appears.

Passing this gate establishes only:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

## 18. Stage effect

After 2B integration and closure, Curve Representation still remains
stage-unqualified.

The next transition is a separately pre-registered **Continuous Curve Geometry
Regression** that cumulatively verifies the admitted continuous curve
semantics before physical discretization opens.

## 19. Decision integration and closure

PR #68 integrated this implementation decision as
`f0faaf53e5b898e0270fc0e406cf7337e8d95bb1`.

Validation:

- PR FAST `35592555795`: PASS;
- PR INTEGRATION `35592555778`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- post-merge FAST `35592839727`: PASS;
- post-merge INTEGRATION `35592839775`: PASS.

The decision checkpoint is closed.

The sole next work item is implementation of exactly the bounded
**Certified Inverse Arc-Length Bracketing** contract in this document. No
physical sampling, lookup table, boundary discretization, surface,
Quad-Dominant, parallel, or stage-qualification capability is authorized.

## 20. Implementation result

Implementation branch:
`curve/certified-inverse-arc-length-bracketing`.

PR: #70.

Implemented surface:

- `CurveInverseLengthError`;
- `CurveInverseLengthResult`;
- `CurveInverseLengthPolicy`;
- `CurveInverseLengthEvidence`;
- absolute-target certified inverse-bracket methods for 2D/3D cubics;
- normalized-fraction certified inverse-bracket methods for 2D/3D cubics.

The implementation internally reuses same-curve regularity certification,
certified total arc length and certified cumulative arc length. It introduces
no generic solver or public interval abstraction.

Validation:

- FAST `35593878035`: PASS;
- INTEGRATION `35593878082`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- header isolation covers the bounded public contract;
- all selected prerequisite semantic contracts remain passing.

Completion-gate status:

1. public API remains inside the authorized policy/result/evidence surface:
   PASS;
2. required focused cases: PASS;
3. same-curve regularity before unique inverse claim: PASS;
4. inspectable lower/upper cumulative proof evidence: PASS;
5. both target modes preserve total uncertainty: PASS;
6. ambiguity/resource paths retain a valid bracket and return
   `indeterminate`: PASS;
7. FAST: PASS;
8. GCC/Clang INTEGRATION: PASS;
9. excluded downstream capability absent: PASS.

Result:
**IMPLEMENTED / FOCUSED CONTRACTS PASS / VALIDATED_UNMERGED / NOT QUALIFIED.**

After integration and closure, pre-register the Continuous Curve Geometry
Regression. Physical discretization remains blocked until that stage-level
regression qualifies Curve Representation.

## 21. Implementation integration and closure

PR #70 integrated the bounded implementation as
`a82fa1c96fc6665e586753d5e4eb698012a79be3`.

Validation:

- final PR FAST `35594075619`: PASS;
- final PR INTEGRATION `35594075595`: PASS in GCC 13 Debug and Clang
  18/libc++ Debug;
- post-merge FAST `35594160853`: PASS;
- post-merge INTEGRATION `35594160817`: PASS.

The Work Unit 2B checkpoint is closed.

Final work-unit result:
**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
NOT QUALIFIED.**

The next admissible work is documentation-only pre-registration of the
Continuous Curve Geometry Regression. No physical boundary discretization is
authorized before stage qualification.
