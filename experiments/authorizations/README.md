# Formal TMR execution authorizations

This directory stores repository-resident authorization records for formal
one-shot scientific executions.

An authorization record is not a configuration template. It is a specific,
immutable decision to consume one already-audited PREPARED package exactly once.

For the current Topological Model campaign:

- the authorization filename is bound to the prepared-manifest SHA-256;
- the record must be introduced as a **new file** by a separate pull request;
- the protected-`main` merge is the authorization event;
- the authorization controller validates the exact candidate, preparation run,
  artifact ID/digest, prepared-manifest hash, preparation-seal hash, preparation
  audit authority, executor path, and terminal-audit requirement;
- the reusable executor independently revalidates the committed authorization
  before checking out the historical scientific candidate;
- an immutable manifest-hash claim is created before `execute`;
- after the claim exists, the formal attempt is consumed even if execution
  later blocks;
- no retry, rescue dispatch, or replacement authorization is implicit;
- the terminal package must be independently audited before any scientific gate
  or qualification decision.

The actual `EXECUTE_ONCE` record is intentionally absent from tooling
implementation changes. It is added only after the authorization mechanism
itself is merged, validated, and operationally closed.
