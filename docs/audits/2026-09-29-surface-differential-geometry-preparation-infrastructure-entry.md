# Surface Differential Geometry — Preparation Infrastructure Entry Audit

Date: 2026-09-29  
Status: IMPLEMENTATION ACTIVE / SYNTHETIC VALIDATION ONLY / NOT QUALIFIED

## Authority

Entry protected `main`:

`bf30ef8e38a1a7de64b12a514176f226e7bda610`.

Entry validation:

- FAST #654: PASS;
- INTEGRATION #645 / GCC 13 Debug: PASS;
- INTEGRATION #645 / Clang 18/libc++ Debug: PASS;
- ordinary semantic registration inventory: **42 tests**;
- open PRs at entry: none.

Decision authority:

`docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_POST_TOOLING_DECISION.md`.

Frozen protocol authority:

`docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md`.

Report-only tooling authority:

- PR #238;
- `experiments/profiles/surface_differential_geometry_qualification.json`;
- `experiments/surface_differential_geometry_qualification_export.cpp`;
- `tools/surface_differential_geometry_qualification_evidence.py`.

## Scope

Implement reusable fail-closed preparation primitives plus synthetic
positive/negative validation only.

Authorized mechanisms:

- candidate/upstream identity validation;
- clean/published/non-divergent candidate-state validation;
- protocol/profile/exporter/validator identity binding;
- 32 frozen semantic Git-blob validation;
- exact 42-test ordinary allowlist validation;
- complete canonical tracked-source inventory validation;
- four-cell Ubuntu 24.04 x86_64 environment-plan validation;
- two-repetition / 56-core-record / 336-ordinary-execution cardinality
  validation;
- deterministic synthetic manifest/state-history generation;
- rejection of consumed, authorized, executed or qualified states.

Explicitly excluded:

- a real PREPARED package;
- a real candidate selection;
- a preparation workflow;
- artifact upload/retention for a formal candidate;
- execution authorization;
- workflow dispatch;
- formal qualification execution;
- production Surface Differential Geometry changes;
- protocol/profile acceptance changes;
- `QUALIFIED` or `BLOCKED` stage status.

## Frozen semantic audit at entry

The current qualification profile contains **32** frozen semantic Git blobs.

Comparison against entry `main`:

- matching: **31 / 32**;
- missing: **0**;
- drifted: **1**.

Drifted path:

`src/geometry/surface_differential.cpp`

- protocol/profile blob:
  `72de10f728237134d7f668883dd316ed4d971da9`;
- entry-main blob:
  `073ff0c9efe031c17df293058d6f3fd852d5910d`.

This drift is the already-recorded metric-conditioning normalization
portability correction from PR #239.

Classification:

**EXPECTED CURRENT FORMAL-PREPARATION BLOCKER / NOT A TOOLING-IMPLEMENTATION
BLOCKER.**

The infrastructure must reject this condition. It must not silently update the
frozen profile or reinterpret PR #239 as an implicit protocol amendment.

A later real preparation action therefore remains blocked until an explicitly
authorized protocol amendment or other formal resolution reconciles the frozen
identity.

## Reused design patterns

Existing TMR/CGR formal runners demonstrate:

- strict candidate/upstream binding;
- complete source inventory binding;
- deterministic manifest/state structures;
- exact CTest allowlist enforcement;
- explicit PREPARED lifecycle state;
- consumed-package rejection;
- fail-closed workflows.

This work item intentionally implements only the reusable validation primitives
needed before those formal mechanisms. It does not copy their workflow or
execution surface.

## Planned implementation

New tool:

`tools/surface_differential_geometry_qualification_preparation.py`.

New focused qualification-only test:

`tests/surface_differential_geometry_qualification_preparation_test.py`.

CTest registration remains behind:

`APMESH_ENABLE_QUALIFICATION_TESTS=ON`.

Ordinary `BUILD_TESTING=ON` inventory must remain exactly **42 tests**.

The CLI will expose only:

- `validate-fixture`;
- `simulate-fixture`;
- `validate-simulated`.

The synthetic lifecycle state is:

`SYNTHETIC_PREPARATION_VALIDATED`.

No CLI subcommand named `prepare`, `execute`, `authorize` or `dispatch`
is admitted in this work item.

## Required negative evidence

Focused validation must reject at least:

- candidate/upstream mismatch;
- dirty candidate;
- unpublished/divergent/moving candidate;
- altered frozen semantic blob;
- missing/extra/duplicate ordinary test registration;
- profile/protocol/exporter/validator identity mismatch;
- incomplete/duplicate/non-canonical tracked-source inventory;
- unexpected runner image/architecture/compiler/library/build cell;
- wrong repetition count;
- wrong 56-core-record cardinality;
- wrong 336 ordinary-test execution cardinality;
- non-`NOT_EXECUTED` SDG gate;
- execution requested/authorized;
- non-null qualification result;
- consumed package;
- execution-claim presence;
- real-PREPARED-package flag;
- a CLI capability that permits prepare/dispatch/authorize/execute;
- malformed/non-canonical synthetic manifest/state history;
- forbidden extra execution/terminal artifacts.

## Validation plan

Before integration:

1. Python syntax compilation for the new tool;
2. focused qualification-only CTest PASS;
3. synthetic positive package deterministic across two independent outputs;
4. all declared negative mutations rejected;
5. ordinary registration inventory still **42**;
6. frozen semantic production/test paths unchanged by the branch;
7. no workflow/authorization file added;
8. FAST PASS on exact PR head;
9. INTEGRATION PASS in GCC 13 Debug and Clang 18/libc++ Debug;
10. continuity/audit documents synchronized.

## Work-class plan

- Implementation/tooling: 55%;
- Tests/validation: 35%;
- Evidence/audit: 5%;
- Documentation/governance: 5%.

## Next action

Implement the synthetic fail-closed preparation primitives and focused contract.
Do not create a real formal package or workflow.
