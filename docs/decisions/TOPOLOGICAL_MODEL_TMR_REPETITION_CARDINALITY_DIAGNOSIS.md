# Topological Model TMR — Repetition-Cardinality Diagnosis

Status: **DIAGNOSIS COMPLETE / MECHANICAL TOOLING DEFECT / SCIENTIFIC CLAIM UNCHANGED**  
Date: 2026-09-20  
Consumed campaign: run `35528077223`  
Blocked terminal audit:
`docs/audits/2026-09-20-topological-model-tmr-terminal-audit.md`

## 1. Question

The first formal Topological Model cumulative-regression campaign retained
eight equivalent topology certificates but only four semantic CTest command
records. The sealed protocol requires four cells with two repetitions per cell
and states that every cell must "discover, build, and execute exactly once per
repetition"; TMR6 separately requires the exact seven-test allowlist to pass in
every repetition.

This diagnosis determines the required repetition scope, the exact evidence
cardinality, and the smallest admissible correction. It does not change the
first campaign, prepare another manifest, or authorize another execution.

## 2. Authorities and fixed observations

The diagnosis uses the exact protocol/profile/runner sealed into the consumed
candidate `e5eda2663d6ff4b93ce1205660ff04d432acb9c0` plus the retained
terminal audit.

The current `main` runner and profile are byte-identical to the consumed
candidate:

- `tools/run_topological_model_cumulative.py` blob
  `ff6de98cc0127642356a127ecfdf8f5f6c9f50bd`;
- `experiments/profiles/topological_model_cumulative.json` blob
  `f27c3fa1d9630abe9976a29f6d81d853f4c196b2`.

The profile correctly declares four cells and
`repetitions_per_cell = 2`. The defect is not the repetition count in the
profile.

## 3. Why Geometry Primitives does not justify the current TMR runner

The qualified Geometry Primitives cumulative protocol required each cell to
build and execute one exact semantic allowlist and separately required three
independent stage certificates. Its runner therefore configured/built/ran
CTest once per cell and repeated only certificate generation.

The Topological Model protocol intentionally uses stronger wording:

- Section 6 assigns **two repetitions to each matrix cell**;
- Section 7 requires every cell to **discover, build, and execute exactly once
  per repetition**;
- TMR6 requires the exact seven-test allowlist to **pass in every repetition**;
- TMR5 requires eight repeated/cross-cell certificates.

Therefore the Geometry Primitives execution shape cannot be imported as an
implicit exception to the Topological Model protocol.

## 4. Required repetition scope

The smallest interpretation that satisfies every sealed requirement without
inventing a stronger claim is:

### Once per cell

- configure the declared compiler/build-type cell;
- run certificate negative outcomes after the repetitions;
- collect dependency inventory;
- inspect exporter runtime dependencies with `ldd`;
- retain the final `compile_commands.json` inventory.

### Once per repetition inside each cell

In this exact order:

1. invoke the sealed build command;
2. run CTest discovery and prove the exact seven-test semantic allowlist;
3. execute that exact semantic CTest allowlist and prove 7/7 PASS;
4. produce one topology certificate;
5. independently validate that certificate.

The protocol does **not** state that each repetition requires a fresh CMake
configuration, a clean build tree, or a distinct build directory. Adding such a
requirement after the blocked campaign would strengthen the claim rather than
repair the identified mismatch.

Therefore the minimal future correction keeps one configured build tree per
cell and re-invokes the sealed build command in each repetition. The second
build may be incremental/no-op; the required evidence is that the exact build
command was invoked successfully for that repetition before discovery,
semantic CTest, and certificate production.

This clarification does not rescue the consumed campaign: it had only one
build/discovery/semantic-CTest record per cell, not two.

## 5. Exact command cardinality

For each of four cells, the corrected execution requires:

| Scope | Command | Records per cell |
| --- | --- | ---: |
| cell | configure | 1 |
| repetition | build | 2 |
| repetition | CTest discovery | 2 |
| repetition | semantic CTest allowlist | 2 |
| repetition | certificate | 2 |
| repetition | certificate validation | 2 |
| cell | negative outcomes | 1 |
| cell | dependency inventory | 1 |
| cell | runtime dependency / `ldd` | 1 |
| **total** |  | **14** |

Across four cells:

- required command records: **56**;
- required semantic CTest records: **8**;
- required individual semantic test executions: **56** = 8 × 7;
- required discovery records: **8**;
- required build records: **8**;
- required certificates: **8**;
- required certificate-validation records: **8**;
- required negative-outcome records: **4**;
- required dependency-inventory records: **4**;
- required runtime-dependency records: **4**;
- required configure records: **4**.

The consumed campaign retained 44 command records. The exact deficit is 12:

- 4 missing second-repetition build records;
- 4 missing second-repetition CTest-discovery records;
- 4 missing second-repetition semantic-CTest records.

The eight certificate and eight certificate-validation records already had the
correct cardinality.

Because every command record owns stdout/stderr logs, the correction adds 24
required command-log files relative to the consumed execution.

## 6. Runner defect

Current `execute()` performs:

`configure -> build -> discovery -> semantic CTest`

once per cell, then enters the repetition loop only for:

`certificate -> certificate validation`.

That control flow contradicts Sections 6–7 and TMR6 of the sealed protocol.

The minimal correction is structural, not scientific:

- configure once before the repetition loop;
- move `build`, `ctest_discovery`, and `semantic_ctest` into the existing
  repetition loop before certificate generation;
- preserve `certificate` and `certificate_validation` in that loop;
- keep negatives, dependency inventory, `ldd`, and compile-command retention
  once per cell after the repetitions.

Record IDs and log names for build/discovery/semantic CTest must include the
repetition ordinal, for example:

- `gcc-debug-build-1`;
- `gcc-debug-ctest-discovery-1`;
- `gcc-debug-semantic-ctest-1`;
- `gcc-debug-build-2`;
- `gcc-debug-ctest-discovery-2`;
- `gcc-debug-semantic-ctest-2`.

The observed-discovery inventory must retain both cell and repetition ordinal.

## 7. Planned-inventory correction

`planned_inventories()` currently plans configure/build/discovery/semantic
logs once per cell and certificate logs once per repetition.

The corrected inventory must:

- retain configure logs once per cell;
- retain build/discovery/semantic/certificate/certificate-validation logs once
  per repetition;
- retain negative/dependency/`ldd` logs once per cell;
- retain eight certificates and four compile-command inventories as before.

The profile itself does not need a scientific change: it already fixes the
correct four cells, two repetitions, seven-test allowlist, cases, gates, and
limitations.

## 8. Focused correction contracts

The next correction work item must add focused assertions that were missing
from the first runner contract.

A synthetic successful execution must prove:

1. exactly 56 command records;
2. exactly 14 command records per cell;
3. exactly two build, two discovery, two semantic CTest, two certificate, and
   two certificate-validation records per cell;
4. exactly one configure, negative-outcomes, dependency-inventory, and
   runtime-dependency record per cell;
5. repetition ordinals `1` and `2` appear exactly once for every
   repetition-scoped stage in every cell;
6. exactly eight discovery observations, each bound to cell + repetition;
7. every semantic CTest record proves the exact seven-test allowlist and 7/7
   PASS;
8. the planned retained logs match the same cardinality;
9. a focused failure in a second-repetition semantic CTest fails closed and
   prevents later scientific commands;
10. a consumed successful or blocked attempt remains immutable.

No production topology test expectation changes.

## 9. Secondary protocol-guard defect

During this diagnosis, TMR tooling run `35529611062` failed in both GCC and
Clang cells before tooling configuration with:

`ERROR: protocol is not the pre-registered TMR0-TMR7 authority`.

Cause: `protocol_check()` requires the literal transient heading
`## 13. Next bounded action`. The terminal-audit integration legitimately
renamed Section 13 to
`## 13. First formal execution result and next bounded action`.

The protocol's scientific requirements did not disappear; the guard is coupled
to operational prose rather than stable scientific invariants.

The same focused correction work item may repair this mechanical guard because
it is required for any future runner validation and does not change the
scientific claim. The guard should validate stable protocol invariants,
including at minimum:

- the TMR0–TMR7 gate section;
- the two-repetition matrix;
- the exact per-repetition build/discover/execute requirement;
- TMR6's allowlist-in-every-repetition requirement;
- fail-closed missing-evidence semantics;
- detached verification; and
- the admitted cloud-environment supplement.

It should not require the exact title of an operational "next action" section.

Run `35529611062` is a diagnosis-time tooling failure, not a formal
scientific campaign attempt. It created no PREPARED package, execution claim,
or scientific evidence.

## 10. Protocol clarification for future campaigns

A prospective clarification may be added to the active protocol:

> Configure each matrix cell once. For each declared repetition, invoke the
> sealed build command on that configured cell build tree, rediscover the exact
> semantic CTest allowlist, execute it exactly once, then produce and validate
> one topology certificate. A fresh configuration, clean rebuild, or distinct
> build directory is not required unless separately declared. Negative,
> dependency, runtime, and compile-command inventories are cell-scoped.

This text makes the original cardinality executable without weakening the
blocked first-campaign decision. It applies only to a newly prepared future
campaign.

## 11. Requirements before a second formal campaign

The consumed first campaign must never be retried or amended.

A future campaign requires, in order:

1. a focused runner/protocol-guard correction work item only;
2. focused TMR tooling contracts PASS in both declared tooling cells;
3. correction PR merge plus post-merge FAST/INTEGRATION;
4. a closed correction checkpoint on `main`;
5. a **new** clean candidate containing the authorized tooling correction and
   no undeclared production topology change;
6. a **new** PREPARED package with new input hashes, plan, planned inventories,
   preparation seal, manifest hash, run ID, and artifact ID;
7. independent audit of that new PREPARED package;
8. a new exact authorization binding for the audited new manifest/artifact
   because the existing authorization-as-code constants are intentionally
   bound to the consumed first package;
9. one new repository-resident `EXECUTE_ONCE` authorization;
10. exactly one formal execution;
11. an independent terminal TMR0–TMR7 audit.

No new preparation or authorization belongs to the correction work item.

## 12. Decision

**Diagnosis: CLOSED.**

Classification:

- production topology defect: **NOT EVIDENCED**;
- first campaign evidence defect: **CONFIRMED**;
- primary cause: **runner repetition-scope mismatch**;
- secondary cause: **fragile protocol-title guard**;
- scientific acceptance criteria change: **NONE**;
- profile scientific change required: **NONE**;
- runner correction required: **YES**;
- new PREPARED package required: **YES**;
- first-campaign retry authorized: **NO**.

The next admissible work item is the focused mechanical correction described
above. It must not prepare or execute a formal campaign.
