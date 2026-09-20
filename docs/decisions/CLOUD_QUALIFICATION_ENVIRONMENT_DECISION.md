# Cloud Qualification Environment — Admission Decision

Status: ADMITTED / CQE0-CQE7 PASS / DISTINCT FROM WSL  
Date: 2026-09-20  
Repository baseline: `6f2827bf180fce19f39be8ae44d72f8d74a473b2`

## Question

Can GitHub-hosted Ubuntu 24.04 provide a bounded, explicitly identified cloud
environment suitable for future AP Mesh scientific qualification campaigns
without silently treating it as equivalent to the previously qualified WSL
Ubuntu 24.04 environment?

## Motivation

GitHub-hosted runner labels such as `ubuntu-24.04` are maintained images rather
than immutable scientific environments. GitHub documents that the installed
software on hosted images is updated regularly, so the label alone is
insufficient provenance for a qualification claim.

The cloud transition therefore requires a fail-closed environment identity
contract before any TMR manifest is prepared or any scientific qualification
campaign is executed.

## Historical WSL reference

The accepted historical qualification envelope used:

| Component | Historical value |
| --- | --- |
| OS | WSL Ubuntu 24.04 |
| GCC | 13.3.0 |
| Standard library | libstdc++ |
| Clang | 18.1.3 |
| libc++ | 18.1.3 |
| CMake | 3.28.3 |
| Ninja | 1.11.1 |

Historical scientific evidence remains valid only inside its declared WSL
envelope unless a later decision explicitly extends or supersedes that scope.

## Cloud candidate identity

The first bounded cloud candidate is:

| Component | Required value |
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

The explicit `/usr/bin` CMake and Ninja tools are used because the hosted
runner also provides newer preinstalled toolcache versions. The qualification
candidate intentionally aligns these two build tools with the historical WSL
baseline instead of accepting whichever tool happens to appear first on
`PATH`.

## Environment drift policy

The cloud qualification environment fails closed when any required identity
field changes, including the GitHub runner image version or a declared package
version.

A runner-image or package update is not automatically accepted. It requires a
new bounded environment review and a new recorded profile revision. No workflow
may silently update the expected identity to make a failed probe pass.

Kernel version, Azure host identity, worker ID, region, process IDs, and
timestamps are recorded provenance but are not used to claim WSL equivalence.

## Validation design

The admission workflow must:

1. execute only on GitHub-hosted `ubuntu-24.04`;
2. install the declared Ubuntu packages at the exact recorded versions;
3. validate runner image identity, Ubuntu family, architecture, tool paths,
   compiler versions, and package versions;
4. configure and build the current candidate in the four declared cells:
   GCC Debug/Release and Clang/libc++ Debug/Release;
5. keep `APMESH_ENABLE_QUALIFICATION_TESTS=OFF`;
6. prove that each cell discovers exactly the current seven-test semantic
   allowlist;
7. execute those seven tests once in every cell;
8. retain one environment observation and CTest log per cell as workflow
   artifacts; and
9. make no TMR0–TMR7 or stage-qualification decision.

The four-cell semantic execution is environment admission evidence only. It is
not a substitute for the pre-registered Topological Model qualification
protocol.

## Admission gates

| Gate | PASS condition |
| --- | --- |
| CQE0 — Identity | Runner label/image, Ubuntu family, architecture and declared package/tool versions match exactly. |
| CQE1 — Build tools | The workflow uses `/usr/bin/cmake` 3.28.3 and `/usr/bin/ninja` 1.11.1, not mutable hosted-toolcache versions. |
| CQE2 — Four-cell build | GCC/Clang Debug/Release configure and build successfully with the current project presets. |
| CQE3 — Semantic inventory | Every cell discovers exactly the seven current direct/focused semantic tests and no qualification tooling. |
| CQE4 — Semantic execution | The seven tests pass once in every cell. |
| CQE5 — Evidence | Per-cell environment JSON and CTest logs are retained by the workflow. |
| CQE6 — Scope | No production C++, scientific claim, TMR manifest, or formal qualification execution changes. |
| CQE7 — Transition decision | PASS admits a new bounded cloud qualification environment candidate only; WSL equivalence remains explicitly unclaimed. |

Overall admission requires CQE0–CQE7.

## Non-claims

A PASS does not establish:

- bitwise identity of the GitHub-hosted VM and WSL;
- identical kernel, virtualization, CPU scheduling, or host behavior;
- portability beyond the declared x86_64 Ubuntu 24.04 cloud candidate;
- equivalence of historical evidence packages to newly generated cloud
  evidence; or
- Topological Model qualification.

## Execution and decision

The first admission run, `35512991310`, was retained as
`BLOCKED_BY_CMAKE_CACHE_TYPE_ASSERTION`: environment identity and configure
passed, but a workflow assertion required the wrong CMake cache type for the
already correct `/usr/bin/ninja` value. One focused mechanical correction was
made without changing environment identity, production C++, or scientific
acceptance.

The corrected run, `35513051098`, passed all four cells on functional candidate
`952695f0456f095e4f7204d34a7652738dbd75da`. Each cell matched the declared
runner image and package/tool identities, discovered exactly seven semantic
tests, retained qualification tooling as OFF, and completed 7/7 semantic tests.

CQE0–CQE7 are therefore **PASS**. The cloud environment is admitted as a
distinct bounded qualification envelope candidate. It is not declared
scientifically equivalent to WSL.

Full retained audit:
`docs/audits/2026-09-20-cloud-qualification-environment-admission.md`.



## Final closure checkpoint

The final PR #13 head,
`6a934de6e8f6fae35e6c38ec45b9b1f23b170acb`, passed Qualification Environment
run `35513250315` and Major Semantic Regression run `35513567930`.
PR #13 was then squash-merged as
`0a7095d431e4bea3c9c73e75d22df2e713c7a8ab`.

The final reviewed candidate tree and merged tree are identical:

`7144943abc7ffd861b92587217a112c0edf6f9b4`.

Post-merge FAST run `35513658207` and INTEGRATION run `35513658197` both
passed. The environment-admission phase is therefore closed. No additional
cloud-infrastructure work is required before implementing the report-only TMR
workflow.

## Next action

The next bounded scientific implementation action is the smallest reusable
report-only TMR0–TMR7 workflow. Before any formal TMR cloud execution, the
Topological Model qualification protocol must explicitly name this admitted
cloud envelope; no environment transition may be inferred silently.
