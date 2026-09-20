# Topological Model TMR — PREPARED Package Audit

Status: **PASS / PREPARED / NOT EXECUTED**  
Audit date: 2026-09-20  
Preparation workflow run: `35524700979`  
Candidate: `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`  
Artifact ID: `10609500629`

## 1. Scope

This audit validates only the first formal cloud TMR **PREPARED** package.
It does not execute the TMR0–TMR7 campaign, decide any scientific gate, or
qualify the Topological Model stage.

Authority:

- `docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md`;
- `docs/decisions/TOPOLOGICAL_MODEL_CLOUD_QUALIFICATION_ENVIRONMENT_SUPPLEMENT.md`;
- `docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md`;
- `docs/audits/2026-09-20-cloud-qualification-environment-admission.md`.

## 2. Preparation run

The workflow `Topological Model TMR Preparation` was dispatched exactly once
on canonical `main`.

Observed run:

- run: `35524700979`;
- event: `workflow_dispatch`;
- branch: `main`;
- head SHA: `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`;
- conclusion: `success`;
- job: `Prepare sealed TMR manifest`;
- checkout, canonical-main guard, admitted toolchain installation, preparation,
  validation, and artifact retention all completed successfully.

No `execute` command was present in the workflow.

## 3. Retained PREPARED artifact

GitHub retained one artifact:

- name:
  `tmr-prepared-e5eda2663d6ff4b93ce1205660ff04d432acb9c0`;
- artifact ID: `10609500629`;
- size: `112837` bytes;
- GitHub digest:
  `sha256:2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72`;
- expiration: 2026-12-19.

The independently downloaded ZIP has the same SHA-256:
`2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72`.

The package contains exactly seven files:

1. `prepared-manifest.json`;
2. `preparation-seal.json`;
3. `plan.json`;
4. `planned-inventories.json`;
5. `profile.json`;
6. `state.json`;
7. `state-history.jsonl`.

No `execution-claim.json`, `terminal-manifest.json`, or
`command-records.json` exists.

## 4. Lifecycle and seal integrity

The package records:

- manifest state: `PREPARED`;
- `execution_requested=false`;
- TMR0–TMR7: all `NOT_EXECUTED`;
- candidate commit:
  `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`;
- candidate upstream commit: identical to candidate;
- candidate tracked tree: clean;
- tracked-source inventory: 1534 files;
- planned retained artifacts: 123.

Important retained hashes:

- `prepared-manifest.json`:
  `d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd`;
- `preparation-seal.json`:
  `982e1441f08bc3f11c3cffcf73113ce69a07e2066924ad01442b3ab94eeeb71e`;
- `state-history.jsonl`:
  `a41d22e2efb30c2e0200929f5f031ffe267eeaec94b8ae5399e063822921dc98`.

Every hash referenced by `preparation-seal.json` recomputes exactly. The
initial and only state-history record is identical to `state.json`, and its
prepared-manifest hash matches the retained manifest.

## 5. Candidate and input binding

The PREPARED manifest binds the exact candidate and twelve critical inputs:
profile, protocol, exporter, validator, runner, runtime, CMake configuration,
cloud profile, cloud validator, cloud supplement, cloud admission decision,
and cloud admission audit.

All twelve SHA-256 values were independently recomputed from the files stored
at commit `e5eda2663d6ff4b93ce1205660ff04d432acb9c0` on GitHub. All twelve
match the sealed manifest exactly.

Therefore the PREPARED package is bound to the published candidate rather than
to mutable branch state.

## 6. Cloud environment binding

All four declared cloud observations are `PASS`:

- `gcc-debug`;
- `gcc-release`;
- `clang-debug`;
- `clang-release`.

Each observation records the admitted envelope:

- runner label: `ubuntu-24.04`;
- image OS: `ubuntu24`;
- image version: `20260907.300.1`;
- architecture: `x86_64`;
- GCC: 13.3.0, package `13.3.0-6ubuntu2~24.04.1`;
- Clang: 18.1.3, package `1:18.1.3-1ubuntu1`;
- libc++ / libc++abi: `1:18.1.3-1ubuntu1`;
- CMake: 3.28.3, package `3.28.3-1build7`;
- Ninja: 1.11.1, package `1.11.1-2`.

The manifest additionally records executable identities and hashes for Python,
CMake, CTest, Ninja, GCC, Clang, and `ldd`.

## 7. Fixed campaign plan

The sealed launch plan contains exactly four cells and two repetitions per
cell:

| Cell | Compiler | Library | Build type | Repetitions |
| --- | --- | --- | --- | ---: |
| `gcc-debug` | `/usr/bin/g++-13` | libstdc++ | Debug | 2 |
| `gcc-release` | `/usr/bin/g++-13` | libstdc++ | Release | 2 |
| `clang-debug` | `/usr/bin/clang++-18` | libc++ | Debug | 2 |
| `clang-release` | `/usr/bin/clang++-18` | libc++ | Release | 2 |

Every cell seals:

- `/usr/bin/cmake`;
- `/usr/bin/ctest`;
- `/usr/bin/ninja`;
- the exact admitted compiler path;
- `-DCMAKE_MAKE_PROGRAM=/usr/bin/ninja`.

The semantic allowlist contains the exact seven required tests exactly once.
The order stored in the manifest follows CMake declaration order while the
profile lists the same seven names in protocol order. This is not a contract
difference: the runner explicitly validates sorted equality and uniqueness,
and Section 7 of the protocol requires exact membership/execution, not a
normative ordering.

## 8. Execution binding consequence

The current runner's `validate_execution_binding` requires a future execution
to reproduce the exact PREPARED binding. In particular it must:

- check out candidate
  `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`;
- use the same admitted cloud identity;
- restore the retained PREPARED package to its sealed output root
  `/home/runner/work/_temp/apmesh-tmr-prepared`;
- use the repository workspace
  `/home/runner/work/apmesh-core/apmesh-core`;
- reproduce the exact twelve input hashes, allowlist, plan, and inventories;
- consume the PREPARED lifecycle exactly once.

Any candidate, path, environment, input, or plan drift fails closed.

## 9. Audit decision

**PREPARATION AUDIT PASS.**

The first formal TMR package is a valid, sealed, unconsumed PREPARED package for
candidate `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`.

This decision does **not**:

- execute the TMR campaign;
- decide TMR0–TMR7;
- create a terminal scientific result;
- qualify the Topological Model stage;
- authorize automatic execution from branch updates.

The next bounded work is to implement the smallest manual execution-only GitHub
Actions path that restores this exact artifact, checks out the exact candidate,
revalidates the PREPARED binding, and invokes `execute` at most once. That
workflow must be integrated and statically/focused validated **without being
dispatched in the same change**.
