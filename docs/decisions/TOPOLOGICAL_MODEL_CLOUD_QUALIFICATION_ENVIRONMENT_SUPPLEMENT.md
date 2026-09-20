# Topological Model — Cloud Qualification Environment Supplement

Status: ACCEPTED / PROTOCOL SCOPE ONLY / NO MANIFEST / NOT EXECUTED  
Date: 2026-09-20  
Stage: Topological Model — Explicit Identity and Incidence

## 1. Purpose

This supplement resolves the execution-environment boundary left open by
`docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md`.

It authorizes a future formal TMR0–TMR7 campaign to make the same bounded
Topological Model claim in the already admitted GitHub-hosted Ubuntu 24.04
qualification envelope. It does not reclassify historical WSL evidence, assert
WSL/cloud equivalence, prepare a TMR manifest, execute the campaign, or qualify
the stage.

## 2. Authorities

This supplement is subordinate to and must be read with:

- `docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md`;
- `docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md`;
- `docs/audits/2026-09-20-cloud-qualification-environment-admission.md`;
- `docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md`; and
- `docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md`.

The TMR scientific claim, fixed matrix, two repetitions per cell, seven-test
semantic allowlist, case index, TMR0–TMR7 gates, failure policy, and retained
outputs are unchanged.

## 3. Admitted cloud envelope

A formal cloud TMR campaign is admissible only when every field below matches
the already admitted environment exactly:

| Component | Required identity |
| --- | --- |
| GitHub runner label | `ubuntu-24.04` |
| Runner image OS | `ubuntu24` |
| Runner image version | `20260907.300.1` |
| Ubuntu release family | 24.04 |
| Architecture | x86_64 |
| GCC | 13.3.0 |
| `g++-13` package | `13.3.0-6ubuntu2~24.04.1` |
| Clang | 18.1.3 |
| `clang-18` package | `1:18.1.3-1ubuntu1` |
| `libc++-18-dev` | `1:18.1.3-1ubuntu1` |
| `libc++abi-18-dev` | `1:18.1.3-1ubuntu1` |
| CMake | `/usr/bin/cmake` 3.28.3 |
| CMake package | `3.28.3-1build7` |
| Ninja | `/usr/bin/ninja` 1.11.1 |
| Ninja package | `1.11.1-2` |

The formal TMR matrix remains exactly:

- GCC 13/libstdc++ Debug, two repetitions;
- GCC 13/libstdc++ Release, two repetitions;
- Clang 18/libc++ Debug, two repetitions; and
- Clang 18/libc++ Release, two repetitions.

## 4. Fail-closed environment rule

The cloud environment is part of the scientific input. A formal TMR preparation
or execution must fail closed when any required identity field differs.

A hosted-runner image update, package update, tool-path change, compiler/library
change, or architecture change is not an admissible automatic substitution. It
requires a new bounded environment review and an explicit recorded update before
another formal campaign can be prepared.

Kernel build, Azure host identity, worker identity, region, process IDs, and
timestamps remain provenance fields. They do not establish equivalence to WSL
and are not acceptance substitutes for the fixed environment identity above.

## 5. Claim scope after a future PASS

If and only if a later revision-bound cloud TMR campaign satisfies TMR0–TMR7
under the exact envelope in Section 3, the permitted result is:

**Topological Model is QUALIFIED within the declared GitHub-hosted Ubuntu 24.04
x86_64 cloud envelope identified by this supplement.**

That result would be additional environment-scoped evidence. It would not:

- convert historical WSL evidence into cloud evidence;
- claim bitwise or scientific equivalence between WSL and GitHub-hosted VMs;
- extend qualification to a different runner image or package set;
- establish portability to native Windows or other operating systems;
- authorize adjacency, pairing, boundary/manifold classification, geometry,
  curves, surfaces, discretization, meshing, parallel execution, or performance
  claims; or
- weaken any TMR0–TMR7 gate or prerequisite-preservation requirement.

## 6. Preparation binding required

Before a formal cloud manifest can become `PREPARED`, the TMR preparation path
must bind and retain, by declared identity and hash where applicable:

1. this supplement;
2. the Cloud Qualification Environment admission decision and audit;
3. the exact runner image identity and package versions in Section 3;
4. the exact `/usr/bin/cmake` and `/usr/bin/ninja` paths and versions;
5. the unchanged TMR matrix, repetitions, semantic allowlist, case index, gates,
   authorities, limitations, and evidence plan; and
6. the clean published candidate revision and tracked-source inventory.

The TMR preparation/runner path now binds this supplement, the cloud admission
decision and audit, the machine-readable cloud profile/validator, and exact
observations for all four declared cells. The binding fails closed on drift.
Focused validation passed in GitHub Actions run `35516246789`. The launch
plan additionally seals `/usr/bin/cmake`, `/usr/bin/ctest`,
`/usr/bin/ninja`, `/usr/bin/g++-13`, and `/usr/bin/clang++-18` rather
than relying on PATH resolution; focused validation passed in run
`35516578411`. No manifest whose environment or planned-tool binding is
weaker than Section 3 is authorized.

## 7. Decision effect

This environment supplement is accepted as protocol authority for future cloud
TMR work.

No TMR manifest is prepared by this change. No TMR command is executed. No
TMR0–TMR7 gate is decided. No production C++ changes. Topological Model remains
unqualified.

The fail-closed cloud identity binding and exact planned tool paths are
implemented. The manual preparation-only GitHub Actions path is also
implemented and statically constrained to canonical-`main` manual dispatch,
a new external runner-temp output root, PREPARED-only state, pinned artifact
retention, and no formal execution step. Runs `35516864464` and
`35516972035` are retained as consecutive mechanical quoting failures; the
required tooling stop and simplification preceded the successful focused run
`35517077819`.

PR #20 integrated the preparation-only workflow as
`d9297ffad4f503b4ea11b056885749fff5872201`; post-merge FAST run
`35519501704` and INTEGRATION run `35519501663` passed.

Formal preparation run `35524700979` then produced artifact `10609500629`
for candidate `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`. The retained package
records the exact admitted cloud identity in all four cells and passed the
independent preparation audit in
`docs/audits/2026-09-20-topological-model-tmr-preparation-audit.md`.
It remains unconsumed and no TMR gate has been executed or decided.

A later execution workflow must restore and consume this exact PREPARED package;
environment, candidate, input, path, or plan drift remains fail-closed.
