# Topological Model — Cumulative Regression Protocol

Status: PRE-REGISTERED / NOT PREPARED / NOT EXECUTED
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

- `PASS`: Topological Model becomes `QUALIFIED` only in the declared WSL
  envelope. A separate scientific entry decision may then open Curve
  Representation; no curve implementation begins automatically.
- `BLOCKED`: Topological Model remains `IN INVESTIGATION`; accepted Foundation
  and Geometry qualifications remain intact unless the evidence directly
  contradicts one of them.

This pre-registration creates no profile, tooling, manifest, execution, gate
result, or qualification claim.

## 13. Next bounded action

Implement the smallest reusable report-only TMR0–TMR7 workflow and its focused
contracts without changing production C++, preparing a manifest, or executing
the formal campaign.
