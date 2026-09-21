# Continuous Curve Geometry Regression — PREPARED Package Audit

Status: **PASS / PREPARED / NOT EXECUTED**  
Audit date: 2026-09-21  
Candidate: `f7dc8d82d881858b6481d6d2d1383d8a561684c5`  
Preparation run: `35620525792`

## 1. Scope

This audit independently verifies the first formal Continuous Curve Geometry
Regression PREPARED package after the CGR formal infrastructure was integrated
and closed.

This is preparation-only evidence. It does not authorize execution, create an
execution claim, or decide CGR0–CGR7.

## 2. Preparation provenance

The canonical workflow `Continuous Curve Geometry Regression Preparation` was
dispatched exactly once on branch `main`.

Observed run:

- run ID: `35620525792`;
- event: `workflow_dispatch`;
- branch: `main`;
- head SHA:
  `f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
- conclusion: `success`.

The single retained artifact is:

- artifact ID: `10649325906`;
- name:
  `cgr-prepared-f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
- size: `123662` bytes;
- GitHub artifact SHA-256:
  `952cadc3d5cc761105d5100319cf24077cd9b0ac5d0a42000a8ae1e3eac91063`.

The independently downloaded ZIP recomputed to the same SHA-256.

## 3. Retained PREPARED package

The archive contains exactly seven files:

1. `plan.json`;
2. `planned-inventories.json`;
3. `preparation-seal.json`;
4. `prepared-manifest.json`;
5. `profile.json`;
6. `state-history.jsonl`; and
7. `state.json`.

No execution artifact is present.

Verified absence includes:

- `execution-claim.json`;
- `command-records.json`;
- `certificate-index.json`;
- `per-cell-comparisons.json`;
- `cross-cell-comparison.json`;
- `observed-inventories.json`;
- `derived-evidence.json`;
- `gate-summary.json`;
- `terminal-manifest.json`;
- `failure.json`; and
- `retention-manifest.json`.

Lifecycle is exactly `PREPARED`,
`execution_requested=false`, and CGR0–CGR7 are all `NOT_EXECUTED`.

## 4. Independently recomputed package hashes

| File | SHA-256 |
| --- | --- |
| `prepared-manifest.json` | `201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd` |
| `preparation-seal.json` | `686c5192f5373c42e54339fdd38519e62ebef009dc74ae927c34fe97919b5353` |
| `plan.json` | `5c1df6308a13a101692c142cf9ac6462f7c47cdedb6e6e34371b1de4b9cf5e27` |
| `planned-inventories.json` | `47fd3286708aacb2b88e2161ffabda40ee74e14d746489dabb4480fb0f7ec75b` |
| `profile.json` | `115ef0b5d8c3cef806c246d644b672e73ee6da57ca535f8a8afc0f11084345d0` |
| `state-history.jsonl` | `54a6ac988387214d431bc198e92482303200201002d954a6bbefb558c9bf1e9e` |

Every retained hash in `preparation-seal.json` recomputes exactly. The seal
also binds the initial PREPARED state-history record.

`state.json` binds prepared-manifest SHA-256:

`201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`.

## 5. Candidate identity and full source inventory

The manifest binds:

- candidate commit:
  `f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
- upstream commit:
  `f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
- tree clean: `true`;
- semantic baseline:
  `438620efa1f93d29b442e9ba199882a09d2359d9`.

The PREPARED source inventory contains exactly **1595** tracked files.

An independent recursive Git tree query for the candidate also returns exactly
**1595** blobs and is not truncated.

The normalized path-list SHA-256 is identical in both views:

`46b39d68c68f8f580f4c1c61a08ee3014c834ecccb08a3b9e7d5e80a498b8b69`.

Therefore the PREPARED source inventory and published GitHub candidate agree on
the complete tracked path set.

## 6. Critical input binding

All **18/18** preparation-critical inputs were independently fetched from the
exact GitHub candidate and recomputed to the SHA-256 values retained by the
manifest.

This includes:

- CGR profile and protocol;
- CGR preparation decision;
- exporter;
- evidence validator;
- negative-evidence tool;
- report-only runner;
- formal campaign runner;
- experiment runtime;
- CMake authority;
- admitted cloud profile/validator/decision/audit;
- prepare, execute and authorize workflows; and
- authorization validator.

Result: **18/18 PASS**.

## 7. Frozen semantic baseline

All **11/11** protocol-frozen production/test semantic files were independently
recomputed against candidate
`f7dc8d82d881858b6481d6d2d1383d8a561684c5`.

Every candidate SHA-256 equals the corresponding baseline SHA-256 retained from
semantic baseline
`438620efa1f93d29b442e9ba199882a09d2359d9`.

Result: **11/11 PASS**.

No unreviewed curve-semantic change occurred between protocol baseline and the
formal prepared candidate.

## 8. Admitted cloud environment

All four preparation observations are `PASS` and bind the accepted cloud
envelope:

- GitHub runner label: `ubuntu-24.04`;
- image OS: `ubuntu24`;
- image version: `20260907.300.1`;
- architecture: `x86_64`;
- GCC: `13.3.0`;
- Clang: `18.1.3`;
- CMake: `3.28.3`;
- Ninja: `1.11.1`;
- libc++/libc++abi: `18.1.3`.

No cloud observation is blocked or drifting.

## 9. Formal four-cell × two-repetition plan

The campaign is fixed to:

- GCC 13 / libstdc++ / Debug;
- GCC 13 / libstdc++ / Release;
- Clang 18 / libc++ / Debug;
- Clang 18 / libc++ / Release.

Each cell has exactly two repetitions.

The semantic CTest allowlist contains exactly **14** tests.

Per cell the formal plan contains:

- configure once;
- for each of two repetitions:
  - build;
  - CTest discovery;
  - exact semantic CTest;
  - certificate production;
  - certificate validation;
- negative evidence once;
- dependency inventory once; and
- runtime dependency inventory once.

Total planned cardinality:

- command records: **56**;
- command logs: **112**;
- CTest discoveries: **8**;
- semantic CTest records: **8**;
- individual semantic test executions: **112**;
- certificates: **8**.

This is exactly the cardinality pre-registered by the CGR preparation decision
and protocol.

## 10. Planned terminal evidence

`planned-inventories.json` contains exactly **156** artifact entries:

- 12 preparation/control;
- 112 command logs;
- 8 semantic certificates;
- 4 compile-command inventories;
- 4 negative-evidence artifacts;
- 9 derived-evidence files;
- 6 terminal-success controls; and
- 1 terminal-failure file.

The planned inventory distinguishes `always`, `executed`, `success`, and
`failure` evidence.

These are plans only. No terminal evidence is present in the PREPARED archive.

## 11. Audit decision

**PASS / PREPARED / NOT EXECUTED.**

The package is:

- revision-bound;
- upstream-bound;
- clean-tree-bound;
- cloud-bound;
- source-inventory-consistent;
- frozen-semantic-baseline-consistent;
- input-hash-consistent;
- internally sealed; and
- prepared with the exact pre-registered CGR campaign cardinality.

No CGR gate has been executed or decided.

## 12. Next bounded action

The integrated CGR authorization infrastructure is already manifest-driven and
machine-readable-audit-driven.

After this preparation-audit checkpoint is merged, post-merge FAST/INTEGRATION
pass, and the checkpoint is closed, create one separate exact
`EXECUTE_ONCE` authorization-record PR for prepared-manifest SHA-256:

`201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`.

That authorization PR must contain exactly one newly added manifest-bound JSON
record and no other repository change.

Merging that later one-file PR to protected `main` will be the formal
one-shot execution authorization event.

Machine-readable audit:

`docs/audits/2026-09-21-continuous-curve-geometry-regression-preparation-audit.json`.
