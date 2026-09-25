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
- `docs/parametric-curve-family-abstraction-closure-sync`: **MERGED /
  HISTORICAL** via PR #100; reconciles the terminal post-PR #99 authority
  without changing scientific or production scope.
- `curve/bounded-parametric-curve-contract`: **MERGED / HISTORICAL** via
  PR #101; bounded parameter-domain/static-concept/Cubic-Bézier conformance
  implementation only.
- `docs/bounded-parametric-curve-contract-closure`: **MERGED /
  HISTORICAL** via PR #102; records PR #101 integration and post-merge
  validation.
- `curve/bounded-line-segment-decision`: **MERGED / HISTORICAL** via
  PR #103; literature-backed first-concrete-family decision only.
- `docs/bounded-line-segment-decision-closure`: **MERGED / HISTORICAL**
  via PR #104; closes the bounded line-segment decision checkpoint and
  authorizes only its mapped implementation as the next work item.
- `curve/bounded-line-segment`: **MERGED / HISTORICAL** via PR #105;
  bounded 2D/3D directed line-segment representation implementation.
- `docs/bounded-line-segment-implementation-closure`: **MERGED /
  HISTORICAL** via PR #106; records PR #105 integration, retained initial
  mechanical validation failure, corrected final validation and closure.
- `curve/rational-quadratic-bezier-decision`: **MERGED / HISTORICAL**
  via PR #107; literature-backed second-concrete-family decision only.
- `docs/rational-quadratic-bezier-decision-closure`: **MERGED /
  HISTORICAL** via PR #108; closes the rational-quadratic decision checkpoint
  and authorizes only its bounded implementation as the next work item.
- `curve/rational-quadratic-bezier`: **MERGED / HISTORICAL** via PR #109;
  fixed-degree positive-weight 2D/3D rational quadratic Bézier implementation.
- `docs/rational-quadratic-bezier-implementation-closure`: **MERGED /
  HISTORICAL** via PR #110; records PR #109 integration and post-merge
  validation.
- `docs/rational-quadratic-bezier-closure-sync`: **MERGED / HISTORICAL**
  via PR #111; terminally reconciles the rational-quadratic closure.
- `curve/trimmed-parametric-subcurve-decision`: **MERGED / HISTORICAL**
  via PR #112; literature-backed trimming-vs-family comparison decision.
- `docs/trimmed-parametric-subcurve-decision-closure`: **MERGED /
  HISTORICAL** via PR #113; closes the trim decision checkpoint.
- `curve/trimmed-parametric-subcurve`: **MERGED / HISTORICAL** via PR #114;
  statically typed 2D/3D oriented trim wrapper implementation.
- `docs/trimmed-parametric-subcurve-implementation-closure`: **MERGED /
  HISTORICAL** via PR #115; closes the trimmed-subcurve implementation
  checkpoint.
- `curve/two-span-cubic-bspline-decision`: **MERGED / HISTORICAL** via
  PR #116; literature-backed bounded first B-spline decision.
- `docs/two-span-cubic-bspline-decision-closure`: **MERGED / HISTORICAL**
  via PR #117; closes the bounded first B-spline decision checkpoint.
- `curve/two-span-cubic-bspline`: **MERGED / HISTORICAL** via PR #118;
  fixed two-span cubic polynomial B-spline implementation.
- `docs/two-span-cubic-bspline-implementation-closure`: **MERGED /
  HISTORICAL** via PR #119; closes the fixed two-span cubic B-spline
  implementation checkpoint.
- `docs/two-span-cubic-bspline-closure-sync`: **MERGED / HISTORICAL**
  via PR #120; terminally reconciles the fixed B-spline closure.
- `curve/two-span-cubic-nurbs-decision`: **MERGED / HISTORICAL** via
  PR #121; bounded fixed NURBS decision.
- `docs/two-span-cubic-nurbs-decision-closure`: **MERGED / HISTORICAL**
  via PR #122; closes the fixed NURBS decision checkpoint.
- `docs/two-span-cubic-nurbs-closure-sync`: **MERGED / HISTORICAL**
  via PR #123; terminally reconciles the decision closure before production.
- `curve/two-span-cubic-nurbs`: **MERGED / HISTORICAL** via PR #124;
  fixed five-control/two-span cubic positive-weight NURBS implementation.
- `docs/two-span-cubic-nurbs-implementation-closure`: **MERGED /
  HISTORICAL** via PR #125; closes the fixed NURBS implementation checkpoint.
- `docs/two-span-cubic-nurbs-implementation-closure-sync`: **MERGED /
  HISTORICAL** via PR #126; terminally reconciles the fixed NURBS
  implementation closure.
- `docs/fixed-nurbs-terminal-state`: **MERGED / HISTORICAL** via PR #127;
  removes the final stale sync marker and establishes a genuinely idle
  representation-breadth checkpoint.
- `curve/multi-span-cubic-nurbs-decision`: **MERGED / HISTORICAL** via
  PR #128; bounded multi-span cubic NURBS breadth decision.
- `docs/multi-span-cubic-nurbs-decision-closure`: **MERGED / HISTORICAL**
  via PR #129; closes the multi-span cubic NURBS decision checkpoint.
- `curve/multi-span-cubic-nurbs`: **MERGED / HISTORICAL** via PR #130;
  bounded runtime-variable span-count cubic positive-weight NURBS
  implementation.
- `docs/multi-span-cubic-nurbs-implementation-closure`: **MERGED /
  HISTORICAL** via PR #131; closes the multi-span cubic NURBS implementation
  checkpoint.
- `docs/multi-span-cubic-nurbs-closure-sync`: **MERGED / HISTORICAL**
  via PR #132; terminally reconciles the multi-span NURBS implementation
  closure.
- `curve/cubic-nurbs-double-knot-continuity-decision`: **MERGED /
  HISTORICAL** via PR #133; bounded repeated-knot continuity decision.
- `docs/cubic-nurbs-double-knot-decision-closure`: **MERGED / HISTORICAL**
  via PR #134; closes the double-knot C1 continuity decision checkpoint.
- `curve/cubic-nurbs-double-knot-continuity`: **MERGED / HISTORICAL** via
  PR #135; bounded multiplicity-1/2 implementation work item.
- `docs/cubic-nurbs-double-knot-implementation-closure`: **MERGED /
  HISTORICAL** via PR #136; closes the multiplicity-1/2 implementation
  checkpoint.
- `docs/cubic-nurbs-double-knot-closure-sync`: **MERGED / HISTORICAL**
  via PR #137; terminally reconciles the closed C1 implementation.
- `surface/representation-entry-decision`: **MERGED / HISTORICAL** via
  PR #138; literature-backed Surface Representation entry decision.
- `docs/surface-representation-entry-decision-closure`: **MERGED /
  HISTORICAL** via PR #139; closes the Surface Representation entry decision.
- `surface/bicubic-bezier-patch`: **MERGED / HISTORICAL** via PR #140;
  first bounded production work item in Surface Representation.
- `docs/surface-bicubic-bezier-implementation-closure`: **MERGED /
  HISTORICAL** via PR #141; closes the first Surface Representation
  implementation checkpoint.
- `docs/surface-bicubic-bezier-closure-sync`: **MERGED / HISTORICAL**
  via PR #142; terminally reconciles the closed bicubic patch checkpoint.
- `surface/rational-bicubic-bezier-decision`: **MERGED / HISTORICAL**
  via PR #143; bounded rational bicubic Bézier surface decision.
- `docs/surface-rational-bicubic-bezier-decision-closure`:
  **MERGED / HISTORICAL** via PR #144; closes the rational bicubic surface
  decision checkpoint.
- `surface/rational-bicubic-bezier`: **MERGED / HISTORICAL** via PR #145;
  bounded positive-weight rational bicubic surface implementation.
- `docs/surface-rational-bicubic-bezier-implementation-closure`:
  **MERGED / HISTORICAL** via PR #146; closes the rational bicubic surface
  implementation checkpoint.
- `docs/surface-rational-bicubic-bezier-closure-sync`: **MERGED /
  HISTORICAL** via PR #147; terminally reconciles the closed rational bicubic
  checkpoint.
- `docs/surface-rational-terminal-checkpoint`: **MERGED / HISTORICAL**
  via PR #148; normalizes the terminal rational-bicubic checkpoint.
- `surface/bicubic-nurbs-decision`: **MERGED / HISTORICAL** via PR #149;
  bounded bicubic NURBS surface breadth decision.
- `docs/surface-bicubic-nurbs-decision-closure`: **MERGED / HISTORICAL**
  via PR #150; closes the bicubic NURBS surface decision checkpoint.
- `surface/bicubic-nurbs`: **MERGED / HISTORICAL** via PR #151;
  bounded simple-knot bicubic positive-weight NURBS surface implementation.
- `docs/surface-bicubic-nurbs-implementation-closure`: **MERGED /
  HISTORICAL** via PR #152; closes the simple-knot bicubic NURBS surface
  implementation checkpoint.
- `surface/bicubic-nurbs-double-knot-decision`: **MERGED / HISTORICAL**
  via PR #153; bounded surface C1 continuity decision.
- `docs/surface-bicubic-nurbs-double-knot-decision-closure`:
  **MERGED / HISTORICAL** via PR #154; closes the bicubic NURBS surface C1
  decision checkpoint.
- `surface/bicubic-nurbs-double-knot-continuity`: **MERGED / HISTORICAL**
  via PR #155; bounded multiplicity-one/two bicubic NURBS surface
  implementation.
- `docs/surface-bicubic-nurbs-double-knot-implementation-closure`:
  **MERGED / HISTORICAL** via PR #156; closes the multiplicity-one/two
  bicubic NURBS surface implementation checkpoint.
- `surface/coons-patch-decision`: **MERGED / HISTORICAL** via PR #157;
  bounded Coons/transfinite Surface Representation decision.
- `docs/surface-coons-patch-decision-closure`: **MERGED / HISTORICAL**
  via PR #158; closes the bounded Coons surface decision checkpoint.
- `surface/coons-patch`: **MERGED / HISTORICAL** via PR #159; bounded
  oriented four-boundary cubic Bézier Coons patch implementation.
- `docs/surface-coons-patch-implementation-closure`: **MERGED /
  HISTORICAL** via PR #160; closes the oriented cubic Bézier Coons
  implementation checkpoint.
- `docs/surface-coons-terminal-sync`: **MERGED / HISTORICAL** via PR #161;
  terminally reconciles the closed Coons work unit.
- `surface/rectangular-trimmed-surface-decision`: **MERGED / HISTORICAL**
  via PR #162; bounded static rectangular trimmed-surface breadth decision.
- `docs/surface-rectangular-trim-decision-closure`: **MERGED /
  HISTORICAL** via PR #163; closes the static rectangular trimmed-surface
  decision checkpoint.
- `docs/surface-rectangular-trim-terminal-sync`: **MERGED / HISTORICAL**
  via PR #164; terminally reconciles the closed decision before production.
- `surface/rectangular-trimmed-surface`: **MERGED / HISTORICAL** via
  PR #165; bounded static rectangular trimmed-surface implementation.
- `docs/surface-rectangular-trim-implementation-closure`: **MERGED /
  HISTORICAL** via PR #166; closes the static rectangular trim
  implementation checkpoint.
- `docs/surface-rectangular-trim-closure-sync`: **MERGED / HISTORICAL**
  via PR #167; terminally reconciles the closed rectangular-trim work unit.
- `surface/linear-extrusion-decision`: **MERGED / HISTORICAL** via PR #168;
  bounded swept-surface decision.
- `docs/surface-linear-extrusion-decision-closure`: **MERGED / HISTORICAL** via
  PR #169; closes the bounded linear-extrusion decision checkpoint.
- `surface/cubic-bezier-linear-extrusion`: **MERGED / HISTORICAL** via
  PR #170; bounded cubic Bézier linear-extrusion implementation.
- `docs/surface-linear-extrusion-implementation-closure`: **MERGED /
  HISTORICAL** via PR #171; closes the bounded linear-extrusion implementation
  checkpoint.
- `docs/surface-linear-extrusion-closure-sync`: **MERGED / HISTORICAL**
  via PR #172; terminally reconciles the closed linear-extrusion work unit.
- `surface/arbitrary-axis-placement-decision`: **MERGED / HISTORICAL** via
  PR #173; bounded arbitrary-axis placement prerequisite decision.
- `docs/arbitrary-axis-placement-decision-closure`: **MERGED / HISTORICAL**
  via PR #174; closes the arbitrary-axis placement decision checkpoint.
- `docs/arbitrary-axis-placement-closure-sync`: **MERGED / HISTORICAL**
  via PR #175; terminally reconciles the closed decision before production.
- `surface/arbitrary-axis-placement`: **MERGED / HISTORICAL** via PR #176;
  bounded `AxisPlacement3` implementation work item.
- `docs/arbitrary-axis-placement-implementation-closure`: **MERGED /
  HISTORICAL** via PR #177; closes the `AxisPlacement3` implementation
  checkpoint.
- `docs/arbitrary-axis-placement-implementation-closure-sync`: **MERGED /
  HISTORICAL** via PR #178; terminally reconciles the closed implementation
  checkpoint.
- `surface/cubic-bezier-revolution-decision`: **MERGED / HISTORICAL** via
  PR #179; bounded revolution breadth decision.
- `docs/surface-cubic-bezier-revolution-decision-closure`: **MERGED /
  HISTORICAL** via PR #180; closes the bounded revolution decision checkpoint.
- `surface/cubic-bezier-revolution`: **MERGED / HISTORICAL** via PR #181;
  bounded revolution implementation work item.
- `docs/surface-cubic-bezier-revolution-implementation-closure`:
  **MERGED / HISTORICAL** via PR #182; closes the bounded revolution
  implementation checkpoint.
- `docs/surface-cubic-bezier-revolution-closure-sync`:
  **MERGED / HISTORICAL** via PR #183; terminally reconciles the closed
  revolution work unit.
- `surface/analytic-plane-decision`: **MERGED / HISTORICAL** via PR #184;
  bounded analytic plane breadth decision.
- `docs/surface-analytic-plane-decision-closure`: **MERGED / HISTORICAL**
  via PR #185; closes the bounded analytic plane decision checkpoint.
- `surface/analytic-plane`: **MERGED / HISTORICAL** via PR #186;
  bounded analytic plane production work item.
- `docs/surface-analytic-plane-implementation-closure`: **MERGED /
  HISTORICAL** via PR #187; closes the bounded analytic plane implementation.
- `docs/surface-analytic-plane-terminal-sync`: **MERGED / HISTORICAL**
  via PR #188; terminally reconciles the bounded analytic plane work unit.
- `surface/analytic-cylinder-decision`: **MERGED / HISTORICAL** via PR #189;
  bounded analytic cylinder-sector breadth decision.
- `docs/surface-analytic-cylinder-decision-closure`: **MERGED /
  HISTORICAL** via PR #190; closes the analytic cylinder decision checkpoint.
- `surface/analytic-cylinder`: **MERGED / HISTORICAL** via PR #191;
  bounded analytic cylinder-sector production work item.
- `docs/surface-analytic-cylinder-implementation-closure`: **CLOSURE-ONLY**;
  records PR #191 integration and protected-main validation.

The presence of historical branches on the remote does not make them active.

## Current active work item

**None. Bounded Analytic Circular Cylinder Sector implementation is integrated
and ready for closure.**

Implementation evidence:

1. candidate head:
   `9536a0a9849021d47acdfb03b1179866e5de87a4`;
2. candidate FAST `36088557775`: PASS, 36/36;
3. candidate INTEGRATION `36088557702`: PASS, 36/36 in GCC and Clang;
4. final PR head:
   `e06c1344570daa7ff76cda61dc8165e2fc0dc8b2`;
5. final PR FAST `36088704029`: PASS, 36/36;
6. final PR INTEGRATION `36088704205`: PASS, 36/36 in GCC and Clang;
7. PR #191 merged as
   `6edc598637ddedd54cf62732e33a62e504cb82c8`;
8. protected-main FAST `36088816019`: PASS, 36/36;
9. protected-main INTEGRATION `36088816031`: PASS, 36/36 in GCC and Clang;
10. focused `apmesh_core.surface_cylinder`: PASS throughout;
11. no broader analytic/topological/differential capability was introduced.

This closure branch changes documentation only.

## Next admissible work item after closure

After this closure is integrated and its own post-merge FAST/INTEGRATION pass,
open one fresh literature-backed Surface Representation breadth decision
comparing:

1. opening Surface Differential Geometry;
2. bounded analytic sphere;
3. bounded analytic cone;
4. bounded analytic torus;
5. general trimming / p-curves / topological faces.

No candidate is pre-authorized.
