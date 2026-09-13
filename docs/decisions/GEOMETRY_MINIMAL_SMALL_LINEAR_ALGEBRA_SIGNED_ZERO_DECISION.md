# Geometry Primitives — Minimal Small Linear Algebra Signed-Zero Decision

Status: ACCEPTED / EVIDENCE CORRECTION IMPLEMENTED / FIRST ATTEMPT BLOCKED
Date: 2026-09-13
Stage: Geometry Primitives — Exact Semantics Before Curves
Authority: `docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md`
Affected protocol: `docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md`
Blocked candidate: `6fa00bd34a95673822203a7291c0eb6c062e4606`

## 1. Question

Does the LA `exact_hex` comparison treat `+0.0` and `-0.0` as distinct
scientific results, and what is the smallest correction required after the
first formal LA0–LA7 attempt reported those encodings as an exact match?

## 2. Evidence from the blocked attempt

The retained attempt `la0-la7-6fa00bd-20260913T082500BRT` executed four
GCC/Clang Debug/Release cells and produced twelve identical certificates. In
`mat2_quarter_turn_square`, the independent expectation contains `0x0p+0`
while the observed result contains `-0x0p+0` in one matrix entry. The record
nevertheless declares `exact_match=true` because the validator compares parsed
floating values with numeric equality.

The matrix is algebraically the expected negative identity. The discrepancy is
therefore in the declared evidence equivalence, not evidence of an accepted
`Mat2`/`Mat3` behavior defect.

## 3. Authorities and interpretation

The Numeric Contract states that positive and negative zero compare
numerically equal and that their byte encodings are not scientific identities.
The Reproducible Experiment Contract requires any canonical serialization to be
versioned, tested, and documented. Consequently, LA may preserve the raw sign
of zero as diagnostic provenance, but it may not use that sign as a scientific
distinction.

## 4. Decision

For LA evidence, `exact_hex` means exact comparison of a canonical hexadecimal
numeric representation:

1. every finite nonzero value retains its exact hexadecimal representation;
2. both `+0.0` and `-0.0` canonicalize to `0x0p+0` before semantic comparison;
3. raw expected and observed hexadecimal strings remain recorded unchanged;
4. error outcomes and non-finite classifications remain exact and are never
   normalized into values;
5. no tolerance, proximity, clamp, retry, or fallback is introduced.

The comparison schema must declare the signed-zero policy explicitly. An
`exact_match` field without that declaration is insufficient evidence.

## 5. Approved minimum correction

The next implementation work unit is limited to evidence code and contracts:

- add one canonical-hex helper that normalizes only signed zero;
- compare canonical hexadecimal strings instead of relying on floating numeric
  equality;
- require `signed_zero_policy=normalize_to_positive` in the profile and every
  exact comparison record;
- retain raw expected and observed strings in certificates and reports;
- add a positive contract for `+0.0` versus `-0.0` and negative contracts for
  an undeclared policy and a one-ULP nonzero mismatch;
- make the cross-cell comparer and retention verifier reject any missing or
  inconsistent policy declaration.

Production `Mat2`, `Mat3`, Geometry adapters, numeric policy, and admitted
operations must not change.

## 6. Gate effect

The attempt on `6fa00bd` remains immutable and is classified:

- `LA0`, `LA1`, `LA2`, `LA4`, `LA5`, and `LA6`: evidence supports `PASS`;
- `LA3`: `BLOCKED` by underdeclared signed-zero equivalence;
- `LA7`: `BLOCKED` because the validator could assert exact match without the
  required canonicalization contract;
- overall: `BLOCKED`.

Correcting the tooling does not retroactively qualify `6fa00bd`. After focused
contracts pass, one new clean revision-bound LA0–LA7 execution and independent
audit are required. The blocked package remains retained as negative evidence.

## 7. Retained limitations

This decision does not qualify native Windows, parallel execution, arbitrary
conditioning, transformations, predicates, topology, curves, surfaces, or
meshing. It does not assign orientation, rank, degeneracy, or topology meaning
to a determinant.

## 8. Next admissible action

Independently audit the implemented evidence correction before authorizing a
new revision-bound formal LA0–LA7 manifest. Do not relaunch formal LA0–LA7
qualification in that work unit.
