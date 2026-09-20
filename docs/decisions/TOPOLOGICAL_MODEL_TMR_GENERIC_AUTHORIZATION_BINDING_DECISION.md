# Topological Model TMR — Generic Authorization Binding Decision

Status: **ACCEPTED FOR INTEGRATION / TOOLING ONLY / NO EXECUTION AUTHORIZED**  
Date: 2026-09-20

## 1. Context

The first authorization-as-code implementation was intentionally bound to one
exact audited PREPARED package. That was appropriate for the first formal
campaign, but the corrected second PREPARED package has a different candidate,
run, artifact, manifest, seal and audit identity.

Recompiling those identities into the validator/controller/executor for every
future campaign would turn campaign data into infrastructure constants and
would require a tooling PR before every scientific execution.

This decision generalizes the authorization machinery while preserving the
same one-shot, repository-resident, fail-closed governance model.

## 2. Human authorization remains a repository commit

Formal execution is still authorized only by adding exactly one new file:

`experiments/authorizations/topological-model-tmr-<prepared-manifest-sha256>.json`

The authorization record remains the explicit human authorization event. A
tooling merge, PREPARED audit, branch push, workflow success, or artifact
existence does not authorize execution.

The protected-main controller is triggered only when an authorization file in
that namespace changes.

## 3. Closed authorization schema

Every authorization record contains exactly:

- `schema_version = 1`;
- `kind = topological-model-tmr-execution-authorization`;
- `authorization = EXECUTE_ONCE`;
- exact candidate commit;
- exact preparation run ID;
- exact prepared artifact ID;
- exact prepared artifact SHA-256;
- exact prepared-manifest SHA-256;
- exact preparation-seal SHA-256;
- exact integrated preparation-audit path;
- exact reusable execution workflow path; and
- `terminal_audit_required = true`.

Unknown fields, retries, alternate workflow paths, malformed hashes, nonpositive
IDs or filename/manifest mismatch fail closed.

## 4. Preparation-audit binding

New campaigns use a machine-readable preparation audit under
`docs/audits/`.

The validator requires that audit to state, structurally:

- decision `PASS`;
- lifecycle `PREPARED`;
- `execution_requested=false`;
- exact candidate;
- exact preparation run;
- exact artifact ID and artifact SHA-256;
- exact prepared-manifest SHA-256;
- exact preparation-seal SHA-256;
- no execution claim;
- no terminal manifest;
- no command records;
- no certificate index;
- no failure record;
- all gates `NOT_EXECUTED`; and
- `formal_execution_authorized=false`.

Legacy Markdown preparation audits remain readable only for historical
reproducibility of the consumed first campaign. New authorization records are
expected to bind the machine-readable audit.

## 5. Protected-main controller

On a qualifying push to `main`, the controller requires the complete
before→after commit diff to contain exactly one changed path.

That path must:

1. be newly added, never modified or replaced; and
2. match
   `experiments/authorizations/topological-model-tmr-[0-9a-f]{64}.json`.

The controller then validates the exact record and checks that the derived
manifest-hash claim tag does not already exist.

This prevents combining an execution authorization with unrelated repository
changes in the same authorization commit.

## 6. Reusable executor

The executor accepts only validated values propagated by the controller and
revalidates the committed authorization before candidate checkout.

The executor derives:

- authorization filename from the prepared-manifest hash;
- concurrency group from the prepared-manifest hash;
- immutable claim tag from the prepared-manifest hash; and
- retained terminal artifact name from the prepared-manifest hash and workflow
  run ID.

It contains no campaign-specific candidate, run, artifact, manifest or seal
constant.

## 7. Artifact provenance before download

Before restoring a PREPARED artifact, the executor queries GitHub Actions
artifact metadata and requires:

- exact artifact ID;
- artifact not expired;
- exact GitHub artifact SHA-256;
- exact preparation workflow run ID;
- exact candidate head SHA; and
- preparation branch `main`.

After download, the executor independently verifies the prepared-manifest hash,
preparation-seal hash and the complete `validate-prepared` contract before
claim creation.

## 8. One-shot semantics remain unchanged

The execution order remains:

1. validate committed authorization;
2. validate exact artifact metadata;
3. restore exact PREPARED package;
4. revalidate complete PREPARED binding;
5. create one immutable remote claim tag;
6. execute exactly once;
7. remove reproducible build trees;
8. verify terminal retention; and
9. retain the terminal package.

No retry, direct workflow dispatch, hidden continue-on-error, claim reuse or
authorization-file modification path is introduced.

## 9. Validation

Final generic-binding TMR Tooling run:

- run `35532220165`;
- GCC 13 Debug: PASS;
- Clang 18/libc++ Debug: PASS.

The run validates:

- cloud identity binding;
- evidence contract;
- corrected 56-command runner contract;
- preparation workflow contract;
- generic reusable execution workflow contract;
- generic authorization-record contract; and
- generic authorization-controller contract.

Intermediate runs `35532036957` and `35532118323` failed only because the
then-current focused authorization test still expected the historical
hardcoded `EXPECTED` object. They created no PREPARED package, claim,
authorization record or scientific execution. Subsequent test replacement
closed that mechanical mismatch.

## 10. Scientific boundary

This work changes authorization infrastructure only.

It does not change:

- production topology C++;
- the TMR scientific matrix;
- repetitions;
- semantic allowlist;
- cases;
- TMR0–TMR7 gates;
- the corrected second PREPARED package; or
- any scientific acceptance criterion.

It does not itself add `EXECUTE_ONCE` and does not authorize execution.

## 11. Next transition

After this generic-binding work item is merged, post-merge checks pass and its
checkpoint is closed, create one separate authorization-record PR containing
only the exact `EXECUTE_ONCE` record for the audited second PREPARED package.

Merging that future one-file PR to protected `main` will be the formal
execution authorization event.
