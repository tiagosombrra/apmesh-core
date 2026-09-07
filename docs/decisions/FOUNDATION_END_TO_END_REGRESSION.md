# Foundation End-to-End Regression — Bounded Pre-registration

Status: PRE-REGISTERED / QUALIFIER IMPLEMENTED / FOCUSED CONTRACT PASS / ADMISSION REVIEW REQUIRED / NOT PREPARED / NOT EXECUTED
Last updated: 2026-09-07
Stage: Foundation — Architecture, Numerics, and Reproducibility

## 1. Question

Do the qualified Architecture, Numeric, and Reproducible Experiment contracts
remain jointly satisfied by one clean current revision, with their evidence
reproduced, compared with the accepted Foundation baseline, and retained without
an undeclared runtime dependency?

This is the cumulative Foundation exit gate. It does not introduce geometry,
topology, meshing, a new numerical policy, or a new scientific claim.

## 2. Preconditions

Preparation is forbidden until all of the following are true:

1. the candidate is one clean, published commit with local `HEAD` equal to its
   upstream branch;
2. the Architecture Contract is `QUALIFIED` in the declared WSL Ubuntu 24.04
   GCC 13/Clang 18 envelope;
3. the Numeric Contract is `QUALIFIED` in the same envelope;
4. the Reproducible Experiment Contract is `QUALIFIED`, and its retained
   package for candidate `85d215a` passes revision-bound verification;
5. the candidate diff contains no geometry or meshing implementation;
6. the existing REC runner can execute the cumulative matrix without hidden
   retry, fallback, relaxed acceptance, or fixture-specific rescue;
7. any missing final-stage comparison is implemented as a report-only
   extension and passes focused contracts before preparation.

Passing this pre-registration does not satisfy any execution gate.

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

The existing REC runner and validator cover the cumulative execution evidence
for FND2 and FND4, plus the REC portion of FND6: fixed matrix execution,
same-revision prerequisites, command provenance, artifact seals, replay and
cross-configuration equivalence, negative fixtures, and explicit terminal
state. They do not establish Foundation publication alignment, authority
identity, the complete CTest inventory, runtime dependencies, or retention of
the Foundation closure outputs.

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
The qualifier independently compares publication with Git, recomputes CTest
coverage and dependency findings from raw artifacts, verifies authority bytes,
and seals its own output into a Foundation retention manifest.

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

The focused qualifier contract passes in native Python and WSL Python, and its
CTest registration passes in the GCC Debug build tree. This validates the
report-only mechanism only; it does not satisfy an FND gate.

## 10. Next bounded action

Independently audit the profile, qualifier, focused contract, and their mapping
to FND0–FND7. Decide only whether preparation may be authorized. Do not prepare
or execute the formal regression.
