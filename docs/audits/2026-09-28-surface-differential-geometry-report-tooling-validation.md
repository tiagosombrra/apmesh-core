# Surface Differential Geometry Report-Only Tooling — Candidate Validation Audit

Date: 2026-09-28  
Status: IMPLEMENTED CANDIDATE / INITIAL ORDINARY VALIDATION PASS /
LOCAL FOCUSED QUALIFICATION CTEST PASS /
UPDATED-HEAD ORDINARY REVALIDATION PENDING / NOT QUALIFIED

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

## 8. Initial validation infrastructure limitation

At the time of the initial entry audit, the execution environment could not
resolve or download the public GitHub repository through its local
shell/container network.

The connected GitHub API available to this conversation can read/write
repository content and observe Actions, but exposes no workflow-dispatch action.

The work-item entry audit explicitly prohibits adding a new workflow, campaign
runner, preparation path or authorization mechanism merely to validate this
tooling.

Therefore no focused CTest PASS was claimed in the initial entry audit. A later
local follow-up is recorded below.

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
FOCUSED QUALIFICATION CTEST PASS ON LOCAL CANDIDATE /
ORDINARY PR VALIDATION PENDING FOR UPDATED HEAD / MERGE GATE BLOCKED /
PREPARATION NOT AUTHORIZED /
FORMAL EXECUTION NOT AUTHORIZED /
NOT QUALIFIED.**

No SDG0–SDG7 formal gate has been executed.


## 11. Initial ordinary PR validation

PR #238 initial documented head:

`59bc9cbfb9550a82464b24dd53168347e12ffae6`.

Required ordinary validation on that exact revision:

- FAST #636 / GCC 13 Debug: PASS;
- INTEGRATION #627 / GCC 13 Debug: PASS;
- INTEGRATION #627 / Clang 18/libc++ Debug: PASS.

The retained job logs show:

**100% tests passed, 0 tests failed out of 42**

in FAST, GCC INTEGRATION and Clang INTEGRATION.

This confirms that the ordinary semantic inventory remained exactly 42 and no
ordinary regression was introduced.

This documentation synchronization changes the PR head. The new exact head must
therefore receive ordinary FAST/INTEGRATION again before any merge decision.

The focused qualification-tooling CTest remained **PENDING** at the time of the
initial PR validation. Green ordinary checks alone did not satisfy that gate.

## 12. Local focused-gate follow-up

On 2026-09-28, the exact PR head
`bfd0dd7d19cce9ff7ee2241e1906cebf52306e4a` was fetched and checked out in an
isolated detached worktree. Qualification-enabled CMake configuration passed
with Ubuntu 24.04, GCC 13 Debug and
`APMESH_ENABLE_QUALIFICATION_TESTS=ON`.

The initial exporter build failed before CTest because `-Werror` promoted
mechanical warnings: omitted optional `Fields` initializers, one unused error
helper overload, and two unused sphere constants. These findings did not involve
fixture values, numeric policy, expected results or tolerances. The candidate
was corrected by explicitly initializing the absent optional values and
removing the unused overload/constants. No test ran on the failed attempt.

The mechanical correction compiled successfully on the single permitted
focused rerun. The focused CTest then failed during independent certificate
validation with:

`ERROR: sphere_latitude.observed umbilic state differs`

The initial exporter and independent validator marked the analytic sphere's
expected latitude point as umbilic. Production reports exact represented-data
umbilicity by comparing the whitened off-diagonal entry with zero and the
diagonal entries for exact equality. The observed floating-point evaluation at
this latitude does not satisfy that equality.

The reference decision is that `sphere_latitude` must retain its independent
analytic proximity oracles for the first and second forms, curvature values,
normal and conditioning, but must not assert a predetermined `is_umbilic=true`
from the ideal sphere identity. That identity concerns the mathematical surface;
the public boolean concerns the represented curvature operator. The protocol
requires an exact sphere-umbilic oracle at the canonical equator fixture, not
at every non-special latitude. The exact synthetic umbilic and adjacent-value
near-umbilic fixtures independently retain their strict boolean oracles.

For the non-special latitude, the report must still carry a non-null observed
boolean and the validator must still compare it exactly across repetitions and
all admitted compiler cells. The independent reference may leave only this
boolean unspecified; it must not accept a missing observed boolean or alter
any numeric policy, tolerance, production computation, case inventory, frozen
semantic file, or SDG gate. The same principle applies to the non-special
near-pole sphere sample. The bounded correction now leaves the boolean reference
unspecified for those two samples in both exporter and independent validator.
For those cases the validator requires an observed boolean, while retaining
exact reference/observed checks for the canonical equator and synthetic
umbilic/near-umbilic fixtures. The existing cross-cell discrete comparison still
includes every observed umbilic boolean. Production code, policies, tolerances
and frozen semantics are unchanged. At this checkpoint the bounded correction
had not yet received a focused CTest run; the later rebuilt run and its result
are recorded in Sections 13 and 14.

At the first focused run, the CTest was **FAIL / MERGE GATE BLOCKED**. The
two-consecutive-mechanical-failure stop threshold was not reached: the first
failure was a compiler-warning defect and the second was a semantic oracle
mismatch. The bounded reference/validator correction and successful later
focused run are recorded in Sections 13 and 14. The ordinary semantic inventory,
frozen blobs and absence of preparation/dispatch mechanisms must be confirmed on
the final published candidate.

## 13. Focused CTest attempt after the reference correction

The first invocation was made from PowerShell against the WSL-configured build
and did not start the test: CTest could not resolve `/usr/bin/python3` on
Windows. An initial actual focused execution was then made from Ubuntu 24.04,
using the same isolated worktree and GCC 13 Debug build. It failed at
`sphere_latitude.reference umbilic state differs`.

This failure does not evaluate the corrected exporter. The exporter source was
modified at 09:06 local time, while the executable used by CTest was last built
at 08:54 local time. The CTest contract invokes the existing executable and
does not build it. Classification at that attempt: **STALE EXPORTER / CANDIDATE
NOT VALIDATED**. No repeat was run against that stale binary; the exporter was
rebuilt before the subsequent focused execution recorded below.

## 14. Focused CTest PASS after rebuilding the exporter

The qualification exporter was rebuilt in Ubuntu 24.04 / GCC 13 Debug from the
corrected source. The build completed successfully and updated the executable
at 09:32 local time. The following single focused CTest execution passed:

`apmesh_core.surface_differential_geometry_qualification_evidence`: **1/1
PASS**, 1.76 seconds.

The CTest's `validate-profile` step accepted the current source tree: the
ordinary allowlist has 42 unique entries and all 32 frozen semantic file hashes
matched. `git diff --check` passed. This local candidate modifies only the audit,
qualification exporter and independent qualification validator; production
semantics, numeric policies and CMake registration are unchanged.

Logs and compact summaries are retained outside the checkout at:

- `%LOCALAPPDATA%/Temp/apmesh-pr238-umbilic-focused-20260928/exporter-rebuild-wsl.log`;
- `%LOCALAPPDATA%/Temp/apmesh-pr238-umbilic-focused-20260928/ctest-after-rebuild-wsl.log`.

The local branch `qualification/surface-differential-geometry-report-tooling`
tracks the PR branch and is staged at its current head
`bfd0dd7d19cce9ff7ee2241e1906cebf52306e4a`. The three reviewed files are
staged, with no commit created. PR #238 remains open and mergeable at that
unchanged remote head; its green ordinary checks belong to that older head.
Ordinary FAST/INTEGRATION checks must run for the updated published head before
integration.
