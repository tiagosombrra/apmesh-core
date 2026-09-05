# Architecture Decisions and Bounded Bootstrap Regression

Status: QUALIFIED / AUDITED
Date: 2026-09-05 (America/Fortaleza)
Protocol version: 1
Authority: `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md`
Current status: `docs/APMESH_CORE_STATE.md`

## Question and claim boundary

Can an independent application consume the minimal C++23 library and reproduce
its structured bootstrap result under the declared serial toolchains, with
explicit checks, isolated build requirements, and traceable evidence?

This is an engineering verification claim. It contains no mathematical theorem,
geometric admissibility definition, mesh-quality claim, or topology-preservation
claim. Formal definitions, software contracts, and empirical tests remain
distinct. Passing this protocol qualifies the Architecture Contract only;
Numeric, Reproducible Experiment, and Foundation end-to-end gates remain open.

## Observed starting point

The pre-registration began before the first local commit. The formal candidate
is now `b468676411745e13e530b08e3b44b2cb2656d661` on `main`, with a clean
working tree and no configured remote. Earlier GCC and Clang Debug results are
retained as local toolchain evidence, not evidence for this protocol. The
formal four-cell regression was subsequently executed on the final committed
candidate; this opening record remains the immutable pre-registration context.

## Final qualification evidence

The final candidate is `238dba4c95f90406dfe30aedfc1f9cb74bc03158` on
`foundation/architecture-contract-regression`. The external manifest
`apmesh-core-architecture-contract-final-20260905-110722/manifest.json` has
SHA-256 `110babf560e9a3ffd2bf720b3f42ad330a4409c348e0209a2ea6361529867f8a`.
All four cells passed configure, build, CTest, consumer, and certificate checks;
all twelve certificates were byte-identical, both reports matched, five
negative fixtures were rejected, and all twelve scratch checks recorded
unchanged directories. The eight requirements are qualified for WSL Ubuntu
24.04.

This qualification does not cover native Windows, numeric correctness,
geometry, topology, meshing, or the remaining Foundation contracts.

| Evidence inspected | Observation | Consequence |
| --- | --- | --- |
| `CMakeLists.txt:5`, `include/apmesh/core/bootstrap.hpp:17` | One static library and one no-input bootstrap operation | Preserve the small implementation surface |
| `tests/bootstrap_smoke.cpp:8` | All result checks use `assert` | Checks must remain active with `NDEBUG`; Debug PASS does not establish Release verification |
| `build/gcc-debug/build.ninja`, `build/clang-debug/build.ninja` | Library uses `-std=c++23`; smoke uses `-std=gnu++23` | Set extensions policy explicitly on each project-owned target |
| `CMakePresets.json:28` | libc++ selected through global `CMAKE_CXX_FLAGS` | Replace this dependency on global flags with target compile/link requirements |
| `src/core/bootstrap.cpp:5` | Returns fixed strings; no runtime error branch | Do not claim tested scientific failure handling |
| GCC external log and Clang `LastTest.log` | One passing Debug smoke per compiler | Retain these limited results; repetitions and external consumer remain untested |
| Repository file inventory | No experiment exporter, comparer, or independent consumer | These are implementation prerequisites, not assumed PASS results |

Starting source bytes (SHA-256; no line-ending normalization):

| File | SHA-256 |
| --- | --- |
| `CMakeLists.txt` | `67f80ac4cdae48e23700570c8e7f9701f4d6f6e8bc9c020ef36e6446c47a43c1` |
| `CMakePresets.json` | `fd3dae13fb82c6a484edb26dcf543fd0c83df2cff07ac5a50be7aca3952b5cae` |
| `include/apmesh/core/bootstrap.hpp` | `7f653299b0d1173709c3760fc83f5c71189d6efdc3464b490786f0d026583d90` |
| `src/core/bootstrap.cpp` | `f68d5f7ccca3218ec1416eabe92d2d0baddf142e7583844cfc6e413b66c3dc99` |
| `tests/bootstrap_smoke.cpp` | `5ae152a3e49bc14667a0be67b110a9fd17ae984b81a4fb5134a8234119088ab9` |

## Resolved architecture choices

| Question | Decision | Alternative not selected / boundary |
| --- | --- | --- |
| Public and private layout | Public headers: `include/apmesh/<module>/*.hpp`; private implementation: `src/<module>/`; tests: `tests/`. Preserve current `core/bootstrap.hpp` and `src/core/bootstrap.cpp` | No file moves or empty future-module folders; no private header exposed transitively |
| Build and package identity | Keep `apmesh_core` with alias `apmesh::core`. Reserve `find_package(apmesh_core CONFIG REQUIRED)` and `apmesh_coreConfig.cmake` for an installed package | Qualify `add_subdirectory` consumption now. Install/export, version compatibility, and binary distribution are deferred until an installation user exists; no installed-package claim |
| Core result and ownership | Keep the bootstrap value and its literal-backed `string_view` lifetime. Core returns data; callers own serialization and output | No filesystem, environment discovery, logging, or mutable global state in core; no new ownership framework |
| Bootstrap failure vocabulary | In the next implementation, make infallible `inspect_bootstrap()` return `BootstrapCertificate` directly and remove the unreachable `BootstrapError`. Exercise `std::expected<int, LocalProbeError>` success and `std::unexpected` in the test only | The current public return type will change, with the same returned data; record this engineering API change. Do not fabricate a production failure or claim scientific error handling. Future fallible operations must declare real failure conditions |
| Certificate output | A small experiment executable calls the library and emits a fixed-schema JSON record. One Python-standard-library script validates it and regenerates a Markdown result table | No JSON dependency/parser inside core, general serializer, plugin framework, or separate runner/collector/comparer stack |
| Build requirements | C++23 and standard-library ABI requirements propagate with the target. Warnings remain private; extensions are explicitly off on project-owned executables too. A declared build option selects libc++ through target compile/link usage requirements | Compiler paths belong to the declared preset/environment; a toolchain file may record them but must not inject global libc++ flags. Consumers linking the core inherit its library choice; unrelated targets do not. Mixed standard-library consumers are outside the supported envelope |
| Platform scope | WSL Ubuntu 24.04 is the current qualification envelope | Native Windows remains NOT QUALIFIED and a separate portability work unit before any Windows support claim; WSL PASS is not native Windows PASS |

### Identity allocation and canonical order: specification only

Future IDs are distinct types backed by `uint64_t`, scoped by owning model and
entity kind. Zero is invalid. The model builder assigns increasing IDs from one
in declared insertion order, with checked exhaustion before increment. No ID
is reused within the builder; a failed allocation produces an explicit error
and does not partially publish the model. External IDs are preserved as adapter
provenance and mapped explicitly; they do not become pointer identities.

Relations use IDs; ownership remains in value containers. Serialization orders
entity kinds in a documented schema order and entities by increasing ID, while
preserving semantic order for boundary traversals and orientations. Integers
used as persistent IDs are emitted as decimal strings without leading zeroes
to avoid JSON consumer precision loss. No graph-isomorphism or input-permutation
canonicalization is claimed: identical declared construction order is a
precondition of identity repeatability. Reordered construction may assign new
IDs without implying a topological change.

This resolves the allocation policy, not the future topology representation.
No ID class, builder, edge use, or topology test is required for bootstrap.
Their implementation, incidence validation, and cross-model mappings belong to
the Topological Model stage. Numeric error thresholds remain in Numeric Contract.

## Fixed regression design

Exactly four core build configurations, each with its own initially absent build
directory: GCC 13/libstdc++ Debug, GCC 13/libstdc++ Release, Clang 18/libc++ Debug,
and Clang 18/libc++ Release, on WSL Ubuntu 24.04. Record installed minor versions;
the currently observed versions are 13.3.0 and 18.1.3. Use the same candidate
source and fixtures in all cells. No optimization, compiler substitution,
parallel backend, numerical tolerance, or per-cell rescue policy is admitted.

Each cell runs the focused CTest suite once, then the experiment executable in
three independent processes: twelve certificate-producing runs total. Each
cell also builds and runs one separate minimal consumer using `add_subdirectory`
with `BUILD_TESTING=OFF`, linking only `apmesh::core`. The consumer includes the
public header first, supplies no private include paths, and uses the same compiler
and standard library as that cell. It also checks that an unrelated target did
not inherit project warnings or libc++ flags. Four consumer validations total.

No architecture gate is defined by a global test count. The required properties
below must all be covered by named checks; combining related checks in a single
small executable is acceptable. No scientific fixture campaign is involved.

### Certificate and oracle

The only accepted bootstrap JSON schema has exactly these fields and values:

```json
{"schema_version":1,"component":"apmesh::core","language":"C++23"}
```

Encode in UTF-8 without BOM, the displayed key order, no spaces and one trailing
LF. The two values must be obtained from the returned core result, not replaced
with constants by the exporter. Schema version belongs to the experiment layer.
Store a reviewed expected file under `experiments/expected/`; do not generate
the expected result from the candidate during execution. Check parsed field
types/values and exact bytes of all twelve outputs against that file and each
other. Certificate report rows identify the configuration and repetition, and
the two reports are generated in distinct initially empty report directories.
There are no floating-point comparisons in this protocol.

The comparer must reject five deterministic negative fixtures: a missing
certificate, malformed JSON, an unsupported schema version, a changed component
value, and a changed source-hash entry in a copied manifest. Negative fixtures
must be isolated copies and cannot overwrite approved evidence. A nonzero exit
from a checker is a successful negative test only for its expected diagnostic;
a crash, timeout, or unrelated parser exception is BLOCKED.

### Mandatory acceptance matrix

The numbers map directly to section 18 of the Architecture Contract, not to new
roadmap phases. Before execution every row is NOT RUN.

| Contract requirement | Required evidence and PASS predicate |
| --- | --- |
| 1. Clean configure/build/test | All four configurations exit zero; complete focused test coverage with checks active in Release; runtime success/error-state vocabulary checks and explicit mismatch detection remain active under `NDEBUG` |
| 2. Target-level build requirements | Compile/link command inspection shows C++23 without GNU extensions on project-owned targets, correct standard library, private warnings, no project-injected global flags; all four independent consumers pass with no unrelated-target leakage |
| 3. Declared dependencies | Inspect source includes, link commands, and runtime dependency lists; only declared compiler/C++ ABI/unwind/C/OS runtimes are present, all paths/versions recorded; no legacy, geometry kernel, or other library dependency |
| 4. No mutable scientific globals | Focused source review of all core files and static-storage sites finds no mutable global state; record files inspected, not merely a keyword-search PASS |
| 5. Repeated certificate | All twelve certificates match the independent expected file and each other exactly; checker negative fixtures are correctly rejected; no observed compiler/configuration effect on this fixed record |
| 6. No core filesystem I/O | Source review covers the entire current three-file surface; core contains no I/O, logging, environment or random lookup. A direct smoke process and both consumer processes execute against an initially empty scratch directory; its before/after hashes must match. Exporter I/O is confined to experiment layer |
| 7. Reproducible result from core | Experiment layer regenerates a Markdown table from saved, validated certificates twice into fresh report directories; tables match exactly and identify all four cells/three repetitions. This is the generated result required by section 18; a scientific figure is not applicable to two categorical strings |
| 8. Public header isolation | Header compiles as first include in each consumer; build dependency/include graph contains only declared public and standard headers; no private or legacy header path |

Requirement 7 selects the existing contract's generated-result alternative.
It does not remove the later Foundation end-to-end figure requirement or invent
a scientific figure for the bootstrap. That later artifact and its meaning
remain owned by the Reproducible Experiment Contract.

## Provenance, execution limits, and closure

Formal qualification requires a committed candidate, the committed protocol and
expected file, a clean working tree, and recorded source/tool hashes before and
after execution. The committed candidate now meets the revision/clean-tree
entry conditions; the launcher preflight must still record its tool and input
hashes before execution.
Local development checks may precede the commit but cannot close this protocol.
Do not use the legacy bootstrap-document commit as the new core's source revision.
No remote publication is necessary to run the local qualification.

The manifest records: protocol version/hash, candidate commit/tree, clean status,
relative source-file SHA-256 inventory, expected-file and tooling hashes, compiler,
standard library, CMake/Ninja/Python versions and absolute tool paths, OS/WSL,
working directory, arguments, environment overrides, start/end, PID, exit code,
build/binary paths and hashes, and every expected artifact/check. Hash raw bytes;
never normalize source or silently refresh mismatched evidence. Keep volatile
provenance out of the deterministic certificate/table. The final artifact index
does not recursively hash itself.

Use one experiment entrypoint and one unique OS-temporary output root. Keep
bookkeeping outside initially empty per-run output directories. Validate all
tool paths and matching compiler libraries before starting. Preserve previous
builds/logs; never auto-delete a build tree or reuse an occupied output root.
After source/test changes, old Debug results cannot substitute for the new matrix.

Estimated total machine time: 1-5 minutes, to be measured. Execution limits are
300 seconds per configure/build stage, 30 seconds per test/process, and 15 minutes
overall. These are failure limits, not scientific acceptance tolerances. On a
failure or missing evidence, stop, retain the failing stage and return BLOCKED.
Do not raise limits, retry, switch compiler, or skip cells automatically.
Commands expected to exceed 30 seconds run unattended with PID, start/estimate
in Fortaleza time, external logs, and a compact completion summary. No continuous
polling or full-log streaming.

The final summary records each of the eight requirements as PASS or BLOCKED,
supporting artifact hashes, failed/absent checks, and retained scope limitations.
Overall PASS requires every requirement PASS and unchanged candidate/input hashes.
Any unresolved defect or missing evidence is BLOCKED. Native Windows, installation,
topology, and scientific correctness remain explicitly unqualified; they are not
represented as passed checks. Changes to this protocol require a versioned
amendment before another run; failed evidence remains available.

## Versioned entrypoint

`tools/run_architecture_bootstrap_regression.py` is the single entrypoint for
this protocol. Without `--execute`, it requires a clean committed candidate and
creates only a unique external manifest and command plan. `--execute` consumes
only that existing `PREPARED` manifest after rechecking the candidate commit,
clean tree, complete source inventory, and every input hash. It is the only mode
that may configure, build, or run a cell. It must be explicitly authorized after
the launcher itself is committed. Preparing a plan never qualifies a requirement.

## Sources and limits

Sources were inspected for this decision; project choices are not mandated by them.

| Source | Supported engineering point | Not established |
| --- | --- | --- |
| [CMake packages](https://cmake.org/cmake/help/v3.28/manual/cmake-packages.7.html) | Named targets and separate installed-package contract | This repository already installs or exports a usable package |
| [CMake extensions](https://cmake.org/cmake/help/latest/prop_tgt/CXX_EXTENSIONS.html) and local 3.28.3 help | Extensions policy is a target property | Strict flags on the library propagate to all executables |
| [CMake language flags](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_FLAGS.html) and local 3.28.3 help | Global language flags affect compiler/linker invocations | Consumer isolation with the existing preset |
| [C++ assertions](https://eel.is/c++draft/assertions) | `NDEBUG` disables `assert` checks | Current Release behavior has been tested |
| [Clang toolchain](https://clang.llvm.org/docs/Toolchain.html) | Compiler, standard library, ABI, and runtime choices must be compatible | Cross-standard-library binary compatibility |
| [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259) | JSON syntax/encoding and interoperable numeric considerations | Project canonical ordering, graph identity, or scientific validity |
