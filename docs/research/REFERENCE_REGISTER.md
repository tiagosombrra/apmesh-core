# AP Mesh Core — Scientific and Engineering Reference Register

Status: ACTIVE
Last updated: 2026-09-22
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

## Topological model

### CGAL Halfedge Data Structures - oriented incidence vocabulary

Status: `FOUNDATIONAL` for the bounded Topological Model entry, reviewed
2026-09-19.

Official reference:
https://doc.cgal.org/latest/HalfedgeDS/index.html

Project relevance:

- supports an oriented edge-use vocabulary with an explicit opposite
  orientation and incidence relations;
- demonstrates that incidence storage can be separated from higher-level
  algorithms;
- does not require AP Mesh to adopt CGAL, its storage representation, paired
  halfedges, or a two-manifold surface assumption.

### Open CASCADE TopoDS_Shape - identity and orientation separation

Status: `FOUNDATIONAL` for the bounded Topological Model entry, reviewed
2026-09-19.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_topo_d_s___shape.html

Project relevance:

- provides a mature B-rep example in which underlying topological identity is
  distinct from the orientation of a use of that identity;
- supports keeping edge identity stable while orientation belongs to an
  `EdgeUse`;
- does not admit Open CASCADE as a dependency, import its B-rep hierarchy, or
  qualify patch, manifold, curve, surface, or CAD semantics.

The bounded entry decision uses these references only to support identity and
orientation separation. The first work unit deliberately defers halfedge face
cycles, complete patch incidence, non-manifold fans, and canonical topology
serialization.

### ISO 10303-42 face and face-bound schema - loop-bounded face vocabulary

Status: `FOUNDATIONAL` for the bounded Face Identity and Boundary Cycles
contract, reviewed 2026-09-19.

Primary-standard rendering:
https://steptools.com/stds/smrl/data/resource_docs/geometric_and_topological_representation/sys/5_schema.htm

Project relevance:

- distinguishes a topological face from its one-or-more loop bounds;
- treats outer-bound classification as additional semantics and notes that a
  unique outer bound is not always available on closed or partially closed
  surfaces;
- supports multiple boundary loops without forcing an early geometric
  outer/inner decision;
- does not make AP Mesh STEP-conformant or import the complete STEP validity,
  manifold, geometry, or exchange schema.

### Open CASCADE shape hierarchy - face, wire, edge, and vertex separation

Status: `FOUNDATIONAL` for the bounded Face Identity and Boundary Cycles
contract, reviewed 2026-09-19.

Official reference:
https://dev.opencascade.org/doc/refman/html/_top_abs___shape_enum_8hxx.html

Project relevance:

- distinguishes face, wire, edge, and vertex as separate topological levels;
- describes a wire as a connected edge sequence and a face as bounded by
  closed wire data;
- supports a generic face/loop model rather than a universal four-sided patch;
- does not admit Open CASCADE as a dependency, storage model, or acceptance
  oracle.

The second bounded topology contract also reuses the existing CGAL reference
for ordered oriented uses around a face. AP Mesh deliberately does not adopt
paired-halfedge storage, a one-loop-only face restriction, or a two-manifold
assumption.

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


## Curve representation

### Farin 2002 — Curves and Surfaces for CAGD

Status: `FOUNDATIONAL` for Curve Representation entry, reviewed 2026-09-20.

Gerald Farin. *Curves and Surfaces for CAGD: A Practical Guide*, 5th ed.,
Morgan Kaufmann / Academic Press, 2002. ISBN 978-1-55860-737-8.

Publisher entry:
https://www.sciencedirect.com/book/9781558607378/curves-and-surfaces-for-cagd

Project relevance:

- chapters on the de Casteljau algorithm and Bernstein-form Bézier curves
  provide the mathematical/CAGD basis for recursive evaluation;
- supports beginning with direct control-point interpolation rather than an
  unnecessary power-basis conversion;
- does not require arbitrary-degree, rational, spline, derivative, or
  integration capability in the first work unit.

### Farouki and Rajan 1987 — Numerical condition of Bernstein form

Status: `FOUNDATIONAL` for the first Bézier evaluator, reviewed 2026-09-20.

Rida T. Farouki and V. T. Rajan. *On the numerical condition of polynomials in
Bernstein form*. Computer Aided Geometric Design 4(3), 191–216, 1987.
DOI: 10.1016/0167-8396(87)90012-4.

Publisher entry:
https://www.sciencedirect.com/science/article/pii/0167839687900124

Project relevance:

- documents favorable numerical properties of Bernstein-form computation in
  geometric design;
- supports retaining the curve in Bernstein/Bézier form for the first
  evaluator;
- does not prove that every Bernstein algorithm is unconditionally stable, so
  explicit adversarial floating-point evidence remains required.

### CGAL Bézier curve traits — control-point and parameter semantics

Status: `FOUNDATIONAL` interface/reference evidence for Curve Representation
entry, reviewed 2026-09-20.

Official reference:
https://doc.cgal.org/latest/Arrangement_on_surface_2/classCGAL_1_1Arr__Bezier__curve__traits__2_1_1Curve__2.html

Project relevance:

- mature computational-geometry example of a Bézier curve defined by ordered
  control points and evaluated by a parameter;
- documents the normalized Bézier parameter interval and endpoint/control-point
  relationship;
- CGAL is not admitted as a dependency or numerical oracle.

### Open CASCADE Bézier geometry — 2D/3D and rational separation

Status: `FOUNDATIONAL` interface/reference evidence for Curve Representation
entry, reviewed 2026-09-20.

Official references:

- https://dev.opencascade.org/doc/refman/html/class_geom___bezier_curve.html
- https://dev.opencascade.org/doc/refman/html/class_geom2d___bezier_curve.html

Project relevance:

- provides mature CAD examples of separate 2D and 3D Bézier geometry;
- documents control-point representation and parameter range `[0,1]`;
- distinguishes polynomial/non-rational curves from weighted rational curves;
- does not admit Open CASCADE, arbitrary degree, weights, or CAD ownership into
  the first AP Mesh work unit.

### C++ `std::lerp` — finite linear interpolation contract

Status: `FOUNDATIONAL` engineering/numeric primitive for the first de
Casteljau evaluator, reviewed 2026-09-20.

C++ draft:
https://eel.is/c++draft/numerics

Reference:
https://en.cppreference.com/cpp/numeric/lerp

Project relevance:

- for finite endpoints, the standard requires exact return of the first/second
  endpoint for `t==0`/`t==1`;
- for finite endpoints and `t∈[0,1]`, the returned interpolation is finite;
- supports component-wise de Casteljau interpolation without the avoidable
  overflow behavior of naïve `a + t*(b-a)` on extreme opposite-sign values;
- the standard-library guarantee is a primitive contract, not by itself a
  scientific qualification of curve evaluation.


### CGAL Segment_2 / Segment_3 — directed bounded segment semantics

Status: `FOUNDATIONAL` for the first concrete curve-family decision,
reviewed 2026-09-22.

Official references:

- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Segment__2.html
- https://doc.cgal.org/latest/Kernel_23/classCGAL_1_1Segment__3.html

Project relevance:

- models a segment as a directed closed straight segment between source and
  target;
- source and target are part of the segment;
- reversal/opposite swaps source and target;
- coincident source/target is explicitly classified as a degenerate segment;
- provides independent mature-kernel evidence for keeping value representation
  distinct from later regularity/admissibility claims;
- CGAL is not admitted as a runtime dependency or numerical oracle.

### CGAL Arrangement curve-family guidance — minimal family first

Status: `FOUNDATIONAL / SCOPING` for representation-breadth sequencing,
reviewed 2026-09-22.

Official reference:
https://doc.cgal.org/latest/Arrangement_on_surface_2/index.html

Project relevance:

- distinguishes line segments, circular/conic arcs, polylines, Bézier curves
  and other curve-family traits;
- explicitly recommends using the smallest traits model that satisfies the
  actual need;
- supports implementing the smallest independent linear family before adding
  conic, rational or spline machinery;
- does not define AP Mesh parameterization, error semantics or qualification
  criteria.

### Rational Bézier and conic-section notes — weighted quadratic bridge

Status: `FOUNDATIONAL` for the second concrete curve-family decision,
reviewed 2026-09-22.

Michigan Technological University references:

- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB-conics.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB-circles.html

Project relevance:

- rational Bézier curves are a knot-free special case of NURBS;
- quadratic rational Bézier curves represent conic segments;
- suitable positive weights represent circular arcs, including the standard
  quarter-circle construction;
- supports introducing weighted denominator semantics before B-spline knot and
  NURBS complexity;
- the pages are mathematical/reference evidence only and are not admitted as
  runtime code or a floating-point oracle.

### Open CASCADE conic and trimmed-curve semantics — analytic alternative

Status: `FOUNDATIONAL / SCOPING` evidence for comparing a dedicated analytic
arc against the selected rational representation, reviewed 2026-09-22.

Official references:

- https://dev.opencascade.org/doc/refman/html/class_geom___conic.html
- https://dev.opencascade.org/doc/refman/html/class_geom2d___circle.html
- https://dev.opencascade.org/doc/refman/html/class_geom___trimmed_curve.html

Project relevance:

- mature CAD kernels keep circle, ellipse, hyperbola and parabola as explicit
  analytic conic families;
- circles have angular periodic semantics rather than the AP Mesh bounded
  `[0,1]` convention used by current concrete families;
- trimmed curves introduce basis-curve, orientation and parameter-bound
  semantics as a distinct concern;
- supports retaining dedicated analytic conics/trimming as later explicit work,
  rather than conflating them with the first rational quadratic family;
- AP Mesh does not adopt Open CASCADE inheritance, tolerances, ownership or a
  runtime dependency.

### Open CASCADE trimmed-curve semantics — basis subdomain and orientation

Status: `FOUNDATIONAL` for the Oriented Trimmed Parametric Subcurve
decision, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___trimmed_curve.html

Project relevance:

- models a trimmed curve as a portion of a basis curve limited by two basis
  parameters;
- treats trim orientation and reversed-parameter mapping explicitly;
- documents additional ambiguity for periodic bases, supporting AP Mesh
  deferral of periodic wrapping;
- supports a separate trim semantic layer instead of duplicating trim logic in
  each concrete family;
- AP Mesh does not adopt Open CASCADE ownership, inheritance, exception,
  tolerance or periodic-adjustment behavior.

### CGAL polycurve traits — composition remains a separate later layer

Status: `FOUNDATIONAL / SCOPING` for separating trimming from heterogeneous
composition, reviewed 2026-09-22.

Official reference:
https://doc.cgal.org/latest/Arrangement_on_surface_2/classCGAL_1_1Arr__polycurve__traits__2.html

Project relevance:

- supports chains built from subcurves such as line segments, conic/circular
  arcs and Bézier curves;
- requires neighboring subcurves to meet and carry coherent orientation;
- demonstrates that piecewise composition is a distinct representation layer
  over already-defined subcurves;
- supports deferring runtime heterogeneous storage/type erasure while AP Mesh
  first establishes trimming of one statically known bounded basis curve.

### Open CASCADE B-spline breadth — why spline semantics remain deferred

Status: `FOUNDATIONAL / SCOPING` for the third representation-breadth
decision, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Project relevance:

- B-splines may be uniform/non-uniform, rational/non-rational and
  periodic/non-periodic;
- construction exposes degree, poles, knots and multiplicities;
- reversal and segmentation interact with knot data;
- supports treating B-spline/NURBS as later bounded decisions rather than
  combining knot/continuity/periodic semantics with the first trim wrapper.

### B-spline basis, knots and local support — MTU notes

Status: `FOUNDATIONAL` for the bounded two-span cubic B-spline decision,
reviewed 2026-09-22.

Official teaching/reference notes:

- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/bspline-basis.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/bspline-property.html

Project relevance:

- defines B-spline basis functions through a nondecreasing knot vector and
  degree;
- identifies knot spans and local support as semantics absent from global
  Bézier basis functions;
- records continuity `C^(p-k)` at a knot of multiplicity `k`;
- supports choosing one simple interior knot in a degree-three curve to isolate
  C2 multi-span semantics;
- does not pre-authorize arbitrary degree, multiplicity, periodicity or dynamic
  spline storage.

### de Boor evaluation and Bézier special case — MTU notes

Status: `FOUNDATIONAL` for the bounded production/reference split, reviewed
2026-09-22.

References:

- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/de-Boor.html
- https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/de-boor-special-case.html

Project relevance:

- de Boor provides the standard local B-spline point-evaluation construction;
- with endpoint-only clamped knots it reduces to de Casteljau;
- supports production de Boor evaluation while using existing cubic Bézier
  parity after one knot insertion as an independent prerequisite oracle.

### B-spline derivatives — MTU notes

Status: `FOUNDATIONAL` for D1/D2 semantics, reviewed 2026-09-22.

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/B-spline/bspline-derv.html

Project relevance:

- the derivative of a degree-`p` B-spline is another B-spline of degree
  `p-1` with derived control points;
- higher derivatives follow recursively;
- clamped curves pass through endpoint controls and have explicit endpoint
  tangent relations;
- supports independent derivative-control-polygon evidence for the fixed cubic
  work unit.

### B-spline before NURBS — weighted extension evidence

Status: `FOUNDATIONAL / SEQUENCING` for isolating knots before NURBS,
reviewed 2026-09-22.

Reference:
https://pages.mtu.edu/~shene/PUBLICATIONS/2004/NURBS.pdf

Project relevance:

- a B-spline is defined by controls, knots and degree;
- NURBS adds control weights/rational basis normalization;
- all-one NURBS weights reduce to the underlying B-spline;
- supports isolating knot/local-support semantics before combining them with
  the already separately tested rational-weight layer.

### Open CASCADE B-spline breadth — mature CAD comparison

Status: `FOUNDATIONAL / SCOPING` for why the first spline work unit is fixed
and non-periodic, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Project relevance:

- mature CAD B-splines expose degree, knots and multiplicities;
- general production B-splines may be rational/non-rational and
  periodic/non-periodic;
- continuity depends on degree and multiplicity;
- supports deferring general degree/count/multiplicity/periodicity while
  introducing only one simple interior knot first;
- Open CASCADE is not admitted as a dependency or numerical oracle.

### NURBS local support and equal-weight reduction — MTU notes

Status: `FOUNDATIONAL` for the fixed two-span cubic NURBS decision,
reviewed 2026-09-22.

Official reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS-property.html

Project relevance:

- rational basis functions inherit local support from the B-spline basis;
- at a knot of multiplicity `m`, continuity follows the underlying
  degree/multiplicity relation;
- equal nonzero weights reduce NURBS basis functions to ordinary B-spline
  basis functions;
- supports mandatory parity against the integrated fixed two-span polynomial
  B-spline without introducing new continuity semantics.

### NURBS weight influence and common scale semantics

Status: `FOUNDATIONAL` for positive-weight storage/evidence, reviewed
2026-09-22.

Reference:
https://pages.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS-mod-weight.html

Project relevance:

- NURBS adds control weights to degree/knot/control data;
- weights modify rational influence while preserving the B-spline support
  structure;
- all-one weights recover the polynomial spline;
- supports combining the already tested positive-weight rational layer with the
  already tested two-span knot layer in a bounded work unit.

### Open CASCADE / STEP rational B-spline data model

Status: `FOUNDATIONAL / SCOPING` for the fixed NURBS decision, reviewed
2026-09-22.

Official references:

- https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html
- https://dev.opencascade.org/doc/refman/html/class_step_geom___b_spline_curve_with_knots_and_rational_b_spline_curve.html

Project relevance:

- mature CAD representations combine degree, controls, knots/multiplicities and
  weights for rational B-spline geometry;
- rational/non-rational and periodic/non-periodic semantics are explicit;
- supports introducing weights on the already fixed cubic/two-span topology
  before broadening degree, span count, multiplicity or periodicity;
- Open CASCADE is not admitted as a runtime dependency or numerical oracle.

### Open CASCADE 8.0.1 — variable-span B-spline/NURBS curve data

Status: `FOUNDATIONAL / ACTIVE REVIEW` for the multi-span cubic NURBS
breadth decision, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Project relevance:

- mature CAD spline curves own runtime-sized pole, weight, knot and
  multiplicity arrays;
- the flat knot-sequence length is tied to pole count and degree;
- rational/non-rational, periodic/non-periodic, degree and multiplicity are
  independent semantics;
- supports isolating runtime span/control/knot count now while keeping degree
  three, simple knots and non-periodicity frozen;
- Open CASCADE is design/reference evidence only, not a runtime dependency or
  numerical oracle.

### Open CASCADE 8.0.1 — B-spline surface data model as sequencing evidence

Status: `FOUNDATIONAL / SEQUENCING` for future Surface Representation,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html

Project relevance:

- B-spline/NURBS surfaces combine a control-point grid, weights, independent
  U/V knot arrays, multiplicities and U/V degrees;
- variable 1D control/knot ownership and deterministic span location are
  therefore direct curve-level prerequisites for later tensor-product surface
  work;
- this source does not authorize a surface implementation in the current curve
  stage.

### C++ `std::vector` and `std::span` — runtime spline storage seam

Status: `FOUNDATIONAL / ENGINEERING` for the multi-span cubic NURBS storage
policy, reviewed 2026-09-22.

References:

- https://en.cppreference.com/cpp/container/vector
- https://en.cppreference.com/cpp/container/span

Project relevance:

- `std::vector` provides owning contiguous runtime-sized storage;
- `std::span` provides a non-owning contiguous view;
- supports immutable owning spline data with read-only public views without a
  custom allocator, arbitrary compile-time capacity or third-party container;
- allocation/resource failure remains ordinary C++ resource behavior and is not
  reclassified as a scientific curve-domain error.

### Open CASCADE 8.0.1 — arbitrary-degree Bézier deferral evidence

Status: `FOUNDATIONAL / SCOPING` for the breadth comparison, reviewed
2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___bezier_curve.html

Project relevance:

- mature Bézier curves support runtime pole count, rational weights and degree
  greater than the current cubic baseline;
- this is a distinct degree/storage generalization but remains single-span and
  globally supported;
- supports deferring arbitrary-degree Bézier while the newly available spline
  seam is generalized across multiple spans.

### Open CASCADE — knot multiplicity and guaranteed B-spline continuity

Status: `FOUNDATIONAL / ACTIVE REVIEW` for the cubic double-knot C1
continuity decision, reviewed 2026-09-22.

Official references:

- https://dev.opencascade.org/doc/refman/html/class_geom_convert___b_spline_curve_knot_splitting.html
- https://dev.opencascade.org/doc/refman/html/class_law___b_spline_knot_splitting.html

Project relevance:

- spline continuity loss is localized at knot values;
- guaranteed continuity at a knot is degree minus multiplicity;
- for cubic degree three, multiplicity one gives C2 and multiplicity two gives
  C1;
- supports isolating the double-knot case before multiplicity three/C0;
- local derivative facilities in mature kernels reinforce that side/local
  derivative semantics are distinct from ordinary globally guaranteed
  derivatives.

### Open CASCADE Geom_Curve — derivative continuity requirements

Status: `FOUNDATIONAL / SEMANTIC` for typed D2 failure at C1 knots,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___curve.html

Project relevance:

- D1 requires C1 continuity;
- D2 requires C2 continuity;
- supports explicit differentiation between a valid first derivative and an
  unavailable unique ordinary second derivative at a cubic double knot;
- AP Mesh adopts its own parameter-local expected-value failure semantics rather
  than copying Open CASCADE exception/global-continuity behavior.

### Open CASCADE — B-spline surface knot continuity as sequencing evidence

Status: `FOUNDATIONAL / SEQUENCING` for keeping Surface Representation
blocked until C1 knot semantics are explicit, reviewed 2026-09-22.

Official references:

- https://dev.opencascade.org/doc/refman/html/class_geom_convert___b_spline_surface_knot_splitting.html
- https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html

Project relevance:

- surface continuity changes are localized on U/V knot lines;
- U/V multiplicity controls continuity in each parametric direction;
- unresolved curve-level repeated-knot derivative semantics would otherwise be
  duplicated for first/second/mixed surface partials;
- this evidence supports sequencing only and does not authorize any production
  surface family.

### CGAL polycurve — composition remains a separate continuity layer

Status: `FOUNDATIONAL / SCOPING` for deferring heterogeneous composition
while spline-internal continuity is resolved, reviewed 2026-09-22.

Official reference:
https://doc.cgal.org/latest/Arrangement_on_surface_2/classCGAL_1_1Arr__polycurve__traits__2.html

Project relevance:

- a polycurve is a continuous, well-oriented chain of subcurves;
- endpoint compatibility and chain orientation are separate from the internal
  continuity of one spline family;
- supports deferring runtime heterogeneous composition from the double-knot
  work unit.

### Internal arbitrary-orientation constraint — qualified Cartesian Frames

Status: `FOUNDATIONAL / INTERNAL` sequencing constraint for the second
concrete-family decision.

Authority:
`docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md`.

Project relevance:

- qualified frames admit exact signed-permutation bases and reciprocal-safe
  power-of-two scale only;
- arbitrary-angle rotations and approximate frames are explicitly outside the
  qualified claim;
- a general analytic 3D circular arc cannot silently treat this qualified frame
  as an arbitrary supporting-plane frame;
- rational control geometry can represent planar conic segments in arbitrary
  2D/3D positions without first broadening frame semantics.

### Global cubic regularity — Bernstein zero-exclusion references

Status: `FOUNDATIONAL` for the bounded Global Cubic Regularity Certification
decision, reviewed 2026-09-20.

Rida T. Farouki and V. T. Rajan. *On the numerical condition of polynomials in
Bernstein form*. Computer Aided Geometric Design 4(3), 191–216, 1987.
https://doi.org/10.1016/0167-8396(87)90012-4

Project relevance:

- supports retaining the squared-speed polynomial in Bernstein form;
- documents favorable conditioning and improvement under subdivision;
- does not by itself make ordinary floating coefficients certified bounds.

Qing Xian Meng and Hui Li Liu. *Regularity of Bézier Curves*. Applied Mechanics
and Materials 48–49, 877–880, 2011.
https://doi.org/10.4028/www.scientific.net/AMM.48-49.877

Project relevance:

- frames Bézier regularity as existence/nonexistence of zeros in derivative
  polynomial equations;
- supports treating global regularity as a zero-exclusion problem rather than
  sampled speed inspection;
- AP Mesh does not adopt the paper's complete algebraic method as a dependency.

B. Mourrain and J. P. Pavone. *Subdivision methods for solving polynomial
equations*. Journal of Symbolic Computation 44(3), 292–306, 2009.
https://doi.org/10.1016/j.jsc.2008.04.016

Project relevance:

- supports Bernstein-basis subdivision as a principled bounded-domain
  root/exclusion technique;
- motivates hierarchical refinement rather than one fixed parameter grid;
- does not remove the need for conservative floating enclosures and explicit
  `indeterminate` outcomes.

The bounded AP Mesh decision uses these references only to justify mathematical
structure. No external solver, arbitrary-degree polynomial subsystem, or
third-party runtime dependency is admitted.


### Piegl and Tiller 1997 — The NURBS Book

Status: `FOUNDATIONAL` for the Parametric Curve Family Abstraction decision,
reviewed 2026-09-22.

Les Piegl and Wayne Tiller. *The NURBS Book*, 2nd ed., Springer, 1997.

Accessible reference copy:
https://home.zcu.cz/~bastl/GM1/the-nurbs-book.pdf

Project relevance:

- treats B-spline and NURBS curves/surfaces together with their evaluation and
  derivative algorithms;
- documents NURBS as a generalization that includes non-rational B-splines and
  rational/non-rational Bézier forms;
- supports separating common parametric semantics from family-specific
  control/weight/knot storage;
- does not require AP Mesh to adopt one universal NURBS representation for all
  analytic geometry or to implement spline/rational families in one work unit.

### Open CASCADE Geom_Curve — common parametric curve semantics

Status: `FOUNDATIONAL` interface/reference evidence for the Parametric Curve
Family Abstraction decision, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___curve.html

Project relevance:

- provides a mature CAD-kernel example of common parameter bounds, evaluation,
  derivatives, reversal, continuity, closedness and periodicity across concrete
  line, conic, Bézier and B-spline curve families;
- demonstrates that parameter-domain and derivative semantics are genuinely
  cross-family concepts;
- AP Mesh does not adopt OCCT ownership, inheritance, exception behavior,
  tolerance policies or runtime dependency.

### Open CASCADE Geom_BSplineCurve — knot domain and rational/periodic variation

Status: `FOUNDATIONAL` scoping evidence for later B-spline/NURBS work,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_curve.html

Project relevance:

- first and last curve parameters are knot values rather than a universal
  normalized interval;
- knot multiplicity affects continuity;
- B-spline curves may be periodic and rational;
- supports explicitly deferring continuity partitions, periodic wrapping,
  knots and weights from the first common abstraction work unit.

### Open CASCADE Geom_Circle — non-normalized bounded periodic example

Status: `FOUNDATIONAL` counterexample to a universal `[0,1]` domain,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___circle.html

Project relevance:

- documents a circle parameter domain `[0,2π]`;
- documents periodicity and reversed-parameter relation `2π-u`;
- supports an explicit per-curve parameter domain and reversal mapping;
- does not pre-authorize a circle implementation in AP Mesh.

### C++23 constraints/concepts — static semantic interface mechanism

Status: `FOUNDATIONAL` engineering evidence for the bounded abstraction,
reviewed 2026-09-22.

References:

- https://en.cppreference.com/w/cpp/language/constraints
- https://en.cppreference.com/w/cpp/language/requires
- https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

Project relevance:

- C++ concepts provide compile-time named requirements for generic algorithms;
- C++ Core Guidelines recommend concepts model meaningful semantic categories
  rather than incidental syntax;
- compile-time satisfaction does not prove semantic correctness, so runtime
  analytic/metamorphic contracts remain mandatory;
- supports a static value-oriented abstraction before considering virtual
  runtime polymorphism.


## Surface Representation

### Open CASCADE Geom_BoundedSurface — finite rectangular surface domains

Status: `FOUNDATIONAL / ACTIVE REVIEW` for Surface Representation entry,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___bounded_surface.html

Project relevance:

- treats a bounded surface as finite over independent U/V parameter intervals;
- identifies four isoparametric boundary curves;
- supports a dedicated two-parameter bounded-surface contract rather than
  reusing a one-parameter curve interface implicitly.

### Open CASCADE Geom_BezierSurface — rational-weight surface seam

Status: `FOUNDATIONAL / ACTIVE REVIEW` for the rational bicubic surface
breadth decision, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___bezier_surface.html

Project relevance:

- the same 2D pole/control-net model supports polynomial and rational Bézier
  surfaces;
- rationality is introduced through a weight array associated with the control
  net, while the U/V tensor-product structure remains unchanged;
- supports isolating positive rational weighting after the polynomial bicubic
  patch and before U/V knot/multiplicity semantics;
- Open CASCADE is design evidence only, not a runtime dependency or numerical
  oracle.

### Open CASCADE Geom_BSplineSurface 8.0.1 — independent U/V spline structure

Status: `FOUNDATIONAL / ACTIVE REVIEW` for the bicubic NURBS surface
breadth decision, reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___b_spline_surface.html

Project relevance:

- represents B-spline/NURBS surfaces through a rectangular pole/control net
  with independent U/V degree, knot, multiplicity and periodicity state;
- confirms that rational surface weighting and U/V spline structure are
  separable concerns;
- supports selecting runtime-variable U/V span counts with degree three,
  positive weights, simple interior knots and non-periodicity while deferring
  repeated-knot surface continuity;
- Open CASCADE remains design/reference evidence only, not a runtime
  dependency or numerical oracle.

### MIT Hyperbook — tensor-product B-spline surface locality

Status: `FOUNDATIONAL / ACTIVE REVIEW` for the bicubic NURBS surface
decision, reviewed 2026-09-22.

Reference:
https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node19.html

Project relevance:

- defines a B-spline surface as a tensor product over a rectangular control
  net with independent U/V knot vectors;
- identifies isoparametric curves as B-spline curves in the corresponding
  direction;
- supports four-boundary parity with integrated cubic NURBS curves and
  independent tensor-product Cox-de Boor validation.

### Open CASCADE BSplSLib / STEP — why NURBS follows rational Bézier

Status: `FOUNDATIONAL / SEQUENCING` for the surface breadth decision,
reviewed 2026-09-22.

Official references:

- https://dev.opencascade.org/doc/refman/html/class_b_spl_s_lib.html
- https://dev.opencascade.org/doc/refman/html/class_step_geom___b_spline_surface_with_knots_and_rational_b_spline_surface.html
- https://dev.opencascade.org/doc/refman/html/class_geom_convert___b_spline_surface_knot_splitting.html

Project relevance:

- NURBS surfaces combine a two-dimensional control/weight net with independent
  U/V degrees, knots, multiplicities and periodicity state;
- U/V knot multiplicity controls continuity independently by parametric
  direction;
- supports validating the rational surface quotient seam first, then combining
  it with already-established curve spline/knot semantics in a later NURBS
  surface work unit.

### Open CASCADE Geom_BezierSurface — tensor-product control-net surface

Status: `FOUNDATIONAL / ACTIVE REVIEW` for the first surface work unit,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___bezier_surface.html

Project relevance:

- represents polynomial/rational Bézier surfaces with a two-dimensional
  control net;
- supports isolating a fixed bicubic polynomial patch before weights, knots,
  periodicity and trimming;
- external API/inheritance/exception semantics are not adopted.

### Patrikalakis, Maekawa and Cho — Bézier/B-spline surfaces

Status: `FOUNDATIONAL` for tensor-product surface structure, reviewed
2026-09-22.

References:

- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node8.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node19.html

Project relevance:

- documents Bézier and B-spline surfaces as tensor-product parametric
  representations;
- B-spline surfaces use a rectangular control net and independent U/V knot
  vectors;
- supports using a direct Bernstein-sum oracle independent of production
  tensor-product de Casteljau evaluation.

### Open CASCADE geometry model taxonomy — required surface breadth

Status: `FOUNDATIONAL / SCOPING` for the Surface Representation family map,
reviewed 2026-09-22.

Reference:
https://dev.opencascade.org/sites/default/files/pdf/Geometry.pdf

Project relevance:

- separates elementary surfaces (plane/cylinder/cone/sphere/torus), free-form
  Bézier/B-spline surfaces, sweeping surfaces, offsets and trimming;
- supports retaining analytic, free-form, swept and trimmed families as
  distinct AP Mesh obligations;
- prevents one polynomial patch from being documented as full CAD surface
  coverage.

### Open CASCADE Geom_RectangularTrimmedSurface — trimming is separate from
supporting geometry

Status: `FOUNDATIONAL / SCOPING` for later trimmed-surface work, reviewed
2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/class_geom___rectangular_trimmed_surface.html

Project relevance:

- separates a supporting surface from parameter-domain trimming;
- exposes independent U/V reversal/orientation semantics;
- supports keeping first surface representation untrimmed and value-oriented.

### Open CASCADE GeomFill_BSplineCurves — Coons/boundary filling

Status: `FOUNDATIONAL / SCOPING` for later AP Mesh transfinite/Coons work,
reviewed 2026-09-22.

Official reference:
https://dev.opencascade.org/doc/refman/html/_geom_fill___b_spline_curves_8hxx.html

Project relevance:

- boundary-generated filling is a construction mechanism distinct from a
  tensor-product value type;
- supports retaining Coons/transfinite work as an explicit later surface
  family rather than folding it into the first bicubic patch.

### Open CASCADE — B-spline surface knot splitting / continuity

Status: `FOUNDATIONAL / ACTIVE REVIEW` for bicubic NURBS surface C1
continuity, reviewed 2026-09-22.

Reference:
https://dev.opencascade.org/doc/occt-7.2.0/refman/html/class_geom_convert___b_spline_surface_knot_splitting.html

Project relevance:

- surface discontinuities are localized at knot values;
- continuity in one parametric direction is degree minus knot multiplicity;
- for degree three, multiplicity one is C2 and multiplicity two is C1;
- supports a bounded surface-specific continuity decision without changing
  degree or periodicity.

### STEP/Open CASCADE — independent U/V surface multiplicities

Status: `FOUNDATIONAL / DATA-MODEL` for repeated-knot NURBS surfaces,
reviewed 2026-09-22.

Reference:
https://dev.opencascade.org/doc/refman/html/class_step_geom___b_spline_surface_with_knots.html

Project relevance:

- U and V knots/multiplicities are separate surface data;
- supports explicit directional multiplicity storage and validation;
- no STEP runtime dependency is admitted.

### Open CASCADE GeomFill_BezierCurves — Coons/filling deferral evidence

Status: `FOUNDATIONAL / SCOPING` for the breadth comparison, reviewed
2026-09-22.

Reference:
https://dev.opencascade.org/doc/occt-7.9.0/refman/html/class_geom_fill___bezier_curves.html

Project relevance:

- boundary-driven filling requires contiguous boundary curves and a filling
  style;
- supports treating Coons/transfinite construction as a separate boundary
  compatibility/blending seam rather than part of knot continuity.

### Open CASCADE elementary surfaces — analytic-surface deferral evidence

Status: `FOUNDATIONAL / SCOPING` for later analytic Surface Representation,
reviewed 2026-09-22.

References:

- https://dev.opencascade.org/doc/occt-7.0.0/refman/html/class_geom___elementary_surface.html
- https://dev.opencascade.org/doc/refman/html/class_el_s_lib.html

Project relevance:

- plane/cylinder/cone/sphere/torus use family-specific analytic
  parameterizations and 3D placement;
- supports a separate decision for placement, periodic directions and
  singular parameter behavior.

### Open CASCADE / IGES trimmed surfaces — trimming deferral evidence

Status: `FOUNDATIONAL / SCOPING` for later trimming, reviewed 2026-09-22.

Reference:
https://dev.opencascade.org/doc/refman/html/class_i_g_e_s_geom___trimmed_surface.html

Project relevance:

- trimmed surfaces retain a supporting surface plus outer/inner boundary data;
- supports deferring curve-on-surface, loop orientation and topology identity
  to a separate trimming decision.

## Curve Differential Geometry

### do Carmo — Differential Geometry of Curves and Surfaces

Status: `FOUNDATIONAL` for Curve Differential Geometry entry, reviewed
2026-09-21.

Manfredo P. do Carmo. *Differential Geometry of Curves and Surfaces*.

MIT OpenCourseWare syllabus reference:
https://ocw.mit.edu/courses/18-994-seminar-in-geometry-fall-2004/pages/syllabus/

Project relevance:

- supports the classical regular-curve requirement and local curvature/Frenet
  framework;
- supports treating curvature as differential geometry of a regular curve,
  distinct from representation and discretization;
- does not specify floating-point robustness, Bézier-specific algorithms or an
  AP Mesh API.

### MIT OpenCourseWare 18.950 — Local and global geometry of plane curves

Status: `FOUNDATIONAL` mathematical reference for pointwise curvature,
reviewed 2026-09-21.

Course:
https://ocw.mit.edu/courses/18-950-differential-geometry-fall-2008/

Lecture notes:
https://ocw.mit.edu/courses/18-950-differential-geometry-fall-2008/pages/lecture-notes/

Project relevance:

- presents regular curves, Frenet frames and curvature;
- supports curvature invariance under admissible reparameterization;
- supports separating local curvature from stronger global feature claims;
- does not establish certified floating arithmetic or global feature
  classification for cubic Bézier curves.

### Wolfram MathWorld — Curvature

Status: `REFERENCE` for independent analytic fixture formulas, reviewed
2026-09-21.

Reference:
https://mathworld.wolfram.com/Curvature.html

Project relevance:

- records standard parametric curvature formulas for plane curves;
- provides an independent formula source for selected test expectations;
- is not a numerical oracle or implementation dependency.

### Miura and Salvi 2021 — Curvature extrema of special cubic Bézier curves

Status: `SCOPING` evidence for deferring global curvature features, reviewed
2026-09-21.

Kenjiro T. Miura and Péter Salvi.
*On the curvature extrema of special cubic Bézier curves*.
arXiv:2101.08138, 2021.

Reference:
https://arxiv.org/abs/2101.08138

Project relevance:

- demonstrates that curvature-extremum structure is a separate analytical
  problem even for restricted cubic Bézier families;
- supports keeping the first Curve Differential Geometry work unit pointwise
  only;
- does not establish a general extrema/monotonicity classifier for arbitrary
  cubic Bézier curves.

### Farin 2002 — Planar signed curvature

Status: `FOUNDATIONAL` for the bounded signed-curvature decision, reviewed
2026-09-21.

Gerald Farin. *Curves and Surfaces for CAGD: A Practical Guide*, 5th ed.,
Morgan Kaufmann / Academic Press, 2002.

References:

- https://www.sciencedirect.com/book/9781558607378/curves-and-surfaces-for-cagd
- https://www.sciencedirect.com/science/article/pii/B9781558607378500107

Project relevance:

- records that a planar parametric curve can use the sign of
  `det(B',B'')` to define signed curvature;
- distinguishes orientation-sensitive planar signed curvature from ordinary
  nonnegative spatial curvature;
- supports an explicitly 2D-only AP Mesh signed-curvature contract;
- does not certify floating determinant signs near cancellation and does not
  establish global inflection isolation.

### Patrikalakis, Maekawa and Cho — signed/zero curvature points

Status: `FOUNDATIONAL / SCOPING` for signed curvature and later inflection
work, reviewed 2026-09-21.

MIT Hyperbook references:

- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node153.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node219.html

Project relevance:

- records the signed-curvature formulation for regular planar parametric
  curves;
- makes an explicit planar sign convention relevant to orientation-sensitive
  geometric quantities;
- relates zero curvature to inflection-point investigation under regularity;
- supports separating a pointwise signed-curvature value from certified
  interval-wide inflection isolation.

### Existing qualified AP Mesh curve evidence

Status: `FOUNDATIONAL / INTERNAL` prerequisite authority.

Relevant integrated authorities:

- `docs/decisions/CURVE_DERIVATIVES_REGULARITY_DECISION.md`;
- `docs/decisions/CURVE_GLOBAL_REGULARITY_CERTIFICATION_DECISION.md`;
- `docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md`;
- `docs/audits/2026-09-21-continuous-curve-geometry-regression-terminal-audit.md`.

Project relevance:

- first/second derivatives, speed, global regularity, reversal, frame/scale
  relations and explicit error semantics are already qualified prerequisites;
- Curve Differential Geometry must reuse those semantics rather than fork them;
- prerequisite qualification does not itself establish curvature semantics.
