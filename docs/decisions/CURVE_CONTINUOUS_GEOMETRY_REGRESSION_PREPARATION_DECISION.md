# Continuous Curve Geometry Regression — Formal PREPARED Lifecycle Decision

Status: **APPROVED FOR DESIGN INTEGRATION / NO PREPARED PACKAGE / NO EXECUTION AUTHORIZED**  
Date: 2026-09-21  
Stage: **Curve Representation — Continuous Geometry Before Discretization**  
Protocol:
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`  
Frozen semantic baseline:
`438620efa1f93d29b442e9ba199882a09d2359d9`

## 1. Question

How must one formal Continuous Curve Geometry Regression candidate be prepared,
sealed and independently auditable so that a later one-shot execution consumes
exactly the pre-registered CGR0–CGR7 plan without allowing execution logic,
candidate identity, scientific semantics, environment identity or retained
evidence requirements to change after preparation?

## 2. Decision boundary

This decision defines the formal campaign infrastructure and PREPARED lifecycle
only.

It does not:

- create a PREPARED package;
- authorize formal execution;
- create an execution claim;
- execute any CGR command;
- decide CGR0–CGR7;
- qualify Curve Representation; or
- modify any Section 4 frozen semantic file.

The report-only runner remains report-only and keeps exposing no formal
`prepare` or `execute` command.

## 3. Architecture

The formal campaign uses a separate formal runner:

`tools/run_continuous_curve_geometry_campaign.py`

The formal runner must reuse the integrated report-only planning authority from:

`tools/run_continuous_curve_geometry_regression.py`

instead of silently defining a second scientific matrix or semantic allowlist.

The formal runner may adapt report-only root placeholders to the PREPARED output
root, but the following values must remain exactly equivalent to the
report-only plan:

- four admitted cells;
- two repetitions per cell;
- configure once per cell;
- build/discovery/semantic CTest/certificate/certificate-validation once per
  repetition;
- negative evidence, dependency inventory and runtime inventory once per cell;
- 56 command records;
- 112 command logs;
- eight discovery records;
- eight semantic CTest records;
- fourteen tests per semantic repetition;
- 112 individual semantic test executions; and
- eight certificate slots.

A focused contract must compare the formal plan to the report-only plan and fail
closed on any scientific-plan drift.

## 4. Candidate identity and cleanliness

A formal PREPARED candidate must satisfy all of the following:

1. checkout is on protected canonical `main`;
2. `HEAD` equals the workflow SHA;
3. the local upstream commit equals `HEAD`;
4. the worktree is completely clean, including untracked files;
5. the candidate is a descendant of semantic baseline
   `438620efa1f93d29b442e9ba199882a09d2359d9`;
6. all eleven protocol-frozen semantic files are byte-identical to that
   baseline;
7. the complete tracked-source inventory is retained with path and SHA-256;
8. the normalized tracked path set is retained and independently recomputable;
9. no candidate identity is inferred from branch name alone.

The preparation workflow must run in a fresh GitHub-hosted checkout and place
the PREPARED output outside the repository worktree.

## 5. Exact preparation inputs

The PREPARED manifest must hash and bind at least:

- `experiments/profiles/continuous_curve_geometry_regression.json`;
- `docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`;
- `experiments/continuous_curve_geometry_regression_export.cpp`;
- `tools/continuous_curve_geometry_regression_evidence.py`;
- `tools/continuous_curve_geometry_regression_negative.py`;
- `tools/run_continuous_curve_geometry_regression.py`;
- `tools/run_continuous_curve_geometry_campaign.py`;
- `tools/experiment_runtime.py`;
- `CMakeLists.txt`;
- `experiments/profiles/cloud_qualification_environment.json`;
- `tools/cloud_qualification_environment.py`;
- `docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md`;
- `docs/audits/2026-09-20-cloud-qualification-environment-admission.md`;
- `.github/workflows/continuous-curve-geometry-regression-prepare.yml`;
- `.github/workflows/continuous-curve-geometry-regression-execute.yml`;
- `.github/workflows/continuous-curve-geometry-regression-authorize.yml`;
- `tools/continuous_curve_geometry_regression_authorization.py`; and
- this preparation decision.

The preparation implementation may bind additional focused contract files, but
it may not omit a file that can alter candidate selection, plan construction,
artifact validation, authorization or formal execution.

## 6. Admitted cloud environment binding

Preparation must validate every one of the four CGR matrix cells against the
already admitted cloud environment profile.

The environment binding must require:

- exact runner image identity;
- exact architecture;
- exact compiler/library package identities;
- exact CMake/Ninja paths and package versions; and
- PASS observation for each of the four cells.

The cloud admission profile's historical seven-test semantic allowlist is
environment-admission evidence and is **not** the CGR semantic allowlist.

Therefore the formal CGR environment binding must require equality of the
declared matrix cells and environment/tool identity, but must not incorrectly
require the cloud admission profile's seven-test list to equal the CGR
fourteen-test list.

The formal semantic allowlist remains governed solely by the CGR profile and
protocol.

## 7. Formal runner commands

The formal campaign runner must expose exactly the lifecycle commands required
for preparation and later execution:

- `self-check`;
- `prepare`;
- `validate-prepared`;
- `execute`; and
- `verify-retention`.

The existence of `execute` does not authorize execution. Its implementation
must exist **before preparation** so its exact hash is sealed into the PREPARED
manifest.

The preparation workflow may invoke only `prepare` and
`validate-prepared`.

The reusable execution workflow may invoke `execute` exactly once only after
a separately merged repository-resident authorization record.

## 8. PREPARED package

A successful preparation retains exactly the preparation/control package:

1. `profile.json`;
2. `plan.json`;
3. `planned-inventories.json`;
4. `prepared-manifest.json`;
5. `preparation-seal.json`;
6. `state.json`; and
7. `state-history.jsonl`.

No execution artifact may exist in a PREPARED archive.

At minimum the following must be absent:

- `execution-claim.json`;
- `command-records.json`;
- `certificate-index.json`;
- `per-cell-comparisons.json`;
- `cross-cell-comparison.json`;
- `observed-inventories.json`;
- `derived-evidence.json`;
- `gate-summary.json`;
- `terminal-manifest.json`;
- `failure.json`; and
- `retention-manifest.json`.

## 9. PREPARED manifest

`prepared-manifest.json` must use a closed schema and bind at least:

- schema/kind;
- lifecycle state `PREPARED`;
- `execution_requested=false`;
- exact candidate and upstream identity;
- complete tracked-source inventory;
- frozen semantic baseline and per-file frozen hashes;
- exact preparation input hashes;
- exact environment/tool identity;
- four-cell admitted cloud observations;
- working/output-root provenance;
- command/process timeout limits;
- exact fourteen-test semantic allowlist;
- exact formal launch plan;
- planned retention inventory;
- CGR0–CGR7 all `NOT_EXECUTED`;
- retained limitations/nonclaims; and
- preparation timestamp as provenance only.

Any missing, extra or inconsistent scientific/control field fails closed.

## 10. Plan and planned retention

`plan.json` must be the formal representation of the already pre-registered
report-only plan.

`planned-inventories.json` must enumerate every file required by either a
successful or blocked terminal campaign, including at least:

Preparation/control:

- the seven PREPARED files;
- future `execution-claim.json`;
- `command-records.json`;
- `terminal-manifest.json`;
- `detached-verification.json`;
- `retention-manifest.json`.

Per-command evidence:

- exactly 112 stdout/stderr command logs.

Scientific evidence:

- exactly eight curve certificates;
- certificate index;
- per-cell repeat comparisons;
- cross-cell comparison;
- four negative-evidence artifacts;
- four retained compile-command inventories;
- observed discovery/dependency/runtime inventories;
- deterministic derived-evidence manifest;
- four CSV files;
- four SVG files;
- gate-summary JSON and Markdown.

Failure closure:

- `failure.json` when terminal state is BLOCKED.

The planned inventory must distinguish `always`, `executed`,
`success` and `failure` requirements so terminal retention can be verified
without accepting partial evidence as success.

## 11. Preparation seal

`preparation-seal.json` must hash the immutable preparation inputs retained in
the PREPARED package and the initial state-history record.

At minimum it seals:

- `profile.json`;
- `plan.json`;
- `planned-inventories.json`;
- `prepared-manifest.json`; and
- the initial `state-history.jsonl` PREPARED record.

Any later PREPARED validation or execution preflight must recompute and match
this seal before claim creation.

## 12. Lifecycle state

The only preparation-time lifecycle state is:

`PREPARED`

A later authorized execution may transition:

`PREPARED -> RUNNING -> EXECUTED_PENDING_AUDIT`

or:

`PREPARED -> RUNNING -> BLOCKED`

The runner may never write `PASS` or `QUALIFIED`. Those are independent
terminal-audit decisions.

Once `execution-claim.json` or the immutable remote claim exists, the attempt
is consumed.

## 13. Formal preparation workflow

The preparation workflow is:

`.github/workflows/continuous-curve-geometry-regression-prepare.yml`

It must:

1. expose `workflow_dispatch` only;
2. require `refs/heads/main`;
3. checkout full history at the exact dispatch SHA;
4. require clean candidate/upstream equality;
5. install the exact admitted cloud packages;
6. write the PREPARED output only below `runner.temp`;
7. invoke formal runner `prepare`;
8. invoke formal runner `validate-prepared`;
9. independently assert `PREPARED`, `execution_requested=false`, and
   CGR0–CGR7 all `NOT_EXECUTED`;
10. independently assert absence of execution/terminal evidence;
11. upload exactly one immutable 90-day artifact named
    `cgr-prepared-<candidate-sha>`;
12. never invoke `execute`; and
13. use pinned GitHub Actions.

The preparation workflow does not authorize execution.

## 14. Future execution and authorization infrastructure

The infrastructure implementation that precedes formal preparation must also
include:

- reusable-only
  `.github/workflows/continuous-curve-geometry-regression-execute.yml`;
- protected-main controller
  `.github/workflows/continuous-curve-geometry-regression-authorize.yml`;
- closed-schema authorization validator
  `tools/continuous_curve_geometry_regression_authorization.py`.

The controller must require the complete authorization commit to add exactly one
manifest-hash-named authorization record and no other path.

The authorization record namespace is:

`experiments/authorizations/continuous-curve-geometry-regression-<prepared-manifest-sha256>.json`

The reusable executor must revalidate:

- committed authorization;
- exact artifact metadata/digest/provenance;
- exact candidate;
- prepared-manifest SHA-256;
- preparation-seal SHA-256;
- complete PREPARED binding; and
- absence of a prior immutable manifest-hash claim

before creating the claim and invoking `execute` exactly once.

None of those files authorizes execution until the exact authorization record is
later merged to protected `main`.

## 15. Focused infrastructure contracts

Before a formal PREPARED package may be created, focused contracts must prove:

1. formal plan equals the report-only plan scientifically and cardinally;
2. formal runner self-check reports no execution;
3. PREPARED success contains exactly the seven preparation files;
4. all CGR gates remain `NOT_EXECUTED`;
5. source inventory and frozen semantic hashes are bound;
6. cloud drift fails preparation;
7. malformed/mutated PREPARED state fails validation;
8. a consumed PREPARED package cannot validate as unconsumed;
9. synthetic successful execution yields exactly 56 command records, 112 logs,
   eight discoveries, eight semantic CTest records, eight certificates and the
   complete derived-evidence inventory;
10. synthetic mid-campaign failure fails closed and retains only the declared
    blocked terminal set;
11. retained success/failure packages are immutable after claim;
12. preparation workflow has no execute path;
13. reusable executor has exactly one execute invocation and no direct trigger;
14. authorization controller uses complete-commit isolation;
15. all action versions are pinned.

These are tooling contracts only and make no CGR0–CGR7 decision.

## 16. Independent PREPARED audit

After one PREPARED artifact is produced, a separate audit work item must
independently verify:

- workflow run provenance;
- artifact ID, name, size and SHA-256;
- exact seven-file PREPARED archive;
- absence of execution/terminal evidence;
- prepared-manifest and preparation-seal hashes;
- all sealed file hashes;
- lifecycle/state history;
- exact candidate/upstream/clean identity;
- complete tracked-source inventory against the GitHub tree;
- all frozen semantic files against baseline;
- every critical input hash against the exact candidate;
- all four cloud environment observations;
- exact 56-command / 112-log planned shape;
- exactly eight planned semantic CTest repetitions and 112 planned individual
  semantic tests;
- eight certificate slots;
- complete planned derived evidence;
- CGR0–CGR7 all `NOT_EXECUTED`.

Only a PREPARED audit decision of:

**PASS / PREPARED / NOT EXECUTED**

may permit a later authorization-binding/record phase.

## 17. Transition rule

After this decision is integrated and closed, the next bounded work item is the
formal campaign infrastructure implementation described above.

That infrastructure work must still create no formal PREPARED package and no
`EXECUTE_ONCE` record.

After infrastructure integration and closure, exactly one formal preparation may
be dispatched on canonical `main`, followed by independent PREPARED audit.

Formal execution remains unauthorized until that later audit is integrated and
a separate exact authorization record is merged.

## 18. Formal infrastructure implementation result

The formal campaign infrastructure defined by this decision is implemented on
`curve/cgr-formal-campaign-infrastructure`.

Implemented files:

- `tools/run_continuous_curve_geometry_campaign.py`;
- `.github/workflows/continuous-curve-geometry-regression-prepare.yml`;
- `.github/workflows/continuous-curve-geometry-regression-execute.yml`;
- `.github/workflows/continuous-curve-geometry-regression-authorize.yml`;
- `tools/continuous_curve_geometry_regression_authorization.py`;
- `.github/workflows/continuous-curve-geometry-regression-formal-tooling.yml`;
- focused runner/workflow/authorization contracts registered through
  `CMakeLists.txt`.

The formal runner imports the integrated report-only runner as its scientific
plan authority. The focused formal contract requires command-ID equivalence and
the exact pre-registered 56-command / 112-log / eight-certificate /
112-semantic-test cardinality.

The synthetic lifecycle contract proves:

- PREPARED contains exactly seven control files;
- cloud drift and PREPARED mutation fail closed;
- consumed PREPARED state cannot validate as unconsumed;
- successful synthetic execution retains exactly 56 command records,
  112 logs, eight discoveries, eight semantic CTest records, eight
  certificates and complete deterministic derived evidence;
- a second-repetition semantic failure closes BLOCKED before later scientific
  work;
- successful and blocked terminal packages remain byte-immutable after claim.

The authorization infrastructure uses a closed schema, machine-readable
PREPARED-audit binding, complete-commit isolation, manifest-hash claim identity
and exact GitHub artifact provenance before download.

Validation:

- formal tooling run `35613409036`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug;
- synchronized documentation-head run `35613745665`: PASS in both declared
  tooling cells.

Audit authority:

`docs/audits/2026-09-21-continuous-curve-geometry-regression-formal-infrastructure-audit.md`

This implementation result creates no real PREPARED package and authorizes no
formal execution.

After integration and checkpoint closure, exactly one formal preparation may be
dispatched from canonical clean `main`; the resulting package must then stop
for independent PREPARED audit.
