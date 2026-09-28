# Surface Differential Geometry Report-Only Qualification Tooling — Entry Audit

Date: 2026-09-28  
Status: ACTIVE / REPORT-ONLY TOOLING / PREPARATION NOT AUTHORIZED /
FORMAL EXECUTION NOT AUTHORIZED / NOT QUALIFIED

## 1. Entry authority

Protected `main` and work branch both resolve to:

`03c24b409e126f65687df68aa11ccdc50d009abc`.

Entry audit:

- open PRs: none;
- branch relation: identical to `main`;
- FAST #631: PASS;
- INTEGRATION #622 / GCC 13 Debug: PASS;
- INTEGRATION #622 / Clang 18/libc++ Debug: PASS;
- ordinary semantic registration inventory: **42 tests**;
- protocol lifecycle:
  **PRE-REGISTERED / INTEGRATED / CLOSED / NOT QUALIFIED**.

Scientific authority:

`docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md`.

## 2. Authorized work item

Exactly one bounded work item is active:

**Surface Differential Geometry — Report-Only Qualification Tooling.**

The tooling may implement only deterministic evidence/report mechanisms needed
by the frozen protocol:

- immutable qualification profile;
- scientific-certificate exporter;
- independent certificate/profile validator;
- same-cell and cross-cell semantic comparison;
- negative-evidence self-checks;
- deterministic figures derived from machine-readable evidence;
- report/gate-summary generation;
- focused tooling contracts;
- qualification-only CMake registration.

## 3. Explicit nonauthorization

This work item does **not** authorize:

- a campaign runner;
- preparation/launch infrastructure;
- a PREPARED manifest;
- workflow dispatch;
- one-time execution authorization;
- formal qualification execution;
- evidence retention/sealing for a formal candidate;
- a `QUALIFIED` or `BLOCKED` stage decision;
- production Surface Differential Geometry changes;
- changes to ordinary semantic tests;
- changes to the frozen 42-test ordinary allowlist.

Any need for one of those mechanisms is a stop condition for this work item.

## 4. Frozen semantic boundary

The 32 semantic paths named in Section 5 of the protocol are immutable for this
work item.

No change is admissible under:

- `include/apmesh/geometry/` production surface/differential headers;
- `src/geometry/` production surface/differential sources;
- current ordinary surface semantic tests.

The intended diff is limited to:

- `experiments/profiles/`;
- `experiments/*_export.cpp`;
- `tools/`;
- focused qualification-tooling tests;
- qualification-only CMake registration;
- continuity/audit documentation.

## 5. Tooling classification

All new CTest registration must remain behind
`APMESH_ENABLE_QUALIFICATION_TESTS=ON`.

The ordinary `BUILD_TESTING=ON` inventory must remain exactly **42 tests**.

Tooling output must use a non-terminal scientific status such as:

`EVIDENCE_COLLECTED_PENDING_AUDIT`.

SDG0–SDG7 must remain `NOT_EXECUTED` in tooling-only reports. A tooling test
must fail if the tool claims `PASS`, `QUALIFIED`, formal execution, or a
prepared candidate.

## 6. Design basis

The repository already provides proven patterns for report-only qualification
mechanisms:

- `tools/topological_model_cumulative_evidence.py`;
- `tools/continuous_curve_geometry_regression_evidence.py`;
- `tools/geometry_point_vector_evidence.py`;
- their corresponding profiles, exporters and focused tests.

The new tooling will reuse the existing strict JSON/hash primitives from
`tools/experiment_runtime.py` and the existing qualification-only CMake seam,
rather than introduce another framework.

## 7. Planned validation

Before integration the exact final tooling head must show:

1. ordinary FAST: PASS;
2. ordinary INTEGRATION GCC 13 Debug: PASS;
3. ordinary INTEGRATION Clang 18/libc++ Debug: PASS;
4. focused qualification-tooling CTest: PASS under
   `APMESH_ENABLE_QUALIFICATION_TESTS=ON`;
5. ordinary registration inventory unchanged at 42;
6. no frozen semantic path changed;
7. no preparation/runner/workflow/authorization mechanism added.

A PR merge is prohibited until the required ordinary checks are green on the
exact final head. Focused tooling validation evidence must be recorded against
that same revision or a tree-equivalent revision.

## 8. Work-class plan

- Implementation: 25%;
- Tests/validation: 35%;
- Evidence/experiments design: 30%;
- Documentation/governance: 10%.

This accounting reflects qualification tooling rather than production
scientific implementation.

## 9. Next action

Implement the smallest coherent report-only tooling package conforming to the
frozen protocol, validate it without formal execution, then update this audit
with exact candidate evidence.


## 10. Candidate implementation checkpoint

The report-only tooling candidate was implemented after the entry audit.

Current protected `main` after independent maintenance PRs #236 and #237:

`d5c6d37b56d5434e7c907ec248eb1f6b5bc24136`.

Current-main validation:

- FAST #635: PASS;
- INTEGRATION #626 / GCC 13 Debug: PASS;
- INTEGRATION #626 / Clang 18/libc++ Debug: PASS.

The active tooling branch incorporated those maintenance changes through
non-destructive merge commits. No rebase or force push was used.

Technical candidate before continuity synchronization:

`7cd9ca1d221577b1730a7d05aa4a4584b4cc270d`.

Implemented package:

- qualification profile;
- scientific certificate exporter;
- independent validator/comparer;
- negative-evidence self-checks;
- deterministic figure/report generation;
- focused qualification CTest;
- qualification-only CMake registration.

Frozen-boundary audit:

- 32/32 semantic blob SHAs match the preregistered profile;
- ordinary semantic allowlist remains 42 tests;
- no production Surface Differential Geometry file changed;
- no ordinary surface semantic test changed;
- no runner/workflow/preparation/authorization mechanism was added.

A profile inconsistency was found before validation: the two new `mixed`
cases lacked explicit numeric policies even though the protocol requires one
for every rounded comparison. Both cases were bound to the already
preregistered `binary_strict` policy. No tolerance was added or changed.

Detailed candidate audit:

`docs/audits/2026-09-28-surface-differential-geometry-report-tooling-validation.md`.

Current validation state:

**ORDINARY PR VALIDATION PENDING /
FOCUSED QUALIFICATION CTEST PENDING.**

The current assistant shell cannot materialize the repository through its local
network, and the connected GitHub API exposes no workflow-dispatch action.
Because Section 7 requires a focused CTest PASS and Section 3 forbids adding a
validation-only workflow/runner, no focused PASS is claimed.

The merge gate remains fail-closed.
