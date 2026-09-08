# Foundation End-to-End Regression — Bounded Pre-registration

Status: PRE-REGISTERED / FPR0–FPR6 IMPLEMENTED / FOCUSED CONTRACT PASS / NOT PREPARED / NOT EXECUTED
Last updated: 2026-09-08
Stage: Foundation — Architecture, Numerics, and Reproducibility

## 1. Question

Do the qualified Architecture, Numeric, and Reproducible Experiment contracts
remain jointly satisfied by one clean current revision, with their evidence
reproduced, compared with the accepted Foundation baseline, and retained without
an undeclared runtime dependency?

This is the cumulative Foundation exit gate. It does not introduce geometry,
topology, meshing, a new numerical policy, or a new scientific claim.

## 2. Preparation readiness versus scientific closure

Preparation readiness and Foundation closure are distinct decisions. The
preflight gates below may authorize creation of one `PREPARED` manifest for one
named candidate and two named external roots. They cannot declare Foundation
`PASS`, change Foundation progress, waive an FND gate, or authorize execution.

| Gate | Preparation-readiness condition |
| --- | --- |
| FPR0 — current candidate identity | The candidate is one clean, published, upstream-aligned commit, and its diff contains no geometry, topology, meshing, or undeclared scientific-policy change. |
| FPR1 — qualified authorities | The candidate contains the exact qualified Architecture, Numeric, and REC authorities, and their accepted evidence remains independently verifiable. |
| FPR2 — historical REC baseline | The retained REC package for `85d215a` passes revision-bound semantic verification and is identified only as the accepted comparison baseline. |
| FPR3 — executable path | The existing REC runner can express the fixed four-configuration, two-replay Foundation plan for the current candidate without retry, fallback, relaxed acceptance, fixture-specific rescue, or a parallel campaign framework. |
| FPR4 — report-only infrastructure | Focused contracts validate candidate/baseline separation, source identity, comparison, dependency inventory, failure classification, and output sealing. |
| FPR5 — external output safety | The proposed control and evidence roots are new, external to the repository, empty, distinct, and recorded; no execution occurs during this check. |
| FPR6 — explicit preparation authorization | A scientific audit records FPR0–FPR5 as jointly satisfied and authorizes only one `PREPARED` manifest for the named candidate and roots. |

The historical REC package at `85d215a` may establish its own E0–E7 result,
its byte and semantic integrity, its role as the accepted comparison baseline,
its historical expected claims, and that the qualified execution path worked
for that historical candidate. It cannot establish execution, replay or
cross-configuration equivalence, retained outputs, or closure for the current
Foundation candidate.

FND0–FND7 remain post-execution scientific closure gates. In particular,
historical REC evidence cannot satisfy FND2, FND4, or FND6 for the current
candidate. It also cannot replace current-candidate evidence required by FND0,
FND1, FND3, FND5, or FND7.

The decision relation is:

```text
FPR0 ∧ ... ∧ FPR6  =>  preparation of one bound manifest is permitted
prepared manifest  !=  execution authorization
historical REC PASS !=  current-candidate FND2 ∧ FND4 ∧ FND6
Foundation PASS    <=> current-candidate FND0 ∧ ... ∧ FND7 after execution
```

Facts checked during preparation that also contribute to FND0, FND1, FND3,
FND5, or FND7 must be bound to the prepared candidate and revalidated in the
terminal closure package. Preparation evidence is not carried forward as an
unconditional pass.

## 3. Fixed execution envelope

The formal regression will use the already qualified REC execution path rather
than introduce a parallel campaign framework:

- platform: WSL Ubuntu 24.04 only;
- configurations: GCC 13/libstdc++ Debug and Release, Clang 18/libc++ Debug and
  Release;
- replays: two independent replays per configuration;
- source: one detached, clean candidate revision;
- outputs: one new empty external control root and one new empty external
  evidence root;
- prerequisites: the Architecture and Numeric formal regressions executed and
  linked on the same candidate;
- attempt policy: one attempt, no retry;
- retention: one canonical revision-bound package under `evidence/foundation/`.

The accepted comparison baseline is the qualified REC package at
`evidence/foundation/reproducible-experiment-contract/rec-e0-e7-85d215a/`.
Source inventory, documentation, tooling, absolute path, PID, timestamp,
duration, and output-root differences must be declared and classified; their
presence alone does not authorize a claim change. Any observed behavior or
claim-field effect is a regression or investigation until justified.
Architecture/Numeric claim fields, gate outcomes, artifact roles, and
deterministic scientific values must remain equivalent.

## 4. Required evidence

The terminal package must contain or link, with hashes:

- candidate identity and complete tracked-source inventory;
- prepared manifest, launch plan, state history, and command records;
- all eight REC replay bundles and their seals;
- current-candidate Architecture and Numeric prerequisite manifests;
- replay and cross-configuration comparisons;
- eight negative-fixture outcomes;
- comparison with the accepted Foundation baseline;
- dependency inventory for the produced executables;
- compact stage certificate, table, status figure, and retained limitations;
- canonical retention manifest and successful detached-worktree verification.

The status figure is a gate/provenance visualization. No spatial or geometric
figure is required before geometry exists.

## 5. Pre-registered gates

| Gate | PASS condition |
| --- | --- |
| FND0 — scope and identity | Candidate is clean, published, upstream-aligned, and contains no geometry/meshing implementation or undeclared scientific change. |
| FND1 — qualified authorities | Architecture, Numeric, and REC decisions are qualified and their accepted evidence is present and independently verifiable. |
| FND2 — cumulative execution | The fixed four-configuration, two-replay REC matrix completes once on the candidate, including same-revision Architecture and Numeric prerequisites. |
| FND3 — build and contract health | Every clean configure/build and registered Foundation contract test succeeds in the declared compiler/standard-library envelope. |
| FND4 — deterministic claims | Same-configuration replays and cross-configuration claim fields are equivalent under the qualified REC rules. |
| FND5 — accepted-baseline preservation | Comparison with `85d215a` reports only `NO_CHANGE` or pre-declared `EXPECTED_CHANGE`; no `REGRESSION` or unresolved `INVESTIGATION_REQUIRED` remains. |
| FND6 — artifacts and retention | Certificates, table, figure, logs, inventories, hashes, failure evidence, and package seals are complete; the retained package verifies from a detached candidate worktree. |
| FND7 — dependency and closure | No undeclared third-party runtime dependency exists, limitations are explicit, and the Foundation closure package names the exact evidence and revision. |

Overall `PASS` requires FND0–FND7 to pass together. Missing evidence is not a
pass. An unexpected difference is `INVESTIGATION_REQUIRED` until resolved by a
separate decision; it may not be silently reclassified.

## 6. Failure and stop policy

Any failed command produces `BLOCKED`, preserves partial evidence, and stops
the single attempt. A failure does not authorize a retry, workaround, expected
result update, or contract relaxation. If evidence invalidates a qualified
prerequisite, that prerequisite is marked `REOPENED` before further work.

## 7. Decision effect

- `PASS`: Foundation advances from 75% to 100%, becomes `QUALIFIED`, and
  Geometry Primitives may be opened by a separate bounded decision.
- `BLOCKED`: Foundation remains at 75%; exactly one bounded diagnosis is opened
  for the first failed gate.

## 8. Retained limitations

- Qualification remains limited to WSL Ubuntu 24.04 and the declared compiler
  envelope; native Windows is not qualified.
- This gate makes no geometry, topology, meshing, convergence, performance,
  parallel-equivalence, or universal-portability claim.
- It validates composition of the current Foundation contracts; it is not a
  proof that future stages preserve them.

## 9. Infrastructure audit and qualifier

The existing REC runner and validator provide mechanisms that a future
current-candidate execution may use to produce evidence for FND2 and FND4 and
the REC portion of FND6: fixed matrix execution, same-revision prerequisites,
command provenance, artifact seals, replay and cross-configuration equivalence,
negative fixtures, and explicit terminal state. Their historical use at
`85d215a` does not itself establish those gates for a later Foundation
candidate. They also do not establish current Foundation publication alignment,
authority identity, the complete CTest inventory, runtime dependencies, or
retention of current Foundation closure outputs.

The bounded final-stage addition consists only of:

- `experiments/profiles/foundation_end_to_end.json`, which fixes the accepted
  baseline, permitted non-scientific source scope, executables, runtime
  dependency envelope, limitations, and FND0–FND7 identities;
- `tools/foundation_end_to_end_evidence.py`, which validates publication
  alignment, compares REC/Architecture/Numeric claim projections with the
  accepted baseline, validates the cumulative contract-test inventory, rejects
  undeclared dependencies and non-system resolutions, and emits a compact
  certificate, table, report, status figure, and artifact inventory;
- one focused contract covering accepted evidence, a claim regression, an
  undeclared dependency, a non-system dependency resolution, incomplete
  contract-test coverage, a non-aligned candidate, and an undeclared protected
  source change.

### Revision-bound admission correction

The qualifier now requires one hash-bound admission-input manifest. It binds
the candidate revision, profile, retained REC package, publication record,
authority record, CTest discovery/JUnit outputs, and executable/`ldd` evidence.
For every declared cell, it records clean Git source inventories before and
after the actual CMake configure and build
commands, produced executable hashes, verbose CTest discovery, CTest JUnit
result, and `ldd` invocation. The qualifier independently compares publication
with Git, recomputes CTest coverage and dependency findings from raw artifacts,
verifies authority bytes, and seals its own output into a Foundation retention
manifest.

Each retained CTest and `ldd` artifact must carry its completed command record:
argv, child PID, timestamps, exit status, and hash-bound stdout/stderr. The
Foundation package retains the transitive executables and raw outputs, the
candidate REC package, and proves candidate identity through a detached Git
worktree. In that worktree it revalidates retained authority, build,
dependency, discovery, and CTest evidence against recorded roots and the
candidate revision. These are admission safeguards only; they do not prepare
or execute the formal regression.

The former broad path allowlist is replaced by exact support-path digests, an
exact verified historical-evidence prefix, and protected core paths. FND1 and
FND6 are not constants: they require qualified-authority verification and a
verified Foundation retention package. The focused contract includes forged
publication, authority mutation, incomplete CTest, binary-hash mismatch,
protected-source, and post-seal tampering negatives.

The qualifier is report-only. Complete evidence is emitted as
`EVIDENCE_COLLECTED_PENDING_AUDIT`; it cannot declare scientific `PASS`.
No campaign runner, scientific threshold, C++ behavior, preparation, or
execution is introduced by this correction.

### Preparation-readiness implementation

The qualifier now exposes a separate FPR0–FPR6 preflight. It receives the
historical REC baseline package, a clean current source root, and proposed
external control and evidence roots. It verifies source publication and scope,
current qualified-authority bytes, historical baseline integrity, fixed-matrix
runner identity, report-only tool/contract identity, and output-root safety.
It does not create a manifest, write campaign evidence, or invoke the runner.

FPR6 remains `EVIDENCE_COLLECTED_PENDING_AUDIT` whenever FPR0–FPR5 are
complete. It can be closed only by a separate scientific decision. The focused
contract uses a distinct current source revision and keeps the REC authority
marker unchanged; it proves that the historical package cannot be supplied as
the current candidate and exercises FPR0, FPR1, and FPR5 negative cases.

The focused qualifier contract creates a disposable CMake project and executes
the four GCC/Clang × Debug/Release cells. It rejects altered executable and
CTest-discovery evidence, then revalidates a retained package in a detached
worktree. This validates the report-only mechanism only; it does not satisfy
an FND gate.

## 10. Next bounded action

Audit the FPR0–FPR5 evidence for the current clean candidate and decide whether
FPR6 may authorize one `PREPARED` manifest. Do not prepare or execute the formal
regression in that decision.
