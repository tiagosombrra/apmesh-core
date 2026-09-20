# AP Mesh Core — Operational Work Ledger

Status: AUTHORITATIVE FOR OPERATIONAL CONTINUITY  
Last updated: 2026-09-20  
Canonical integration branch: `main`  
Scientific continuation authority: `docs/APMESH_CORE_STATE.md`  
Roadmap authority: `docs/APMESH_CORE_ROADMAP.md`

## Purpose

This ledger records the operational state that must survive chat/session
interruptions: active work item, branch lineage, validation, PR/merge status,
explicit non-actions, and the next admissible transition.

The repository, not conversation history, is the primary continuation memory.
A new session must reconcile this ledger with GitHub remote state before
writing.

## Mandatory transition discipline

1. Audit `main`, open PRs, relevant branches, ruleset, and recent Actions
   before starting or resuming a work item.
2. Exactly one work item may be `ACTIVE` or `VALIDATED_UNMERGED`.
3. Every relevant branch must be classified as `ACTIVE`, `VALIDATED_UNMERGED`,
   `MERGED`, `SUPERSEDED`, or `BLOCKED_RETAINED`.
4. A work item may not advance to the next scientific action until:
   - its PR is merged to `main`;
   - required post-merge checks pass;
   - `APMESH_CORE_STATE.md`, `APMESH_CORE_ROADMAP.md`, this ledger, and any
     active protocol/decision are synchronized on `main`.
5. Do not create the next implementation branch before the previous work item's
   merged checkpoint is authoritative on `main`.
6. Mechanical failures remain recorded with cause and successor. A superseded
   branch is never silently reused as active work.
7. After any UI/session/tool interruption, perform a fresh remote reconciliation
   before writing. Never reconstruct operational state from chat memory alone.
8. A formal scientific preparation/execution action is recorded separately from
   tooling implementation. Tooling validation must never be described as a
   formal campaign attempt.

## Current canonical baseline

- `main`: `503b023dd00059300c1b27e275df5e2d27534aac`;
- PR #19: merged;
- post-merge FAST: run `35516767520`, PASS;
- post-merge INTEGRATION: run `35516767491`, PASS;
- open scientific stage: **Topological Model — Explicit Identity and Incidence**;
- formal TMR manifest: **none**;
- formal TMR execution: **none**;
- Topological Model qualification: **not qualified**.

## Work-item ledger

| Work item | Operational status | Branch / PR | Validation / evidence | Explicit boundary |
| --- | --- | --- | --- | --- |
| TMR report-only tooling | MERGED | `topology/tmr-report-only-workflow` / PR #16 | focused run `35514834796` PASS | no formal manifest, no TMR execution |
| Cloud environment supplement | MERGED | `docs/tmr-cloud-environment-supplement` / PR #17 | FAST/INTEGRATION merged checkpoint PASS | protocol/documentation only |
| Fail-closed cloud identity binding | MERGED | `topology/tmr-cloud-identity-binding` / PR #18 | run `35516246789` PASS; `35516204233` retained mechanical protocol-guard failure | no formal manifest, no TMR execution |
| Exact admitted cloud tool paths | MERGED | `topology/tmr-exact-tool-path-plan` / PR #19 | run `35516578411` PASS; post-merge FAST/INTEGRATION PASS | no formal manifest, no TMR execution |
| Preparation-only manual workflow | VALIDATED_UNMERGED | technical lineage `topology/tmr-preparation-only-workflow` -> `topology/tmr-preparation-only-workflow-v2`; continuation branch `ci/tmr-preparation-workflow-simplification` | `35516864464` and `35516972035` retained mechanical quoting failures; corrected `35517077819` PASS in GCC/Clang focused/static contracts | workflow itself has never been dispatched; no PREPARED package exists |
| TMR tooling-contract correction | SUPERSEDED | `topology/tmr-tooling-contract-correction` | historical focused run `35515277674` PASS | superseded by later integrated tooling lineage |

## Relevant branch classification

- `ci/tmr-preparation-workflow-simplification`: **ACTIVE /
  VALIDATED_UNMERGED**; contains the validated preparation-only workflow plus
  synchronized continuation documentation.
- `topology/tmr-preparation-only-workflow-v2`: **SUPERSEDED BY ACTIVE
  CONTINUATION BRANCH**; validated technical ancestor, run `35517077819`.
- `topology/tmr-preparation-only-workflow`: **BLOCKED_RETAINED /
  SUPERSEDED**; retains mechanical quoting failures.
- `topology/tmr-exact-tool-path-plan`: **MERGED / HISTORICAL** via PR #19.
- `topology/tmr-cloud-identity-binding`: **MERGED / HISTORICAL** via PR #18.
- `docs/tmr-cloud-environment-supplement`: **MERGED / HISTORICAL** via PR #17.
- `topology/tmr-report-only-workflow`: **MERGED / HISTORICAL** via PR #16.
- `topology/tmr-tooling-contract-correction`: **SUPERSEDED / HISTORICAL**.

The presence of historical branches on the remote does not make them active.

## Current active work item

**Integrate the preparation-only manual workflow and synchronized continuation
documentation.**

Acceptance before this work item can close:

1. open a PR from `ci/tmr-preparation-workflow-simplification` to `main`;
2. required FAST and both INTEGRATION checks pass;
3. merge using the repository-allowed linear method;
4. post-merge FAST and INTEGRATION pass on the merged revision;
5. verify this ledger and `APMESH_CORE_STATE.md` on `main` identify the
   preparation workflow as integrated but **not dispatched**.

Only after all five conditions pass may the next work item become active.

## Next admissible work item after closure

One explicit manual dispatch of `Topological Model TMR Preparation` from
canonical `main`.

That future action may create one sealed PREPARED package with
`execution_requested=false` and all TMR0-TMR7 gates `NOT_EXECUTED`. It must
then stop for audit. Formal `execute` remains unauthorized.
