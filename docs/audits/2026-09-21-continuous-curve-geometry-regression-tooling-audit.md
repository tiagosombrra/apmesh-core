# Continuous Curve Geometry Regression — Report-Only Tooling Audit

Status: **PASS / REPORT-ONLY TOOLING / NO FORMAL PREPARED PACKAGE / NO EXECUTION**  
Audit date: 2026-09-21  
Protocol:
`docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`  
Frozen semantic baseline:
`438620efa1f93d29b442e9ba199882a09d2359d9`

## 1. Scope

This audit evaluates only the report-only tooling phase required by the
pre-registered Curve Representation stage-exit protocol.

It does not:

- prepare a formal evidence package;
- create an execution claim;
- authorize execution;
- execute a formal campaign;
- decide CGR0–CGR7; or
- qualify Curve Representation.

Every CGR gate remains `NOT_EXECUTED`.

## 2. Final focused tooling validation

Final focused workflow:

- workflow: `Continuous Curve Geometry Regression Tooling`;
- run: `35601303879`;
- head:
  `d4f9c238ec93225abd69ffb548c5ef13e4d2de48`;
- GCC 13 Debug / CGR TOOLING: **PASS**;
- Clang 18 libc++ Debug / CGR TOOLING: **PASS**;
- overall workflow conclusion: **success**.

Each cell passed:

1. report-only runner/baseline/command-shape self-check;
2. opt-in qualification-tooling configuration;
3. exporter build under `-Wall -Wextra -Wpedantic -Werror`;
4. exact focused CTest inventory;
5. CGR scientific evidence contract;
6. CGR report-only runner contract; and
7. CGR workflow static contract.

## 3. Tooling inventory

The phase adds or modifies only report-only tooling, build registration and
mapping documentation:

- `experiments/profiles/continuous_curve_geometry_regression.json`;
- `experiments/continuous_curve_geometry_regression_export.cpp`;
- `tools/continuous_curve_geometry_regression_evidence.py`;
- `tools/continuous_curve_geometry_regression_negative.py`;
- `tools/run_continuous_curve_geometry_regression.py`;
- `tests/continuous_curve_geometry_regression_evidence_test.py`;
- `tests/continuous_curve_geometry_regression_runner_test.py`;
- `tests/continuous_curve_geometry_regression_workflow_test.py`;
- `.github/workflows/continuous-curve-geometry-regression-tooling.yml`;
- `CMakeLists.txt`; and
- repository mapping/audit documentation.

No production curve source/header or frozen semantic test is modified.

## 4. Frozen semantic baseline verification

All eleven files pre-registered in protocol Section 4 are byte-identical to
baseline commit
`438620efa1f93d29b442e9ba199882a09d2359d9`.

| Path | Git blob |
| --- | --- |
| `include/apmesh/geometry/curve.hpp` | `ed23351ede741d37235b258e9f8a072759f2688a` |
| `src/geometry/curve.cpp` | `faf237a7408b472410bedbd2380e9d8d1160134a` |
| `src/geometry/detail/curve_length_interval.hpp` | `aea17e7f296d3df33acc1bf8e556de04b68eb20f` |
| `src/geometry/detail/curve_regularity_interval.hpp` | `7b06dc56b87b12a66a41ca08e513fdff5c2dac6c` |
| `tests/curve_representation.cpp` | `f9960a620405e3fc7cd3beb71569dc084e9eb897` |
| `tests/curve_differential.cpp` | `00457311396229208dcbbf6e2737cf47e39a44af` |
| `tests/curve_regularity.cpp` | `93420e2b7c6c4a678c7cde62446b94f8c9879fd8` |
| `tests/curve_arc_length.cpp` | `2d5c75200fca43539fa6406b1fa9be5711aa327f` |
| `tests/curve_cumulative_arc_length.cpp` | `c33508ba50fd839295fef617513559948aa17dcd` |
| `tests/curve_inverse_arc_length.cpp` | `afc1dd7d2c66e20e8b8f566d2a397152cb87a332` |
| `tests/curve_header_isolation.cpp` | `7a9235d7bfc37f0b16c5f5c264c25eead4c50889` |

The branch and baseline blob identities match for every row.

## 5. Declarative profile

The profile fixes:

- four admitted cells:
  GCC Debug, GCC Release, Clang/libc++ Debug, Clang/libc++ Release;
- two repetitions per cell;
- exact fourteen-test semantic allowlist;
- CGR0–CGR7 identifiers;
- fifteen certificate case families;
- thirteen negative/adversarial cases;
- eleven frozen semantic files;
- four derived-evidence families; and
- retained limitations/nonclaims.

The profile is closed-schema and independently validated.

## 6. Scientific certificate exporter

The exporter uses the public curve API and emits a closed certificate with
provenance separated from the scientific projection.

The scientific projection covers:

- representation and analytic value checks;
- first/second derivative and speed checks;
- regular, degenerate and bounded-resource indeterminate regularity;
- total and cumulative certified arc-length enclosures;
- certified inverse arc-length brackets;
- reversal relations;
- translation and exact power-of-two scaling relations;
- planar 2D/3D parity;
- explicit policy/domain/finiteness failure semantics;
- deterministic reference series for derived data; and
- explicit nonclaims.

No legacy AP Mesh result is used as an oracle.

## 7. Independent evidence validator

The validator independently recomputes or verifies:

- closed profile/certificate schemas;
- independent Bernstein value references;
- analytic derivative/speed references;
- analytic parabola arc-length containment;
- exact line-length relations;
- inverse-bracket containment of known analytic parameters;
- reversal, translation and scaling residuals;
- planar embedding parity;
- error/categorical semantics;
- same-cell scientific projection equality; and
- cross-cell categorical equality while independently validating numeric
  relations.

Floating enclosure endpoints are not required to be bit-identical across
libstdc++ and libc++.

## 8. Negative/adversarial evidence

Negative evidence is not a static declaration.

The focused tooling actually forges and requires validator rejection for:

- duplicate case inventory;
- categorical regularity forgery;
- analytic-reference containment forgery;
- invalid inverse-bracket ordering; and
- undeclared semantic claims.

It also verifies the certificate's production-observed failure semantics for:

- invalid regularity policy;
- invalid length policy;
- invalid inverse policy;
- non-finite curve parameter;
- non-finite inverse target;
- target outside the certified domain;
- uncertifiable target-domain membership; and
- regularity not certified.

Any accepted forgery fails the tooling contract.

## 9. Deterministic derived evidence

The validator deterministically derives the four pre-registered evidence
families as CSV plus SVG, with JSON manifest:

1. curve value/reference residual;
2. derivative/speed residual;
3. arc-length enclosure width versus resource/refinement setting;
4. inverse parameter-bracket width versus refinement iteration.

The focused contract derives the complete tree twice and requires byte-identical
files.

Expected derived inventory contains exactly nine files:

- four CSV files;
- four SVG files; and
- `derived-evidence.json`.

No third-party plotting or numerical runtime is required.

## 10. Report-only runner boundary

The runner exposes only:

- `self-check`;
- `plan`;
- `simulate`; and
- `validate-simulation`.

It exposes no `prepare` and no `execute` command.

Its simulated formal shape is exactly:

- 4 cells;
- 2 repetitions per cell;
- 56 unique command records;
- 112 unique command logs;
- 8 CTest-discovery records;
- 8 semantic CTest records;
- 112 individual semantic test executions;
- 8 certificate slots; and
- all CGR0–CGR7 gates `NOT_EXECUTED`.

It also verifies that every frozen semantic file remains identical to the
pre-registered baseline.

Untracked local build directories are tolerated only for this report-only
self-check. Formal PREPARED-package candidate cleanliness remains a later,
stricter lifecycle requirement.

## 11. Focused workflow boundary

The workflow runs only report-only tooling on:

- GCC 13 Debug; and
- Clang 18/libc++ Debug.

It uses the admitted Ubuntu 24.04 package versions and checks out complete Git
history so the frozen semantic baseline can be verified.

The workflow contains no:

- preparation command;
- execution command;
- `EXECUTE_ONCE`;
- claim handling;
- artifact upload/download; or
- formal qualification decision.

## 12. Mechanical development failures

The following runs are retained as development history, not formal scientific
attempts:

- `35600447561`: runner initially imposed accidental CMake declaration-order
  equality on the semantic allowlist;
- `35600549395`: superseded intermediate state before that allowlist-order
  correction was fully reflected;
- `35600718350`: exporter translation unit lacked the anonymous-namespace
  closure;
- `35600780940`: superseded exporter-build state before the TU correction;
- `35601176646`: static workflow test used an incidental substring-count
  assertion for build directories.

No run above created a PREPARED package, execution authorization, claim,
terminal package or CGR gate result.

## 13. Audit decision

**PASS / REPORT-ONLY TOOLING VALIDATED.**

The report-only tooling is sufficient to represent and validate the
pre-registered CGR campaign shape without executing a formal campaign.

Curve Representation remains **IN INVESTIGATION / NOT QUALIFIED**.

## 14. Next bounded action

After this tooling branch is integrated, ordinary PR/post-merge
FAST/INTEGRATION pass, the focused CGR tooling remains green, and the tooling
checkpoint is closed, open one separate **formal preparation design /
PREPARED-package work item** governed by CGR0–CGR7.

That later phase must establish exact candidate cleanliness, preparation
sealing, source/environment identity, planned retention, and lifecycle
boundaries before any execution authorization is considered.

Formal execution remains unauthorized.

Machine-readable audit:

`docs/audits/2026-09-21-continuous-curve-geometry-regression-tooling-audit.json`.
