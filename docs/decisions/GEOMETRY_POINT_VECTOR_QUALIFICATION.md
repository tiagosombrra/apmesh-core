# Geometry Primitives — Point and Vector Semantics Qualification

Status: QUALIFIED / PV0–PV7 PASS / WSL Ubuntu 24.04
Date: 2026-09-10
Protocol: `docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md`
Entry authority: `docs/decisions/GEOMETRY_PRIMITIVES_ENTRY_DECISION.md`

## Question

Can the bounded `Point2`, `Point3`, `Vector2`, and `Vector3` capability be
qualified as deterministic, finite-valued affine and Euclidean primitives
within the already qualified Foundation envelope?

## Candidate and retained evidence

The audit covers clean, published candidate
`ededf6584941de9dfe6a47633ffd67caf17d4f51`, aligned with its upstream at the
time of execution. The unchanged prepared manifest has SHA-256
`20b01cb46866ec70dd7bb3095c0ba2a59ca0e3bb3940e18186fd6f11f74874ae`.

The canonical retained package is
`evidence/geometry-primitives/point-vector/pv0-pv7-ededf65/`. It contains the
prepared and terminal manifests, 12 certificates, command records,
runtime-dependency inventory, compile-command inventories, comparison report,
negative evidence, focused logs, and canonical evidence manifest. Build trees,
caches, object files, and reproducible binaries are intentionally excluded.
The canonical manifest binds every retained file to its original path and hash,
including the source retention-manifest hash. Detached verification covered 559
tracked source files; the source retention manifest covered 313 evidence files.

## Execution and audit

The one authorized execution ran for 71 seconds in the declared WSL Ubuntu
24.04 envelope. It used GCC 13/libstdc++ and Clang 18/libc++, each in Debug and
Release, with three independent Point/Vector processes per cell.

| Gate | Decision | Audit basis |
| --- | --- | --- |
| PV0 | PASS | Clean published candidate, revision-bound manifest, and admitted Point/Vector scope. |
| PV1 | PASS | Compile-time point/vector separation accepted authorized expressions and rejected excluded ones. |
| PV2 | PASS | Fixed finite, non-finite, zero, and classified-failure cases passed. |
| PV3 | PASS | Independent affine, dot, cross, antisymmetry, and parallel-vector cases passed. |
| PV4 | PASS | Norm, normalization, extrema, subnormal, and power-of-two scale cases passed under declared rules. |
| PV5 | PASS | All 12 certificates were semantically identical across the fixed four-cell matrix. |
| PV6 | PASS | The exact Architecture, Numeric, and Reproducible Experiment preservation CTest set passed in every cell. |
| PV7 | PASS | Input hashes, observed inventories, dependency records, negative cases, retention hashes, and detached verification passed. |

The three negative checks rejected a duplicated case, unsafe compile option, and
forged proximity value in every cell. No command failed or timed out. The earlier `b7f8fe9`
attempt remains retained as `BLOCKED_BY_CONTRACT_SELECTION_DEFECT`; it is not
used as positive evidence for this decision.

## Decision and limitations

**Decision: PASS WITH RETAINED LIMITATIONS.** Point and Vector Semantics is
`QUALIFIED` only in the WSL Ubuntu 24.04 GCC 13 / Clang 18 libc++ envelope.

This decision does not qualify native Windows, portability beyond the declared
toolchains, parallelism, matrices, transformations, predicates, topology,
curves, surfaces, meshing, geometric coincidence, orientation predicates, or
the full Geometry Primitives stage. The mandatory cumulative Geometry Primitives
regression remains open.

## Next bounded action

PR #3 integrated the qualified Point/Vector package into `main` at
`1ff6568c908ea144b903a70a2497c000a89e35eb`; its tree matches reviewed source
head `b17067312b523e934c81d56b6cde7948f30ff93f`. Review the separately
accepted Minimal Small Linear Algebra Contract before its bounded
implementation. No matrix implementation is authorized by this Point/Vector
qualification itself.
