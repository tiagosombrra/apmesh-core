# Foundation Numeric Contract — Bounded Qualification Protocol

Status: EVIDENCE TOOLING IMPLEMENTED / FORMAL REGRESSION PENDING
Last updated: 2026-09-05
Contract: `docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md`

## Question

Does the minimal numeric-primitives implementation satisfy the Numeric Contract
inside the already qualified WSL Ubuntu 24.04 compiler envelope without adding
geometry or weakening the Architecture Contract?

## Fixed scope

The candidate implements only:

- floating classification;
- explicit scale/tolerance validation;
- scalar scale-aware proximity;
- structured numeric failure and diagnostics required by those operations.

It may not implement geometry types, determinant predicates, topology, meshing,
I/O, a universal epsilon, hidden retry/fallback, or fixture-specific policy.

## Preconditions

- Architecture Contract remains `QUALIFIED`.
- Candidate revision is committed and the worktree is clean.
- All expected source, test, profile, and tool hashes are recorded before the
  formal regression.
- Focused tests pass before the formal gate is launched.

## Fixed implementation constraints

1. Public numeric policy uses value semantics and immutable inputs.
2. No mutable global or thread-local tolerance/state is introduced.
3. Invalid inputs and non-finite intermediates return explicit failures.
4. No default scientific tolerance is provided.
5. Topological identity is absent from the implementation and cannot be derived
   from proximity.
6. No compiler cell uses unsafe floating-point optimization flags.

## Verification matrix

Exactly four build cells are required:

| Compiler | Library | Build |
| --- | --- | --- |
| GCC 13 | libstdc++ | Debug |
| GCC 13 | libstdc++ | Release |
| Clang 18 | libc++ | Debug |
| Clang 18 | libc++ | Release |

Each cell executes the same focused numeric contracts three times from clean
processes. The formal runner is launched once for the candidate. A failed cell
is retained as evidence; no silent retry is allowed.

## Required fixture classes

- finite normals: positive, negative, and zero;
- positive and negative zero;
- minimum/maximum finite values and representative subnormals;
- positive/negative infinity and quiet NaN;
- valid zero, absolute-only, relative-only, and mixed allowances;
- invalid negative or non-finite allowances;
- invalid zero, negative, infinite, or NaN scale;
- exact boundary and adjacent inside/outside cases;
- near-zero and large-magnitude comparisons;
- subtraction or limit-overflow cases;
- exact power-of-two rescaling cases.

Expected classifications are declared independently of the production
implementation. Boundary inputs should use exactly representable hexadecimal
floating constants where that removes oracle ambiguity.

## Required evidence

The bounded package contains:

- candidate and input hashes;
- toolchain identities and effective compile commands;
- floating-environment observations;
- per-case expected and observed classification;
- residual and limit or an explicit failure class;
- per-cell and cross-cell summaries;
- three-repetition determinism result;
- Architecture Contract regression result;
- retained limitations.

No figure is required. A compact table and machine-readable certificate are
sufficient for this non-spatial claim. Focused GCC 13 and Clang 18 Debug
contracts pass. The report-only certificate exporter, N1 environment probe,
profile, and semantic comparer are versioned and covered by focused contracts.
The versioned formal runner is covered by a focused no-execution contract. It
writes a revision-bound `PREPARED` manifest by default and only consumes an
unchanged clean candidate when explicitly called with `--execute`. Its clean
candidate manifest has not been prepared or run.

## PASS/BLOCKED decision

The gate is `PASS` only if N0–N7 in the Numeric Contract all pass and every
cross-cell difference is either absent or explicitly demonstrated irrelevant to
the structured classification contract.

The gate is `BLOCKED` if any of the following occurs:

- NaN/Inf or an invalid policy is accepted as a successful comparison;
- overflow produces a false `within` result;
- a global/default tolerance is required;
- Debug and Release disagree on a classification;
- repetitions disagree;
- the implementation needs geometry or fixture-specific rescue;
- the Architecture Contract regression fails;
- evidence identity or provenance is incomplete.

Any unexpected result stops qualification. Correcting the implementation or the
protocol requires a recorded explanation and a new clean candidate; the failed
evidence is preserved.

## Decision effect

- `PASS`: Numeric Contract becomes `QUALIFIED`; Foundation advances from 25% to
  50%; the Reproducible Experiment Contract becomes the next investigation.
- `BLOCKED`: Foundation remains at 25%; geometry implementation remains blocked;
  the next action is a bounded diagnosis of the failed requirement.
- Preparation, documentation, tooling, or focused tests alone do not change
  scientific progress.

## Immediate next implementation slice

Implement the smallest numeric vocabulary and focused analytical contracts for
classification, policy validation, and scale-aware proximity. Do not launch the
four-cell formal regression in that implementation task.
