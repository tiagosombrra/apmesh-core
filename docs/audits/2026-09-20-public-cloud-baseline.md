# Public/Cloud Baseline Repository Audit — 2026-09-20

Status: PASS / ENGINEERING BASELINE ONLY  
Candidate revision: `13c347f87ddba48366ff4131e3cdf9e096bd3c7e`  
Pull request: #9  
Scientific stage: Topological Model — Explicit Identity and Incidence

## Scope

This audit verifies the repository transition to public visibility and
GitHub-hosted development regression. It does not execute TMR0--TMR7, prepare a
TMR manifest, qualify the Topological Model stage, or authorize new scientific
production scope.

Work-class distribution for this audit:

- Implementation: **0%**
- Tests/validation: **55%**
- Evidence/experiments: **15%**
- Documentation/governance: **30%**

These percentages describe the audit work distribution, not scientific stage
completion.

## Repository and continuity audit

PASS:

- repository `tiagosombrra/apmesh-core` is public;
- `main` remains the canonical integration branch;
- pre-audit `main` baseline is
  `b6e4459b4de0b8376eaf4c41848949f7e69fd312`;
- the repository contains an authoritative continuation state and roadmap;
- the scientific workflow now requires repository checkpoints, progress
  accounting, and major-boundary audit/regression;
- no production C++ was changed by this audit;
- no TMR manifest or qualification execution was created;
- current Topological Model limitations remain unchanged.

Governance finding at baseline time:

- the repository initially exposed no GitHub repository rulesets. This did not
  invalidate the engineering regression but was retained as a governance gap.

Resolution on 2026-09-20:

- repository ruleset `23728711` (`main-protection`) is active and targets the
  default branch;
- bypass list is empty and the authenticated owner cannot bypass the ruleset;
- deletion and non-fast-forward updates are blocked;
- linear history is required;
- pull requests are required, with squash/rebase as the only allowed merge
  methods, zero required approvals, and resolved review conversations;
- required status check is exactly `GCC 13 Debug / FAST` from GitHub Actions;
- strict required-status semantics require the PR branch to be current with the
  target branch before merge;
- GitHub reports the public-preview flag
  `require_extra_approval_for_unattributed_changes=true`; because the ruleset
  requires zero approvals, GitHub documents that this flag has no effect on the
  current workflow.

## CI baseline

Existing FAST workflow:

- `main` run `35510340606`: **PASS**;
- GitHub-hosted Ubuntu 24.04;
- GCC 13 Debug;
- configure, build, and FAST contracts all passed.

Audit candidate FAST workflow:

- run `35510690845`: **PASS**.

Major Semantic Regression:

- run `35510690848`: **PASS**;
- GitHub-hosted Ubuntu 24.04;
- CMake 3.31.6;
- GCC 13.3.0 with libstdc++, Debug and Release;
- Clang 18.1.3 with libc++ 18.1.3, Debug and Release;
- qualification tooling remained disabled;
- every cell configured and built successfully;
- every cell executed exactly the seven current direct/focused semantic tests;
- every cell reported **7/7 PASS**.

The seven tests were:

1. `apmesh_core.bootstrap_smoke`;
2. `apmesh_core.numeric_contract`;
3. `apmesh_core.geometry_primitives`;
4. `apmesh_core.cartesian_frames`;
5. `apmesh_core.topological_model`;
6. `apmesh_core.minimal_small_linear_algebra`;
7. `apmesh_core.math_header_isolation`.

## Regression classification

- Current direct/focused semantic behavior: `NO_CHANGE`.
- GCC Debug/Release portability in the declared Ubuntu 24.04 cloud runner:
  `PASS`.
- Clang/libc++ Debug/Release portability in the declared Ubuntu 24.04 cloud
  runner: `PASS`.
- Previously qualified Foundation/Geometry claims: no contradiction detected by
  the current semantic regression.
- Topological Model qualification: unchanged, still `UNQUALIFIED`.
- TMR0--TMR7: unchanged, still `PRE-REGISTERED / NOT PREPARED / NOT EXECUTED`.

This engineering regression is intentionally weaker than TMR0--TMR7. It
provides a reusable major-boundary semantic safety net and does not replace the
stage qualification protocol.

## Current progress

Topological Model scientific completion lanes:

| Lane | Completion |
| --- | ---: |
| Production implementation | 100% |
| Focused validation | 100% |
| Stage-regression / qualification tooling | 0% |
| Formal evidence execution | 0% |
| Closure audit / documentation | 0% |

Cloud execution infrastructure:

| Capability | Completion |
| --- | ---: |
| FAST | 100% |
| Major Semantic Regression | 100% |
| INTEGRATION | 0% |
| QUALIFICATION environment | 0% |

## Decision

The public/cloud repository baseline is accepted for engineering continuation.
No scientific stage is newly qualified by this audit.

After this audit is integrated into `main`, the next infrastructure action is
to add the compact INTEGRATION GitHub Actions profile (GCC 13 Debug and Clang
18/libc++ Debug). The next scientific Topological Model action remains the
smallest reusable report-only TMR0--TMR7 workflow. Neither action may declare
the stage qualified without the pre-registered TMR campaign and terminal audit.
