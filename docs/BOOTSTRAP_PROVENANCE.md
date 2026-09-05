# AP Mesh Core Bootstrap Provenance

Date: 2026-09-04

## Source

- source repository: `tiagosombrra/adaptive-patch-meshing`
- source branch: `research/apmesh-core-bootstrap`
- source commit: `01c4c1d614979350e77632b4c55fd497a7dc51f6`

## Migrated documents

- `docs/APMESH_CORE_ROADMAP.md`
- `docs/APMESH_CORE_STATE.md`
- `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
- `docs/process/APMESH_CORE_SCIENTIFIC_WORKFLOW.md`
- `docs/process/README.md`
- `docs/research/REFERENCE_REGISTER.md`
- `docs/decisions/README.md`

## Boundary

The legacy implementation was not copied as greenfield source. This repository
starts with project contracts, process records, reference records, and decision
records only. Legacy code, fixtures, and results remain comparative evidence
until each greenfield capability is independently specified and implemented.

## Local bootstrap qualification

The first greenfield code is intentionally limited to a target-scoped C++23
library skeleton and its no-I/O smoke test. The GCC 13.3.0 configure/build/CTest
qualification is recorded in
`docs/decisions/FOUNDATION_GCC_BOOTSTRAP_QUALIFICATION.md`.

The matching Clang 18/libc++ result is recorded in
`docs/decisions/FOUNDATION_CLANG_BOOTSTRAP_QUALIFICATION.md`. These records cover
the original local Debug bootstrap; they are preserved without reclassifying
them as the full architecture regression.

On 2026-09-04 the repository still has no HEAD commit and no remote. The bounded
architecture protocol records the inspected source-file hashes in
`docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`.
That protocol is pre-registered but not executed. Its formal qualification must
identify a new core commit, clean status, and input/tool hashes; the legacy source
commit above identifies migrated documents only.
