# AP Mesh Core — Architecture Contract

Status: QUALIFIED / WSL UBUNTU 24.04
Last updated: 2026-09-05
Roadmap: `docs/APMESH_CORE_ROADMAP.md`

## 1. Objective

Define the architectural constraints of the greenfield AP Mesh scientific core before any geometry, discretization, or meshing algorithm is implemented. The architecture must support mathematical verification, deterministic execution, independent module testing, reproducible experimentation, and later integration into larger applications.

## 2. Technology decisions

### Language

- C++23 is mandatory for the reference core.
- The core should use standard-library facilities before introducing third-party equivalents.
- `std::expected` is the preferred vocabulary type for expected domain/scientific failures.

### Build system

- Use modern target-based CMake.
- Require CMake 3.25 or newer; the qualified local environment provides 3.28.3.
- Prefer explicit target usage requirements (`PRIVATE`, `PUBLIC`, `INTERFACE`) over directory-global compile/link state.
- Compiler warnings, standard level, feature tests, sanitizers, and optional developer checks must be target-scoped.
- A clean checkout must be sufficient to reproduce a declared build using documented commands/presets.

### Local bootstrap toolchain

The local language/standard-library baseline is qualified on WSL Ubuntu 24.04:

- GCC 13.3.0 with libstdc++ is the primary reference compiler;
- Clang 18.1.3 with libc++ 18.1.3 is the secondary qualification compiler;
- Ninja 1.11.1 is the available generator.

Both compilers compiled and executed a C++23 `std::expected` probe with
`-Wall -Wextra -Wpedantic -Werror`. Both also cleanly configure, build, and run
the CTest smoke test for the initial `apmesh::core` target. This establishes the
language/library and project-bootstrap paths. Public-header policy is now
specified below; the Architecture Contract regression is qualified for the
declared WSL Ubuntu 24.04 envelope.

Qualification is currently limited to WSL Ubuntu 24.04. Native Windows remains
NOT QUALIFIED and requires a separate portability work unit before claiming
native support. The MSVC condition in CMake does not establish such support.

### Third-party policy

Initial scientific-core runtime dependencies: none beyond the C++ standard library.

A third-party dependency may be admitted only when an Architecture/Engineering Decision records:

1. the exact capability needed;
2. why the standard library/internal implementation is insufficient;
3. scientific correctness/robustness advantages;
4. maintenance and portability impact;
5. licensing implications;
6. reproducibility impact;
7. whether the dependency is core, optional, experimental, or adapter-only;
8. an exit strategy if the dependency becomes unavailable.

Examples of dependencies that may become justified later include a CAD/B-rep kernel for industrial STEP ingestion or a mature robust-predicate/linear-algebra implementation. Admission is not pre-authorized.

## 3. Architectural style

`apmesh-core` is a scientific library, not a monolithic application.

Initial public namespace:

```cpp
namespace apmesh {
}
```

Initial logical modules:

```text
base
math
topology
geometry
model
```

Candidate future modules are introduced only when prerequisite science is qualified:

```text
differential
metric
sizing
boundary
meshing
optimization
adaptation
certification
io
```

Initial dependency direction:

```text
base
├── math
├── topology
└── geometry

math ───────► geometry

topology ──┐
geometry ───┴──► model
```

A lower-level module must not depend on a higher-level scientific capability. In particular:

- geometry must not depend on meshing;
- topology must not infer identity through geometric tolerance;
- curve/surface mathematics must not depend on file formats;
- meshing must consume a validated model rather than repair source topology implicitly;
- certification must inspect algorithm outputs but must not silently mutate them.

## 4. CMake target strategy

Start with one scientific library target and preserve logical source boundaries:

```text
apmesh::core
```

Do not create one linker target per directory merely to appear modular.

Public headers follow `include/apmesh/<module>/*.hpp`; private implementation
stays under `src/<module>/`. Preserve the current bootstrap paths. Logical
modules do not require empty directories or premature types.

Repository/distribution name: `apmesh-core`. CMake package identity:
`apmesh_core`, with future `apmesh_coreConfig.cmake` under
`lib/cmake/apmesh_core`, exposing `apmesh::core`. The bootstrap qualifies a
separate `add_subdirectory` consumer. Installation/export and ABI/version
compatibility are deferred to a concrete packaging use case; no installed-package
support is claimed by bootstrap qualification.

Apply strict C++23 policy to all project-owned targets, including tests and
experiment executables. Standard-library selection must be a declared target
compile/link requirement propagated to consumers of the core. Warnings remain
private; global preset flags must not supply missing target requirements.

Split into multiple library targets only when at least one of the following is demonstrated:

- independent external reuse;
- meaningful compile-time/dependency isolation;
- optional capability boundary;
- different dependency policy;
- independently versioned API.

## 5. Core versus adapters

The scientific core must be independent of external file formats and presentation concerns.

Expected architecture:

```text
analytic builder ─┐
.bp adapter ──────┼──► validated canonical Model ─► scientific algorithms
STEP/B-rep adapter┘
```

The `.bp` legacy format is therefore an adapter concern, not a model-definition authority.

The future STEP/B-rep adapter may depend on a specialized third-party geometry kernel if scientifically and architecturally justified. Such a dependency must not leak into independent mathematical primitives unnecessarily.

## 6. Geometry and topology separation

### Strong identifiers

Topological/geometric entity identity uses strong explicit IDs, not coordinates or pointer addresses.

Candidate types:

```cpp
struct VertexId {
    std::uint64_t value{};
    auto operator<=>(const VertexId&) const = default;
};

struct EdgeId {
    std::uint64_t value{};
    auto operator<=>(const EdgeId&) const = default;
};

struct FaceId {
    std::uint64_t value{};
    auto operator<=>(const FaceId&) const = default;
};

struct CurveId {
    std::uint64_t value{};
    auto operator<=>(const CurveId&) const = default;
};

struct SurfaceId {
    std::uint64_t value{};
    auto operator<=>(const SurfaceId&) const = default;
};

struct PatchId {
    std::uint64_t value{};
    auto operator<=>(const PatchId&) const = default;
};
```

Exact syntax is not yet frozen, but semantic separation is.

The allocation policy is now fixed at specification level: model-local and
entity-kind-local `uint64_t` identities, zero invalid, sequential allocation from
one in declared builder insertion order, no reuse, and checked exhaustion before
increment. Identical construction order must reproduce identities. Pointer
addresses, coordinates, hashes, and unordered traversal cannot assign identity.

Canonical serialization orders non-semantic collections by schema entity kind
then ID and preserves oriented boundary order. Persistent IDs use decimal-string
encoding without leading zeroes. No invariance to input permutation or graph
isomorphism is claimed. External identifiers are adapter provenance with explicit
mapping. These policies require no topology implementation during bootstrap;
their validation belongs to the Topological Model stage.

### Fundamental invariant

```text
coincident coordinates != same topological entity
```

Coordinate proximity may answer a geometric-distance question. It must never, by itself, create topological identity.

### Edge uses / coedges

Incidence and orientation are explicit.

Candidate semantics:

```cpp
enum class Orientation {
    forward,
    reverse
};

struct EdgeUse {
    EdgeId edge;
    Orientation orientation;
};
```

Face boundaries are ordered cycles of `EdgeUse` values. Their valence is not
fixed, and the core does not define a universal four-sided `PatchSide`
enumeration. A later structured quadrilateral method may introduce local side
names in its own bounded layer without changing generic topological incidence.

## 7. Geometry versus topological entity

A topological edge and its continuous curve geometry are separate concepts:

```text
Edge = topological entity
Curve = continuous geometric representation
```

A topological face, a future patch association, and its surface
parameterization are separate concepts:

```text
Face = topological two-dimensional entity bounded by edge-use cycles
Patch = future domain association between a face and geometric data
Surface = continuous geometric parameterization
```

`FaceId` and `PatchId` are distinct identity kinds and have no implicit
conversion or equality relation. The cardinality and ownership of future
face--patch--surface associations remain deferred. This distinction is required
for correct B-rep-style incidence, repeated/coincident geometry, future
alternative parameterizations, trimmed surfaces, and controlled
reparameterization experiments. It refines candidate terminology whose exact
syntax was explicitly unfrozen; it does not change the qualified Foundation
separation between topology and geometry.

## 8. Model construction and immutability

Construction uses a mutable builder:

```text
ModelBuilder
   ↓ validate/finalize
Model
```

`ModelBuilder` may accumulate entities and report incomplete state.

`Model` represents only a successfully validated scientific input and is immutable through scientific algorithms.

A failed finalization produces an explicit error; no partially valid `Model` object may escape.

Scientific algorithms should conceptually consume:

```cpp
const Model&
```

and produce separate result objects such as discretizations, meshes, metrics, or certificates.

## 9. Ownership and lifetime

Default strategy:

- value semantics where practical;
- `std::vector`, `std::array`, and other standard containers;
- IDs for persistent relations between model entities;
- RAII for resource ownership;
- `std::unique_ptr` only where runtime polymorphism or non-value ownership is justified;
- raw pointers/references only as non-owning local views with obvious lifetime.

Avoid graph-wide raw-pointer ownership and cyclic ownership structures.

Pointer address is never part of scientific identity or deterministic ordering.

## 10. Domain error model

Expected scientific/domain failures are explicit results rather than exceptions or silent fallbacks.

Preferred interface form:

```cpp
[[nodiscard]]
std::expected<ResultType, DomainError>
operation(...);
```

Use a direct value for an infallible operation. In particular, the next bootstrap
patch will return `BootstrapCertificate` directly from `inspect_bootstrap()` and
remove the unreachable `BootstrapError`; the returned strings stay unchanged.
Test C++23 `std::expected` success/error states using a test-local specimen.
This changes the bootstrap API type, not a scientific algorithm, and does not
predefine the error taxonomy of future operations.

Initial error taxonomy is conceptual and will be refined by the Numeric Contract:

- `invalid_input`
- `invalid_topology`
- `degenerate_geometry`
- `non_regular_geometry`
- `ill_conditioned_geometry`
- `numerical_failure`
- `non_convergence`
- `unsupported_geometry`
- `contract_violation`

A domain error must preserve enough context to diagnose the failing entity/operation without parsing console text.

### Exceptions

Exceptions are reserved for exceptional infrastructure/programming failures where they provide a clear advantage. Routine geometry classification such as a non-regular curve is not an exception.

### Assertions

Assertions validate internal programmer invariants only. They are not an input-validation mechanism and must not replace scientific error reporting.

## 11. Side-effect policy

Core mathematical/scientific algorithms must not perform:

- file I/O;
- console logging;
- plotting;
- GUI operations;
- hidden environment lookup;
- mutation of global state;
- implicit random seeding.

They return structured results and diagnostics. Adapter/tool layers decide how to serialize, plot, or display them.

For the bounded bootstrap, a small experiment executable emits a fixed-schema
JSON certificate from the returned core value. One Python-standard-library
script validates, compares, and renders a result table. No JSON implementation
or filesystem concern enters the core. This narrow format is not the complete
Reproducible Experiment Contract. Exact encoding, oracle, and negative checks
are fixed in `docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`.

## 12. Numerical-state policy

Mutable global tolerance/configuration variables are forbidden.

No equivalent of a universal `POINT_EQUALITY_TOLERANCE` may be introduced.

Numeric choices must enter explicitly through typed parameters/policies or be mathematically intrinsic to the operation. The detailed policy for absolute/relative error, scale, robust predicates, conditioning, and degeneracy belongs to the Numeric Contract and is not prematurely frozen here.

## 13. Determinism contract

The serial reference implementation must be deterministic under a declared environment and input.

Scientific output must not depend on accidental ordering from:

- pointer addresses;
- thread scheduling;
- filesystem enumeration;
- unspecified hash-container iteration order;
- unrecorded random sources.

Hash maps may be used internally if needed, but externally observable scientific order/certificates must use a canonical ordering.

Parallel implementations are future optimizations and must demonstrate equivalence against the qualified serial reference.

## 14. Testability and evidence

Every public scientific operation should be testable without filesystem/model-conversion side effects.

Whenever an algorithm has useful convergence/error information, return that information with the result rather than only returning a scalar.

Preferred pattern:

```cpp
struct ArcLengthResult {
    double value{};
    double estimated_error{};
    std::uint64_t evaluations{};
    bool converged{};
};
```

The exact fields will depend on the selected algorithm, but evidence-producing APIs are preferred over opaque computations.

## 15. Testing infrastructure

Initial policy:

- CTest orchestrates test executables.
- Start with a very small internal test-support layer if needed.
- Do not build a large home-grown testing framework.
- Reassess Catch2/GoogleTest only when their material maintenance/debugging advantage exceeds the dependency cost.

Tests are separated conceptually into:

- unit/contract tests;
- analytic verification tests;
- adversarial/negative tests;
- scientific validation experiments;
- end-of-stage cumulative regressions.

## 16. Code conventions

Initial conventions:

- C++23;
- UTF-8 source;
- English identifiers and code comments;
- `PascalCase` for types;
- `snake_case` for functions/variables unless a later style decision changes this globally;
- `const` correctness;
- `[[nodiscard]]` for results whose silent loss can hide failure/evidence;
- RAII;
- narrow public interfaces;
- explicit ownership;
- no scientific preprocessor switches.

Macros are restricted to unavoidable portability/build/test mechanics. Scientific alternatives must be represented as types, functions, strategies, or explicit configuration.

## 17. External integration contract

The long-term library must support use from a larger application without requiring that application to adopt AP Mesh internal I/O or experimental infrastructure.

The public API should eventually allow:

```text
external application
   ↓
construct/import validated Model
   ↓
request certified meshing operation
   ↓
receive Mesh + Certificate / explicit failure
```

## 18. Architecture-specific regression gate

Before this Architecture Contract can close, an architecture bootstrap regression must verify at minimum:

1. clean C++23 configure/build/test from documented commands;
2. target-level CMake configuration without legacy global flags leaking into the new core;
3. no undeclared third-party runtime dependency;
4. no mutable scientific globals in the greenfield source tree;
5. deterministic repeated smoke-test certificate;
6. core smoke test requires no filesystem I/O inside the scientific operation;
7. generated result/figure can be reproduced by the experiment layer from structured core output;
8. public headers do not depend on legacy AP Mesh headers.

The contract moved from `REGRESSION PENDING` to `QUALIFIED` after all eight
requirements passed on committed candidate
`238dba4c95f90406dfe30aedfc1f9cb74bc03158` with clean source and an audited closure
record. The bounded protocol, fixed four-configuration matrix, evidence rules,
and PASS/BLOCKED criteria are pre-registered in
`docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`.

The generated-result alternative in requirement 7 is a deterministic Markdown
table derived from the structured bootstrap records. This does not waive the
later Foundation end-to-end figure requirement. Tests using only `assert` are
insufficient for Release qualification. Earlier Debug smoke results remain valid
within their recorded scope and cannot substitute for this regression.

## 19. Explicit non-goals for the bootstrap

The Architecture Contract does not implement or claim correctness of:

- `.bp` import;
- STEP import;
- Bezier curves;
- arc length;
- curvature;
- patches/surfaces;
- sizing fields;
- AFT;
- adaptive refinement;
- compatibility;
- OpenMP/MPI;
- quad-dominant or anisotropic meshing.

Those capabilities must enter only through their roadmap stages.

## 20. Decision completion and remaining qualification work

The three previously open architecture questions are resolved:

- header layout and package naming: section 4;
- certificate serialization and reporting boundary: section 11;
- ID allocation and canonical order: section 6.

Ownership, immutability, fallible/infallible API distinction, and determinism are
specified by this contract; future scientific types remain subject to their own
stages. This document records architecture decisions, not completed geometry or
scientific proof. The later Foundation closure is recorded separately in
`docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md`; Geometry Primitives
remains a separate, not-started stage.
