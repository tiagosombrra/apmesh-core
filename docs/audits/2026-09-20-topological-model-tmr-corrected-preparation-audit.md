# Topological Model TMR — Corrected PREPARED Package Audit

Status: **PASS / PREPARED / NOT EXECUTED**  
Audit date: 2026-09-20  
Candidate: `37f9af77f38e12af0a92d3c0f57f1ad31a218144`  
Preparation run: `35531261000`

## 1. Scope

This audit independently checks the second formal Topological Model TMR
PREPARED package after the repetition-cardinality runner correction was merged
and closed.

The audit is preparation-only. It does not authorize execution, create an
execution claim, or decide any TMR0–TMR7 gate.

## 2. Preparation provenance

The canonical workflow `Topological Model TMR Preparation` was dispatched
exactly once on branch `main`.

Observed run:

- run ID: `35531261000`;
- event: `workflow_dispatch`;
- branch: `main`;
- head SHA:
  `37f9af77f38e12af0a92d3c0f57f1ad31a218144`;
- conclusion: `success`.

The single retained artifact is:

- artifact ID: `10611054028`;
- name:
  `tmr-prepared-37f9af77f38e12af0a92d3c0f57f1ad31a218144`;
- size: `114948` bytes;
- GitHub artifact SHA-256:
  `96a47fcecc524e0a4baccee899bd88be8443dbc9778a55271a8376ebe2f6a1ab`.

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

Verified absence:

- `execution-claim.json`;
- `terminal-manifest.json`;
- `command-records.json`;
- `certificate-index.json`; and
- `failure.json`.

The lifecycle is exactly `PREPARED` and
`execution_requested=false`. TMR0–TMR7 are all `NOT_EXECUTED`.

## 4. Package hashes and preparation seal

Independently recomputed retained hashes:

| File | SHA-256 |
| --- | --- |
| `prepared-manifest.json` | `f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa` |
| `preparation-seal.json` | `366c782c571f6e63e320ac65e51dc464b0e3b3c8de498cfaf49ef50672dba9c2` |
| `plan.json` | `042a436dc4ef42ec6516de377ee5f10080980ad26b209be2802b83694a83d300` |
| `planned-inventories.json` | `1c8037c89506bbb6bea5ae7e76f57135c851933c72051a9ec9f15110205bf51d` |
| `profile.json` | `7ca36e4f0f9a884243071db3914ffc500c13b536f32e3da5560fc8c74cac4399` |
| `state-history.jsonl` | `454d9a8bd020159e060bc263d5b3e67edb9ee1401dca3aea8f57bc9c0d27a087` |

Every hash embedded in `preparation-seal.json` recomputes exactly.

`state.json` binds the same prepared-manifest hash:

`f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`.

## 5. Candidate identity and source inventory

The manifest binds:

- candidate commit:
  `37f9af77f38e12af0a92d3c0f57f1ad31a218144`;
- upstream commit:
  `37f9af77f38e12af0a92d3c0f57f1ad31a218144`;
- tree clean: `true`.

The PREPARED source inventory contains exactly **1546** tracked files.

An independent recursive Git tree query for the candidate also returns exactly
**1546** blobs and is not truncated.

The normalized candidate path-list SHA-256 is identical in both views:

`1afa38a785143adaee41887b23f11e400dd538b73b1492dfae77ffa4d79c4e2a`.

Therefore the retained source inventory and the published candidate tree agree
on the complete tracked path set.

## 6. Critical input binding

The following 12 critical input hashes were independently recomputed from the
exact GitHub candidate and all match the PREPARED manifest:

1. cloud environment audit;
2. cloud environment decision;
3. cloud environment profile;
4. Topological Model cloud supplement;
5. cloud environment validator;
6. root `CMakeLists.txt`;
7. cumulative exporter;
8. TMR profile;
9. cumulative-regression protocol;
10. corrected TMR runner;
11. experiment runtime; and
12. cumulative evidence validator.

Result: **12/12 PASS**.

The corrected runner hash is:

`8a20e1d852f5ec7721eee2d637d524dd404cfcf73cf8cd4a4f5b3169261c56bf`.

The corrected protocol hash is:

`a872749f649c76a60868f190a07c04bc399ebe46b8b203259c22274c0a266c75`.

## 7. Admitted cloud environment

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

No environment observation is blocked or drifting.

## 8. Corrected four-cell × two-repetition plan

The fixed campaign remains:

- GCC 13 / libstdc++ / Debug;
- GCC 13 / libstdc++ / Release;
- Clang 18 / libc++ / Debug;
- Clang 18 / libc++ / Release.

Each cell declares exactly two repetitions.

The exact semantic CTest allowlist still contains seven tests and the scientific
matrix/cases/gates are unchanged.

The corrected plan/inventory now encodes the diagnosed command cardinality.

Per cell:

- 1 configure;
- 2 build;
- 2 CTest discovery;
- 2 semantic CTest;
- 2 certificate production;
- 2 certificate validation;
- 1 negative-outcome command;
- 1 dependency-inventory command;
- 1 runtime-dependency command.

Total per cell: **14 commands**.

Across four cells:

- command records: **56**;
- command logs: **112**;
- build records: **8**;
- CTest discovery records: **8**;
- semantic CTest records: **8**;
- individual semantic test executions: **56**;
- certificate records: **8**;
- certificate-validation records: **8**;
- configure records: **4**;
- negative-outcome records: **4**;
- dependency-inventory records: **4**;
- runtime-dependency records: **4**.

The 56 command IDs are unique and every planned command has one stdout and one
stderr path.

This directly closes the evidence-cardinality defect that blocked the first
formal campaign. It does not change the scientific acceptance criteria.

## 9. Planned artifact inventory

`planned-inventories.json` contains 147 planned artifact entries:

- 12 preparation/control;
- 6 terminal-success control files;
- 1 terminal-failure file;
- 112 command logs;
- 8 semantic certificates;
- 4 compile-command inventories;
- 4 negative-outcome files.

These are plans only. None of the terminal/execution artifacts exists in the
PREPARED archive.

## 10. Audit decision

**PASS / PREPARED / NOT EXECUTED.**

The corrected package is internally sealed, revision-bound, cloud-bound,
source-inventory-consistent, and prepared with the exact corrected
four-cell/two-repetition command cardinality.

No TMR gate has been executed or decided.

## 11. Next bounded action

The current repository-resident authorization machinery remains deliberately
bound to the **consumed first** PREPARED package:

- candidate `e5eda266...`;
- preparation run `35524700979`;
- artifact `10609500629`;
- prepared-manifest hash `d8a7984a...`.

Therefore this newly audited PREPARED package must **not** receive an
`EXECUTE_ONCE` record yet.

After this preparation-audit checkpoint is merged and post-merge validation
passes, open one bounded authorization-binding work item that updates or
generalizes the authorization validator/controller/reusable executor so that
the exact audited second package can be authorized without weakening the
one-shot, fail-closed, repository-resident governance model.

Only after that binding work item is separately merged, validated, and closed
may an `EXECUTE_ONCE` record for manifest
`f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`
be introduced.

Machine-readable audit:
`docs/audits/2026-09-20-topological-model-tmr-corrected-preparation-audit.json`.
