# Topological Model TMR — Terminal Scientific Audit

Status: **BLOCKED / FORMAL ATTEMPT CONSUMED / STAGE NOT QUALIFIED**  
Audit date: 2026-09-20  
Authorization commit: `8a6eafc02d5e69f467e2badfea0b571e253b84bd`  
Formal execution run: `35528077223`  
Scientific candidate: `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`

## 1. Scope and decision rule

This is the independent terminal audit required by the pre-registered
Topological Model cumulative-regression protocol.

A successful GitHub Actions process exit is not a scientific PASS. Section 9 of
the sealed protocol requires TMR0–TMR7 together, and Section 10 requires missing
evidence to be classified as `BLOCKED`, never inferred or rescued after the
fact.

The formal execution attempt is immutable and consumed. No rerun, rescue
dispatch, replacement authorization, or relaxation of the pre-registered
acceptance criteria is authorized by this audit.

## 2. Formal authorization and execution provenance

The repository-resident authorization record was merged by PR #29 at commit
`8a6eafc02d5e69f467e2badfea0b571e253b84bd`.

The protected-main controller run `35528077223`:

1. proved that the exact manifest-bound authorization file was newly added;
2. validated the closed `EXECUTE_ONCE` schema;
3. verified that the PREPARED manifest was not previously claimed;
4. invoked the reusable one-shot executor;
5. revalidated the authorization commit and the exact historical candidate;
6. restored and revalidated the exact PREPARED artifact;
7. created the immutable execution claim;
8. invoked `execute` exactly once;
9. verified retained evidence; and
10. retained one terminal artifact.

The immutable claim is:

`tmr-execution-claim-d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd`

Annotated tag object:
`4fac06f3f518498768dce4bc4dc5b3ecb9bc7512`.

The tag points to candidate
`e5eda2663d6ff4b93ce1205660ff04d432acb9c0` and records the authorization
commit, authorization file, preparation run `35524700979`, prepared-manifest
SHA-256, and execution run `35528077223`.

## 3. Terminal artifact integrity

Retained terminal artifact:

- artifact ID: `10610497080`;
- name: `tmr-terminal-e5eda266-35528077223`;
- size: `247358` bytes;
- GitHub SHA-256:
  `b332b8dde2e8651f4dd66339875378a53c9d4390400afd0d869b54969a2bf983`.

The independently downloaded ZIP recomputed to the same SHA-256.

The extracted terminal package contains 122 retained files. The retention
manifest contains 121 hashed file entries plus the retention manifest itself,
for 122 required paths. Every retained file hash and byte size recomputes
exactly, every required path exists, and no reproducible build directory is
retained.

Additional integrity observations:

- final lifecycle state: `EXECUTED_PENDING_AUDIT`;
- lifecycle: `PREPARED -> RUNNING -> EXECUTED_PENDING_AUDIT`;
- prepared-manifest SHA-256:
  `d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd`;
- preparation seal recomputes exactly;
- execution claim and terminal manifest bind the same prepared-manifest hash;
- detached candidate verification: PASS;
- detached source inventory: 1534 files;
- command records: 44/44 successful, no timeout, no launch error;
- runtime dependency checks: no unresolved `ldd` dependency;
- compile-command inventories: all four hashes verified;
- negative evidence: all five pre-registered forged/invalid outcomes rejected
  in all four cells.

## 4. Certificate and cross-cell evidence

Eight certificates are retained:

- GCC Debug: repetitions 1 and 2;
- GCC Release: repetitions 1 and 2;
- Clang/libc++ Debug: repetitions 1 and 2;
- Clang/libc++ Release: repetitions 1 and 2.

All eight certificate-index hashes recompute and all eight certificate byte
streams are identical:

`e574cc589e2dc75c2ffd218af7336bbdc6202b6e2dcf4fe035a5f4e8d4a791a5`.

The certificates cover the 13 pre-registered cases:

1. empty model;
2. identity and edge order;
3. transactional rejection;
4. invalid lookup and orientation;
5. boundary valence;
6. multiple loops and repeated owner;
7. all five structural classes;
8. three-face multi-use;
9. exact incidence bijection;
10. canonical snapshot;
11. deterministic equal construction;
12. controlled orientation difference; and
13. controlled insertion difference.

Per-cell comparisons report two equal certificate projections in every cell,
and the cross-cell comparison records one identical scientific projection
across all eight certificates.

## 5. Protocol/runner repetition mismatch

This is the blocking finding.

The sealed protocol states in Section 7:

> Every cell must discover, build, and execute exactly once per repetition.

The fixed matrix contains four cells and two repetitions per cell.

TMR6 further requires:

> The exact seven-test allowlist passes in every repetition.

Therefore the campaign requires eight semantic CTest executions, corresponding
to 56 individual semantic test executions (7 tests × 2 repetitions × 4 cells).

Observed terminal evidence contains only four semantic CTest command records:

- `gcc-debug-semantic-ctest`;
- `gcc-release-semantic-ctest`;
- `clang-debug-semantic-ctest`; and
- `clang-release-semantic-ctest`.

Each of those four executions is valid and reports 7/7 PASS. However, the
second repetition in each cell has no corresponding semantic CTest execution.

The cause is visible in the sealed runner: `command_plan()` defines one
`ctest_discovery` and one `semantic_ctest` command per cell, while the
`repetitions` loop is applied only to certificate production/validation.

Observed:

- semantic CTest records: **4**;
- required by protocol: **8**;
- individual semantic tests executed: **28**;
- required by protocol: **56**.

This is not a production-semantic failure. It is an evidence-cardinality
mismatch between the pre-registered protocol and the sealed runner. The
protocol cannot be weakened retroactively to fit the collected evidence.

## 6. TMR0–TMR7 decisions

| Gate | Decision | Audit basis |
| --- | --- | --- |
| TMR0 — Identity and scope | **PASS** | Candidate, upstream, preparation seal, input hashes, clean-tree identity and detached verification agree exactly. |
| TMR1 — Construction and immutability | **PASS** | Eight equal certificates cover strong identity, ordering, transactional rejection/finalization, boundary valences and immutable type contract. |
| TMR2 — Incidence bijection | **PASS** | The declared bijection case records exact 3/3 forward/reverse correspondence with exact owner ordinals/orientation; repeated-owner and multi-use cases are retained. |
| TMR3 — Structural recomputation | **PASS** | All five structural classes and consistency-summary counts are present and identical across all eight certificates. |
| TMR4 — Canonical snapshot | **PASS** | Exact bytes, LF-only/final-LF policy, equal-construction equality and controlled orientation/insertion differences are retained and equal across cells/repetitions. |
| TMR5 — Repeat and cross-cell equivalence | **PASS** | Eight certificates are retained, indexed correctly, hash-identical and scientifically projection-identical. |
| TMR6 — Prerequisite preservation and isolation | **BLOCKED** | Existing CTests are 7/7 PASS, but only once per cell. The protocol requires the exact allowlist in both repetitions of every cell. |
| TMR7 — Evidence integrity and closure | **BLOCKED** | Package/retention integrity is mechanically complete, but the protocol-required command/evidence set is incomplete because the TMR6 repetition evidence is absent. |

## 7. Overall decision

**TMR0–TMR7 overall: BLOCKED.**

The Topological Model stage remains:

**IN INVESTIGATION / NOT QUALIFIED.**

This audit does not invalidate the already accepted Foundation or Geometry
Primitives qualifications. The evidence gap does not contradict those
scientific results; it prevents this Topological Model campaign from satisfying
its own pre-registered exit criteria.

The GitHub Actions run conclusion `success` is retained as a tooling/process
result only. It is not reclassified as scientific qualification.

## 8. Retry and next action

The execution claim exists and the formal attempt is consumed.

Therefore:

- no rerun is authorized;
- no second `EXECUTE_ONCE` record is authorized;
- no manual workflow dispatch is authorized;
- no missing repetition may be filled after the fact and attached to this
  campaign;
- no acceptance criterion may be relaxed retroactively.

The next bounded action is one separately authorized diagnosis of the
protocol/runner repetition-cardinality mismatch. That diagnosis may determine
the minimal runner correction and the requirements for a **new** preparation
and future formal campaign, but it must not execute or prepare another campaign
inside the diagnosis work item.

Machine-readable audit:
`docs/audits/2026-09-20-topological-model-tmr-terminal-audit.json`.
