# Surface Differential Geometry Report-Only Tooling — Candidate Validation Audit

Date: 2026-09-28  
Status: IMPLEMENTED CANDIDATE / ORDINARY PR VALIDATION PENDING /
FOCUSED QUALIFICATION CTEST PENDING / NOT QUALIFIED

## 1. Authority and current remote state

Original tooling-entry protected `main`:

`03c24b409e126f65687df68aa11ccdc50d009abc`.

Entry evidence:

- FAST #631: PASS;
- INTEGRATION #622 / GCC 13 Debug: PASS;
- INTEGRATION #622 / Clang 18/libc++ Debug: PASS;
- ordinary semantic inventory: **42 tests**;
- open PRs at entry: none.

The active branch later incorporated two independent maintenance changes from
protected `main` without rewriting history:

1. PR #236 — host-aware CMake presets;
2. PR #237 — development preset alignment.

Current protected `main` after PR #237:

`d5c6d37b56d5434e7c907ec248eb1f6b5bc24136`.

Validation on that exact main revision:

- FAST #635: PASS;
- INTEGRATION #626 / GCC 13 Debug: PASS;
- INTEGRATION #626 / Clang 18/libc++ Debug: PASS.

The tooling branch incorporated both maintenance commits through explicit
two-parent merge commits. No rebase, force push or history rewrite was used.

## 2. Frozen scientific boundary

Protocol authority:

`docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md`.

The qualification profile records **32 frozen semantic Git blobs**.

A recursive-tree audit on the synchronized tooling branch compared every
recorded blob SHA against the branch tree:

**32 / 32 MATCH — zero frozen semantic mismatches.**

No file under the frozen production/semantic set changed.

The ordinary semantic allowlist remains exactly **42 tests**.

## 3. Implemented tooling package

Current technical candidate before this audit synchronization:

`7cd9ca1d221577b1730a7d05aa4a4584b4cc270d`.

Implemented files:

- `experiments/profiles/surface_differential_geometry_qualification.json`;
- `experiments/surface_differential_geometry_qualification_export.cpp`;
- `tools/surface_differential_geometry_qualification_evidence.py`;
- `tests/surface_differential_geometry_qualification_evidence_test.py`;
- qualification-only registration in `CMakeLists.txt`.

The package provides only report/evidence mechanisms:

- immutable profile;
- scientific certificate exporter;
- independent analytic/profile validator;
- same-cell and cross-cell comparison;
- 17 negative-evidence mutations;
- deterministic machine-readable figure sources and SVG figures;
- non-terminal report/gate summary;
- focused development contract.

No campaign runner, workflow, authorization, PREPARED manifest, formal
execution mechanism or stage-status transition was added.

## 4. Profile audit

The profile freezes:

- four compiler/build cells;
- two repetitions per cell;
- SDG0–SDG7;
- exact 42-test ordinary allowlist;
- 32 frozen semantic blobs;
- 35 certificate cases;
- seven typed errors;
- eleven admitted surface-family conformance identities;
- seventeen negative mutations;
- four scientific figures;
- explicit non-claims;
- signed-zero policy;
- `EVIDENCE_COLLECTED_PENDING_AUDIT` tooling status;
- all formal gates as `NOT_EXECUTED`.

### 4.1 Mixed-policy inconsistency found and corrected

During implementation audit, two profile cases were found with
`comparison_rule = mixed` and `policy = null`:

- `metric_oblique`;
- `second_order_oblique`.

The preregistered protocol explicitly requires an explicit Numeric Contract
policy for every rounded comparison.

Because the profile itself is a new, unintegrated artifact of this tooling work
item, the inconsistency was corrected before validation by binding both cases
to the already preregistered `binary_strict` policy.

No new tolerance was introduced and no frozen scientific acceptance criterion
was changed.

The validator now fails closed if any `mixed` or `proximity` case lacks an
explicit declared policy.

## 5. Exporter audit

The exporter:

- uses public integrated surface/differential APIs only;
- writes floating values as hexadecimal text;
- records typed failures;
- emits independent reference values alongside observed production values;
- includes per-field policy evidence for rounded comparisons;
- retains raw signed-zero evidence while declaring no signed-zero scientific
  claim;
- carries a fixed unprepared identity:
  `REPORT_ONLY_TOOLING_UNPREPARED`;
- emits all 35 preregistered cases;
- includes surface-family conformance and C1 double-knot continuity evidence.

Mechanical implementation defects found during static review were preserved and
corrected before PR validation:

1. a mutable conformance lambda was incorrectly declared `const`;
2. `SurfaceParameterDomain` lacked its namespace alias;
3. two already-materialized `Vector3` values were incorrectly dereferenced
   before scalar multiplication.

These were tooling implementation errors only. No production semantic file was
changed.

## 6. Independent validator audit

The Python validator independently:

- validates profile schema/inventories;
- recomputes Git blob SHA-1 identities without invoking Git;
- independently reconstructs analytic expectations for all 35 cases;
- enforces exact or declared numeric-policy comparisons;
- checks metric/area identity;
- checks principal ordering and Gaussian/mean identities;
- rejects condition numbers below one;
- checks typed error coverage;
- checks transformation/invariance observations;
- checks all admitted surface-family conformance records;
- compares two repetitions in each cell;
- compares cross-cell discrete scientific projections;
- self-tests all 17 negative mutations;
- creates four deterministic figure/source pairs and SHA-256 bindings;
- emits only `EVIDENCE_COLLECTED_PENDING_AUDIT`;
- keeps SDG0–SDG7 at `NOT_EXECUTED`;
- refuses reports that claim prepared/formal/qualified state.

## 7. Focused contract audit

The focused test:

- Python-compiles the validator;
- validates the profile against the checkout source tree;
- runs and validates one exporter certificate;
- asserts the 42/32/35/17/4 preregistered inventories;
- proves an undeclared mixed policy is rejected;
- constructs all eight cell/repetition slots for report-only comparison;
- requires same-cell determinism and cross-cell discrete equivalence;
- requires all 17 negatives to be rejected;
- generates all four figures twice and requires identical hashes;
- builds a non-terminal report;
- requires every SDG gate to remain `NOT_EXECUTED`;
- rejects a forged report where SDG0 is changed to `PASS`.

The CTest is registered only when
`APMESH_ENABLE_QUALIFICATION_TESTS=ON`.

## 8. Validation infrastructure limitation

The current assistant execution environment cannot resolve or download the
public GitHub repository through its local shell/container network.

The connected GitHub API available to this conversation can read/write
repository content and observe Actions, but exposes no workflow-dispatch action.

The work-item entry audit explicitly prohibits adding a new workflow, campaign
runner, preparation path or authorization mechanism merely to validate this
tooling.

Therefore no focused CTest PASS is claimed yet.

This limitation is not a repository failure. It is a **merge gate**:

**PR integration remains prohibited until
`apmesh_core.surface_differential_geometry_qualification_evidence`
passes with `APMESH_ENABLE_QUALIFICATION_TESTS=ON` on the exact final
revision or a demonstrably tree-equivalent revision.**

## 9. Required validation before integration

The exact final candidate must still obtain:

1. ordinary FAST: PASS;
2. ordinary INTEGRATION / GCC 13 Debug: PASS;
3. ordinary INTEGRATION / Clang 18 libc++ Debug: PASS;
4. focused qualification-tooling CTest: PASS with qualification tests enabled;
5. ordinary semantic inventory remains **42**;
6. all 32 frozen semantic blobs remain unchanged;
7. no preparation/runner/workflow/authorization mechanism appears.

The branch may be opened as a PR to obtain the ordinary checks. Green ordinary
checks alone are insufficient for merge.

## 10. Scientific status

Current result:

**REPORT-ONLY TOOLING IMPLEMENTED CANDIDATE /
ORDINARY PR VALIDATION PENDING /
FOCUSED QUALIFICATION CTEST PENDING /
PREPARATION NOT AUTHORIZED /
FORMAL EXECUTION NOT AUTHORIZED /
NOT QUALIFIED.**

No SDG0–SDG7 formal gate has been executed.
