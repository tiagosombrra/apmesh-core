# Foundation Numeric Contract — Bounded Qualification Decision

Status: QUALIFIED within the declared WSL Ubuntu 24.04 compiler envelope
Last updated: 2026-09-06
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
unchanged clean candidate when explicitly called with `--execute`.

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

## Initial audited decision — retained negative evidence

### Implementation status

The bounded implementation slice exists at candidate revision
`7827a9633e97aadcc2b2777648b02ffa7a308b8e`. This decision record does not
modify or requalify C++, tests, tooling, thresholds, or the Numeric Contract.

### Evidence status

The revision-bound Numeric Contract manifest is:

- path: `C:\Users\tiago\AppData\Local\Temp\apmesh-core-numeric-contract-prepared-20260905-181000-1939d248\manifest.json`;
- SHA-256: `ec37ca0e2f93011782f5de42535ccf95783e46c374b691a9e1748506c3d4f9e5`;
- audited dependency decision: `BLOCKED`.

The verified audit supplies the following formal gate classification. This
record makes no additional diagnosis of the blocked gates.

| Gate | Audited result |
| --- | --- |
| N0 — scope | PASS |
| N1 — environment | PASS |
| N2 — classification | BLOCKED |
| N3 — policy | BLOCKED |
| N4 — proximity | BLOCKED |
| N5 — separation | BLOCKED |
| N6 — reproducibility | BLOCKED |
| N7 — preservation | BLOCKED |

### Initial scientific qualification

**Decision: BLOCKED.** The Numeric Contract is not `QUALIFIED`, because the
N0–N7 gate requires every requirement to pass and the verified audit blocks
N2–N7. Foundation remains `IN INVESTIGATION` at 25%. This is a qualification
decision, not an implementation change or a scientific capability claim.

Geometry and robust-predicate implementation remained blocked at that point.
The admissible recovery was a bounded diagnosis of N2–N7, with any correction
requiring a new clean candidate and preservation of this failed evidence.

## NQ-R1 requalification decision

### Candidate and provenance

NQ-R1 was evaluated on the clean committed candidate
`74fede5ae5999580f2ef76e944cf61e334f44064`. The original numeric behavior in
`src/core/numeric.cpp` and its public declarations were unchanged by the
recovery. NQ-R1 added missing contract cases, independent evidence oracles,
three CTest processes per build cell, and execution of the already qualified
Architecture Contract regression.

The revision-bound evidence is:

- manifest: `C:\Users\tiago\AppData\Local\Temp\apmesh-core-nq-r1-531d0795e1a94e1e9f43a99a578f63ce\manifest.json`;
- manifest SHA-256: `bcc39af9b75a7bd6fe1a0b02607f617372dd9bd62d8014ea69d31a097eed2a9f`;
- comparison report SHA-256: `6f254659716d2c737fd715a20e85030987ff62eedda972d947ba84d7e7af652d`;
- Architecture Contract manifest SHA-256: `db57bb80cde8aea24d6b598e97f4f41b6572b817cd56b3c2a83d6ef2e1346977`;
- execution interval: `2026-09-06T10:48:19+00:00` to
  `2026-09-06T10:49:17+00:00`.

The manifest candidate, complete tracked-source inventory, and all nine input
hashes match the clean candidate. The four required GCC/Clang Debug/Release
cells completed without a nonzero positive-path command. Each cell executed
the numeric CTest set three times and produced three certificates. All twelve
CTest executions passed, all twelve certificates are byte-identical, and the
cross-cell semantic comparison passed. The dependency regression completed all
four Architecture Contract cells; all positive-path commands passed and all
five negative fixtures were rejected as required.

### Gate classification

| Gate | Audited result | Evidence basis |
| --- | --- | --- |
| N0 — scope | PASS | Core changes remain limited to scalar numeric primitives; geometry, topology, meshing, hidden policy, and core I/O are absent. Evidence I/O remains isolated in `experiments/` and `tools/`. |
| N1 — environment | PASS | Every cell reports radix 2, 53 digits, IEC 559, subnormal support, and round-to-nearest; compile commands contain no prohibited floating-point flags. |
| N2 — classification | BLOCKED | Positive/negative representative normals, zero, subnormal, maximum finite, positive/negative infinity, and quiet NaN match. The pre-registered “minimum/maximum finite values” class is not complete because neither `std::numeric_limits<double>::min()` nor `lowest()` is explicitly classified. |
| N3 — policy | PASS | Valid zero/absolute/relative/mixed policies pass; negative or non-finite allowances and zero/negative/non-finite scales are rejected with explicit classes. |
| N4 — proximity | PASS | Symmetry, reflexivity, boundary-adjacent, near-zero, large-scale, overflow, and power-of-two cases match independently encoded residual and limit oracles. |
| N5 — separation | PASS | The result and evidence types have no implicit identity conversion; evidence explicitly excludes identity creation and predicate-sign production. |
| N6 — reproducibility | PASS | GCC 13/libstdc++ and Clang 18/libc++, Debug and Release, pass three CTest processes each and produce one byte-identical certificate across all twelve processes. |
| N7 — preservation | PASS | The Architecture Contract regression passes on the same candidate in all four cells, including its five required negative checks. |

### Scientific qualification

**Decision: BLOCKED.** N0, N1, and N3–N7 satisfy their individual obligations,
but N2 does not fully satisfy the pre-registered finite-extrema fixture class.
Because the gate requires every N0–N7 obligation to pass, the Numeric Contract
is not `QUALIFIED` and Foundation remains at 25%.

No defect in `numeric.cpp` is demonstrated: the blocker is missing explicit
classification evidence for the finite extrema. The next admissible action is a
new clean candidate that adds `min()` and `lowest()` to the focused test,
certificate, and independent oracle, followed by one revision-bound rerun. It
must not change numeric semantics unless that added evidence exposes a defect.
Geometry, robust predicates, topology, meshing, native Windows, general
conditioning, arbitrary precision, and a universal scientific tolerance remain
unqualified.

### NQ-R2 re-entry scope

Candidate NQ-R2 adds only two explicit N2 cases: `min_normal` for
`std::numeric_limits<double>::min()` and `lowest_finite` for
`std::numeric_limits<double>::lowest()`. Both are independently expected to be
`normal`. The focused C++ contract, report-only certificate, and independent
evidence oracle were updated; `src/core/numeric.cpp` and the public numeric API
remain unchanged. Focused GCC 13 Debug and Clang 18 Debug contract suites pass.

Before execution, NQ-R2 was not a qualification decision: it had to run one
new clean four-cell regression and be audited from its revision-bound retained
artifacts before N2 or the overall Numeric Contract status could change.

## NQ-R2 revision-bound audit

Candidate `236d290a20227f0abd646073499c0d3e20a19f8e` was executed once from a
clean tree in the four pre-registered cells: GCC 13/libstdc++ and Clang
18/libc++, each in Debug and Release. The retained manifest has SHA-256
`8ab37917a8e8de90cbbebe7ef5d393ef76ce877acc0e64275fd5d31cc15bdbd6`.

All four cells passed three independent numeric CTest processes. Their twelve
certificates are byte-identical; all twelve environments agree on binary64,
radix 2, 53 digits, IEC 559, subnormal support, and round-to-nearest. The
independent evidence comparer passed and the complete source inventory (47
tracked files) and all nine declared input hashes match the executed candidate.
The qualified Architecture Contract regression also passed in the same four
cells; its five negative fixtures were rejected as required.

| Gate | Audited result | Evidence basis |
| --- | --- | --- |
| N0 — scope | PASS | The candidate changes only focused tests and report-only evidence; `numeric.cpp` and the public numeric API are unchanged. |
| N1 — environment | PASS | All twelve environment records satisfy the declared binary64 and rounding assumptions; no prohibited floating-point flag is present. |
| N2 — classification | PASS | The certificates explicitly classify `min()` and `lowest()` as normal, alongside zero, normals, subnormals, `max()`, infinities, and quiet NaN. |
| N3 — policy | PASS | Valid policies succeed; invalid tolerances and scales return the declared explicit errors. |
| N4 — proximity | PASS | The independently declared boundary, scaling, near-zero, large-scale, and overflow cases match. |
| N5 — separation | PASS | Evidence records no implicit identity conversion and no predicate-sign production. |
| N6 — reproducibility | PASS | All twelve certificates are byte-identical across the four cells and three repetitions. |
| N7 — preservation | PASS | The Architecture Contract passes across the four cells and rejects all five required negative fixtures. |

### Scientific qualification

**Decision: PASS.** N0–N7 satisfy the pre-registered obligations. The Numeric
Contract is `QUALIFIED` only inside the declared WSL Ubuntu 24.04 GCC 13/Clang
18 compiler envelope. Foundation advances from 25% to 50%.

This decision does not qualify native Windows, geometry, robust predicates,
topology, meshing, arbitrary precision, a universal tolerance, or a general
conditioning claim. The NQ-R1 blocked result remains retained historical
evidence; it is not erased or reclassified.
