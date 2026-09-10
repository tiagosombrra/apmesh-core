# AP Mesh Core — Scientific and Engineering Reference Register

Status: ACTIVE
Last updated: 2026-09-06
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

### National Academies 2019 — Reproducibility and Replicability in Science

Status: `FOUNDATIONAL`

National Academies of Sciences, Engineering, and Medicine.
*Reproducibility and Replicability in Science*. National Academies Press, 2019.
DOI: 10.17226/25303.

Official report:
https://nap.nationalacademies.org/catalog/25303/reproducibility-and-replicability-in-science

Project relevance:

- supports the project's explicit use of computational reproducibility for
  recomputation using the same data, code, and methods;
- distinguishes that claim from replication using a new study or method;
- does not establish that reproduced AP Mesh output is scientifically correct.

### NIST TN 1297 — Repeatability and reproducibility terminology

Status: `FOUNDATIONAL`

Official terminology:
https://www.nist.gov/pml/nist-technical-note-1297/nist-tn-1297-appendix-d1-terminology

Project relevance:

- motivates stating which conditions remain fixed for repeatability and which
  conditions change for reproducibility;
- supports reporting the exact compiler/library/build envelope instead of an
  unqualified portability claim.

### W3C PROV-DM — Provenance data model

Status: `FOUNDATIONAL`

W3C Recommendation, 30 April 2013:
https://www.w3.org/TR/prov-dm/

Project relevance:

- supports explicit identities and derivation relations between experiment
  inputs, execution activities, and generated artifacts;
- informs the bundle lineage model without requiring full PROV serialization.

### RFC 8785 — JSON Canonicalization Scheme

Status: `FOUNDATIONAL`

RFC Editor entry:
https://www.rfc-editor.org/rfc/rfc8785.html

Project relevance:

- documents why invariant serialization is required before byte-stable hashes
  can represent structured JSON identity;
- informs a tested versioned canonical representation where byte identity is
  claimed;
- remains an informational reference and does not by itself qualify AP Mesh
  artifact equivalence.

### Wilkinson et al. 2016 — FAIR Guiding Principles

Status: `FOUNDATIONAL`

Mark D. Wilkinson et al. *The FAIR Guiding Principles for scientific data
management and stewardship*. Scientific Data 3, 160018 (2016).
DOI: 10.1038/sdata.2016.18.

Open-access article:
https://doi.org/10.1038/sdata.2016.18

Project relevance:

- supports machine-actionable metadata, explicit provenance, and reusable
  research objects covering data, algorithms, tools, and workflows;
- FAIRness is treated as guidance for evidence stewardship, not as proof of
  reproducibility or scientific validity.

### GitHub evidence-storage mechanics

Status: `FOUNDATIONAL` for repository storage boundaries.

Official documentation:

- https://docs.github.com/en/actions/concepts/workflows-and-actions/workflow-artifacts
- https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github
- https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases

Project relevance:

- workflow artifacts can retain logs and test outputs, but their availability
  is tied to the workflow run and therefore cannot be the only canonical copy;
- normal Git history should retain small decision-bearing evidence rather than
  generated builds or large binary products;
- tagged releases are a candidate durable distribution location for larger
  indispensable evidence, provided the repository retains immutable locator,
  size, SHA-256, retention expectation, and recovery metadata;
- these storage mechanisms improve availability and provenance but do not
  establish scientific validity or REC qualification.

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

## Geometry primitives

### CGAL 6.2.1 — Point, vector, and affine-transformation semantics

Status: `FOUNDATIONAL` for Geometry Primitives entry, reviewed 2026-09-08.

Official references:

- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Point__3.html
- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Vector__3.html
- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Aff__transformation__3.html

Project relevance:

- provides an authoritative mature-kernel example of distinct point and vector
  types, point subtraction yielding a displacement vector, and point translation
  by a vector;
- confirms that affine transformations form a separate explicit concept;
- supports semantic comparison and test design only: CGAL is not admitted as a
  dependency and is not an implementation or numerical oracle.

The Geometry Primitives entry decision combines this interface evidence with
the existing IEEE 754, Goldberg, Higham, and Shewchuk references. Raw vector
operations do not qualify robust predicate signs or topological decisions.

## Curves, surfaces, and meshing — pending focused reviews

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
