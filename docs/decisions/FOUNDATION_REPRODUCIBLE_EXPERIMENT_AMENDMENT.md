# Foundation Reproducible Experiment Contract — Completeness and Retention Amendment

Status: APPROVED FOR IMPLEMENTATION / REGRESSION NOT AUTHORIZED
Date: 2026-09-06 (America/Fortaleza)
Contract: `docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md`
Qualification protocol: `docs/decisions/FOUNDATION_REPRODUCIBLE_EXPERIMENT_QUALIFICATION.md`

## Question

What minimum experiment infrastructure and durable evidence package are
required before the pre-registered E0–E7 campaign can be prepared or executed?

## Decision boundary

This amendment resolves implementation and evidence-governance ambiguities. It
does not change the four compiler/build configurations, two replays per
configuration, E0–E7 gates, Numeric Contract semantics, or Architecture
Contract semantics. It does not qualify the Reproducible Experiment Contract.

The current local REC implementation is preserved as an implementation
candidate. Focused tests demonstrate useful pieces, but they do not establish
complete coverage of E0–E7. The candidate must be completed against this
amendment before a formal manifest may be prepared.

## Required lifecycle

Two roots have distinct roles:

```text
control root
  prepared-manifest.json
  launch-plan.json
  launcher bookkeeping

evidence root
  created empty when execution begins
  immutable positive and negative execution records
  collected and derived evidence
```

Preparation creates only the control root and records the intended evidence
root. Execution revalidates the complete prepared manifest and requires that
the evidence root does not exist or is empty. The launcher then persists
`RUNNING` before the first command. A terminal manifest is never rewritten to
`PREPARED` or reused.

Every attempted cell and negative fixture receives a durable record. A failed
command changes the campaign to `BLOCKED` while retaining the records already
produced. No implicit retry, command substitution, fallback, or acceptance
change is allowed.

## Completeness obligations for E0–E7

| Gate | Implementation evidence required before audit |
| --- | --- |
| E0 — scope | A revision-bound diff classification shows no change to the scientific core, Numeric semantics, Architecture semantics, geometry, topology, or meshing. Tooling and evidence changes are enumerated explicitly. |
| E1 — specification | The versioned profile declares question, claims, matrix, two replays, commands, timeouts, required and optional artifact roles, schemas, derivations, claim fields, volatile fields, equivalence rules, gates, limitations, and explicit `PASS`/`BLOCKED` conditions. Unknown or duplicate fields are rejected. |
| E2 — identity | Preparation and execution bind and recheck the candidate commit, clean tree, tracked-source inventory, exact profile and plan bytes, launcher/runtime/evaluator bytes, scientific inputs, prerequisite evidence identities, executable paths, and queried tool versions. Any difference blocks execution. |
| E3 — execution | The persisted state advances `PREPARED -> RUNNING -> EXECUTED_PENDING_AUDIT | BLOCKED`. Every command records argument vector, working directory, declared environment delta, start/end, elapsed time, process identity, timeout, exit code, and separate stdout/stderr hashes. Planned and observed cell identities must match exactly. |
| E4 — artifacts | Each bundle is schema-validated and contains exactly the declared required artifacts plus declared optional artifacts. The inventory records relative path, role, schema/version where applicable, size, and SHA-256. Derived artifacts identify the source artifact and its hash. The inventory is sealed by the enclosing campaign manifest rather than recursively hashing itself. |
| E5 — repeatability | After independent bundle validation, replay 1 and replay 2 of each fixed cell agree under the pre-registered per-artifact rules. Every volatile field remains present and is reported as an allowed difference. A stale inventory cannot substitute for a semantic comparison. |
| E6 — reproducibility | Claim fields are compared across all four GCC/Clang Debug/Release configurations after E5 passes. The summary lists every observed cross-cell difference and its pre-declared classification. No new tolerance may be introduced by the experiment layer. |
| E7 — rejection and preservation | All eight pre-registered negative fixtures reject for their intended reason. Architecture and Numeric regressions execute on the same candidate revision through their qualified protocols and their complete terminal evidence is linked into the REC campaign. Focused CTest is not a substitute. |

Collectors report evidence only. They may use `EVIDENCE_COLLECTED_PENDING_AUDIT`
for a gate only after checking every obligation mapped to that gate. Missing
coverage is `BLOCKED`, not pending audit.

## Minimum reusable infrastructure

The smallest shared infrastructure is a standard-library Python module with
these responsibilities:

- strict UTF-8 JSON loading with duplicate-key rejection and versioned
  canonical writing;
- SHA-256, clean-candidate identity, tracked-source inventory, and declared
  input identity;
- tool resolution/version capture;
- command execution records, timeouts, logs, and lifecycle transitions;
- artifact inventory and derivation links;
- profile/plan identity verification.

The implementation must declare and enforce a Python version consistent with
the language features it uses. The current tooling uses Python 3.10 features,
so the existing CMake declaration of Python 3.8 is not sufficient unless those
features are removed and the lower version is covered by tests.

A generic entrypoint may prepare and execute a profile using that module.
Scientific evaluators remain separate: bootstrap/Architecture, Numeric, and
future stage evaluators keep their own schemas, oracles, and gate semantics.

Existing Architecture and Numeric runners remain authoritative until an
adapter migration reproduces their accepted manifests, command plans, failure
classification, and resulting evidence on a fixed fixture. Shared code must be
extracted incrementally; their behavior must not be replaced by assumption.
No plugin framework, class hierarchy, scheduler, or general workflow engine is
authorized by this amendment.

## Durable evidence policy

Evidence has three classes:

1. **scratch** — build trees, caches, intermediate objects, and exploratory
   logs; disposable and never cited as the sole basis for a decision;
2. **campaign evidence** — complete external execution directory retained
   through audit and packaging;
3. **canonical evidence** — the smallest self-contained package needed to
   verify a recorded decision from a future checkout.

Canonical evidence is stored under:

```text
evidence/<stage>/<contract>/<campaign-id>/
```

The package must retain:

- the original prepared and terminal manifests;
- candidate, profile, plan, input, tool, and prerequisite-evidence identities;
- machine-readable certificates, environments, summaries, inventories, gate
  results, and negative-fixture results;
- command records and the focused log excerpts needed to support failures or
  acceptance;
- metrics, report, figure, audit metadata, and retained limitations;
- a `retention-manifest.json` that maps original external paths to retained
  relative paths and hashes every retained file except itself.

The candidate commit and the later archival commit are recorded separately.
Copying an older artifact into Git does not retroactively make its original
campaign compliant with this amendment.

Build directories, object files, compiler caches, and reproducible binaries are
excluded from Git. A large indispensable artifact may use a tagged release or
another durable store only when the canonical package records its immutable
locator, byte size, SHA-256, retention expectation, and recovery procedure.
GitHub Actions artifacts are transport or short-term execution evidence, never
the only canonical copy, because their lifecycle is tied to workflow-run
retention and deletion.

The current temporary Architecture/Numeric evidence must be inventoried before
cleanup. Missing historical artifacts remain recorded as `UNAVAILABLE` or
`PARTIAL`; their hashes and negative decisions must not be rewritten.

## CI boundary

CI may later guard configure, build, focused contracts, schema checks, and
repository hygiene for a commit. It cannot move a scientific gate to
`QUALIFIED`, update accepted evidence, or replace the explicit clean-candidate
campaign and separate audit. CI introduction is a later bounded Foundation
work unit after the reusable REC infrastructure passes its focused contracts.

## Admission gate for the formal campaign

The formal REC campaign remains blocked until one review confirms all of:

1. the E0–E7 implementation map has no uncovered obligation;
2. positive and eight negative focused contracts pass;
3. cross-replay and cross-configuration comparisons are both implemented;
4. Architecture and Numeric qualified protocols are invoked and linked;
5. control/evidence-root separation and lifecycle persistence are tested;
6. a canonical retention package can be assembled and independently verified;
7. roadmap, continuation state, contract, and protocol agree;
8. the candidate is committed, pushed, clean, and revision-bound.

Only then may a new manifest be prepared. Preparation does not authorize
execution. E0–E7 remain subject to a separate scientific audit after execution.

## Retained limitations

- Qualification remains limited to the declared WSL Ubuntu 24.04 toolchain
  envelope.
- Durable retention improves auditability but does not prove scientific
  correctness, independent replication, or archival permanence.
- No native Windows, geometry, topology, meshing, parallelism, or performance
  claim is introduced.
- Historical evidence whose original bytes cannot be recovered remains a
  traceability limitation.

## Decision

**APPROVED FOR IMPLEMENTATION.** Complete the current candidate through one
bounded infrastructure work unit. The pre-registered campaign remains
`REGRESSION NOT AUTHORIZED` until its admission gate passes.
