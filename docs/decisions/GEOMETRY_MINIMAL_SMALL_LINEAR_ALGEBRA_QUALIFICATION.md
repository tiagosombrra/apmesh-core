# Geometry Primitives - Minimal Small Linear Algebra Qualification

Status: QUALIFIED / LA0-LA7 PASS / WSL Ubuntu 24.04
Date: 2026-09-13
Protocol: `docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md`
Authority: `docs/contracts/APMESH_CORE_MINIMAL_SMALL_LINEAR_ALGEBRA_CONTRACT.md`
Candidate: `3804e903f56a226d38319fd44255b5832639815d`

## Question

Can the bounded `Mat2`/`Mat3` capability be qualified as a deterministic,
finite-valued, fixed-size algebra layer while preserving the qualified
Foundation and Point/Vector semantics?

## Candidate and retained evidence

The audit covers the clean, published candidate above, aligned with its
upstream at the time of execution. The immutable prepared-manifest SHA-256 is
`420e607ff88ea86fde6b62078a4b3a8e5973054294012da02b6a42ffc96a3afd`.

The retained package `la0-la7-3804e90-prepared-20260913-02` contains the
prepared and terminal manifests, 12 certificates, cross-cell comparison,
56 command records, source and runtime inventories, 40 expected negative
rejections, retention manifest, and detached verification. It retains 201
files and 18 required paths while excluding reproducible build trees, caches,
object files, and binaries.

The original package was located without reconstruction and retained
byte-for-byte at
`evidence/geometry-primitives/minimal-small-linear-algebra/la0-la7-3804e90-prepared-20260913-02/`.
Its retention manifest has SHA-256
`24894a2cf9875254862a27ac80582d2652a2543c891a59ed7920478d7c6aeb65`.
Independent size and SHA-256 verification of all 201 declared entries found
zero missing or mismatched files. The 169 source-side build/cache artifacts not
declared by the manifest were intentionally excluded as reproducible extras.

## Execution and audit

The one authorized execution used GCC 13/libstdc++ and Clang 18/libc++, each
in Debug and Release, with three independent certificate processes per cell.
All 91 case records were equivalent under the declared `exact_hex` rule with
`signed_zero_policy=normalize_to_positive`; no proximity tolerance was used.
Raw signed-zero encodings remain diagnostic provenance. All 40 negative cases
were rejected for their declared reasons, all 56 commands exited successfully,
and detached verification passed for the candidate source inventory.

| Gate | Decision | Audit basis |
| --- | --- | --- |
| LA0 | PASS | Clean published candidate, immutable manifest, complete revision binding, and admitted scope. |
| LA1 | PASS | Distinct dimensions, checked access, one-way dependencies, and rejection cases passed. |
| LA2 | PASS | Non-finite placement and deliberate non-finite-result cases returned exact declared errors. |
| LA3 | PASS | Independent algebraic cases, including quarter-turn, composition, transpose, and application, matched exactly. |
| LA4 | PASS | Determinant observations retained the required predicate, rank, degeneracy, orientation, incidence, and topology non-claims. |
| LA5 | PASS | Five power-of-two scale cases and three repeats per cell were semantically identical across four cells. |
| LA6 | PASS | The exact 16-test Architecture, Numeric, Reproducible Experiment, and Point/Vector allowlist passed in every cell. |
| LA7 | PASS | Certificates, negatives, command records, inventories, hashes, retention seal, and detached verification were complete. |

## Decision and limitations

**Decision: PASS WITH RETAINED LIMITATIONS.** Minimal Small Linear Algebra is
`QUALIFIED` only in the WSL Ubuntu 24.04 GCC 13 / Clang 18 libc++ envelope.

The earlier `6fa00bd` attempt remains immutable `BLOCKED` evidence for an
underdeclared signed-zero equivalence. The successful candidate corrected only
evidence semantics: signed zero is canonical numeric zero for comparison, raw
encodings are retained diagnostically, and finite nonzero values remain exact.
No production `Mat2`, `Mat3`, Geometry adapter, numeric policy, tolerance, or
accepted behavior changed.

This decision does not qualify native Windows, portability beyond the declared
toolchains, parallelism, transforms, frames, inverses, solves, decompositions,
predicates, topology, curves, surfaces, meshing, or the full Geometry
Primitives stage. The cumulative Geometry Primitives regression remains open.

## Next bounded action

No implementation follows automatically. A separate scientific entry decision
is required before Transformations and Coordinate Frames can begin.
