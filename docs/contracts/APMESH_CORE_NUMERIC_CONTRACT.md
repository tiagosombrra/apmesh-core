# AP Mesh Core — Numeric Contract

Status: IMPLEMENTED / QUALIFICATION PENDING
Last updated: 2026-09-05
Scope: Foundation numeric policy before geometry implementation

## 1. Question and boundary

Can AP Mesh Core make scalar numerical decisions without confusing floating-point
agreement, geometric coincidence, topological identity, degeneracy, and
scientific acceptance?

This contract fixes the answer at the policy and minimal numeric-primitives
level. It does not implement points, vectors, predicates, curves, surfaces,
meshing, or a universal tolerance. Passing its bounded gate will qualify the
numeric decision framework, not every future numerical algorithm.

## 2. Declared arithmetic envelope

The initial qualified scalar is C++ `double` on an environment that demonstrates:

- radix 2;
- 53 significand bits;
- IEEE 754 semantics reported by `std::numeric_limits<double>::is_iec559`;
- round-to-nearest as the active reference rounding mode;
- no compiler option that permits unsafe reassociation, ignores NaN/Inf, or
  silently changes the declared floating-point model.

The qualification gate must observe these properties rather than infer them
from compiler names. A different scalar, rounding mode, accelerator, or
extended-precision path requires its own evidence.

## 3. Numeric values and failure classes

Scientific inputs and successful scientific outputs are finite unless an API
explicitly defines another domain. NaN and infinities are diagnostic states,
not valid geometry and not values that may flow silently into later stages.

The shared vocabulary is:

- `invalid_numeric`: a required input, parameter, or result is NaN or infinite;
- `invalid_policy`: a scale or tolerance policy violates its preconditions;
- `degenerate`: a declared mathematical regularity or rank condition is
  certified as not satisfied;
- `ill_conditioned`: the input is admissible, but the operation's declared
  error model cannot support the requested conclusion reliably;
- `indeterminate`: a bounded computation cannot certify the required decision;
- `valid`: the operation's explicit postconditions are certified.

`ill_conditioned` and `indeterminate` are not synonyms for `degenerate`.
Algorithms must not convert one class into another to make a fixture pass.

Positive and negative zero compare numerically equal. Their byte encodings are
not scientific identities. Canonical result encoding, including zero
normalization where needed, belongs to the Reproducible Experiment Contract.

## 4. Identity, equality, proximity, and acceptance

These relations are distinct:

1. **Topological identity** uses explicit typed IDs only.
2. **Exact numeric equality** uses an operation's exact/discrete invariant or
   exact floating comparison when that is the stated contract.
3. **Numeric proximity** is a contextual bounded-error decision.
4. **Scientific acceptance** is a claim-specific gate defined by the owning
   algorithm or experiment.

Coordinate proximity must never create or merge topological identity. Numeric
proximity must never be used implicitly as a predicate sign or as a universal
scientific acceptance rule.

## 5. Physical scale and units

Every dimensional numeric decision declares the quantity's unit and a positive,
finite characteristic scale `S` with the same dimension. The owning operation
defines how `S` is obtained and why it is physically meaningful.

`S` must not be:

- hidden global state;
- an undocumented constant;
- inferred from entity identity;
- based only on absolute coordinate magnitude when translation would change the
  decision;
- tuned per fixture to obtain acceptance.

Unit conversion must transform values, absolute bounds, and `S` consistently.
Relative bounds are dimensionless. A future units type may strengthen this
contract, but no dependency or abstraction is selected here.

## 6. Scale-aware proximity

A proximity policy contains:

- an absolute allowance `a >= 0`, with the quantity's dimension;
- a relative allowance `r >= 0`, dimensionless.

For finite values `x` and `y` and declared scale `S > 0`, the conceptual bound is

```text
residual = |x - y|
limit    = a + r S
within   = residual <= limit
```

The implementation must detect overflow or a non-finite intermediate and
return an explicit failure. It must not accept a result merely because the
computed limit became infinite.

There is no default policy and no project-wide epsilon. The caller supplies an
immutable, validated policy owned by the mathematical operation. ULP distance
may be reported as a diagnostic but is not the universal acceptance rule.

The following properties are required for admissible inputs:

- symmetry in `x` and `y`;
- reflexivity for finite `x`;
- exact-boundary inclusion through `<=`;
- consistent classification under exact power-of-two rescaling of `x`, `y`,
  `a`, and `S`, where no overflow or underflow occurs;
- explicit rejection of invalid policies and non-finite data.

Transitivity is not promised: approximate proximity is not an equivalence
relation.

## 7. Robust decision predicates

A branch that changes topology or combinatorics requires a certified sign:

```text
negative | zero | positive | indeterminate/error
```

A raw determinant compared with a geometric epsilon is not a robust predicate.
An implementation may use a fast floating filter only when it has a justified
error bound. If that filter cannot certify the sign, it must invoke an approved
adaptive/exact stage or return an explicit indeterminate result. It must not
guess, clamp, retry with a hidden tolerance, or change topology by proximity.

This contract adopts that obligation but does not select or implement a
predicate library. Predicate algorithms, dependency choices, admissible input
classes, and their proofs/evidence are bounded work units of the geometry or
meshing stage that first needs them.

## 8. Degeneracy and conditioning

Each operation that can become singular or lose regularity must define:

- the mathematical quantity whose zero or rank loss denotes degeneracy;
- the dimensions and scale of that quantity;
- an error or residual model;
- what can be certified exactly or with a rigorous bound;
- when the answer is `degenerate`, `ill_conditioned`, or `indeterminate`;
- diagnostics returned with failure.

No universal determinant, derivative, Jacobian, curvature, or area threshold is
admitted. A small magnitude alone is insufficient without dimensional scale and
an operation-specific error analysis.

## 9. API and state obligations

The first implementation introduces only the smallest vocabulary needed for:

- floating classification;
- validation of scale and proximity policy;
- scale-aware scalar proximity with structured failure.

It uses value semantics, immutable inputs, and explicit results. It does not
introduce mutable global numeric state, geometry types, predicate algorithms,
I/O, logging, environment lookup, or fixture-specific constants.

Scientific algorithms added later must return useful residual, error,
conditioning, convergence, or evaluation diagnostics when such information is
material to the decision.

## 10. Determinism and portability

Determinism is assessed inside a declared arithmetic environment. Bitwise
identity across different compilers is not promised by this contract. Cross-cell
evidence compares declared semantic results and diagnostic fields; any numeric
difference must be bounded and classified rather than ignored.

The qualified reference must not depend on accidental excess precision,
fast-math, mutable floating environment, unspecified container order, or hidden
hardware state. Later experiment manifests will record the full environment.

## 11. Bounded qualification gate

The Numeric Contract can move to `QUALIFIED` only after one implementation and
one pre-registered regression demonstrate all of the following:

| Gate | Required evidence |
| --- | --- |
| N0 — scope | No geometry, topology, meshing, I/O, or hidden global policy entered the slice. |
| N1 — environment | Binary64 and reference rounding assumptions are checked in every build cell; unsafe floating flags are absent. |
| N2 — classification | Finite, zero, subnormal, infinity, and NaN cases are classified explicitly. |
| N3 — policy | Negative/non-finite tolerances and non-positive/non-finite scales are rejected. |
| N4 — proximity | Symmetry, reflexivity, exact boundary, outside-boundary, near-zero, large-scale, overflow, and power-of-two scale cases pass against independently declared expected results. |
| N5 — separation | Tests show proximity cannot create identity or silently provide a predicate sign. |
| N6 — reproducibility | GCC 13/libstdc++ and Clang 18/libc++, Debug and Release, produce equivalent structured classifications in three clean repetitions. |
| N7 — preservation | The qualified Architecture Contract regression remains PASS. |

The regression produces a manifest, machine-readable certificate, and compact
table. No spatial figure is required because this gate makes no spatial claim.
Unexpected differences are `INVESTIGATION_REQUIRED`, not automatically accepted.

Focused GCC/Clang Debug development checks cover the minimal implementation but
are not N0–N7 qualification evidence. Passing N0–N7 advances Foundation from
25% to 50%. Merely writing this contract, implementing code, or passing focused
development tests does not.

## 12. Retained limitations

- No point/vector, geometry, topology, curve, surface, or mesh semantics are
  implemented or qualified.
- No robust geometric predicate implementation is selected or qualified.
- No general conditioning estimator is provided.
- No unit system or arbitrary/multiprecision dependency is selected.
- Native Windows remains outside the qualified toolchain envelope.
- The Reproducible Experiment Contract still owns general manifests,
  serialization, and evidence retention.

## 13. Basis and non-claims

| Source | Supports | Does not prove |
| --- | --- | --- |
| IEEE 754-2019 | Declared floating formats, operations, exceptional values, rounding, and exception handling | Correctness of AP Mesh algorithms |
| Goldberg 1991 | Distinguishing representation, rounding error, relative error, and exceptional behavior | A universal geometric tolerance |
| Higham 2002 | Separating conditioning, forward/backward error, residuals, and stability | That every future AP Mesh operation is stable |
| Shewchuk 1997 | Error-bounded adaptive precision for certified geometric predicate signs | That an unimplemented predicate or arbitrary mesh algorithm is robust |

The full references and links are maintained in
`docs/research/REFERENCE_REGISTER.md`.
