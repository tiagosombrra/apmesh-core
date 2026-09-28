# AP Mesh Core — Continuation State

Status: ACTIVE
Last updated: 2026-09-28
Authoritative roadmap: `docs/APMESH_CORE_ROADMAP.md`
Operational continuity ledger: `docs/APMESH_CORE_WORKLOG.md`
Repository state: verify `main`, open PRs, relevant branches, ruleset, recent
Actions, and the operational ledger before continuation. `main` is the
canonical integration branch.

## Purpose

This file is the compact scientific continuation entry point for a new work
session. Read this file first, then `docs/APMESH_CORE_WORKLOG.md`, then the
authoritative roadmap, then the currently active contract/decision document.
The worklog is authoritative for branch/work-item/PR/run state. Historical
discussion is not required to determine the next admissible action.

## Current strategic decision

The doctoral implementation is being reconstructed as a greenfield, modular, deterministic C++23 scientific core instead of progressively refactoring the legacy AP Mesh architecture.

The legacy implementation is preserved as:

- published/historical reference;
- source of fixtures and previous evidence;
- source of candidate algorithms to be re-derived and independently verified;
- comparison target, but never a mathematical oracle.

The greenfield implementation must eventually be usable as a library inside a larger application.

## Permanent rules already accepted

1. C++23.
2. Avoid third-party dependencies by default. Admit one only through an explicit technical/scientific justification showing material advantage.
3. Maximum three hierarchy levels: Scientific Stage -> Investigation Problem -> Executable Work Unit.
4. Use descriptive action names; short codes are secondary and never sufficient.
5. Keep `docs/APMESH_CORE_ROADMAP.md` synchronized with every meaningful advance.
6. Document every stage, decision, experiment, failure, retained limitation, and correction so another session can continue without conversation history.
7. Perform focused literature/best-practice research before changing or implementing scientific mechanisms whenever sources are available.
8. Every Scientific Stage ends with a mandatory cumulative regression campaign. A stage cannot become `QUALIFIED` with unresolved regressions.
9. If a later regression invalidates an earlier qualified result, reopen the earlier stage explicitly.
10. Correctness precedes optimization and parallelism.
11. Serial deterministic implementation precedes OpenMP/MPI or other parallel execution.
12. Topological identity is explicit and never inferred from geometric proximity.
13. Mutable global scientific state and universal global tolerances are forbidden.
14. Scientific/domain failures must be explicit and diagnosable; silent fallbacks are forbidden.
15. Generated results must be reproducible from declared inputs, revision, environment, and experiment manifest.
16. Small components close with focused analytic contracts and may integrate as
    implemented but unqualified; formal qualification occurs cumulatively at
    scientific-stage exit.
17. Formal manifests and four-cell campaigns are not created per value type
    unless a separate scientific decision proves that the claim cannot wait for
    stage closure.
18. Ordinary work targets 60--65% scientific/C++ implementation, 25--30%
    focused validation/tooling, and about 10% documentation/governance. This is
    an anti-overengineering guideline rather than rigid accounting.
19. One reusable stage-level evidence workflow is preferred over new runners,
    collectors, comparers, and schemas for each component.
20. A mechanical tooling defect gets one focused regression contract without a
    new scientific decision cycle; two consecutive mechanical failures stop the
    standalone campaign before any third execution.
21. Documentation records current authority and terminal evidence without
    creating a chronological micro-record for each implementation step.
22. Validation has three profiles: FAST for ordinary development, INTEGRATION
    for coherent pre-merge semantic regression, and QUALIFICATION only for
    explicit scientific-stage closure. Qualification tooling is opt-in and its
    historical evidence remains preserved without running by default.
23. Operational continuity is persisted in `docs/APMESH_CORE_WORKLOG.md`.
    Exactly one work item may be active/validated-unmerged at a time.
24. Do not start the next work item until the current item is merged,
    post-merge checks pass, and STATE/ROADMAP/WORKLOG plus active protocol
    documents are synchronized on `main`.
25. After any UI/session/tool interruption, reconcile the remote repository
    before writing. Chat memory is never authoritative for branch or run state.
26. Historical/superseded branches must be classified in the worklog and may
    not be silently treated as active.
27. Do not embed the ledger's own eventual `main` commit SHA as authoritative
    state. Live `main` identity is always re-audited remotely; documentation
    records stable closed-work-item anchors instead.

## Historical repository checkpoint

Snapshot after protected-main validation on 2026-09-26. Its detailed evidence
is retained below; its operational status is superseded by the latest closed
repository actions and next admissible actions below.

- repository: `tiagosombrra/apmesh-core`;
- canonical integration branch: `main`;
- current scientific stage:
  **Surface Representation — Continuous Patch Geometry — BOUNDED ANALYTIC
  SPHERE IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED /
  CLOSED / NOT QUALIFIED**;
- Surface Differential Geometry remains:
  **PRINCIPAL CURVATURE VALUES IMPLEMENTED / FOCUSED CONTRACTS PASS /
  INTEGRATED / CLOSED / NOT QUALIFIED**;
- ordinary semantic inventory: **41 tests**;
- sphere final PR head:
  `0fb729054b0d3fa97045c0238ed4de9ce2e94d3d`;
- final PR FAST `36263188269`: PASS, 40/40;
- final PR INTEGRATION `36263188266`: PASS, 40/40;
- implementation PR #214 merged as
  `461c3f04454cc4f8aa789d9baba5085362aa6254`;
- implementation post-merge FAST `36263360375`: PASS, 40/40;
- implementation post-merge INTEGRATION `36263360805`: PASS, 40/40;
- implementation audit:
  `docs/audits/2026-09-26-surface-analytic-sphere-implementation-audit.md`;
- no sphere production work item is active during closure;
- Surface Representation remains **IN INVESTIGATION / NOT QUALIFIED** with
  cone, torus, full-periodic seams and general trimming/p-curve/face
  obligations retained;
- most recently qualified stage remains **Curve Representation — Continuous
  Geometry Before Discretization — QUALIFIED / CGR0–CGR7 PASS** in the
  admitted cloud envelope.

## Portable spline maintenance — terminal state

**PASS / INTEGRATED / CLOSED / NOT QUALIFIED**

PR #217 integrated the bounded portability maintenance as
`79057c03e432fa9116ced829f9a2c246e9d13ee9` after final reviewed head
`51d62b4297d82286e2f027c10fde9193af47bb1a` passed FAST #589 and
INTEGRATION #580. Protected-main FAST #590 and INTEGRATION #581 passed.

Documentation closure PR #218 used head
`c465a305fa28b05b5973c610fe6911c777c9357d`, passed FAST #591 and
INTEGRATION #582, and squash-merged as
`8d4a6e2d46c942966ceeb5e3fa735c983ef80909`. Closure protected-main FAST
#592 and INTEGRATION #583 both passed.

Terminal continuity publication PR #219 used head
`c3677b6063c2976ee04dae00a18fde47956fcff9`, passed FAST #593 and
INTEGRATION #584, and squash-merged as
`fbe611296bcf4869b642de5f8352845510e7d643`. Protected-main FAST #594 and
INTEGRATION #585 both passed on that terminal publication revision.

This final receipt records the already-complete terminal publication only. It
does not reopen the maintenance item or authorize the next scientific decision.

The exact production/test revision validated by the six local
GCC/Clang/MSVC Debug/Release development cells remains
`65ec6e8514e82e5add62f0c2fb53f049c29be215`, with 41/41 ordinary contracts
in every cell. The integrated ordinary semantic inventory is **41 tests**.

Native Windows remains ordinary development evidence only. No scientific
stage, public numeric contract, acceptance criterion, formal campaign or
qualification envelope was advanced. The initial GCC Release build failure
and its comparator-equivalent correction remain preserved in
`docs/audits/2026-09-26-portable-spline-arithmetic.md`.

No portable spline arithmetic production or closure work item remains active.

## Latest closed repository actions

**Surface Differential Geometry report-only qualification tooling:
INTEGRATED / CLOSED / NOT QUALIFIED.**

**Metric-conditioning normalization portability maintenance:
INTEGRATED / CLOSED / DEVELOPMENT VALIDATION PASS / NOT QUALIFIED.**

PR #239 is the closed maintenance anchor. Obtain the current `main` revision
from Git rather than storing a self-referential current-main SHA here.

Final PR #238 validation on head
`1e54a127e1e06cb399e96eb5b9fbf6673d30e4ce`:

- FAST `36424140384`: PASS;
- INTEGRATION `36424140173` / GCC 13 Debug: PASS;
- INTEGRATION `36424140173` / Clang 18/libc++ Debug: PASS;
- focused qualification-only CTest: 1/1 PASS locally on Ubuntu 24.04/GCC 13;
- ordinary semantic registration inventory: **42 tests**.

Integrated tooling branch:

`qualification/surface-differential-geometry-report-tooling`.

Initial technical candidate before continuity/audit synchronization:

`7cd9ca1d221577b1730a7d05aa4a4584b4cc270d`.

Protocol authority:

`docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md`.

Validation audit:

`docs/audits/2026-09-28-surface-differential-geometry-report-tooling-validation.md`.

Implemented report-only package:

- immutable qualification profile;
- scientific certificate exporter;
- independent certificate/profile validator;
- same-cell/cross-cell comparison;
- 17 negative-evidence self-checks;
- four deterministic figures;
- non-terminal gate-summary/report generation;
- focused qualification-tooling contract;
- qualification-only CMake registration.

Frozen boundary audit:

- **32 / 32** semantic Git blobs match the preregistered SHAs;
- ordinary semantic allowlist remains exactly **42 tests**;
- no production Surface Differential Geometry file changed;
- no ordinary surface semantic test changed;
- no runner/workflow/preparation/authorization mechanism was added.

Initial PR validation on head
`59bc9cbfb9550a82464b24dd53168347e12ffae6`:

- FAST #636: PASS, 42/42;
- INTEGRATION #627 / GCC 13 Debug: PASS, 42/42;
- INTEGRATION #627 / Clang 18/libc++ Debug: PASS, 42/42.

The final tooling head passed FAST and both INTEGRATION jobs. The focused
qualification-only CTest passed locally with the same exporter, validator,
profile and production inputs; subsequent audit wording did not affect the
test. PR #238 was integrated by squash as
`6fb3296096111915a812e1420de62d09edbd392a` without a formal qualification
campaign.

The completed metric-conditioning portability correction allows a positive
original metric coefficient to underflow during normalization while retaining
finite-result and nonzero-area checks. The `2^900` oracle, error vocabulary and
tolerances are unchanged. On 2026-09-28, MSVC Debug and Ubuntu 24.04/GCC 13
Debug both passed configure, build, the focused
`apmesh_core.surface_metric_conditioning` CTest (1/1), and all 42 ordinary
tests on this candidate based on `6fb3296`. The focused test includes the exact
`2^900` assertion. This is development evidence, not a scientific
qualification claim. PR #239 head
`c6c27cb863db7f5a572405d00bbcbb3af1e4401a` passed FAST run `36436600970` and
GCC 13 / Clang 18 libc++ INTEGRATION run `36436600851`. The final PR head
`14189c0e5bfa2581e6fd86e4de85efa194de7743` passed FAST run `36446587893` and
GCC 13 / Clang 18 libc++ INTEGRATION run `36446588003`. PR #239 was squash
merged as `a03cbd4090339cf2b26dd43016492830a3fc177e`; protected-main FAST
`36447759953` and INTEGRATION `36447759920` both passed. The correction remains
development evidence, not a scientific qualification claim. No metric
conditioning work item remains active.

PR #240 synchronized the integration record after PR #239. The ordinary
semantic inventory is 42 tests. No production or documentation work item is
currently active. The next admissible action is the fresh literature-backed
Surface Differential Geometry decision recorded below; no candidate is
pre-authorized.

Explicitly unauthorized:

- campaign runner;
- preparation/launch infrastructure;
- PREPARED manifest;
- workflow dispatch;
- one-time execution authorization;
- formal qualification execution;
- evidence retention/sealing for a formal candidate;
- production Surface Differential Geometry changes beyond this bounded
  portability correction;
- `QUALIFIED` or `BLOCKED` stage status.

Surface Representation remains **IN INVESTIGATION / NOT QUALIFIED**.

Surface Differential Geometry remains **IN INVESTIGATION / NOT QUALIFIED**.

### Retained historical Topological Model / cloud-infrastructure checkpoint

The material below is retained for provenance of the earlier Topological Model
and cloud-infrastructure lifecycle. It is **historical evidence, not current
stage/work-item authority**. Current authority is the snapshot above together
with WORKLOG and ROADMAP.

- repository: `tiagosombrra/apmesh-core`;
- visibility: `PUBLIC`;
- canonical branch: `main`;
- accepted functional `main` baseline after cloud QUALIFICATION-environment admission: `0a7095d431e4bea3c9c73e75d22df2e713c7a8ab`;
- post-merge GitHub Actions FAST on that baseline: `PASS`, run `35513658207`;
- post-merge GitHub Actions INTEGRATION on that baseline: `PASS`, run `35513658197`;
- final pre-merge Major Semantic Regression on tree-equivalent candidate `6a934de6e8f6fae35e6c38ec45b9b1f23b170acb`: `PASS`, run `35513567930`;
- open scientific stage: **Topological Model — Explicit Identity and Incidence — QUALIFIED /
TMR0–TMR7 PASS**;
- stage status: production bounded scope implemented, focused contracts pass,
  cumulative TMR0--TMR7 report-only tooling implemented, the admitted cloud
  envelope explicitly supplemented, the preparation/runner path fail-closed on
  that identity, and a manual preparation-only workflow implemented and
  statically qualified; that workflow has not been dispatched, no formal TMR
  manifest exists, and the stage remains unqualified;
- TMR report-only tooling validation: candidate
  `ad8f6f379d2284b782cb93bf4e0a6f3b1aa61822`, run `35514834796`, GCC 13
  Debug and Clang 18/libc++ Debug PASS;
- TMR cloud-identity binding validation: candidate
  `967072efdee49af04b06c3f73b2b89fcb66f874c`, run `35516246789`, exact
  admitted cloud identity plus focused GCC/Clang Debug tooling contracts PASS;
  prior run `35516204233` is retained as a mechanical protocol-guard failure
  before environment evaluation;
- formal TMR launch-plan tool paths are sealed to the admitted absolute cloud
  paths (`/usr/bin/cmake`, `/usr/bin/ctest`, `/usr/bin/ninja`,
  `/usr/bin/g++-13`, `/usr/bin/clang++-18`); focused GCC/Clang Debug
  validation PASS in run `35516578411` on candidate
  `84149dc1cb7181fd18760927f76a51e1c3b1ce6d`;
- manual preparation-only workflow implemented with `workflow_dispatch` only,
  canonical-`main` restriction, external runner-temp output, immutable
  PREPARED-state checks, pinned artifact retention, and no `execute` path.
  Runs `35516864464` and `35516972035` are retained as consecutive
  mechanical quoting failures; the tooling stop was honored, the shell command
  was simplified, and run `35517077819` passed in both focused cells.
  PR #20 integrated the workflow as
  `d9297ffad4f503b4ea11b056885749fff5872201`; post-merge FAST
  `35519501704` and INTEGRATION `35519501663` passed.
- first formal preparation: run `35524700979`, workflow_dispatch on canonical
  `main` candidate `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`, PASS;
  retained artifact `10609500629`, archive SHA-256
  `2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72`;
  preparation audit PASS in
  `docs/audits/2026-09-20-topological-model-tmr-preparation-audit.md`;
  package remains unconsumed, with `execution_requested=false` and TMR0--TMR7
  all `NOT_EXECUTED`.

Historical first-attempt Topological Model completion lanes:

| Lane | Completion | Current basis |
| --- | ---: | --- |
| Production implementation | 100% | Five authorized bounded topology work units implemented; no production-semantic defect is shown by the blocked campaign. |
| Focused validation | 100% | Focused GCC 13 Debug and Clang 18/libc++ Debug contracts pass. |
| Stage-regression / qualification tooling | 100% implemented, correction pending | The complete preparation/authorization/execution/retention path exists and ran once; diagnosis is required because runner repetition cardinality does not satisfy the sealed protocol. |
| Formal evidence preparation | 100% for first attempt | Run `35524700979` produced the audited PREPARED package, later consumed exactly once by the authorized formal campaign. |
| Formal evidence execution | 100% for first attempt | Run `35528077223` executed once and retained the terminal package; the attempt is immutable and consumed. |
| Terminal scientific audit | 100% | TMR0–TMR5 PASS; TMR6–TMR7 BLOCKED; overall BLOCKED. |
| Stage qualification | 0% closed | Topological Model remains NOT QUALIFIED until a future newly prepared campaign satisfies TMR0–TMR7. |

Historical cloud-execution infrastructure checkpoint:

| Capability | Completion | Status |
| --- | ---: | --- |
| FAST | 100% | GitHub-hosted Ubuntu 24.04, GCC 13 Debug; `main` PASS. |
| Major semantic regression | 100% | Final cloud-environment boundary regression PASS in run `35513567930`; merged tree is identical to the reviewed candidate tree. |
| INTEGRATION | 100% | GCC 13 Debug and Clang 18/libc++ Debug required checks PASS; closure audit recorded in `docs/audits/2026-09-20-cloud-integration-closure.md`. |
| QUALIFICATION environment | 100% | CQE0-CQE7 PASS; final candidate revalidation PASS in run `35513250315`, then squash-merged with identical tree. |

Historical TMR preparation and audit work-class allocation:

- Implementation: **25%**;
- Tests/validation: **45%**;
- Evidence/experiments: **10%**;
- Documentation/governance: **20%**.

These percentages describe the report-only tooling work distribution, not
scientific completion. The independent completion lanes above are the
authoritative stage-progress view.

Accepted public/cloud engineering audit: `PASS`, recorded in
`docs/audits/2026-09-20-public-cloud-baseline.md`. The accepted `main` revision
`c542a237d0bb08b9a2c9cba2ee731dd3b046afe0` passed FAST run `35510879988`
and four-cell Major Semantic Regression run `35510879978` without production
C++ change.

`main` is protected by active repository ruleset `23728711`
(`main-protection`). The ruleset targets the default branch, has no bypass
actors, requires pull requests, linear history, resolved review conversations,
and all three ordinary pre-merge checks — `GCC 13 Debug / FAST`,
`GCC 13 Debug / INTEGRATION`, and `Clang 18 libc++ Debug / INTEGRATION` — with
strict up-to-date branch semantics; it blocks deletion and non-fast-forward
updates. Allowed merge
methods are squash and rebase, with zero required approvals. GitHub currently
reports `require_extra_approval_for_unattributed_changes=true`; with zero
required approvals this setting has no effect on the present workflow.

Cloud INTEGRATION is accepted at **100%**. PR #11 was squash-merged to
`ae8ca45d861c27c643e022cb61318db5b22737f9`. Its tree
`781850bea922cadcb8a2eaa5168e15964f07e8f9` is byte-identical to the reviewed
PR head tree, so the final pre-merge Major Semantic Regression remains bound to
the merged functional content. Post-merge FAST run `35512303620` and
INTEGRATION run `35512303629` both passed; both INTEGRATION cells executed the
exact seven-test semantic inventory. The `main-protection` ruleset requires FAST plus
both INTEGRATION checks. Final major-boundary run `35512093405` passed all four
Debug/Release GCC/Clang cells with 7/7 semantic tests and no Node.js 20 checkout
warning after pinning `actions/checkout` v7.0.1 by commit SHA. The closure audit
is `docs/audits/2026-09-20-cloud-integration-closure.md`.

Cloud QUALIFICATION-environment admission is accepted at **100%**. CQE0-CQE7
passed in run `35513051098`; the final PR head
`6a934de6e8f6fae35e6c38ec45b9b1f23b170acb` was revalidated by the four-cell
Qualification Environment run `35513250315` and by Major Semantic Regression
run `35513567930`. The admitted envelope pins GitHub
runner image `ubuntu-24.04` version `20260907.300.1`, exact Ubuntu
compiler/library packages, and `/usr/bin` CMake 3.28.3 / Ninja 1.11.1. All
four Debug/Release GCC/Clang cells matched environment identity, discovered the
exact seven-test semantic inventory, kept qualification tooling OFF, and passed
7/7 semantic tests. The first run `35512991310` is retained as the single
mechanical `BLOCKED_BY_CMAKE_CACHE_TYPE_ASSERTION` attempt. Full audit:
`docs/audits/2026-09-20-cloud-qualification-environment-admission.md`.

PR #13 was squash-merged to `0a7095d431e4bea3c9c73e75d22df2e713c7a8ab`.
Its tree `7144943abc7ffd861b92587217a112c0edf6f9b4` is byte-identical to the final
reviewed PR head tree, so the final pre-merge Qualification Environment and Major
Semantic Regression results remain bound to the merged functional content.
Post-merge FAST run `35513658207` and INTEGRATION run `35513658197` both passed.

The cloud envelope is distinct from the historical WSL qualification envelope;
no equivalence is claimed. The smallest reusable report-only TMR0--TMR7 tooling
layer is implemented and its focused GCC/Clang Debug contracts passed in run
`35514834796`. The TMR protocol now explicitly accepts the admitted cloud
envelope through
`docs/decisions/TOPOLOGICAL_MODEL_CLOUD_QUALIFICATION_ENVIRONMENT_SUPPLEMENT.md`.
No manifest is prepared and no formal campaign is authorized. The next bounded
action is fail-closed cloud-identity binding in the existing preparation/runner
path, with focused contracts only.

## Session handoff

This repository is sufficient to resume the project without prior chat history.

Read in this order:

1. `docs/APMESH_CORE_STATE.md`;
2. `docs/APMESH_CORE_WORKLOG.md`;
3. `docs/APMESH_CORE_ROADMAP.md`;
4. `docs/decisions/SURFACE_ANALYTIC_SPHERE_DECISION.md`;
5. `docs/decisions/SURFACE_PRINCIPAL_CURVATURE_VALUES_DECISION.md`;
6. `docs/decisions/SURFACE_SECOND_ORDER_CURVATURE_DECISION.md`;
7. `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_ENTRY_DECISION.md`;
8. `docs/decisions/SURFACE_ANALYTIC_CYLINDER_DECISION.md`;
9. `docs/decisions/SURFACE_RECTANGULAR_TRIM_DECISION.md`;
10. `docs/decisions/SURFACE_COONS_PATCH_DECISION.md`;
11. `docs/decisions/SURFACE_BICUBIC_NURBS_DOUBLE_KNOT_CONTINUITY_DECISION.md`;
12. `docs/decisions/SURFACE_BICUBIC_NURBS_DECISION.md`;
13. `docs/decisions/SURFACE_RATIONAL_BICUBIC_BEZIER_DECISION.md`;
14. `docs/decisions/SURFACE_REPRESENTATION_ENTRY_DECISION.md`;
15. the latest relevant audit under `docs/audits/`;
16. verify live `main`, open PRs, ruleset `23728711`, branches and recent
    Actions before writing.

Historical Topological Model qualification documents remain authoritative for
their frozen claims but are no longer the active continuation documents.

Accepted functional cloud-infrastructure baseline:

- PR #13 merged as `0a7095d431e4bea3c9c73e75d22df2e713c7a8ab`;
- final reviewed PR tree and merged tree:
  `7144943abc7ffd861b92587217a112c0edf6f9b4`;
- final Qualification Environment revalidation: run `35513250315`, PASS;
- final phase-boundary Major Semantic Regression: run `35513567930`,
  four cells PASS, 7/7 semantic tests per cell;
- post-merge FAST: run `35513658207`, PASS;
- post-merge INTEGRATION: run `35513658197`, PASS;
- subsequent documentation checkpoint on `main` was also validated by FAST run
  `35513813548` and INTEGRATION run `35513813545`, both PASS.

Infrastructure status is closed at 100% for FAST, INTEGRATION, Major Semantic
Regression availability, and the admitted cloud QUALIFICATION environment.
Infrastructure status alone does not qualify a scientific stage; the formal
Topological Model qualification decision is recorded below.

Historical bounded-action snapshot (superseded by the authoritative checkpoint
at the top of this file):

**After the arbitrary-axis placement decision closure, implement the bounded
Right-Handed Arbitrary 3D Axis Placement work unit.**

Decision evidence:

- PR #173:
  `36381dec1f7af3a723fd386a3f55e0f109d804b1`;
- PR FAST `35894230229`: PASS;
- PR INTEGRATION `35894230134`: PASS;
- post-merge FAST `35894377748`: PASS;
- post-merge INTEGRATION `35894377875`: PASS.

Closure branch:
`docs/arbitrary-axis-placement-decision-closure`.

Decision authority:
`docs/decisions/SURFACE_ARBITRARY_AXIS_PLACEMENT_PREREQUISITE_DECISION.md`.

The sole future implementation is a separate right-handed `AxisPlacement3`.

`CartesianFrame3` remains unchanged and keeps its qualified exact
signed-permutation/power-of-two semantics.

No analytic elementary, revolution, trimming, Coons, NURBS,
surface-differential-geometry or meshing production is authorized.

## Historical active-stage snapshot (superseded)

The following snapshot is retained only for provenance of the earlier
arbitrary-axis placement transition. It is not current continuation authority.

**Surface Representation — Continuous Patch Geometry — ARBITRARY 3D
PLACEMENT PREREQUISITE DECISION INTEGRATED / HISTORICAL SNAPSHOT /
NOT QUALIFIED**

Paused prerequisite investigation:

**Curve Differential Geometry — Curvature, Regularity, and Features —
IN INVESTIGATION / POINTWISE CURVATURE INTEGRATED /
SIGNED PLANAR CURVATURE INTEGRATED /
SIMPLE-INFLECTION INTEGRATED /
FOCUSED CONTRACTS PASS / NOT QUALIFIED / PAUSED FOR REPRESENTATION-BREADTH
SEAM**

## Representation breadth retained limitation

The original qualified Curve Representation claim remains intentionally
narrow: CGR0–CGR7 qualifies only polynomial cubic Bézier curves.

Production has since been extended under separate focused, unqualified work
units with:

- `LineSegment2/3`;
- `RationalQuadraticBezier2/3`;
- oriented static trimmed subcurves;
- fixed two-span cubic B-spline;
- fixed and multi-span cubic positive-weight NURBS;
- cubic NURBS interior multiplicity one/two with parameter-local C1/D2
  semantics.

These extensions do **not** widen the frozen CGR qualification.

Curve breadth still retained for later decisions includes:

- dedicated analytic circular/general conic arc curve types;
- arbitrary-degree polynomial/rational Bézier;
- arbitrary-degree and periodic B-spline/NURBS;
- cubic multiplicity-three/C0 knot semantics;
- heterogeneous composite/polycurve semantics.

Surface Representation is also broader than the earlier snapshot. Integrated
focused work units now include:

- polynomial bicubic Bézier;
- positive-weight rational bicubic Bézier;
- bicubic NURBS including interior multiplicity one/two;
- oriented cubic Bézier Coons patch;
- static oriented rectangular trimming;
- cubic Bézier linear extrusion;
- cubic Bézier revolution;
- arbitrary right-handed 3D axis placement;
- bounded analytic plane;
- bounded analytic circular-cylinder sector.

Surface breadth still retained includes at minimum sphere, cone, torus,
full-periodic analytic semantics where admitted, broader NURBS degree/knot
breadth, and general arbitrary-loop trimming / p-curves / topological faces.

General trimming/topological identity and remaining curve breadth are separate
prerequisites for later CAD-like boundary certification; they are not silently
implied by the integrated focused families.

The current Surface Differential Geometry layer already includes regularity,
first fundamental form, area density, oriented normal, second fundamental form,
Gaussian curvature and mean curvature. Principal curvatures/directions and
conditioning policy remain unimplemented.

None of these later focused integrations invalidates or silently widens the
most recent qualified Curve Representation claim.

## Most recently qualified stage

**Curve Representation — Continuous Geometry Before Discretization —
QUALIFIED / CGR0–CGR7 PASS**

Qualification authority:

- protocol:
  `docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`;
- terminal audit:
  `docs/audits/2026-09-21-continuous-curve-geometry-regression-terminal-audit.md`;
- candidate:
  `f7dc8d82d881858b6481d6d2d1383d8a561684c5`;
- execution run: `35630423134`;
- terminal artifact: `10654358199`;
- terminal archive SHA-256:
  `a4b59453dcc9f9cacb4265ba540e1a3a443f3b6c80509411e0d6d4b18eff7aa6`.

CGR0–CGR7 independently PASS. Qualification is cloud-envelope-scoped and does
not establish WSL/cloud equivalence.

All admitted production work units remain integrated and frozen at semantic
baseline `438620efa1f93d29b442e9ba199882a09d2359d9`.

Current completed work unit:

**Polynomial Cubic Bézier Value Representation and Evaluation — IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

Current bounded decision:

**Cumulative Arc-Length Mapping and Certified Inverse Bracketing — DECISION
APPROVED / INTEGRATED / WORK UNIT 2A CLOSED / WORK UNIT 2B CLOSED /
FOCUSED CONTRACTS PASS / NOT QUALIFIED.**

Work Unit 2A provides conservative cumulative prefix-length evidence for
`S(t)` using private outward edge-vector de Casteljau construction and the
integrated certified total-length engine. PR #66 merged as
`3cfb580cae2e2d26e87e9dfcfeab0aade0a3a3be`; post-merge FAST
`35591774007` and INTEGRATION `35591773982` pass.

All admitted continuous-curve production work units are integrated and closed.
The formal CGR lifecycle has now completed and the independent terminal audit
records CGR0–CGR7 PASS. No additional Curve Representation semantic work is
authorized by this closure; the next scientific work is a separate Curve
Differential Geometry entry decision after audit integration/closure.



### Curve Representation stage-exit result

**Continuous Curve Geometry Regression — FIRST FORMAL EXECUTION CONSUMED /
TERMINAL AUDIT COMPLETE / CGR0–CGR7 PASS / OVERALL PASS / QUALIFIED.**

Formal authorization PR #83 merged as
`e8b17256924e907d0859b8ac7061600ffc404b9e`. Run `35630423134`
created the immutable claim
`cgr-execution-claim-201a38120ab3858f1621ab19041898d0eb7f8a915e32b9aa7e7354b7937531fd`
and executed exactly once.

The terminal package independently verifies 56 successful command records, 112
semantic test executions, eight validated certificates, complete negative and
dependency isolation evidence, nine derived-evidence files, lifecycle/sealing,
source identity and exact retention.

Per the pre-registered protocol, Curve Representation is therefore QUALIFIED
only in the admitted GitHub-hosted Ubuntu 24.04 x86_64 cloud environment.
The next stage is not physical discretization: Curve Differential Geometry
requires a separate scientific entry decision.

## Previously qualified prerequisite stage

**Topological Model — Explicit Identity and Incidence**

Current stage decision:

**Identity and Oriented Edge Incidence Kernel — IMPLEMENTED / FOCUSED CONTRACT
PASS / NOT QUALIFIED.** The bounded candidate implements only strong
`VertexId`/`EdgeId` semantics, explicit edge endpoints, oriented edge uses,
deterministic mutable construction, and atomic finalization into an immutable
topology model. Its focused CTest passed in WSL Ubuntu 24.04 GCC 13 Debug and
Clang 18/libc++ Debug. It authorizes no face/patch cycle, manifold or
non-manifold classification, coordinate welding, curve/surface association,
canonical serialization, qualification infrastructure, or campaign.

**Face Identity and Ordered Boundary Cycles — IMPLEMENTED /
FOCUSED CONTRACT PASS / NOT QUALIFIED.** The amended active decision
separates topological `FaceId` from future `PatchId`, defines one or more
ordered non-empty closed `EdgeUse` cycles with arbitrary positive valence, and
permits multiple loops without assigning outer/inner meaning. It admits
repeated-edge and arbitrary face-incidence inputs without making a manifold
claim. The focused GCC 13 Debug and Clang 18/libc++ Debug CTest passed for
strong identity, transactional rejection, exact oriented closure, valence 1,
2, 3, and 5, repeated edges, multiple loops, and arbitrary face incidence.
Curves, surfaces, trimming evaluation, geometric winding, shells, manifold
classification, meshing, and formal qualification remain excluded.

**Deterministic Edge-Use Incidence Enumeration — IMPLEMENTED / FOCUSED CONTRACT
PASS / NOT QUALIFIED.** Immutable factual records map
each stored `EdgeUse` occurrence back to
its `FaceId`, boundary-loop ordinal, use ordinal, and declared orientation. It
must preserve every repeated occurrence and deterministic traversal order
without inferring adjacency, pairing, boundary status, fan order, manifoldness,
geometry, or persistent loop/use identity.

**Deterministic Edge-Incidence Structural Classification — IMPLEMENTED /
FOCUSED CONTRACT PASS / NOT QUALIFIED.** The implementation derives only the
exact structural signature of each edge: occurrence count,
distinct face and boundary-loop counts, forward/reverse counts, repeated-owner
presence, and one of `unused`, `single_use`, `two_use_opposed`,
`two_use_cooriented`, or `multi_use`. These are combinatorial observations, not
boundary, adjacency, pairing, manifold, shell, or geometric conclusions. The
focused GCC 13 Debug and Clang 18/libc++ Debug CTest passed. The fifth bounded
contract below defines the authorized continuation.

**Immutable Topology Consistency and Canonical Snapshot — IMPLEMENTED /
FOCUSED CONTRACT PASS / NOT QUALIFIED.** Finalization now revalidates the
complete authoritative topology and its derived reverse incidence relation
atomically. The immutable model exposes a deterministic count-only summary and
the forward-only canonical `apmesh-topology-v1` snapshot. Focused GCC 13 Debug
and Clang 18/libc++ Debug CTests passed. This adds no repair, deserialization,
adjacency, pairing, manifold policy, geometry, or embedded cryptography.

**Topological Model Cumulative Regression — SECOND FORMAL
EXECUTION CONSUMED / TERMINAL AUDIT COMPLETE / TMR0–TMR7 PASS / OVERALL PASS /
QUALIFIED.** The stage-exit protocol remains fixed in
`docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md`.

The first formal campaign remains immutable historical negative evidence:
run `35528077223` completed operationally, but its terminal audit recorded
TMR0–TMR5 PASS and TMR6–TMR7 BLOCKED because only four semantic CTest records
were retained instead of the required eight. That consumed attempt was not
reinterpreted or rescued.

The bounded diagnosis and correction established the prospective exact
four-cell/two-repetition execution shape of 56 command records. The corrected
second PREPARED package was produced in run `35531261000` for candidate
`37f9af77f38e12af0a92d3c0f57f1ad31a218144` and independently audited
PASS / PREPARED / NOT EXECUTED.

PR #43 merged the exact second manifest-bound `EXECUTE_ONCE` authorization as
`cddd959574ed6a677ac755a5b329d53a9cfe32ec`. Run `35533702004`
validated the full authorization commit, machine-readable preparation audit,
artifact provenance and PREPARED binding; created immutable claim
`tmr-execution-claim-f43da78df89814a7baab5bf962054fb11ca7f407cf6e711cbff7148a15bae5fa`;
and executed exactly once.

Terminal artifact `10612032787` has independently recomputed archive
SHA-256
`dd5c12f54ed60a106059a9f42acf5c07884fa83230ae4dfe81d073cb6f7111b8`.

Independent audit in
`docs/audits/2026-09-20-topological-model-tmr-corrected-terminal-audit.md`
records:

- TMR0 identity/scope PASS;
- TMR1 construction/immutability PASS;
- TMR2 incidence bijection PASS;
- TMR3 structural recomputation PASS;
- TMR4 canonical snapshot PASS;
- TMR5 repeat/cross-cell equivalence PASS;
- TMR6 prerequisite preservation/isolation PASS;
- TMR7 evidence integrity/closure PASS;
- overall PASS.

The corrected terminal package retains exactly 56 successful command records,
112 planned/retained command logs, eight exact semantic CTest records with 7/7
PASS in every repetition, eight byte-identical certificates, complete
negative/dependency inventories, detached verification and exact retention.

Per protocol decision effect, the Topological Model stage is therefore
**QUALIFIED only in the exact formally admitted GitHub-hosted Ubuntu 24.04
x86_64 cloud envelope**. This establishes no WSL/cloud equivalence and expands
no claim beyond the declared topology contracts/nonclaims.

The next scientific transition is not curve implementation itself. After this
audit is integrated and closed, one separate entry decision may open Curve
Representation — Continuous Geometry Before Discretization.


Current prerequisite closure evidence:

**Geometry Primitives: QUALIFIED / GPR0-GPR7 PASS.** The bounded GPR0-GPR7
protocol qualifies only the joint
Point/Vector, Minimal Small Linear Algebra, and Cartesian Similarity Frames
capability already integrated in post-merge `main` baseline
`ca20ad64cd0dd14b869225dae73401ad2b93464c`. It authorizes no new production
type or behavior. The reusable profile, experimental integrated exporter, and
report-only validator/collector pass focused GCC 13 Debug and Clang 18 libc++
Debug contracts. The revision-bound runner seals preparation, consumes one
manifest exactly once, retains either terminal outcome, and leaves every gate
pending scientific audit. Its focused contracts cover immutable preparation and
retained synthetic success and partial failure. The first external manifest,
`a890b988559eb041c63104a45e0edbd66d6a9c07fdb912a45896bbe9ded81711`, is
immutable `BLOCKED_BY_STALE_LIMITATION_CONTRACT`: it was never executed because
its sealed limitations contradicted the `PREPARED` state. The correction is
limited to the profile contract and remains immutable. The second external
manifest, `ad999a236cf68f537dd5d6bc159885c2e9404a50e824622f867ac24df59b9aa2`,
is immutable `BLOCKED_BY_CTEST_DISCOVERY_PARSER_DEFECT`: configure, build, and
CTest discovery succeeded, but a spacing-sensitive parser rejected the valid
CTest listing before semantic tests or certificates ran. The correction is
limited to runner tooling; a new manifest is required and execution remains
unauthorized.

The third external manifest,
`16bac9f16868ccd98c3a437cb524098fa0671b86a60d532010a837ebcb14e9bf`, is
immutable `BLOCKED_BY_EVIDENCE_OUTPUT_DIRECTORY_DEFECT` evidence. Its GCC
Debug configure, build, CTest discovery, and semantic CTest commands succeeded,
but the first certificate producer could not open its output because the runner
had not created `certificates/`. The correction is limited to runner output
directory initialization; no certificate, gate, or component qualification was
reached. A new manifest is required and execution remains unauthorized.

Current correction status: a fourth external manifest,
`f36a8fa1801eca6f992d7cb46f11b34c9d088abec8df769568cf3bb7f41dff5b`, bound
to clean candidate `8811a00`, is immutable
`BLOCKED_BY_SEMANTIC_CTEST_REGEX_DEFECT` evidence. It retained twelve
cross-cell-equivalent certificates, negative evidence, and detached retention,
but all four semantic CTest commands used an unsupported `(?:...)` group. CTest
selected no tests while returning zero; consequently GPR6 prerequisite
preservation and GPR7 closure are not evidenced. This is a runner defect, not
a production or component-qualification regression.

The fifth external manifest,
`a39069569625bbc16637d2078bddb2e91c654840782ad0c4026fc9b75ef42718`, bound
to clean published candidate `2f22ffd`, executed once and passed the separate
GPR0-GPR7 audit. All four GCC/Clang Debug/Release cells ran the exact sealed
six-test allowlist and produced three certificates each. The 52 command
records, 12 certificate hashes, 20 negative rejections, dependency/runtime
inventories, and detached retention verification all passed. Geometry Primitives
is QUALIFIED only in the declared WSL Ubuntu 24.04 envelope.

Cartesian Similarity Frames is **QUALIFIED / CF0–CF7 PASS / candidate
`8e6b5872072bb8077731b5879d1c5b751e375087` / WSL Ubuntu 24.04 GCC 13 and
Clang 18 libc++.** `CartesianFrame2` and `CartesianFrame3` remain bounded to
an exact signed-permutation basis, finite origin, and positive power-of-two
scale. The replacement revision-bound execution produced twelve identical
semantic certificates across four compiler/build cells and three repetitions,
with 1,696 unique exact cases per certificate, exact prerequisite preservation,
and a detached-verified 168-file canonical retention package. The earlier
`e69e804` package remains immutable
`BLOCKED_BY_COMPILE_COMMANDS_SCHEMA_DEFECT` evidence; it is not a defect in
Cartesian Frames behavior and was not overwritten. This qualification covers
only matching point/vector local-world mappings within the declared envelope;
it does not authorize general transformations, tolerance-based validation,
inversion, predicates, topology, curves, surfaces, or meshing. The entry
decision and fixed qualification record are in
`docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md`
and `docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`.

PR #5 was squash-merged into `main` at
`12afa394af24c8f0627b12e3697010735826a9a1`. Its tree is identical to reviewed
source head `5c96593780b88f9e297253e600ccf5b5f2ba1c95`; both resolve to tree
`8597575861c4573c850fb6e30e193e610674ce5c`. The source branch was removed only
after that identity was verified.

PR #6 was squash-merged into `main` at
`ca20ad64cd0dd14b869225dae73401ad2b93464c`. Local `main`, `origin/main`, and
`HEAD` were aligned at that commit with clean tree
`01154f434aaffe8e2b5b57710f21a38efbe95d8d`. The merge added the bounded
Cartesian Frames qualification record and evidence infrastructure; it did not
change the already integrated production implementation.

Minimal Small Linear Algebra remains `QUALIFIED` on clean published candidate
`3804e903f56a226d38319fd44255b5832639815d` within the declared WSL Ubuntu
24.04 GCC 13 / Clang 18 libc++ envelope. Its LA-only integration regression on
`95258e9faaeaae89e801874fe1ce600e80285427` passed before its reviewed squash
merge into `main` as `ca51c333b5fcae33f05b5e25f6b0780ec195ad77`.

Foundation is `QUALIFIED` at 100% within the declared WSL Ubuntu 24.04
envelope. Bounded point/vector geometry is implemented and has passed its
focused four-cell contract; meshing remains unimplemented.

PR #3 was squash-merged into `main` at
`1ff6568c908ea144b903a70a2497c000a89e35eb`. Its tree is identical to the
reviewed source head `b17067312b523e934c81d56b6cde7948f30ff93f`; both
resolve to tree `bea6ffaa71877811daa98d1f69a299446b12f401`. The source
branch was removed only after that identity was verified. Point and Vector
Semantics therefore remains qualified and is now integrated into the canonical
`main` history.

Geometry Primitives entry is approved by
`docs/decisions/GEOMETRY_PRIMITIVES_ENTRY_DECISION.md`. The authorized first
implementation is limited to distinct `Point2`, `Point3`, `Vector2`, and
`Vector3` value semantics, finite construction, affine/Euclidean operations,
explicit failures, and focused analytic evidence. Matrices, transforms,
predicates, topology, curves, surfaces, and meshing remain blocked. The focused
contract passed on GCC/Clang Debug/Release. The revision-bound Point/Vector
qualification protocol is recorded. Its first formal attempt on
`b7f8fe9` stopped in `gcc-debug` because the PV6 selector included a historical
Foundation preflight self-test rather than only the required Architecture,
Numeric, and Reproducible Experiment contracts. The terminal evidence is
retained as `BLOCKED_BY_CONTRACT_SELECTION_DEFECT`; the complete stage remains
unqualified.

The corrected exact preservation selector was executed once on clean published
candidate `ededf6584941de9dfe6a47633ffd67caf17d4f51`. The formal matrix
completed four GCC/Clang Debug/Release cells with three independent Point/Vector
processes per cell. All twelve certificates were semantically identical across
the matrix; every Foundation-preservation CTest set passed; negative evidence
rejected duplicate cases, unsafe compiler flags, and forged proximity values;
and detached retention verification passed. The separate audit accepted
PV0–PV7. **Point and Vector Semantics is QUALIFIED** only in the declared WSL
Ubuntu 24.04 GCC 13 / Clang 18 libc++ envelope. The full Geometry Primitives
stage remains `IN INVESTIGATION`; no matrix, transform, predicate, topology,
curve, surface, or meshing capability is authorized.
The canonical retained evidence package is
`evidence/geometry-primitives/point-vector/pv0-pv7-ededf65/`; it excludes
scratch builds, caches, and reproducible binaries while preserving hash-bound
manifests, certificates, inventories, reports, negative evidence, and focused
logs.

Foundation End-to-End closed with FND0–FND7 `PASS` on clean published candidate
`b333755442b934c490abaecda886dd2a40e981ca`. The report-only qualifier produced
revision-bound evidence; the separate audit made the scientific closure
decision. Historical REC evidence at `85d215a` remained a qualified comparison
baseline and did not substitute for current-candidate FND2, FND4, or FND6.
The original 586-file source manifest is preserved in the canonical Foundation
package at `evidence/foundation/foundation-end-to-end/fnd0-fnd7-b333755/`.
The canonical package retains 560 non-rebuildable files; its manifest SHA-256
is `ce873c59e663a570773469639a96e1f75587954adda0064d1a9d775773064040`.
The hash-bound exclusion inventory records the 28 reproducible blobs
(1,252,808 bytes) with their path, size, SHA-256, command, and provenance.

The retained limitations are unchanged: qualification is restricted to WSL
Ubuntu 24.04 and makes no native-Windows, geometry, topology, meshing,
convergence, performance, parallel-equivalence, or universal-portability claim.

The Architecture Contract is QUALIFIED for WSL Ubuntu 24.04 and is integrated
into `main` at `4927383`. All eight requirements passed the final audited
four-cell protocol.

NQ-R1 preserved `numeric.cpp` and collected complete passing evidence for N0,
N1, and N3–N7 on clean candidate
`74fede5ae5999580f2ef76e944cf61e334f44064`. N2 remains blocked because the
pre-registered minimum/maximum finite fixture class does not explicitly classify
`std::numeric_limits<double>::min()` or `lowest()`. No numeric implementation
defect has been demonstrated. The contract still separates topological
identity, exact equality, numeric proximity, certified predicate sign, and
scientific acceptance.

NQ-R2 added explicit `min()` and `lowest()` classification evidence only in the
focused test, report-only certificate, and independent oracle; `numeric.cpp`
and the public API remained unchanged. Its revision-bound four-cell regression
on candidate `236d290a20227f0abd646073499c0d3e20a19f8e` passed the formal audit:
N0--N7 all pass. The Numeric Contract is `QUALIFIED` inside the declared WSL
Ubuntu 24.04 GCC 13/Clang 18 envelope.

Evidence status: the revision-bound NQ-R1 manifest identified as
`apmesh-core-nq-r1-531d0795e1a94e1e9f43a99a578f63ce/manifest.json`
with SHA-256
`bcc39af9b75a7bd6fe1a0b02607f617372dd9bd62d8014ea69d31a097eed2a9f`
was audited as N0/N1/N3–N7 `PASS` and N2 `BLOCKED`. The earlier N0/N1 `PASS`,
N2–N7 `BLOCKED` decision remains preserved as negative historical evidence.

Scientific qualification: the Architecture, Numeric, and Reproducible Experiment
Contracts are `QUALIFIED` in the declared WSL Ubuntu 24.04 envelope. The
Foundation End-to-End Regression passed FND0–FND7 jointly on clean published
candidate `b333755442b934c490abaecda886dd2a40e981ca`; Foundation is
`QUALIFIED` at 100% with retained scope limitations.

## Bootstrap repository and toolchain

The dedicated local `apmesh-core` repository was initialized on 2026-09-04.
Its bootstrap documents were migrated from
`tiagosombrra/adaptive-patch-meshing`, branch
`research/apmesh-core-bootstrap`, commit
`01c4c1d614979350e77632b4c55fd497a7dc51f6`. No legacy implementation source
was copied.

The local language/standard-library probe is qualified on WSL Ubuntu 24.04:

- primary reference: GCC 13.3.0 with libstdc++;
- secondary qualification: Clang 18.1.3 with libc++ 18.1.3;
- CMake available locally: 3.28.3;
- Ninja available locally: 1.11.1.

Both compilers compiled and executed a C++23 `std::expected` probe with
`-Wall -Wextra -Wpedantic -Werror`.

The minimal `apmesh::core` library skeleton is qualified on the GCC reference:

- clean CMake configure with preset `gcc-debug`;
- Ninja build of `apmesh_core` and `apmesh_core_bootstrap_smoke`;
- CTest discovery of one `bootstrap`-labelled test;
- one passing `apmesh_core.bootstrap_smoke` execution.

The same project bootstrap and bounded Architecture Contract regression are
qualified on Clang 18.1.3 with libc++ 18.1.3, including Debug and Release.
The Reproducible Experiment Contract and Foundation End-to-End Regression are
qualified. Later scientific stages remain open. Native Windows remains NOT
QUALIFIED; WSL execution does not qualify it.

## Architecture direction already agreed

Intended dedicated library/repository name: `apmesh-core`.

Public namespace: `apmesh`.

Planned logical modules (not implemented by the current bootstrap):

- `base`
- `math`
- `geometry`
- `topology`
- `model`

Future modules are introduced only when scientifically required; candidate later modules include differential geometry, sizing, boundary discretization, meshing, adaptation, certification, and I/O adapters.

Initial build structure should prefer one library target with logical module boundaries. Separate linker targets are introduced only when independent reuse or dependency control justifies them.

Core algorithms must not perform file I/O, logging, plotting, or environment-dependent discovery.

## Required reading order for continuation

1. `docs/APMESH_CORE_STATE.md`
2. `docs/APMESH_CORE_WORKLOG.md`
3. `docs/APMESH_CORE_ROADMAP.md`
4. `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_POST_TOOLING_DECISION.md`
5. `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md`
6. `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_READINESS_DECISION.md`
7. `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_ENTRY_DECISION.md`
8. `docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md`
9. `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
10. `docs/research/REFERENCE_REGISTER.md`.

## Next admissible actions

The completed Topological Model campaign is `QUALIFIED` only in its declared
hosted Ubuntu 24.04 envelope. Surface Representation and Surface Differential
Geometry remain **IN INVESTIGATION / NOT QUALIFIED**.

The Surface Differential Geometry post-tooling continuation decision is
integrated as PR #243, merge commit
`e53a9797e1603d451819c814c32baeba1794efc8`. Its head
`c080b789241a86f3d5c4efc05f4d6e5f1ba50c6f` passed PR FAST run `36469493920`
and INTEGRATION run `36469493886` (GCC 13 Debug and Clang 18/libc++ Debug).
Post-merge FAST `36471479311` and INTEGRATION `36471479336` passed on the merge
commit.

The active work item is the separate documentation-only closure of that
decision. Until the closure itself is integrated and its post-merge checks pass,
do not begin the selected **Fail-Closed Surface Differential Geometry
Qualification Preparation Infrastructure**. The scientific decision defers
principal directions, cone, torus and general trimming.

No implementation, real PREPARED package, formal execution, protocol amendment
or qualification status has been authorized by the current closure work item.
The metric-conditioning maintenance and report-only tooling integration remain
closed, with the 32 frozen semantic blobs unchanged at integration.

## Stage closure protocol

Before closing any stage:

1. verify all investigation problems are resolved or explicitly blocked;
2. verify required literature/decision records exist;
3. run the stage-specific regression;
4. rerun all relevant prerequisite regressions;
5. regenerate declared figures/tables/certificates;
6. classify every difference;
7. write the stage decision/closure record;
8. update roadmap and this continuation state;
9. only then mark the stage `QUALIFIED` and authorize the next stage.

## Repository transition note

Bootstrap documentation was migrated into this dedicated greenfield repository
with immutable source revision provenance in `docs/BOOTSTRAP_PROVENANCE.md`.
The legacy implementation remains a reference repository and must not be mixed
into greenfield source.
