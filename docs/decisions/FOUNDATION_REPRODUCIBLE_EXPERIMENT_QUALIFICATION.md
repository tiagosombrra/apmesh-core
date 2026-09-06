# Foundation Reproducible Experiment Contract — Bounded Qualification Protocol

Status: PRE-REGISTERED / AMENDED / IMPLEMENTATION CANDIDATE COMPLETE / ADMISSION REVIEW REQUIRED / REGRESSION NOT AUTHORIZED
Last updated: 2026-09-06
Contract: `docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md`
Completeness amendment:
`docs/decisions/FOUNDATION_REPRODUCIBLE_EXPERIMENT_AMENDMENT.md`

## Question

Can one frozen non-geometric AP Mesh experiment be reproduced from a prepared
manifest into independently generated evidence bundles without changing the
qualified Architecture or Numeric contracts?

## Hypothesis

Given the same committed source and input identities, two independent clean
replays of the qualified Numeric Contract specimen will produce equivalent
claim artifacts in all four declared compiler/build cells. Differences caused
only by pre-declared run provenance will be identified rather than hidden.

## Preconditions

- Architecture Contract is `QUALIFIED`.
- Numeric Contract is `QUALIFIED` on candidate lineage including `236d290`.
- The implementation candidate is committed, clean, and pushed before the
  formal campaign.
- Profile, launcher, collector, comparer, expected artifacts, and negative
  fixtures are versioned and hash-bound by the prepared manifest.

## Fixed scope

The qualified Numeric Contract exporter is a frozen deterministic specimen. It
is not treated as an oracle for the Reproducible Experiment Contract; its
already accepted claim fields provide a small non-geometric workload whose
provenance and replay equivalence can be tested.

The bounded implementation may add only:

- one experiment profile and schemas;
- a preparation/execution launcher;
- artifact inventory and compact summary generation;
- semantic comparison of two independent replay bundles;
- a deterministic Markdown table and SVG status matrix derived from the
  machine-readable summary;
- focused positive and negative tooling tests.

It must not change C++, numeric semantics, geometry, topology, meshing,
acceptance policy, or the existing qualification evidence.

## Implementation-candidate status

The current local candidate contains the following implementation components:

- `experiments/profiles/reproducible_experiment_contract.json` declares the
  eight replay cells, claim and volatile fields, required artifacts, limits,
  and E0–E7 gates;
- `tools/run_reproducible_experiment_contract.py` prepares an immutable clean,
  revision-bound manifest and executes it only with `--execute`;
- `tools/reproducible_experiment_evidence.py` validates bundle schemas and
  compares the two replays of a declared cell without accepting undeclared
  claim differences;
- focused CTest contracts exercise profile validation, clean-candidate binding,
  artifact inventory, and one replay-disagreement path.

The completeness amendment defines the admission review. The candidate now has
strict profile/plan revalidation, persisted lifecycle state, command and log
provenance, exact bundle/inventory/derivation checks, replay and cross-cell
comparison, focused negative contracts, and a canonical retention assembler.
The qualified Architecture and Numeric runners remain separate authoritative
protocols; their execution and linkage are required evidence for E7 and have
not been performed by this implementation-only work unit.

No formal REC cell has been executed. A formal manifest must not be prepared
until the amendment admission gate passes. The protocol remains pre-registered
and unqualified.

## Execution matrix

Two independent output roots are executed for each cell:

| Compiler | Standard library | Build | Replays |
| --- | --- | --- | ---: |
| GCC 13 | libstdc++ | Debug | 2 |
| GCC 13 | libstdc++ | Release | 2 |
| Clang 18 | libc++ | Debug | 2 |
| Clang 18 | libc++ | Release | 2 |

Each replay starts from the same immutable prepared experiment definition but
records its own timestamps, process identities, and output root. A single
launcher invocation executes the eight planned replay cells. There is no retry.

## Required bundle

Each independent replay bundle must provide, directly or by explicit inventory:

```text
manifest.json
execution-record.json
artifact-inventory.json
certificate.json
environment.json
summary.json
report.md
metrics.csv
figures/status-matrix.svg
```

The table and SVG are derived only from `summary.json`. They visualize gate
state and provenance coverage; they make no geometric or performance claim.

## Pre-registered equivalence

- candidate, profile, tools, inputs, schemas, claim names, gate results,
  certificate classifications, metric values, artifact roles, and retained
  limitations: semantic equality required;
- canonical machine-readable claim artifacts: byte identity when the
  implementation declares and demonstrates a stable canonical serializer;
- timestamps, process IDs, elapsed time, and absolute temporary output roots:
  may differ, but must remain present and be classified as volatile provenance;
- Markdown and SVG: regenerated from the same summary and either byte-identical
  or compared by one pre-declared canonical representation;
- no numeric tolerance may be introduced by this experiment contract.

## Negative fixtures

The formal evidence must show explicit rejection of:

1. dirty candidate;
2. changed source or scientific input hash;
3. non-empty output root;
4. unsupported schema version;
5. missing required artifact;
6. tampered artifact after generation;
7. simulated nonzero experiment command;
8. attempted reuse of a terminal manifest as `PREPARED`.

Negative fixtures run as isolated tooling contracts and must not mutate a
positive evidence bundle.

## Gate decision

| Gate | PASS condition |
| --- | --- |
| E0 — scope | Repository diff contains no C++ or scientific behavior change. |
| E1 — specification | The prepared profile contains every field required by the contract and no undeclared acceptance default. |
| E2 — identity | Both replay roots bind the same clean candidate, complete tracked-source inventory, tools, profile, and inputs. |
| E3 — execution | Eight planned cells finish with complete records; no retry/fallback occurs; failures would be retained. |
| E4 — artifacts | Every required artifact is present, valid, inventoried, and linked to its source summary. |
| E5 — repeatability | The two replays inside every fixed cell are equivalent under the pre-registered rules. |
| E6 — reproducibility | Claim artifacts are equivalent across GCC/Clang and Debug/Release; every allowed provenance difference is explicitly classified. |
| E7 — rejection and preservation | All eight negative fixtures reject correctly and the Architecture and Numeric regressions pass on the candidate. |

The overall decision is `PASS` only when E0–E7 pass. Any missing provenance,
unexpected difference, prerequisite regression failure, or negative-fixture
acceptance produces `BLOCKED` and is retained.

## Evidence handling

The formal campaign must run from one new empty OS-temporary root and record:

- PID, command, working directory, start/end time, and exit status;
- candidate and remote alignment;
- all input and artifact hashes;
- per-replay and cross-cell compact summaries;
- a revision-bound final manifest in `EXECUTED_PENDING_AUDIT` state.

The first audit reads compact summaries before any focused log. It may not
reinterpret an unexpected difference as acceptable without a new recorded
decision and a new candidate.

Before this campaign may be prepared, the implementation must pass the
admission gate in
`docs/decisions/FOUNDATION_REPRODUCIBLE_EXPERIMENT_AMENDMENT.md`.

## Decision effect

- `PASS`: mark the Reproducible Experiment Contract `QUALIFIED`; Foundation
  advances from 50% to 75%; Foundation End-to-End Regression becomes active.
- `BLOCKED`: Foundation remains at 50%; open exactly one bounded diagnosis for
  the failed E-gate.

Existing Numeric Contract artifacts do not close this gate retrospectively:
they were not prepared to demonstrate two independent end-to-end replay roots,
the complete artifact inventory, or the derived table/figure contract above.
