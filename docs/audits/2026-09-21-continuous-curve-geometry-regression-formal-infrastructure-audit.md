# Continuous Curve Geometry Regression — Formal Infrastructure Audit

Status: **PASS / VALIDATED_UNMERGED / NO FORMAL PREPARATION / NO EXECUTION**  
Date: 2026-09-21  
Branch: `curve/cgr-formal-campaign-infrastructure`  
Base: `28d18d88c4c3a7a199a7657aebb79e7c20b79524`

## 1. Scope

This audit verifies that the formal Continuous Curve Geometry Regression
campaign infrastructure implements the integrated PREPARED lifecycle decision
without modifying scientific curve semantics or executing a formal campaign.

Authority:

`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md`

No CGR0–CGR7 decision is made here.

## 2. Scientific isolation

The branch changes infrastructure/tooling only.

None of the eleven Section 4 frozen semantic files changed:

- `include/apmesh/geometry/curve.hpp`;
- `src/geometry/curve.cpp`;
- `src/geometry/detail/curve_length_interval.hpp`;
- `src/geometry/detail/curve_regularity_interval.hpp`;
- the seven focused curve test files frozen by the protocol.

The semantic baseline remains:

`438620efa1f93d29b442e9ba199882a09d2359d9`

The formal runner reuses the integrated report-only plan authority rather than
defining an independent matrix or allowlist.

## 3. Implemented infrastructure

| Requirement | Implementation |
| --- | --- |
| Formal lifecycle runner | `tools/run_continuous_curve_geometry_campaign.py` |
| Report-only plan authority reuse | `tools/run_continuous_curve_geometry_regression.py` imported by formal runner |
| Preparation-only workflow | `.github/workflows/continuous-curve-geometry-regression-prepare.yml` |
| Reusable one-shot executor | `.github/workflows/continuous-curve-geometry-regression-execute.yml` |
| Protected-main authorization controller | `.github/workflows/continuous-curve-geometry-regression-authorize.yml` |
| Closed authorization validator | `tools/continuous_curve_geometry_regression_authorization.py` |
| Formal infrastructure validation | `.github/workflows/continuous-curve-geometry-regression-formal-tooling.yml` |
| CMake opt-in registration | `CMakeLists.txt` |
| Formal runner focused contract | `tests/continuous_curve_geometry_campaign_runner_test.py` |
| Preparation workflow contract | `tests/continuous_curve_geometry_campaign_prepare_workflow_test.py` |
| Execution workflow contract | `tests/continuous_curve_geometry_campaign_execute_workflow_test.py` |
| Authorization record contract | `tests/continuous_curve_geometry_regression_authorization_test.py` |
| Authorization controller contract | `tests/continuous_curve_geometry_regression_authorization_workflow_test.py` |

## 4. Formal plan equivalence

The formal runner derives its launch plan from the report-only CGR runner and
then replaces only the report output-root placeholder with the formal output
root.

The focused contract verifies identical command IDs and the exact pre-registered
cardinality:

- four cells;
- two repetitions per cell;
- 56 command records;
- 112 stdout/stderr command logs;
- eight CTest discovery records;
- eight semantic CTest records;
- fourteen tests per semantic repetition;
- 112 individual semantic test executions; and
- eight certificate slots.

CGR0–CGR7 remain `NOT_EXECUTED` in self-check and PREPARED state.

## 5. PREPARED lifecycle

The formal runner exposes exactly the required lifecycle commands:

- `self-check`;
- `prepare`;
- `validate-prepared`;
- `execute`;
- `verify-retention`.

A synthetic PREPARED package contains exactly seven files:

1. `profile.json`;
2. `plan.json`;
3. `planned-inventories.json`;
4. `prepared-manifest.json`;
5. `preparation-seal.json`;
6. `state.json`;
7. `state-history.jsonl`.

The focused contract rejects:

- an existing output root;
- cloud-environment drift;
- a mutated PREPARED manifest;
- a consumed PREPARED package presented as unconsumed.

No real PREPARED package is created by this infrastructure branch or its
workflow.

## 6. Synthetic execution and retention contract

The formal runner focused contract executes a synthetic campaign by replacing
external build/CTest process execution while using the real CGR certificate
exporter, evidence validator, negative-evidence tool and deterministic derived
evidence.

It verifies:

- exactly 56 unique successful command records;
- exactly 112 retained command logs;
- eight discovery records;
- eight semantic CTest records;
- fourteen exact tests per semantic repetition;
- eight certificates;
- complete deterministic derived evidence;
- terminal gate state remains
  `EVIDENCE_COLLECTED_PENDING_AUDIT`;
- no runner-generated PASS or QUALIFIED decision.

A synthetic failure injected at
`gcc-debug-semantic-ctest-2` proves fail-fast behavior before the second
certificate and before the next cell.

Both successful and blocked terminal packages are byte-immutable after the
local execution claim: a second `execute` invocation is rejected without
rewriting retained evidence.

## 7. Preparation-only workflow

The preparation workflow:

- exposes only `workflow_dispatch`;
- requires canonical `main`;
- checks exact HEAD/upstream identity and clean worktree;
- installs the admitted cloud toolchain;
- writes output only below `runner.temp`;
- invokes `prepare` and `validate-prepared`;
- independently asserts PREPARED / no gate result;
- rejects execution/terminal artifacts;
- uploads one 90-day `cgr-prepared-<sha>` artifact;
- contains no `execute` path.

## 8. Authorization and reusable execution

The authorization controller:

- triggers only on a protected-main manifest-hash authorization path;
- checks the **complete commit diff**;
- requires exactly one newly added path;
- validates a closed authorization schema;
- requires an integrated PREPARED audit;
- rejects any existing manifest-hash claim;
- calls the reusable executor with only validated values.

The reusable executor:

1. revalidates the committed authorization;
2. checks out the exact prepared candidate;
3. reconstructs its sealed upstream identity;
4. verifies admitted cloud tooling;
5. verifies GitHub artifact ID, digest, run, candidate and `main` provenance;
6. restores the exact PREPARED package;
7. revalidates manifest/seal/full PREPARED binding;
8. creates one immutable manifest-hash claim;
9. invokes `execute` exactly once;
10. removes reproducible build trees;
11. verifies terminal retention;
12. retains a manifest-bound terminal artifact.

No direct executor trigger, retry path or `continue-on-error` weakening is
present.

## 9. Validation

Formal tooling workflow:

`Continuous Curve Geometry Regression Formal Tooling`

Run:

`35613409036`

Results:

- GCC 13 Debug / CGR FORMAL TOOLING: **PASS**;
- Clang 18 libc++ Debug / CGR FORMAL TOOLING: **PASS**.

Exact eight-contract inventory:

1. `apmesh_core.continuous_curve_geometry_regression_evidence`;
2. `apmesh_core.continuous_curve_geometry_regression_runner`;
3. `apmesh_core.continuous_curve_geometry_regression_workflow`;
4. `apmesh_core.continuous_curve_geometry_campaign_runner`;
5. `apmesh_core.continuous_curve_geometry_campaign_prepare_workflow`;
6. `apmesh_core.continuous_curve_geometry_campaign_execute_workflow`;
7. `apmesh_core.continuous_curve_geometry_regression_authorization`;
8. `apmesh_core.continuous_curve_geometry_regression_authorization_workflow`.

All eight passed in both declared tooling cells.

## 10. Explicit non-events

This work item did **not**:

- dispatch formal preparation;
- create a formal PREPARED artifact;
- add an `EXECUTE_ONCE` record;
- create an execution claim;
- run the formal campaign;
- decide CGR0–CGR7;
- qualify Curve Representation.

## 11. Decision

**PASS / FORMAL INFRASTRUCTURE VALIDATED.**

After integration, protected checks and checkpoint closure, the sole next
bounded action is:

**dispatch exactly one formal CGR PREPARED package from canonical clean
`main`, then stop for independent PREPARED audit.**

Formal execution remains unauthorized.

Machine-readable audit:

`docs/audits/2026-09-21-continuous-curve-geometry-regression-formal-infrastructure-audit.json`.
