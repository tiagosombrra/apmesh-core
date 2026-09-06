# AP Mesh Core — Reproducible Experiment Contract

Status: SPECIFIED / IMPLEMENTATION PENDING
Last updated: 2026-09-06
Scope: Foundation evidence production before geometry implementation

## 1. Question and boundary

Can AP Mesh Core execute a declared computational experiment from a clean,
identified revision and produce a self-contained evidence bundle whose claim
fields can be independently checked and reproduced under explicitly changed
conditions?

This contract governs experiment description, provenance, execution records,
artifacts, equivalence, and failure reporting. It does not qualify the
scientific correctness of an algorithm merely because its execution is
reproducible. It does not implement geometry, predicates, topology, meshing,
parallel execution, or a universal numeric policy.

## 2. Terminology

- **repeatability**: agreement under the same declared execution conditions;
- **reproducibility**: agreement when declared conditions such as compiler,
  standard library, or build type change;
- **replication**: a new study or method addressing the same scientific
  question; excluded from this Foundation contract;
- **claim field**: a result field used by an acceptance decision;
- **provenance field**: identity of an input, activity, environment, or output;
- **volatile field**: a pre-declared field, such as a timestamp or output root,
  that may differ without changing a claim;
- **evidence bundle**: the manifest, execution records, artifact inventory,
  certificate, report, and declared derived artifacts for one experiment.

Reproducibility is always stated together with the conditions that changed. A
successful replay establishes agreement only inside that declared envelope.

## 3. Basis and limits of the references

- The National Academies' *Reproducibility and Replicability in Science*
  motivates computational reproduction from the same data, code, and methods.
  It does not define AP Mesh acceptance criteria.
  https://nap.nationalacademies.org/catalog/25303/reproducibility-and-replicability-in-science
- NIST terminology motivates declaring which conditions remain fixed and which
  change when repeatability or reproducibility is claimed. It does not imply
  cross-platform equivalence without evidence.
  https://www.nist.gov/pml/nist-technical-note-1297/nist-tn-1297-appendix-d1-terminology
- W3C PROV-DM motivates explicit identities and derivation links between used
  inputs, execution activities, and generated outputs. Full PROV serialization
  is not required by this contract.
  https://www.w3.org/TR/prov-dm/
- RFC 8785 demonstrates why invariant JSON representations support stable
  hashes. It is informative here: AP Mesh may use a narrower versioned
  canonical form, but must test and document it explicitly.
  https://www.rfc-editor.org/rfc/rfc8785.html
- The FAIR principles motivate machine-actionable metadata and reusable
  research objects. FAIRness is not itself proof of reproducibility.
  https://doi.org/10.1038/sdata.2016.18

## 4. Required experiment definition

Before execution, a versioned profile must declare:

- schema version, experiment ID, descriptive question, and bounded claims;
- admissible inputs and SHA-256 identities;
- source revision and required clean-tree state;
- execution matrix and the exact conditions varied between cells;
- commands as argument vectors, working directory, environment additions, and
  per-step timeout;
- expected artifact names, roles, schemas, and required/optional status;
- claim fields, volatile fields, and equivalence rule for each artifact;
- acceptance gates and explicit `PASS`/`BLOCKED` rules;
- retained limitations and out-of-scope claims.

The profile contains scientific intent. A prepared manifest binds that intent
to one committed candidate, its complete tracked-source inventory, concrete
tool identities, concrete input hashes, and a new empty output root.

## 5. State machine

The only valid forward lifecycle is:

```text
SPECIFIED
  -> PREPARED
  -> RUNNING
  -> EXECUTED_PENDING_AUDIT
  -> QUALIFIED | BLOCKED
```

Preparation must not execute the experiment. Execution requires an explicit
execution flag and an unchanged `PREPARED` manifest. A terminal manifest is
never reused as a prepared plan. An interrupted or failed activity produces an
explicit incomplete/blocked record; it is not rewritten as a clean attempt.

## 6. Provenance and execution invariants

1. **Revision binding** — source commit, tracked-source hashes, profile, tools,
   and all scientific inputs identify the executed candidate.
2. **Clean candidate** — tracked or untracked repository changes are rejected
   before preparation and again before execution.
3. **Empty output root** — scientific outputs start in a new empty directory;
   launcher bookkeeping lives outside that root until execution begins.
4. **Exact invocation** — every command records its argument vector, working
   directory, relevant environment, start/end times, exit code, and separate
   stdout/stderr hashes.
5. **No hidden recovery** — retries, rescue parameters, fallback algorithms,
   acceptance relaxation, and silent command substitution are forbidden.
6. **Complete artifact inventory** — every required output records path, role,
   size, schema where applicable, and SHA-256.
7. **Explicit derivation** — each derived table, report, or figure names the
   machine-readable artifact from which it was generated.
8. **Atomic qualification** — collectors may report evidence, but only a
   separately recorded audit may change a scientific gate to `QUALIFIED`.

## 7. Equivalence rules

Equivalence is declared per artifact before execution:

- `byte_identical`: SHA-256 must match exactly;
- `canonical_json`: parsed content must match after the contract's versioned
  canonical serialization;
- `semantic`: every declared claim field must match under a field-specific
  rule; ignored/volatile fields must be listed exhaustively;
- `numeric`: allowed only when the owning scientific contract declares the
  quantity, units, scale, comparison rule, and failure semantics.

Paths, timestamps, and host-local process IDs may be volatile provenance, but
they may not be silently removed from comparison. A volatile field cannot be a
claim field. Missing, additional, duplicated, non-finite, or schema-invalid
claim data is `BLOCKED`, not equivalent.

## 8. Failure contract

The launcher and collector must distinguish at least:

- invalid profile or schema;
- dirty or changed candidate;
- missing or changed input;
- unsupported environment;
- non-empty output root;
- command failure or timeout;
- missing, unexpected, malformed, or tampered artifact;
- replay disagreement;
- prerequisite regression failure;
- incomplete provenance.

Every failure retains the available records and returns a nonzero status. A
failed cell remains evidence and is never replaced by an implicit rerun.

## 9. Bounded qualification gates

| Gate | Required evidence |
| --- | --- |
| E0 — scope | Experiment tooling remains outside the scientific core; no geometry or algorithm change enters the candidate. |
| E1 — specification | Profile, claims, matrix, artifacts, equivalence rules, gates, and limitations are complete before execution. |
| E2 — identity | Candidate, tools, inputs, and prerequisites are revision-bound and hash-complete in both independent replays. |
| E3 — execution | Every planned cell has complete command/environment/exit/log records with no hidden retry or fallback. |
| E4 — artifacts | Required artifacts are present, schema-valid, derivation-linked, and hash-inventoried. |
| E5 — repeatability | Repeated execution under each fixed cell produces the declared equivalent claim artifacts. |
| E6 — reproducibility | Declared claim fields agree across the pre-registered changed compiler/library/build conditions. |
| E7 — rejection and preservation | Negative fixtures are rejected and the qualified Architecture and Numeric contracts remain passing. |

The contract becomes `QUALIFIED` only if E0–E7 pass in one pre-registered clean
campaign and a separate audit records the decision. Preparation, implementation,
focused tests, or a green summary alone do not qualify it.

## 10. Qualification effect

Passing E0–E7 advances Foundation from 50% to 75% and authorizes preparation of
the Foundation End-to-End Regression. Failure leaves Foundation at 50% and
requires one bounded diagnosis of the failed gate.

## 11. Retained limitations

Initial qualification is limited to local WSL Ubuntu 24.04 execution using the
already qualified GCC 13/libstdc++ and Clang 18/libc++ Debug/Release cells. It
does not qualify native Windows, other operating systems, distributed execution,
external datasets, long-running mesh campaigns, stochastic algorithms,
archival longevity, or independent institutional replication.
