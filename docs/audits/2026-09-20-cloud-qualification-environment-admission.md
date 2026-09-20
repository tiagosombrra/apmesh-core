# Cloud Qualification Environment Admission Audit — 2026-09-20

Status: PASS / CQE0-CQE7  
Accepted functional candidate: `952695f0456f095e4f7204d34a7652738dbd75da`  
Pull request: #13  
Scientific stage: Topological Model — Explicit Identity and Incidence

## Scope

This audit admits a bounded GitHub-hosted cloud environment for future
qualification preparation. It does not establish equivalence to the historical
WSL qualification envelope, does not prepare a TMR manifest, does not enable
qualification tooling, and does not qualify the Topological Model stage.

Work-class distribution for this infrastructure phase:

- Implementation: **0%**
- Tests/validation: **50%**
- Evidence/experiments: **25%**
- Documentation/governance: **25%**

## Pre-registered environment

Authority:
`docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md`.

The admitted identity is:

- runner label: `ubuntu-24.04`;
- image OS: `ubuntu24`;
- image version: `20260907.300.1`;
- observed OS: Ubuntu 24.04.5 LTS;
- architecture: x86_64;
- observed kernel provenance: `6.17.0-1022-azure`;
- GCC 13.3.0, Ubuntu package
  `g++-13=13.3.0-6ubuntu2~24.04.1`;
- Clang 18.1.3, Ubuntu package
  `clang-18=1:18.1.3-1ubuntu1`;
- `libc++-18-dev=1:18.1.3-1ubuntu1`;
- `libc++abi-18-dev=1:18.1.3-1ubuntu1`;
- `/usr/bin/cmake` 3.28.3,
  package `cmake=3.28.3-1build7`;
- `/usr/bin/ninja` 1.11.1,
  package `ninja-build=1.11.1-2`.

The environment validator recorded no identity failure in any accepted cell.

## First execution — retained mechanical failure

Qualification Environment run `35512991310` is retained as a mechanical
workflow failure.

The environment identity probe passed, configuration completed, and
`APMESH_ENABLE_QUALIFICATION_TESTS=OFF` was observed. The workflow then
incorrectly required the CMake cache spelling
`CMAKE_MAKE_PROGRAM:FILEPATH=/usr/bin/ninja`. CMake had correctly retained the
command-line declaration as
`CMAKE_MAKE_PROGRAM:UNINITIALIZED=/usr/bin/ninja`.

Classification:

`BLOCKED_BY_CMAKE_CACHE_TYPE_ASSERTION`

This was not a compiler, semantic, package, or scientific failure. One focused
workflow correction changed only the assertion to accept the cache type while
still requiring the exact value `/usr/bin/ninja`. No production C++,
acceptance threshold, environment identity, or scientific claim changed.

## Accepted execution

Qualification Environment run `35513051098` executed on candidate
`952695f0456f095e4f7204d34a7652738dbd75da` and completed successfully in all
four cells.

| Cell | Environment identity | Exact semantic inventory | Semantic execution |
| --- | --- | --- | --- |
| GCC Debug | PASS | 7 tests | 7/7 PASS |
| GCC Release | PASS | 7 tests | 7/7 PASS |
| Clang/libc++ Debug | PASS | 7 tests | 7/7 PASS |
| Clang/libc++ Release | PASS | 7 tests | 7/7 PASS |

Every cell explicitly observed
`APMESH_ENABLE_QUALIFICATION_TESTS:BOOL=OFF` and
`CMAKE_MAKE_PROGRAM:*=/usr/bin/ninja`.

The exact semantic inventory was:

1. `apmesh_core.bootstrap_smoke`;
2. `apmesh_core.numeric_contract`;
3. `apmesh_core.geometry_primitives`;
4. `apmesh_core.cartesian_frames`;
5. `apmesh_core.topological_model`;
6. `apmesh_core.minimal_small_linear_algebra`;
7. `apmesh_core.math_header_isolation`.

No historical qualification runner or evidence tooling entered the semantic
inventory.

## Retained workflow artifacts

Run `35513051098` retained one environment/evidence artifact per cell for 30
days:

| Cell | Artifact ID | ZIP SHA-256 |
| --- | ---: | --- |
| GCC Debug | `10606220127` | `bc8356b68dfcf3f074a12b38c653a78f0efba01ff90a3f1f06123bfbbcfe4eba` |
| GCC Release | `10605614376` | `2095760ca0c2bf56afe4a4431cb225aa95d4c63ee14ad2e5e068fb0aca15eb09` |
| Clang/libc++ Debug | `10606072224` | `a3f8937fc4398ab44a5cf3e51952e289ace9dead1157453c100ccb989d7ffba2` |
| Clang/libc++ Release | `10605769316` | `7294e1dd35c3bf83f73c28d4770e22456bb3b22ac22b17d4a943cb0c6429e4ba` |

Each artifact contains:

- `environment.json`;
- `ctest-inventory.json`;
- `ctest.log`.

Independent audit of all four downloaded `environment.json` files found
`status=PASS`, an empty `failures` array, the same declared runner image and
tool/package identities, and the recorded Azure kernel provenance.

## CQE0-CQE7 decision

| Gate | Result |
| --- | --- |
| CQE0 — Identity | PASS |
| CQE1 — Build tools | PASS |
| CQE2 — Four-cell build | PASS |
| CQE3 — Semantic inventory | PASS |
| CQE4 — Semantic execution | PASS |
| CQE5 — Evidence | PASS |
| CQE6 — Scope | PASS |
| CQE7 — Transition decision | PASS |

## Qualification-boundary decision

The cloud environment is **ADMITTED** as a distinct bounded qualification
environment candidate.

It is not declared scientifically equivalent to WSL. The historical WSL
qualifications remain scoped to their original environment. A future cloud
scientific campaign must explicitly name this admitted cloud envelope and must
not rewrite historical evidence as cloud evidence.

The `ubuntu-24.04` runner label remains mutable. The admission workflow
therefore fails closed if `ImageVersion` or any declared package/tool identity
changes. A future GitHub image transition requires a new environment review;
the expected identity must never be silently edited merely to restore a green
workflow.

## Next admissible action

The infrastructure prerequisite for cloud qualification preparation is closed.

Before a formal Topological Model cloud campaign can execute, the
pre-registered TMR protocol must be explicitly supplemented or amended to name
the admitted cloud envelope. The next bounded scientific implementation action
remains the smallest reusable report-only TMR0-TMR7 workflow; no manifest is
prepared and no formal campaign is executed by this admission audit.


## Final candidate and post-merge verification

After the admission audit text was recorded, the final PR head became
`6a934de6e8f6fae35e6c38ec45b9b1f23b170acb`.

That final candidate passed:

- FAST;
- both required INTEGRATION jobs;
- all four Qualification Environment cells in run `35513250315`; and
- the explicit major-boundary regression in run `35513567930`, with 7/7
  semantic tests passing in GCC Debug, GCC Release, Clang/libc++ Debug, and
  Clang/libc++ Release.

Run `35513567930` contained no Node.js 20 checkout deprecation warning.

PR #13 was squash-merged as
`0a7095d431e4bea3c9c73e75d22df2e713c7a8ab`.

The final reviewed PR head tree and merged commit tree are exactly equal:

`7144943abc7ffd861b92587217a112c0edf6f9b4`.

Therefore the final pre-merge environment-admission and major-regression
results apply byte-for-byte to the merged functional content.

Post-merge verification on `main`:

- FAST run `35513658207`: **PASS**;
- INTEGRATION run `35513658197`: **PASS**;
- GCC 13 Debug INTEGRATION: **PASS**;
- Clang 18/libc++ Debug INTEGRATION: **PASS**.

No post-merge semantic or infrastructure contradiction was observed. The cloud
qualification environment admission is therefore closed.
