# AP Mesh Core — Scientific and Engineering Reference Register

Status: ACTIVE
Last updated: 2026-09-04
Roadmap: `docs/APMESH_CORE_ROADMAP.md`

## Purpose

This register records external sources used to justify scientific, numerical, experimental, and software-engineering decisions in the greenfield AP Mesh Core implementation.

It is not a bibliography dump. Every source should be connected to a concrete design question, assumption, verification method, or implementation decision.

For every new scientific mechanism:

1. search relevant primary/authoritative literature first;
2. record candidate references here;
3. identify what each source actually supports;
4. do not promote a source into an algorithmic claim that it does not prove;
5. link the relevant references from the stage decision/closure document.

## Reference status vocabulary

- `FOUNDATIONAL` — established reference directly informing project policy.
- `ACTIVE REVIEW` — currently being used to resolve a design question.
- `CANDIDATE` — potentially useful; not yet fully reviewed for a project decision.
- `SUPERSEDED` — retained for traceability but replaced by a better source.

## Numerical computing and robustness

### Shewchuk 1997 — Adaptive precision and robust geometric predicates

Status: `FOUNDATIONAL`

Jonathan Richard Shewchuk. *Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates*. Discrete & Computational Geometry, 18(3), 1997. DOI: 10.1007/PL00009321.

Primary author-hosted paper:
https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf

Project relevance:

- demonstrates why naive floating-point determinant evaluation can make geometric predicates inconsistent near degeneracy;
- establishes adaptive-precision predicate techniques where the work increases with uncertainty;
- directly informs the future Numeric Contract and meshing-predicate design;
- supports the rule that one global geometric epsilon is not a substitute for robust predicates.

Current decision impact:

- do not design orientation/incircle/topological decisions around a universal coordinate tolerance;
- robust predicates require a dedicated literature-backed decision before meshing implementation.

### Goldberg 1991 — Floating-point arithmetic and error

Status: `FOUNDATIONAL`

David Goldberg. *What Every Computer Scientist Should Know About Floating-Point Arithmetic*. ACM Computing Surveys, 23(1), 1991. DOI: 10.1145/103162.103163.

Accessible reprint:
https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html

Project relevance:

- floating-point representation, rounding, relative error, guard digits, exceptional values, and IEEE arithmetic;
- informs distinction between exact equality, approximate numerical agreement, and scientific tolerance;
- foundational input to the Numeric Contract.

### Higham 2002 — Accuracy and stability of numerical algorithms

Status: `FOUNDATIONAL`

Nicholas J. Higham. *Accuracy and Stability of Numerical Algorithms*, 2nd ed., SIAM, 2002. DOI: 10.1137/1.9780898718027.

Publisher entry:
https://epubs.siam.org/doi/book/10.1137/1.9780898718027

Project relevance:

- finite-precision computation;
- conditioning, backward/forward error, and stability;
- informs how numerical failures and ill-conditioning should be classified rather than hidden by tolerance tuning;
- expected to support later derivative, curvature, inversion, eigensystem, and integration investigations.

### IEEE 754-2019 — Floating-point arithmetic standard

Status: `FOUNDATIONAL`

IEEE Standard for Floating-Point Arithmetic, IEEE 754-2019.

Official standard page:
https://standards.ieee.org/ieee/754/6210/

Project relevance:

- baseline semantics for binary/decimal floating-point formats, operations, exceptions, and rounding behavior;
- informs declared environment assumptions for reproducibility and numeric diagnostics.

## Scientific software engineering and reproducibility

### Wilson et al. 2014 — Best Practices for Scientific Computing

Status: `FOUNDATIONAL`

Greg Wilson et al. *Best Practices for Scientific Computing*. PLOS Biology, 12(1), 2014. DOI: 10.1371/journal.pbio.1001745.

Open-access article:
https://pmc.ncbi.nlm.nih.gov/articles/PMC3886731/

Project relevance:

- testing, version control, modularity, automation, documentation, and code review as reliability mechanisms for scientific software;
- supports the project policy that implementation evidence and software process are part of scientific reliability.

### Wilson et al. 2017 — Good Enough Practices in Scientific Computing

Status: `FOUNDATIONAL`

Greg Wilson, Jennifer Bryan, Karen Cranston, Justin Kitzes, Lex Nederbragt, Tracy K. Teal. *Good enough practices in scientific computing*. PLOS Computational Biology, 13(6), 2017. DOI: 10.1371/journal.pcbi.1005510.

Open-access article:
https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1005510

Project relevance:

- organized project structure;
- version-controlled research artifacts;
- reproducible computational workflows;
- tracking exact project versions for publication and review;
- supports the permanent roadmap/state/evidence documentation policy.

### ACM Artifact Evaluation / Reproducibility guidance

Status: `FOUNDATIONAL`

ACM/SIGSIM artifact evaluation guidance, including criteria that artifacts be documented, consistent, complete, exercisable/reusable, and associated with reproducible results.

Example current guidance:
https://sigsim.acm.org/conf/pads/2027/blog/artifact-evaluation/

Project relevance:

- motivates recording environment, expected runtime, claims, manifests, and scripts for reproduced figures/tables;
- informs `Reproducible Experiment Contract` and mandatory end-of-stage regression packages;
- long-term goal is to make doctoral computational claims artifact-evaluation-ready rather than retrofitting reproducibility at publication time.

## Modern C++ and build-system engineering

### C++ Core Guidelines

Status: `FOUNDATIONAL`

ISO C++ community. *C++ Core Guidelines*.

Repository:
https://github.com/isocpp/CppCoreGuidelines

Project relevance:

- RAII;
- explicit ownership;
- resource safety;
- interface design;
- type safety;
- supports the architecture preference for values, RAII, narrow interfaces, and explicit lifetime semantics.

Important limitation:

The guidelines are engineering guidance, not a scientific proof. Individual rules must still be judged against the core's numerical/performance requirements.

### C++23 `std::expected`

Status: `FOUNDATIONAL`

C++23 standard library vocabulary type for representing either an expected value or an error.

Reference:
https://en.cppreference.com/cpp/utility/expected

Project relevance:

- directly supports typed expected scientific/domain failure without exception-driven normal control flow;
- candidate basis for APIs such as validated model construction, integration, inversion, and geometry regularity checks.

### CMake target usage requirements

Status: `FOUNDATIONAL`

CMake official documentation — `target_link_libraries` and target usage requirements.

Reference:
https://cmake.org/cmake/help/latest/command/target_link_libraries.html

Project relevance:

- supports target-scoped `PRIVATE`/`PUBLIC`/`INTERFACE` dependency propagation instead of directory-global build state;
- informs the greenfield build architecture and dependency isolation policy.

### Architecture bootstrap decisions and bounded regression

Status: `FOUNDATIONAL` for the bounded engineering protocol, reviewed 2026-09-04.

Decision/protocol: `docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md`.

| Source inspected | Supports | Does not establish |
| --- | --- | --- |
| [CMake 3.28 packages](https://cmake.org/cmake/help/v3.28/manual/cmake-packages.7.html) | Package configuration naming, exported target interface, relocatable installation boundary | Installation is implemented or mandatory for the current `add_subdirectory` consumer |
| [CXX_EXTENSIONS](https://cmake.org/cmake/help/latest/prop_tgt/CXX_EXTENSIONS.html), also checked with local CMake 3.28.3 help | Per-target language-extension policy; explains observed `c++23` versus `gnu++23` commands | A library's extensions property qualifies its consumer |
| [CMAKE language flags](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_FLAGS.html), also checked with local CMake 3.28.3 help | Global scope of compile/link flags | Current preset proves target isolation |
| [C++ draft assertions](https://eel.is/c++draft/assertions) | Disabling of `assert` with `NDEBUG`; motivates explicit runtime test checks | Release success or failure has been measured here |
| [Clang toolchain documentation](https://clang.llvm.org/docs/Toolchain.html) | Standard library, ABI and runtime composition | A GCC/libstdc++ artifact can be linked into a libc++ consumer |
| [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259) | JSON syntax, encoding, and interoperability limits of numeric values | Project-defined canonical bytes or deterministic graph labelling |

The ID allocation/order choice is a software design decision constrained by
explicit identity; it is not a theorem about topology or arbitrary reorderings.
Fixed bootstrap strings need exact comparison rather than numerical tolerances.

## Geometry, curves, surfaces, and meshing — pending focused reviews

These areas intentionally remain incomplete. References will be added only when the corresponding roadmap investigation becomes active.

Planned focused literature reviews include:

- Bezier/B-spline curve evaluation and derivatives;
- error-controlled arc-length integration;
- curve and surface regularity/degeneracy;
- differential geometry of parametric surfaces;
- geometric approximation error and curvature-driven sizing;
- robust Delaunay/advancing-front predicates;
- isotropic and anisotropic metric meshing;
- mesh quality measures;
- surface meshing under parameterization changes;
- B-rep topology and STEP import semantics;
- non-manifold surface complexes;
- compatibility-preserving multi-patch meshing;
- mesh optimization/projection with invariant preservation;
- verification and validation of adaptive mesh generators.

## Search/decision discipline

For each active investigation, the stage document should contain a compact table:

| Question | Source | What it supports | What it does not prove | Decision impact |
| --- | --- | --- | --- | --- |

A source should not be cited merely because it is famous or related to mesh generation. The connection to the precise project claim must be explicit.

When an external implementation/library is considered as a dependency, the review must additionally cover:

- algorithmic guarantees;
- failure semantics;
- precision/robustness model;
- supported input class;
- determinism;
- license;
- maintenance activity;
- portability;
- reproducibility implications;
- whether it is used as implementation, independent oracle/reference, or both.
