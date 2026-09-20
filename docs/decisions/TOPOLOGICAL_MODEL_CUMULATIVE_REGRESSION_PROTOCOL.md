# Topological Model — Cumulative Regression Protocol

Status: PRE-REGISTERED / REPORT-ONLY TOOLING IMPLEMENTED / FOCUSED CONTRACTS PASS / CLOUD ENVELOPE SUPPLEMENT ACCEPTED / FAIL-CLOSED CLOUD IDENTITY BINDING IMPLEMENTED / EXACT CLOUD TOOL PLAN SEALED / PREPARATION-ONLY WORKFLOW IMPLEMENTED / STATIC CONTRACT PASS / NOT DISPATCHED / NOT PREPARED / NOT EXECUTED
Date: 2026-09-20
Stage: Topological Model — Explicit Identity and Incidence

## 1. Question and boundary

Can the completed bounded topology components operate together on one clean,
published revision while preserving the qualified Foundation and Geometry
Primitives contracts and without inferring topology from coordinates?

This is the single cumulative qualification gate for the Topological Model
stage. It does not authorize another production topology concept, a component-
specific formal campaign, or any curve, surface, patch, meshing, adjacency,
pairing, boundary, or manifold behavior.

## 2. Authority and prerequisites

The protocol is bounded by:

- Foundation `QUALIFIED`, including FND0–FND7;
- Geometry Primitives `QUALIFIED`, including GPR0–GPR7;
- `docs/decisions/TOPOLOGICAL_MODEL_ENTRY_DECISION.md` and its five bounded
  contracts; and
- the focused GCC 13 Debug and Clang 18/libc++ Debug PASS evidence for identity,
  boundary cycles, reverse incidence, structural signatures, consistency, and
  canonical snapshot emission.

Historical retained packages establish prerequisite authority. They do not
replace current-candidate semantic CTests, dependency inspection, repeated
topology certificates, or the evidence required by this protocol.

## 3. Fixed claim

Within WSL Ubuntu 24.04 using GCC 13 with libstdc++ and Clang 18 with libc++, in
Debug and Release, the candidate may claim only that:

1. vertex, edge, and face identities are strong, dense, deterministic, and
   local to their entity kind;
2. edges retain their exact endpoint identities and faces retain declared loop,
   edge-use, and orientation order;
3. successful finalization is atomic and produces an immutable model whose
   reverse edge-use incidence is a bijection with the authoritative forward
   occurrences;
4. structural signatures and the consistency summary exactly recompute from
   that incidence relation without semantic relabeling;
5. equal independent constructions produce equal canonical
   `apmesh-topology-v1` bytes, while declared identity/order/orientation changes
   remain observable;
6. two executions in every declared cell have identical scientific claim
   fields; and
7. Foundation and Geometry Primitives remain preserved and production topology
   has no coordinate, geometry, I/O, logging, filesystem-order, parallel, or
   later-stage dependency.

This is empirical qualification over the enumerated synthetic envelope. It is
not a proof of graph isomorphism, arbitrary B-rep validity, or universal
topological correctness.

## 4. Explicit non-claims

The regression does not qualify or authorize:

- coordinate welding, geometric coincidence, predicates, or orientation tests;
- adjacency, mate/opposite pairing, fan order, connected components, shells,
  regions, or Euler operators;
- boundary, manifold, non-manifold, seam, embedding, or winding classification;
- persistent loop/use identity, external IDs, parsing, deserialization, schema
  migration, repair, normalization, deduplication, or ID reuse;
- `PatchId`, curves, surfaces, trimming, CAD, discretization, or meshing;
- native Windows, OpenMP, MPI, GPU, performance, or portability beyond the
  declared compiler envelope; or
- legacy AP Mesh output as an acceptance oracle.

## 5. Candidate and immutable preparation

The candidate must be one clean, committed, published descendant of the
current `main` authority and must contain exactly the reviewed five bounded
topology work units. Preparation must bind by SHA-256:

- candidate commit and tracked-source inventory;
- this protocol and every authority named in Section 2;
- exact matrix, repetition count, semantic CTest allowlist, and case index;
- any stage-level exporter, independent checker, collector/comparer, profile,
  and runner admitted later;
- invoked and resolved compiler/tool identities;
- planned commands, certificates, comparisons, failures, dependencies, and
  retained outputs; and
- a new external output root that does not exist before preparation.

A manifest is `PREPARED` only while `execution_requested=false`. Preparation
does not decide a scientific gate and execution may consume the exact manifest
once only.

## 6. Bounded execution design

The implementation step must reuse `tools/experiment_runtime.py` and existing
manifest, command-record, failure, inventory, and retention conventions. It may
add at most one stage-level profile, exporter, runner, report-only
collector/comparer, and focused tooling contract when existing entry points
cannot express the required evidence.

The historical Foundation and Geometry campaigns are not relaunched. Their
accepted records remain authority inputs, while their current semantic
contracts execute through the sealed allowlist.

The fixed matrix is:

| Cell | Compiler | Standard library | Build type | Repetitions |
| --- | --- | --- | --- | --- |
| `gcc-debug` | GCC 13 C++ driver | libstdc++ | Debug | 2 |
| `gcc-release` | GCC 13 C++ driver | libstdc++ | Release | 2 |
| `clang-debug` | Clang 18 C++ driver | libc++ | Debug | 2 |
| `clang-release` | Clang 18 C++ driver | libc++ | Release | 2 |

The runner is fail-fast for command failure and retains one structured terminal
package for both `PASS` and `BLOCKED` outcomes. No hidden retry is allowed.

## 7. Fixed semantic allowlist

Every cell must discover, build, and execute exactly once per repetition:

- `apmesh_core.bootstrap_smoke`;
- `apmesh_core.numeric_contract`;
- `apmesh_core.geometry_primitives`;
- `apmesh_core.minimal_small_linear_algebra`;
- `apmesh_core.math_header_isolation`;
- `apmesh_core.cartesian_frames`; and
- `apmesh_core.topological_model`.

The workflow must prove that every name exists exactly once and that no tooling
self-test or historical qualification runner entered the semantic allowlist.

## 8. Pre-registered topology cases

The stable case index must cover:

- exact empty-model counts and snapshot;
- distinct vertices without coordinate input, parallel edges, reversed endpoint
  order, and self-loops;
- face cycles of valence 1, 2, 3, and 5, multiple loops, and repeated edge use;
- invalid handles, invalid IDs/orientations, empty/open boundaries, and
  transactional rejection without identity consumption;
- unused, single-use, opposed two-use, co-oriented two-use, and multi-use
  structural signatures;
- same-face opposed uses, repeated owners, and three-face multi-use without
  boundary/manifold interpretation;
- exact forward↔reverse-incidence bijection and deterministic owner ordinals;
- consistency-summary counts independently recomputed from the forward model;
- byte-exact `apmesh-topology-v1` spelling, decimal IDs, record order, LF-only
  endings, and final LF;
- independent equal construction yielding equal summaries and bytes; and
- controlled insertion/order/orientation changes yielding the declared byte
  differences.

Expected observations must be independently derived from the protocol grammar
and explicit combinatorial fixtures, not copied from production output or the
legacy implementation.

## 9. TMR0–TMR7 gates

| Gate | PASS condition |
| --- | --- |
| `TMR0` — Identity and scope | Candidate is clean, published, revision-bound, hash-sealed, and contains no undeclared production or claim change. |
| `TMR1` — Construction and immutability | Strong identities, exact storage order, atomic rejection/finalization, and absence of a public mutation path pass for every declared case. |
| `TMR2` — Incidence bijection | Every forward edge-use occurrence maps to exactly one reverse record with exact face, loop/use ordinal, and orientation, with no omission or duplication. |
| `TMR3` — Structural recomputation | All five structural classes and every consistency-summary count independently recompute from authoritative topology without semantic labels. |
| `TMR4` — Canonical snapshot | Schema, records, order, decimal spelling, LF policy, final newline, equality, and controlled-difference cases match the independent checker byte-for-byte. |
| `TMR5` — Repeat and cross-cell equivalence | Eight certificates have identical scientific projections; only exhaustively declared provenance fields may differ. |
| `TMR6` — Prerequisite preservation and isolation | The exact seven-test allowlist passes in every repetition, accepted Foundation/Geometry authorities verify, and topology remains free of excluded dependencies. |
| `TMR7` — Evidence integrity and closure | Commands, certificates, comparisons, failures, inventories, authority hashes, limitations, terminal package, retention, and detached verification are complete and independently recomputable. |

Overall `PASS` requires TMR0–TMR7 together. No gate is inferred from a focused
CTest, historical package, another gate, or successful process exit alone.

## 10. Failure and stop policy

- Any unexpected production-semantic difference is `BLOCKED` and opens one
  separately authorized diagnosis; it is not fixed inside the campaign.
- A contradiction of Foundation or Geometry Primitives reopens that qualified
  prerequisite explicitly.
- One mechanical workflow defect may receive one focused correction contract.
  Two consecutive mechanical failures stop formal execution before a third
  attempt and require workflow simplification.
- Missing evidence, hash drift, zero selected tests, partial execution,
  unsealed input, or retention failure is `BLOCKED`, never partial `PASS`.
- Retry, rescue tuning, relaxed acceptance, and hidden fallback are forbidden.
- The cloud execution wrapper must create one immutable repository claim keyed
  by the prepared-manifest SHA-256 only after the complete PREPARED binding
  preflight passes and before invoking `execute`. A pre-existing claim blocks
  execution. Once the claim is created, any later failure consumes the one
  formal attempt; no second dispatch is authorized.
- Formal execution authorization is repository-resident. The only admissible
  authorization is the exact manifest-bound `EXECUTE_ONCE` JSON record
  introduced as a newly added file by a separate pull request and merged to
  protected `main`. Modification, replacement, deletion, a branch-only file,
  or any direct manual execution trigger does not authorize execution.
- The protected-main authorization controller must validate the closed
  authorization schema, reject a pre-existing claim, and call a reusable-only
  executor. The executor must independently bind the caller commit and
  revalidate the committed authorization before checking out the historical
  scientific candidate.

## 11. Required retained outputs

Retain only non-rebuildable evidence needed to recompute the decision:

- prepared and terminal manifests;
- fixed case index and eight topology certificates;
- exact semantic allowlist and command records;
- per-cell, repeated-run, and cross-cell comparisons;
- TMR0–TMR7 JSON and concise Markdown summaries;
- source, input, dependency, and retained-artifact inventories;
- negative and partial-failure evidence;
- explicit environment and claim limitations; and
- canonical retention manifest plus detached-worktree verification.

Build trees, caches, object files, executables, and reproducible binaries are
excluded while their hashes, commands, and provenance remain inventoried. No
figure is required because this stage contains no geometric embedding.

## 12. Decision effect

- `PASS`: Topological Model becomes `QUALIFIED` only in the exact environment
  formally admitted for that campaign. Historical WSL qualification evidence
  remains WSL-scoped; a cloud campaign may qualify only the separately admitted
  cloud envelope and does not establish WSL/cloud equivalence. A separate
  scientific entry decision may then open Curve Representation; no curve
  implementation begins automatically.
- `BLOCKED`: Topological Model remains `IN INVESTIGATION`; accepted Foundation
  and Geometry qualifications remain intact unless the evidence directly
  contradicts one of them.

The bounded report-only profile, experimental exporter, independent
validator/comparer, revision-bound runner, and opt-in focused tooling workflow
are now implemented. Their GCC 13 Debug and Clang 18/libc++ Debug focused
contracts passed on GitHub-hosted Ubuntu 24.04 in run `35514834796`.

This tooling result creates no formal manifest, TMR execution, gate result, or
qualification claim. It does not replace the execution environment declared by
the fixed claim above.

## 12.1 Cloud-environment supplement

The environment boundary is now supplemented by
`docs/decisions/TOPOLOGICAL_MODEL_CLOUD_QUALIFICATION_ENVIRONMENT_SUPPLEMENT.md`.

That supplement admits future formal TMR0–TMR7 execution in the exact
GitHub-hosted Ubuntu 24.04 x86_64 envelope previously admitted by
`docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md` and audited in
`docs/audits/2026-09-20-cloud-qualification-environment-admission.md`.

The scientific claim, matrix, repetitions, allowlist, cases, gates, failure
policy, and retained-output requirements in this protocol are unchanged.
Historical WSL evidence remains WSL-scoped; a future cloud PASS would be a
separate environment-scoped qualification result, not an equivalence claim.

The TMR preparation and execution binding now fails closed on the exact runner
image, package, compiler/library, architecture, and build-tool identities fixed
by the supplement. The bounded validation passed in GitHub Actions run
`35516246789` for both GCC 13 Debug and Clang 18/libc++ Debug focused tooling
cells. Run `35516204233` is retained as a mechanical protocol-guard failure
that stopped before environment evaluation. The formal launch plan now uses
the admitted absolute tool paths instead of PATH-resolved aliases; focused
GCC/Clang Debug validation passed in run `35516578411`.

The manual preparation-only workflow is restricted to explicit dispatch on
canonical `main`, invokes only `prepare` and `validate-prepared`, uses a
new external runner-temp output root, retains the sealed PREPARED package, and
contains no formal `execute` path. Consecutive mechanical quoting failures in
runs `35516864464` and `35516972035` triggered the required tooling stop.
After simplifying the shell invocation, run `35517077819` passed the complete
focused tooling inventory in GCC 13 Debug and Clang 18/libc++ Debug.

The workflow was formally dispatched once in run `35524700979` on canonical
`main` candidate `e5eda2663d6ff4b93ce1205660ff04d432acb9c0`. It retained
artifact `10609500629`, archive SHA-256
`2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72`.
The package audit in
`docs/audits/2026-09-20-topological-model-tmr-preparation-audit.md` passed.
The package remains unconsumed, `execution_requested=false`, and all
TMR0-TMR7 gates remain `NOT_EXECUTED`.

## 13. First formal execution result and next bounded action

The first formal PREPARED package was bound to candidate
`e5eda2663d6ff4b93ce1205660ff04d432acb9c0`, preparation run
`35524700979`, artifact `10609500629`, prepared-manifest SHA-256
`d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd`,
and preparation-seal SHA-256
`982e1441f08bc3f11c3cffcf73113ce69a07e2066924ad01442b3ab94eeeb71e`.

PR #29 merged the exact manifest-bound `EXECUTE_ONCE` authorization as
`8a6eafc02d5e69f467e2badfea0b571e253b84bd`. Protected-main run
`35528077223` validated that authorization, restored and revalidated the
PREPARED package, created the immutable manifest-hash claim, invoked
`execute` exactly once, verified retention, and retained terminal artifact
`10610497080` with archive SHA-256
`b332b8dde2e8651f4dd66339875378a53c9d4390400afd0d869b54969a2bf983`.

The execution claim is consumed and no second authorization, rerun, rescue
dispatch, or after-the-fact evidence completion is authorized for that
campaign.

Independent terminal audit is retained in:

- `docs/audits/2026-09-20-topological-model-tmr-terminal-audit.md`;
- `docs/audits/2026-09-20-topological-model-tmr-terminal-audit.json`.

Audit decision:

- TMR0: `PASS`;
- TMR1: `PASS`;
- TMR2: `PASS`;
- TMR3: `PASS`;
- TMR4: `PASS`;
- TMR5: `PASS`;
- TMR6: `BLOCKED`;
- TMR7: `BLOCKED`;
- overall: `BLOCKED`;
- Topological Model: `IN INVESTIGATION / NOT QUALIFIED`.

The blocking observation is evidence cardinality. Section 7 requires every
cell to discover, build, and execute the exact seven-test semantic allowlist
once per repetition, and TMR6 requires that allowlist to pass in every
repetition. The fixed matrix has four cells and two repetitions per cell.
Therefore eight semantic CTest records are required. The terminal package
contains four semantic CTest records, one per cell; each observed record is
7/7 PASS. The sealed runner repeats certificate production/validation but not
semantic CTest.

This is not evidence of a production-semantic topology defect. It is a
protocol/runner repetition-cardinality mismatch and leaves required TMR6/TMR7
evidence incomplete. The protocol is not weakened retroactively.

PR #30 integrated the terminal audit as
`83a135127302ca328bf49e3e71fbb8ac2e16da2b`; post-merge FAST
`35529230596` and INTEGRATION `35529230618` passed.

The next bounded action is one separately authorized diagnosis of this
mismatch. That diagnosis must determine the exact repetition scope of
discovery/build/semantic CTest and certificate commands, the required
command/evidence cardinality, the minimal tooling correction, and the
requirements for a new PREPARED package. It must not prepare, authorize, or
execute another formal campaign.

