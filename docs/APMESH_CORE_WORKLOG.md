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
- authorization-as-code integration: PR #27 merged as
  `7bf2d409556c8318db72b86ef0d85253aa0583ec`;
- authorization-as-code post-merge FAST: run `35527668634`, PASS;
- authorization-as-code post-merge INTEGRATION: run `35527668624`, PASS in
  GCC 13 Debug and Clang 18/libc++ Debug;
- exact formal execution authorization: PR #29 merged as
  `8a6eafc02d5e69f467e2badfea0b571e253b84bd`;
- formal TMR execution: run `35528077223`, process/workflow PASS, exact
  one-shot attempt consumed;
- terminal artifact: `10610497080`, SHA-256
  `b332b8dde2e8651f4dd66339875378a53c9d4390400afd0d869b54969a2bf983`;
- immutable execution claim:
  `tmr-execution-claim-d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd`;
- terminal scientific audit: **TMR0-TMR5 PASS / TMR6-TMR7 BLOCKED /
  OVERALL BLOCKED**;
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
| TMR authorization-as-code automation | MERGED | `topology/tmr-authorization-as-code`; PR #27 | partial tooling runs `35527269611` and `35527350320` retained as mechanical implementation-contract failures; corrected/final tooling runs `35527446051` and `35527563934` PASS in GCC 13 Debug and Clang 18/libc++ Debug; PR FAST `35527616244` PASS; PR INTEGRATION `35527616258` PASS; post-merge FAST `35527668634` PASS; post-merge INTEGRATION `35527668624` PASS | reusable-only executor + protected-main authorization controller integrated; no authorization JSON yet; no claim; no formal execute |
| Exact TMR execution authorization | MERGED / CONSUMED | `topology/tmr-execution-authorization`; PR #29 | TMR tooling `35527984255` PASS in GCC/Clang; PR FAST `35528019065` PASS; PR INTEGRATION `35528019067` PASS; merged authorization commit `8a6eafc02d5e69f467e2badfea0b571e253b84bd` | exact `EXECUTE_ONCE` record merged once; no second authorization permitted |
| First formal TMR execution | EXECUTED / ATTEMPT CONSUMED | protected-main run `35528077223` | authorization validation PASS; exact PREPARED binding PASS; immutable claim created; execute PASS; retention verification PASS; terminal artifact `10610497080` retained | workflow/process success is not scientific qualification |
| First formal TMR terminal audit | VALIDATED_UNMERGED / BLOCKED | `docs/tmr-terminal-audit-blocked` | terminal package independently audited; TMR0-TMR5 PASS; TMR6-TMR7 BLOCKED; 4 semantic CTest records observed vs 8 required by sealed protocol | no retry/rescue; no production-semantic contradiction shown; diagnosis only after audit checkpoint merge |
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
- `topology/tmr-authorization-as-code`: **MERGED / HISTORICAL** via
  PR #27; protected-main authorization-as-code controller plus reusable
  one-shot executor.
- `ci/tmr-authorization-as-code`: **UNUSED / ZERO-CHANGE**; accidentally
  created from the same closed checkpoint and explicitly excluded from
  continuation.
- `topology/tmr-execution-authorization`: **MERGED / HISTORICAL** via PR #29;
  contained only the exact manifest-bound `EXECUTE_ONCE` authorization.
- `docs/tmr-terminal-audit-blocked`: **MERGED / HISTORICAL** via PR #30;
  contains the independent terminal scientific audit and repository status
  synchronization for the consumed first formal TMR campaign.
- `docs/tmr-terminal-audit-checkpoint`: **MERGED / HISTORICAL** via PR #31;
  closes the audit-integration checkpoint only; no scientific/tooling change.
- `docs/tmr-terminal-audit-final-closure`: **MERGED / HISTORICAL** via PR #32;
  removed the final stale operational marker after PR #31; no scientific or
  tooling change.
- `topology/tmr-repetition-cardinality-diagnosis`: **MERGED / HISTORICAL**
  via PR #33; diagnosis-only lineage, with no runner implementation or formal
  campaign action.
- `docs/tmr-repetition-diagnosis-closure`: **CLOSURE-ONLY**; records PR #33
  integration and post-merge validation without opening a scientific work
  item.

The presence of historical branches on the remote does not make them active.

## Current active work item

**Correct TMR repetition cardinality and stable protocol validation — ACTIVE.**

Active branch: `topology/tmr-repetition-cardinality-correction`.

Authority:
`docs/decisions/TOPOLOGICAL_MODEL_TMR_REPETITION_CARDINALITY_DIAGNOSIS.md`.

Authorized scope:

1. configure each TMR cell once;
2. execute build, CTest discovery, exact semantic CTest, certificate production
   and certificate validation once in each declared repetition;
3. use cell + repetition-qualified record IDs for repetition-scoped commands;
4. retain discovery evidence with both cell and repetition identity;
5. update planned log inventories to the diagnosed 56-command cardinality;
6. strengthen focused runner contracts to prove exact per-cell/per-repetition
   cardinality and fail-closed second-repetition behavior;
7. replace the transient Section-13-heading guard with stable protocol
   invariants;
8. preserve the scientific profile, matrix, allowlist, cases, TMR0–TMR7 gates,
   production topology C++, and the consumed first-campaign evidence;
9. do not create a formal PREPARED package outside focused temporary tests;
10. do not authorize or execute a formal campaign.

## Next admissible work item after closure

After this focused mechanical correction is merged, TMR Tooling passes in both
declared tooling cells, ordinary post-merge FAST/INTEGRATION pass, and the
correction checkpoint is closed, prepare one **new** formal TMR package from a
new clean candidate.

The consumed first PREPARED package, authorization, and claim remain immutable
and cannot be reused.

