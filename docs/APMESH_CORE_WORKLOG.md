# AP Mesh Core — Operational Work Ledger

Status: AUTHORITATIVE FOR OPERATIONAL CONTINUITY  
Last updated: 2026-09-22  
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

## Last closed functional work-item anchor

- Certified Simple Planar Inflection Isolation implementation: **IMPLEMENTED /
  FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED**;
- decision authority:
  `docs/decisions/CURVE_CERTIFIED_SIMPLE_INFLECTION_ISOLATION_DECISION.md`;
- implementation PR #96 merged as
  `c4905589c2ee8700c58560ef1a99a49a3821af4e`;
- final PR FAST `35678624215`: PASS;
- final PR INTEGRATION `35678624192`: PASS;
- post-merge FAST `35678808956`: PASS;
- post-merge INTEGRATION `35678808941`: PASS;
- current stage: **Curve Differential Geometry — IN INVESTIGATION /
  NOT QUALIFIED**;
- most recently qualified prerequisite stage: **Curve Representation —
  QUALIFIED / CGR0–CGR7 PASS** in the admitted cloud envelope.

The current documentation-only closure lineage is
`docs/certified-simple-inflection-implementation-closure` / PR #97.
It does not authorize production code or a later curve capability.

The fresh repository coverage audit also retains one architectural blocker:
the qualified Curve Representation scope contains only polynomial cubic Bézier
curves. Analytic arc/conic, rational Bézier, B-spline, NURBS and production
surface families remain unimplemented/unqualified and require explicit future
scope decisions. This does not weaken the frozen cubic-Bézier qualification.


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
| Corrected formal TMR preparation and audit | MERGED / PREPARED / HISTORICAL | workflow run `35531261000`; audit PR #37 | artifact `10611054028`; ZIP SHA-256 `96a47fcecc524e0a4baccee899bd88be8443dbc9778a55271a8376ebe2f6a1ab`; manifest `f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`; audit PASS | preparation-only historical authority consumed by the second formal campaign |
| Generic TMR authorization binding | MERGED | `topology/tmr-generic-authorization-binding`; PR #39 | final TMR Tooling `35532329855` PASS; PR/post-merge FAST+INTEGRATION PASS | infrastructure only; no authorization or execution |
| Whole-commit authorization isolation guard | MERGED | `topology/tmr-authorization-whole-commit-guard`; PR #41 | final TMR Tooling `35532708479` PASS; post-merge FAST `35533347184`; INTEGRATION `35533347164` | controller proves exactly one changed path in the entire authorization commit |
| Second exact TMR execution authorization | MERGED / CONSUMED | `topology/tmr-second-execution-authorization`; PR #43 | one file / 14 lines; TMR Tooling `35533565419` PASS; PR FAST `35533616076`; INTEGRATION `35533616391`; merged `cddd959574ed6a677ac755a5b329d53a9cfe32ec` | exact second `EXECUTE_ONCE` event; no reuse |
| Second formal TMR execution | EXECUTED / ATTEMPT CONSUMED | protected-main run `35533702004` | complete authorization/preflight PASS; immutable claim created; execute PASS; retention PASS; artifact `10612032787`, SHA-256 `dd5c12f54ed60a106059a9f42acf5c07884fa83230ae4dfe81d073cb6f7111b8` | process success is not itself qualification |
| Corrected terminal scientific audit | VALIDATED_UNMERGED / PASS | `docs/tmr-corrected-terminal-audit-pass` | TMR0–TMR7 independently recomputed PASS; 56 command records; eight semantic repetitions; eight byte-identical certificates; exact retention | integration pending; qualification scoped to admitted cloud environment |
| TMR tooling-contract correction | SUPERSEDED | `topology/tmr-tooling-contract-correction` | historical focused run `35515277674` PASS | superseded by later integrated tooling lineage |
| CGR stage-exit protocol | MERGED | `curve/continuous-geometry-regression-decision`; PR #73 | protocol integration and post-merge FAST/INTEGRATION PASS | documentation/protocol only; no formal execution |
| CGR report-only tooling | MERGED | `curve/continuous-geometry-regression-tooling`; PR #75 | final report-only tooling PASS in GCC/Clang; exact 56-command/8-certificate simulation | no formal PREPARED package or execution |
| CGR formal campaign infrastructure | MERGED | `curve/cgr-formal-campaign-infrastructure`; PR #79 | final formal tooling `35614078272` PASS; post-merge FAST/INTEGRATION PASS | lifecycle infrastructure only |
| First formal CGR preparation and audit | MERGED / PREPARED / HISTORICAL | run `35620525792`; audit PR #81 | artifact `10649325906`; manifest `201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`; PREPARED audit PASS | preparation-only authority consumed by first formal CGR campaign |
| First exact CGR execution authorization | MERGED / CONSUMED | `curve/cgr-execution-authorization`; PR #83 | one file / 14 lines; merged `e8b17256924e907d0859b8ac7061600ffc404b9e` | exact `EXECUTE_ONCE` event; no reuse |
| First formal CGR execution | EXECUTED / ATTEMPT CONSUMED | protected-main run `35630423134` | authorization/preflight/claim/execute/retention PASS; artifact `10654358199`, SHA-256 `a4b59453dcc9f9cacb4265ba540e1a3a443f3b6c80509411e0d6d4b18eff7aa6` | workflow success is not itself qualification |
| CGR terminal scientific audit | MERGED / PASS | `docs/cgr-terminal-audit-pass`; PR #84 | CGR0–CGR7 independently recomputed PASS; 56 commands; 112 semantic test executions; 8 certificates; exact retention and derived evidence | Curve Representation qualified in admitted cloud envelope |


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
- `topology/tmr-generic-authorization-binding`: **MERGED / HISTORICAL**
  via PR #39; generic manifest-bound authorization infrastructure only; no
  EXECUTE_ONCE record or campaign execution.
- `docs/tmr-generic-authorization-closure`: **CLOSURE-ONLY**; records PR #39
  integration and post-merge validation.
- `topology/tmr-authorization-whole-commit-guard`: **MERGED / HISTORICAL**
  via PR #41; complete-commit isolation guard for formal authorization.
- `docs/tmr-whole-commit-guard-closure`: **MERGED / HISTORICAL** via PR #42.
- `topology/tmr-second-execution-authorization`: **MERGED / CONSUMED**
  via PR #43; exact second manifest-bound `EXECUTE_ONCE` record only.
- `docs/tmr-corrected-terminal-audit-pass`: **MERGED / HISTORICAL** via
  PR #44; independent second terminal scientific audit, TMR0–TMR7 PASS.
- `docs/topological-model-qualification-closure`: **CLOSURE-ONLY**; closes
  the qualified Topological Model stage before any Curve Representation entry.
- `curve/continuous-representation-entry-decision`: **MERGED / HISTORICAL**
  via PR #46; literature-backed bounded entry decision only; no curve
  production implementation.
- `docs/curve-entry-decision-closure`: **CLOSURE-ONLY**; records PR #46
  integration and post-merge validation.

- `curve/cubic-bezier-total-arc-length-enclosure`: **MERGED / HISTORICAL**
  via PR #61; certified total-length implementation.
- `docs/curve-total-arc-length-closure`: **MERGED / HISTORICAL** via PR #62;
  closes Work Unit 1.
- `curve/cumulative-arc-length-mapping-decision`: **MERGED / HISTORICAL**
  via PR #63; defines Work Unit 2A and the blocked Work Unit 2B boundary.
- `docs/cumulative-arc-length-decision-closure`: **CLOSURE-ONLY**; records
  PR #63 integration and post-merge validation.
- `curve/cumulative-arc-length-enclosure`: **MERGED / HISTORICAL** via
  PR #66; Work Unit 2A implementation only.
- `docs/cumulative-arc-length-enclosure-closure`: **CLOSURE-ONLY**; records
  Work Unit 2A integration and post-merge validation.
- `curve/inverse-arc-length-bracketing-decision`: **MERGED / HISTORICAL**
  via PR #68; fixes the executable Work Unit 2B contract before inverse
  implementation.
- `docs/curve-inverse-bracketing-decision-closure`: **CLOSURE-ONLY**;
  records PR #68 integration and post-merge validation.
- `curve/certified-inverse-arc-length-bracketing`: **MERGED / HISTORICAL**
  via PR #70; Work Unit 2B implementation only.
- `docs/curve-inverse-bracketing-closure`: **CLOSURE-ONLY**; records
  Work Unit 2B integration and post-merge validation.
- `curve/continuous-geometry-regression-decision`: **MERGED / HISTORICAL**
  via PR #73; CGR0–CGR7 stage-exit protocol only.
- `docs/curve-cgr-protocol-closure`: **CLOSURE-ONLY**; records PR #73
  integration and post-merge validation.
- `curve/continuous-geometry-regression-tooling`: **MERGED / HISTORICAL**
  via PR #75; report-only CGR exporter/validator/runner/negative/derived-
  evidence tooling only.
- `curve/cgr-formal-campaign-infrastructure`: **MERGED / HISTORICAL**
  via PR #79; formal lifecycle infrastructure only; no real PREPARED package
  or formal execution.
- `docs/cgr-formal-infrastructure-closure`: **CLOSURE-ONLY**; records PR #79
  integration, exact-tree identity, and post-merge validation.
- `docs/cgr-prepared-audit-pass`: **MERGED / HISTORICAL** via PR #81;
  independent audit of the first formal CGR PREPARED package only.
- `docs/cgr-prepared-audit-closure`: **CLOSURE-ONLY**; records PR #81
  integration and post-merge validation.

- `curve/cgr-execution-authorization`: **MERGED / CONSUMED** via
  PR #83; exact one-file first formal CGR `EXECUTE_ONCE` authorization.
- `docs/cgr-terminal-audit-pass`: **MERGED / HISTORICAL** via PR #84;
  independent terminal scientific audit of the consumed first formal CGR
  campaign.
- `docs/curve-representation-qualification-closure`: **CLOSURE-ONLY**;
  closes the qualified Curve Representation stage before the next stage entry.
- `curve/differential-geometry-entry-decision`: **MERGED / HISTORICAL**
  via PR #86; literature-backed entry decision and repository mapping only; no
  production curvature implementation.
- `docs/curve-differential-entry-closure`: **CLOSURE-ONLY**; records PR #86
  integration and post-merge validation.
- `curve/pointwise-curvature-magnitude`: **MERGED / HISTORICAL** via PR #88;
  bounded first Curve Differential Geometry production work unit only.
- `docs/curve-pointwise-curvature-closure`: **MERGED / HISTORICAL** via
  PR #89; closes the first Curve Differential Geometry work unit.
- `curve/signed-planar-curvature-decision`: **MERGED / HISTORICAL** via
  PR #90; literature-backed decision only; no production implementation.
- `docs/signed-planar-curvature-decision-closure`: **CLOSURE-ONLY**; records
  PR #90 integration and post-merge validation.
- `curve/certified-simple-inflection-isolation-decision`: **MERGED /
  HISTORICAL** via PR #94; literature-backed decision and repository mapping
  only; no production root-isolation implementation.
- `docs/certified-simple-inflection-decision-closure`: **CLOSURE-ONLY**;
  records PR #94 integration and post-merge validation.
- `curve/certified-simple-inflection-isolation`: **MERGED / HISTORICAL**
  via PR #96; bounded 2D certified simple-inflection implementation.
- `docs/certified-simple-inflection-implementation-closure`:
  **CLOSURE-ONLY**; records PR #96 integration and post-merge validation.
- `curve/parametric-curve-family-abstraction-decision`: **MERGED /
  HISTORICAL** via PR #98; literature-backed representation-breadth transition
  decision only; no production geometry implementation.
- `docs/parametric-curve-family-abstraction-decision-closure`: **MERGED /
  HISTORICAL** via PR #99; closes the decision checkpoint.
- `docs/parametric-curve-family-abstraction-closure-sync`: **CLOSURE-ONLY /
  SYNCHRONIZATION**; reconciles the terminal post-PR #99 authority without
  changing scientific or production scope.

The presence of historical branches on the remote does not make them active.

## Current active work item

**None. Parametric Curve Family Abstraction and Representation Breadth decision
is integrated, closed, and terminally synchronized.**

Closure evidence:

1. decision authority:
   `docs/decisions/CURVE_PARAMETRIC_FAMILY_ABSTRACTION_DECISION.md`;
2. decision PR #98 merged as
   `12ecbf584751dadb0dd142c485b1cd4f220736d8`;
3. decision final PR FAST `35711481469`: PASS;
4. decision final PR INTEGRATION `35711481473`: PASS;
5. decision post-merge FAST `35711563476`: PASS;
6. decision post-merge INTEGRATION `35711563585`: PASS;
7. closure PR #99 merged as
   `60e7677323300d4263d53c616b3081dd2fa03d0f`;
8. closure final PR FAST `35711824443`: PASS;
9. closure final PR INTEGRATION `35711824521`: PASS;
10. closure post-merge FAST `35711924946`: PASS;
11. closure post-merge INTEGRATION `35711924910`: PASS;
12. the qualified cubic-Bézier CGR0–CGR7 baseline remains unchanged;
13. no concrete new curve family or surface implementation was introduced.

No production work item is active at this checkpoint.

## Next admissible work item

Open exactly one implementation work item:

**Bounded Parametric Curve Contract and Cubic Bézier Conformance.**

The implementation is constrained by
`docs/decisions/CURVE_PARAMETRIC_FAMILY_ABSTRACTION_DECISION.md` and may add
only the common finite closed parameter-domain vocabulary, static 2D/3D C++23
curve concepts, reversal-parameter semantics, unchanged Cubic-Bézier
conformance, focused contracts and required build registration.

Concrete line/arc/rational/arbitrary-degree Bézier/B-spline/NURBS,
composite/trimmed curve and all surface implementations remain separate later
decisions/work units.

