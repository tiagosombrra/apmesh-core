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
| First formal TMR terminal audit | MERGED / BLOCKED | `docs/tmr-terminal-audit-blocked`; PR #30 | terminal package independently audited; TMR0-TMR5 PASS; TMR6-TMR7 BLOCKED; 4 semantic CTest records observed vs 8 required by sealed protocol | no retry/rescue; no production-semantic contradiction shown |
| TMR repetition-cardinality diagnosis | MERGED | `topology/tmr-repetition-cardinality-diagnosis`; PR #33 | diagnosis established 56-command future execution shape and stable protocol-guard requirement | diagnosis only; no campaign |
| TMR repetition-cardinality correction | MERGED | `topology/tmr-repetition-cardinality-correction`; PR #35 | final TMR Tooling `35530579354` PASS; PR FAST `35530643533`; PR INTEGRATION `35530643582`; post-merge FAST/INTEGRATION PASS | tooling-only; no production C++; no campaign |
| Corrected formal TMR preparation and audit | VALIDATED_UNMERGED / PREPARED | workflow run `35531261000`; branch `docs/tmr-corrected-preparation-audit` | artifact `10611054028`; ZIP SHA-256 `96a47fcecc524e0a4baccee899bd88be8443dbc9778a55271a8376ebe2f6a1ab`; manifest `f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`; audit PASS | PREPARED only; 56-command plan; no claim/execute; authorization binding still points to first package |
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
- `topology/tmr-repetition-cardinality-correction`: **MERGED / HISTORICAL**
  via PR #35; implements the diagnosed 56-command repetition shape and stable
  protocol guard.
- `docs/tmr-repetition-correction-closure`: **MERGED / HISTORICAL** via
  PR #36; closes the corrected-runner checkpoint and authorizes a new formal
  preparation.
- `docs/tmr-corrected-preparation-audit`: **MERGED / HISTORICAL** via PR #37;
  independent audit of the second PREPARED package only.
- `docs/tmr-corrected-preparation-audit-closure`: **CLOSURE-ONLY**; records
  PR #37 integration and post-merge validation.

The presence of historical branches on the remote does not make them active.

## Current active work item

**Generalize the repository-resident TMR authorization binding — ACTIVE.**

Active branch: `topology/tmr-generic-authorization-binding`.

Authorized scope:

1. preserve authorization-as-code as the explicit human authorization event;
2. allow a newly added manifest-hash-named authorization JSON to carry the
   exact audited PREPARED identity instead of recompiling hardcoded campaign
   constants into the validator/controller/executor;
3. keep a closed authorization schema and exact filename =
   prepared-manifest SHA-256;
4. require the referenced integrated preparation audit to exist and contain
   the exact candidate/run/artifact/manifest/seal identity plus
   `PASS / PREPARED / NOT EXECUTED`;
5. make the controller discover exactly one newly added authorization record
   on protected `main` and reject modification/replacement/multiple records;
6. derive the immutable claim tag from the prepared-manifest hash;
7. make the reusable executor revalidate the committed authorization before
   candidate checkout and independently revalidate the downloaded PREPARED
   manifest/seal before claim creation;
8. preserve no-retry semantics, terminal retention and fail-closed behavior;
9. strengthen static/focused contracts for generic package binding and
   historical consumed-package rejection through the claim;
10. do **not** add the second `EXECUTE_ONCE` record in this work item;
11. do **not** create an execution claim or run a formal campaign;
12. do **not** modify production topology C++ or TMR scientific
    matrix/allowlist/cases/gates.

Target second-package identity remains:

- candidate `37f9af77f38e12af0a92d3c0f57f1ad31a218144`;
- preparation run `35531261000`;
- artifact `10611054028`;
- artifact SHA-256
  `96a47fcecc524e0a4baccee899bd88be8443dbc9778a55271a8376ebe2f6a1ab`;
- manifest `f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`;
- seal `366c782c571f6e63e320ac65e51dc464b0e3b3c8de498cfaf49ef50672dba9c2`.

## Next admissible work item after closure

After this generic authorization-binding mechanism is merged, focused TMR
Tooling and ordinary post-merge checks pass, and its checkpoint is closed,
create one separate exact `EXECUTE_ONCE` authorization-record PR for the
audited second PREPARED package.

Merging that future record to protected `main` will be the formal execution
authorization event. No authorization record belongs to this infrastructure
work item.

