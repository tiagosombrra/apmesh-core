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
9. Do not store a self-referential "current main SHA" in this ledger. The live
   `main` revision is always obtained from the remote audit. Persist only stable
   anchors such as closed work-item merge revisions, PRs, and completed runs.

## Last closed work-item anchor

- preparation-workflow integration merge: `d9297ffad4f503b4ea11b056885749fff5872201`, PR #20;
- post-merge FAST: run `35519501704`, PASS;
- post-merge INTEGRATION: run `35519501663`, PASS in GCC 13 Debug and Clang 18/libc++ Debug;
- operational closure checkpoint: PR #21 merged; its live revision is obtained
  from the remote audit rather than embedded here;
- open scientific stage: **Topological Model — Explicit Identity and Incidence**;
- first formal TMR PREPARED package: run `35524700979`, candidate
  `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`, artifact `10609500629`;
- preparation audit: **PASS**, recorded in
  `docs/audits/2026-09-20-topological-model-tmr-preparation-audit.md`;
- formal TMR execution: **none**;
- Topological Model qualification: **not qualified**.

## Work-item ledger

| Work item | Operational status | Branch / PR | Validation / evidence | Explicit boundary |
| --- | --- | --- | --- | --- |
| TMR report-only tooling | MERGED | `topology/tmr-report-only-workflow` / PR #16 | focused run `35514834796` PASS | no formal manifest, no TMR execution |
| Cloud environment supplement | MERGED | `docs/tmr-cloud-environment-supplement` / PR #17 | FAST/INTEGRATION merged checkpoint PASS | protocol/documentation only |
| Fail-closed cloud identity binding | MERGED | `topology/tmr-cloud-identity-binding` / PR #18 | run `35516246789` PASS; `35516204233` retained mechanical protocol-guard failure | no formal manifest, no TMR execution |
| Exact admitted cloud tool paths | MERGED | `topology/tmr-exact-tool-path-plan` / PR #19 | run `35516578411` PASS; post-merge FAST/INTEGRATION PASS | no formal manifest, no TMR execution |
| Preparation-only manual workflow | MERGED | technical lineage `topology/tmr-preparation-only-workflow` -> `topology/tmr-preparation-only-workflow-v2`; continuation branch `ci/tmr-preparation-workflow-simplification`; PR #20 | `35516864464` and `35516972035` retained mechanical quoting failures; corrected `35517077819` PASS; post-merge FAST `35519501704` PASS and INTEGRATION `35519501663` PASS | workflow integrated; formal dispatch recorded separately below |
| First formal TMR preparation and package audit | MERGED | workflow run `35524700979`; audit branch `docs/tmr-prepared-package-audit`; PR #23 | run PASS; artifact `10609500629`; GitHub/archive SHA-256 `2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72`; preparation audit PASS; post-merge FAST `35525181361` PASS and INTEGRATION `35525181462` PASS | PREPARED only; `execution_requested=false`; TMR0-TMR7 `NOT_EXECUTED`; no execute |
| One-shot manual TMR execution workflow | MERGED | `topology/tmr-execution-only-workflow`; PR #25 | focused/static runs `35525736120` and `35525847660` PASS in GCC 13 Debug and Clang 18/libc++ Debug; post-merge FAST `35525932108` PASS and INTEGRATION `35525932111` PASS; local Git upstream reconstruction contract verified | manual-only; exact candidate/artifact; preflight -> immutable claim tag -> one execute; no formal dispatch yet |
| TMR authorization-as-code automation | VALIDATED_UNMERGED | `topology/tmr-authorization-as-code` | partial tooling runs `35527269611` and `35527350320` retained as mechanical implementation-contract failures; corrected run `35527446051` PASS in GCC 13 Debug and Clang 18/libc++ Debug across evidence, runner, preparation, reusable executor, authorization record, and authorization controller contracts | no authorization JSON yet; no claim; no formal execute |
| TMR tooling-contract correction | SUPERSEDED | `topology/tmr-tooling-contract-correction` | historical focused run `35515277674` PASS | superseded by later integrated tooling lineage |

## Relevant branch classification

- `ci/tmr-preparation-workflow-simplification`: **MERGED / HISTORICAL** via
  PR #20; contains the integrated preparation-only workflow lineage and the
  first operational-continuity checkpoint.
- `topology/tmr-preparation-only-workflow-v2`: **SUPERSEDED / HISTORICAL
  TECHNICAL ANCESTOR**; validated in run `35517077819` and integrated through
  the PR #20 continuation lineage.
- `topology/tmr-preparation-only-workflow`: **BLOCKED_RETAINED /
  SUPERSEDED**; retains mechanical quoting failures.
- `topology/tmr-exact-tool-path-plan`: **MERGED / HISTORICAL** via PR #19.
- `topology/tmr-cloud-identity-binding`: **MERGED / HISTORICAL** via PR #18.
- `docs/tmr-cloud-environment-supplement`: **MERGED / HISTORICAL** via PR #17.
- `topology/tmr-report-only-workflow`: **MERGED / HISTORICAL** via PR #16.
- `topology/tmr-tooling-contract-correction`: **SUPERSEDED / HISTORICAL**.
- `topology/tmr-execution-only-workflow`: **MERGED / HISTORICAL** via
  PR #25; one-shot manual execution wrapper for the audited PREPARED package.
- `topology/tmr-authorization-as-code`: **ACTIVE / VALIDATED_UNMERGED**;
  replaces the standalone execution click with a protected-main
  authorization record plus reusable executor.
- `ci/tmr-authorization-as-code`: **UNUSED / ZERO-CHANGE**; accidentally
  created from the same closed checkpoint and explicitly excluded from
  continuation.

The presence of historical branches on the remote does not make them active.

## Current active work item

**Replace the standalone manual TMR execution click with repository-resident
authorization-as-code — VALIDATED_UNMERGED.**

Validated properties:

1. the audited PREPARED identity remains fixed to candidate
   `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`, preparation run
   `35524700979`, artifact `10609500629`, prepared-manifest SHA-256
   `d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd`,
   and preparation-seal SHA-256
   `982e1441f08bc3f11c3cffcf73113ce69a07e2066924ad01442b3ab94eeeb71e`;
2. the executor is reusable-only through `workflow_call`; it exposes no
   `workflow_dispatch`, push, PR, schedule, or workflow-run trigger;
3. the authorization controller triggers only when the exact manifest-bound
   JSON path is added to protected `main`;
4. the controller requires that record to appear exactly once as a newly added
   file, validates an exact closed schema, and rejects a pre-existing claim;
5. the reusable executor independently checks the caller commit, revalidates
   the committed authorization, then checks out the exact historical candidate;
6. the original full PREPARED binding, immutable claim tag, single `execute`,
   no-retry rule, retention verification, and terminal artifact remain intact;
7. no actual `EXECUTE_ONCE` authorization JSON is part of this branch;
8. no execution claim exists and no formal TMR execution has occurred;
9. tooling run `35527446051` passed in both GCC 13 Debug and Clang 18/libc++
   Debug across all six focused TMR tooling contracts.

Retained implementation-only failures:

- run `35527269611`: the old static contract still required
  `workflow_dispatch` after the executor had been converted to
  `workflow_call`;
- run `35527350320`: JavaScript file-writing absorbed shell continuation
  backslashes, leaving valid but flattened shell and a mismatched static
  contract;
- both failures occurred before any scientific authorization, claim, or
  `execute`; the corrected structured workflow passed in `35527446051`.

Active implementation branch:
`topology/tmr-authorization-as-code`.

The zero-change branch `ci/tmr-authorization-as-code` remains explicitly
non-active.

## Next admissible work item after closure

After this authorization-as-code mechanism is merged, post-merge
FAST/INTEGRATION pass, the focused/static contracts remain green, and the work
item is closed, create one **separate** exact `EXECUTE_ONCE` authorization
record by pull request.

Merging that single record to protected `main` is the formal execution
authorization event. The authorization controller will then call the reusable
one-shot executor automatically. The resulting terminal package must be audited
before deciding TMR0-TMR7 or any Topological Model qualification. No second
authorization file, retry, rescue dispatch, or manual execution path is
authorized.

