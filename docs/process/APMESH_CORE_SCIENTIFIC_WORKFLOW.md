# AP Mesh Core — Scientific Workflow and Regression Policy

Status: ACTIVE / MANDATORY
Last updated: 2026-09-20
Roadmap: `docs/APMESH_CORE_ROADMAP.md`
Continuation state: `docs/APMESH_CORE_STATE.md`

## 1. Purpose

Define the repeated workflow that every AP Mesh Core scientific investigation must follow. This policy exists to prevent undocumented algorithm changes, untraceable experimental decisions, regression drift, and dependence on chat history.

## 2. Mandatory repeated cycle

Every meaningful scientific/algorithmic change follows this sequence:

```text
1. Select one executable work unit
2. State the precise question and expected property
3. Review literature / authoritative best practices
4. Define assumptions and admissible input class
5. Define reference result / oracle strategy
6. Define tests, adversarial cases, metrics, and required figures
7. Observe current implementation if one exists
8. Implement the smallest justified change
9. Run local verification for the work unit
10. Record results and generated artifacts
11. Classify PASS / DEFECT / LIMITATION / BLOCKED
12. Update roadmap, state, references, and decision record
13. At stage end, run cumulative end-of-stage regression
14. Only then qualify the stage and authorize the next one
```

No step should be silently skipped. If a step does not apply, the decision record states why.

## 3. Investigation document template

Every Investigation Problem should eventually have a document containing:

### Question

One precise scientific/software question.

### Motivation

Why the question matters to correctness, robustness, reproducibility, or the doctoral claim.

### Preconditions

Which earlier stages/contracts must already be qualified.

### Literature / best-practice basis

Primary and authoritative sources. Each citation must state what it supports and what it does not establish.

### Mathematical/software contract

Definitions, invariants, admissible input, expected result, error semantics.

### Implementation under investigation

Relevant module/API/algorithm, without assuming legacy behavior is correct.

### Verification fixtures

Analytic, synthetic, adversarial, negative, scale, orientation, parameterization, and real-world fixtures as appropriate.

### Metrics

Exact numeric quantities and acceptance rules.

### Required figures

Each figure must answer a declared question rather than serve as decoration.

### Results

Tables, figures, certificates, logs, and hashes.

### Classification

One of:

- `PASS`
- `DEFECT`
- `LIMITATION`
- `BLOCKED`
- `INVESTIGATION_REQUIRED`

### Decision

What changed, what did not change, retained limitations, and next admissible action.

## 4. Naming policy

Every work item uses descriptive names.

Preferred examples:

- `Curve Representation — Verify Cubic Bezier Reversal Invariance`
- `Topological Model — Reject Coincident but Disconnected Edges`
- `Numeric Contract — Define Scale-Aware Geometric Proximity`
- `Boundary Discretization — Verify Metric-Length Parameterization Invariance`

Avoid standalone labels such as:

- `F2`
- `P4`
- `R7`
- `AV3`

Short numeric/order prefixes may be used in filenames only if accompanied by the descriptive name.

## 5. Literature research policy

Before implementing or changing a scientific mechanism:

1. search for primary academic references where available;
2. search authoritative standards/documentation for language/build/runtime behavior;
3. identify established competing algorithms;
4. identify known numerical/robustness failure modes;
5. identify accepted verification/quality metrics;
6. record relevant references in `docs/research/REFERENCE_REGISTER.md`;
7. state why the selected approach is appropriate for the declared input class.

Search should not be used to justify a preconceived implementation. Alternative methods and counterevidence must be recorded when material.

## 6. Reference hierarchy

Prefer, approximately:

1. mathematical definitions / primary peer-reviewed research;
2. standards;
3. authoritative textbooks/monographs;
4. official library/tool documentation;
5. high-quality survey papers;
6. mature open-source implementations as comparative evidence;
7. informal web material only when stronger sources are unavailable.

An external implementation is not automatically an oracle.

## 7. Verification hierarchy

Prefer independent reference sources in this order when feasible:

1. closed-form analytic result;
2. mathematical invariant;
3. high-precision/reference computation independent of production code;
4. trusted independent library/implementation;
5. convergence study against refined computation;
6. comparison with legacy AP Mesh only as historical evidence.

## 8. Fixture hierarchy

Each capability should build a layered fixture set:

### Analytic fixtures

Simple cases with known answers.

### Adversarial fixtures

Cases intentionally close to known failure boundaries.

### Structural fixtures

Topology/orientation/non-manifold/periodic cases.

### Invariance fixtures

Translation, rotation, scale, reversal, parameterization changes, or equivalent representations where mathematically applicable.

### Integration fixtures

Multiple certified mechanisms interacting.

### Literature / benchmark models

Introduced only when licensing, provenance, expected properties, and relevance are documented.

## 9. Required experiment artifacts

A scientific experiment should produce, as applicable:

```text
manifest.json
certificate.json
metrics.csv
report.md
figures/
```

The manifest records at minimum:

- experiment ID and descriptive name;
- source commit;
- input file/model hashes;
- compiler and version;
- standard library/toolchain where relevant;
- OS/platform;
- build type and relevant flags;
- algorithm configuration;
- numeric policy;
- deterministic seed if randomness is ever introduced;
- expected artifacts.

The certificate records machine-readable acceptance results and retained limitations.

## 10. Figure policy

Figures are mandatory whenever spatial/numerical structure is important to the claim.

Examples:

- geometry with entity IDs;
- incidence/orientation diagram;
- curve with control polygon;
- speed/curvature plots;
- surface normal/curvature fields;
- sizing field;
- boundary trace comparison;
- mesh quality field;
- geometric error field;
- convergence history;
- before/after invariant comparison.

A figure must be reproducible by script/tool from recorded experiment outputs.

Manual image editing must not alter scientific content.

## 11. Validation profiles and local work-unit verification

Validation cost follows the scientific scope of the claim. `BUILD_TESTING=ON`
registers direct semantic and focused contract tests only. Qualification and
evidence tooling is registered only when
`APMESH_ENABLE_QUALIFICATION_TESTS=ON`; enabling it does not itself authorize a
formal campaign.

### Development / FAST

Use GCC Debug, the direct semantic tests, the focused contract for the current
capability, relevant bug reproducers, and `git diff --check`. The canonical
entry point is:

```text
cmake --preset gcc-debug
cmake --build --preset gcc-debug
ctest --preset fast
```

FAST excludes historical evidence, runners, retention, detached verification,
certificate matrices, and previous-stage qualification tooling. A passing FAST
profile permits the status `IMPLEMENTED / FOCUSED CONTRACTS PASS /
UNQUALIFIED`; it does not qualify the stage.

### Integration

Run the relevant FAST tests and compact cumulative semantic tests with GCC
Debug and Clang/libc++ Debug. Add Release only when optimization, `NDEBUG`, or
floating-point behavior is a declared risk. The standard Debug entry points
are `ctest --preset integration-gcc-debug` and
`ctest --preset integration-clang-debug`. Integration does not automatically
create a manifest, retention package, or formal campaign.

### Scientific-stage qualification

Qualification is reserved for stage closure or an explicit earlier scientific
decision. It may use the four compiler/build cells, independent repetitions,
analytic/adversarial matrices, manifests, certificates, retention, detached
verification, prerequisite regression, and scientific audit. Configure the
tooling explicitly with `cmake --preset gcc-debug-qualification`; the
`qualification-gcc-debug` test preset verifies registered contracts but is not
a substitute for the stage protocol.

During an ordinary bounded work unit:

1. declare essential invariants and distinct risk classes;
2. implement the smallest coherent capability;
3. add focused tests for those risks and real bug reproducers;
4. run FAST and correct defects;
5. update roadmap/state only when scientific status changed;
6. integrate as implemented and unqualified.

Use table-driven tests when cases share a property. There is no minimum test
count and no justification for combinatorial enumeration without a scientific
hypothesis. Historical qualification remains valid until new evidence shows a
contradiction; development work uses compact semantic prerequisite regression,
not repeated historical campaigns.

The ordinary-development effort guideline is 60--65% scientific/C++
implementation, 25--30% focused validation/tooling, and about 10%
documentation/governance. This is an anti-overengineering signal, not rigid
accounting. Stage qualification is the explicit exception.

### Current CTest inventory

Cost is an order-of-magnitude estimate after the relevant targets are built.
`Every` means ordinary changes, `Integrate` means investigation integration,
and `Qualify` means scientific-stage qualification.

| CTest | Class | Cost | Every | Integrate | Qualify |
| --- | --- | --- | --- | --- | --- |
| `apmesh_core.bootstrap_smoke` | A DIRECT_SEMANTIC | <1 s | yes | yes | yes |
| `apmesh_core.numeric_contract` | A DIRECT_SEMANTIC | <1 s | yes | yes | yes |
| `apmesh_core.geometry_primitives` | A DIRECT_SEMANTIC | <1 s | yes | yes | yes |
| `apmesh_core.cartesian_frames` | A DIRECT_SEMANTIC | <1 s | yes | yes | yes |
| `apmesh_core.topological_model` | B FOCUSED_CONTRACT | <1 s | yes | yes | yes |
| `apmesh_core.minimal_small_linear_algebra` | A DIRECT_SEMANTIC | <1 s | yes | yes | yes |
| `apmesh_core.math_header_isolation` | B FOCUSED_CONTRACT | <1 s | yes | yes | yes |
| `apmesh_core.geometry_primitives_cumulative_evidence` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.geometry_primitives_cumulative_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.cartesian_frames_evidence` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.cartesian_frames_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.cartesian_frames_retention` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.minimal_small_linear_algebra_evidence` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.minimal_small_linear_algebra_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.minimal_small_linear_algebra_retention` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.geometry_point_vector_evidence` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.geometry_point_vector_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.geometry_point_vector_retention` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.geometry_point_vector_foundation_preservation` | E STAGE_REGRESSION | seconds--minutes | no | no | yes |
| `apmesh_core.numeric_contract_evidence` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.numeric_contract_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.reproducible_experiment_evidence` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.reproducible_experiment_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.reproducible_experiment_retention` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |
| `apmesh_core.reproducible_experiment_negatives` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.foundation_end_to_end_evidence` | E STAGE_REGRESSION | seconds--minutes | no | no | yes |
| `apmesh_core.bootstrap_export` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.bootstrap_tool` | C QUALIFICATION_TOOLING | seconds | no | no | yes |
| `apmesh_core.architecture_bootstrap_runner` | C QUALIFICATION_TOOLING | seconds--minutes | no | no | yes |

There is currently no D HISTORICAL_EVIDENCE CTest. Historical packages and
blocked attempts are retained as immutable repository evidence but are not
executed by the development profile.

## 12. Mandatory end-of-stage cumulative regression

Every Scientific Stage ends with a dedicated regression campaign.

### Regression content

It must include:

1. all accepted tests/fixtures of the current stage;
2. all regression gates from prerequisite stages that remain relevant;
3. every previously discovered bug reproducer promoted into the regression suite;
4. adversarial and negative cases;
5. deterministic repeated runs;
6. regeneration of declared figures/tables/certificates;
7. comparison with accepted expected results;
8. clean-build execution from documented setup.

### Regression classifications

Every difference is classified as:

- `NO_CHANGE`
- `EXPECTED_CHANGE`
- `REGRESSION`
- `INVESTIGATION_REQUIRED`

`EXPECTED_CHANGE` requires a written scientific/engineering rationale and updated expected evidence.

### Regression failure policy

A stage cannot become `QUALIFIED` while any `REGRESSION` or unresolved `INVESTIGATION_REQUIRED` item remains.

If a regression invalidates an older stage:

1. mark the older stage `REOPENED` in the roadmap;
2. identify dependent stages whose claims are affected;
3. suspend advancement past the invalidated prerequisite;
4. resolve and rerun cumulative regression before continuing.

## 13. Stage closure package

A stage closes only with all of:

- roadmap updated;
- continuation state updated;
- relevant references registered;
- investigation/decision documents complete;
- implementation/tests committed;
- experiment manifests committed when appropriate;
- generated scientific results archived according to repository policy;
- end-of-stage regression PASS;
- retained limitations explicitly listed;
- next admissible stage/action named.

## 14. Git workflow policy

For scientific work:

- use descriptive research branches;
- keep each commit narrow and explain what scientific/engineering property changed;
- do not mix unrelated cleanup with algorithmic evidence changes;
- a behavior-changing commit must include or reference its tests/evidence;
- stage closure should be identifiable by a dedicated decision/closure commit;
- tags/releases are created only after explicit qualification gates.

## 15. Continuation policy for a new work session

At the start of a new session, read:

1. `docs/APMESH_CORE_STATE.md`;
2. `docs/APMESH_CORE_ROADMAP.md`;
3. current active contract/investigation document;
4. relevant reference-register entries;
5. latest stage decision/regression report.

Do not infer current project status from historical chat summaries if repository documents disagree. Repository state is authoritative.

At the end of a meaningful session/change:

- update `APMESH_CORE_STATE.md`;
- update roadmap status;
- record new literature/decisions;
- record exact next admissible action.

## 16. Repository checkpoint, progress accounting, and audit cadence

The Git repository is the continuation authority for every work session. Chat
history may explain intent, but it must never be required to discover the
current scientific state, the last accepted revision, or the next admissible
action.

Every meaningful execution report recorded in Git must identify:

- baseline branch and commit;
- exact objective and bounded scope;
- work-class percentages for that execution;
- files or mechanisms changed;
- validation actually executed and its result;
- evidence produced or intentionally not produced;
- scientific status change, if any;
- retained limitations and blockers;
- exact next admissible action; and
- synchronization status of `main`, active branch, and CI.

Work-class percentages describe the planned/observed distribution of the
execution and must sum to 100% across:

- **Implementation** — production scientific/C++ behavior;
- **Tests/validation** — focused tests, semantic regression, bug reproducers,
  build portability, and test infrastructure;
- **Evidence/experiments** — manifests, certificates, retained experimental
  outputs, comparisons, and qualification execution; and
- **Documentation/governance** — roadmap, continuation state, decisions,
  audits, and process maintenance.

These percentages are effort-accounting signals, not scientific completion
scores. For ordinary implementation work, the standing target remains roughly
60--65% implementation, 25--30% tests/validation, and about 10% combined
evidence/documentation unless the work unit justifies another distribution.
Qualification and audit work are explicit exceptions.

Scientific-stage completion is reported separately as independent lanes, each
from 0% to 100%:

1. production implementation;
2. focused validation;
3. stage-regression/qualification tooling;
4. formal evidence execution; and
5. closure audit/documentation.

Do not collapse these lanes into an overall percentage unless a stage decision
pre-registers explicit weights. This avoids presenting arbitrary arithmetic as
scientific progress.

At every major phase boundary, and before authorizing a new Scientific Stage,
perform a repository audit plus cumulative regression appropriate to the claim.
The audit must verify at minimum:

- `main` and documented baseline agreement;
- no stale stage/status contradiction in STATE/ROADMAP/active decisions;
- expected CI profiles and recent terminal results;
- current semantic test inventory and declared allowlists;
- retained prerequisite authority and limitations;
- absence of undeclared production-scope expansion; and
- one exact next admissible action.

The reusable GitHub Actions **Major Semantic Regression** is an engineering
regression over current direct/focused semantic tests on GCC 13 and Clang 18,
Debug and Release, with qualification tooling disabled. It is not a scientific
stage-qualification campaign and cannot substitute for a pre-registered stage
protocol such as TMR0--TMR7.

The continuation state must contain a compact current checkpoint so a new
session can resume from the repository alone. A major audit/closure must record
its terminal commit and CI run(s) in the repository before further scientific
scope is authorized.

## 17. Anti-patterns explicitly prohibited

- advance because a test executable returned zero without inspecting its contract;
- tune a tolerance until a fixture passes without a mathematical/numerical basis;
- infer topology from coordinate proximity;
- copy legacy algorithms before reconstructing their contract and evidence;
- create fixture-specific runtime tuning to hide a general defect;
- close a stage while known regressions remain;
- accept visual mesh quality as sufficient evidence;
- compare only against the previous AP Mesh implementation;
- add a third-party dependency without a decision record;
- introduce parallelism before the serial reference is qualified;
- leave roadmap/state stale after implementation changes.
