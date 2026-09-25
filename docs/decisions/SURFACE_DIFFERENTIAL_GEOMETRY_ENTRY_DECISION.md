# Surface Differential Geometry Entry — Bounded Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-25  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Question

After terminal closure of the bounded analytic cylinder-sector work unit, should
AP Mesh continue widening Surface Representation or open the separate Surface
Differential Geometry stage?

The required comparison is:

1. Surface Differential Geometry entry readiness;
2. bounded analytic sphere;
3. bounded analytic cone;
4. bounded analytic torus;
5. general trimming / p-curves / topological faces.

If differential geometry is ready, select exactly one first work unit without
pre-authorizing curvatures or broader surface representation.

## 2. Fresh repository entry authority

Canonical decision-entry main:
`93eb53c21766d9584290733ef83f838ad9652c98`.

Terminal bounded analytic cylinder evidence:

- implementation PR #191:
  `6edc598637ddedd54cf62732e33a62e504cb82c8`;
- implementation post-merge FAST `36088816019`: PASS, 36/36;
- implementation post-merge INTEGRATION `36088816031`: PASS, 36/36;
- closure PR #192:
  `102be8d81164ec3a5b12162c7e8bd05a772ebaea`;
- closure post-merge FAST `36089081707`: PASS;
- closure post-merge INTEGRATION `36089081776`: PASS;
- terminal sync:
  `93eb53c21766d9584290733ef83f838ad9652c98`;
- terminal-sync FAST `36089325503`: PASS;
- terminal-sync INTEGRATION `36089325498`: PASS.

Protected-main ordinary semantic inventory at entry: **36 tests**.

No open PR or production work item exists at entry.

## 3. Surface capabilities already integrated

The bounded surface contract already exposes:

- `evaluate(u,v)`;
- `first_derivatives(u,v)` -> `S_u,S_v`;
- `second_derivatives(u,v)` -> `S_uu,S_uv,S_vv`.

Integrated focused representations include:

- polynomial bicubic Bézier patch;
- positive-weight rational bicubic Bézier patch;
- bicubic positive-weight NURBS with simple/double interior multiplicities;
- cubic Bézier Coons/transfinite patch;
- rectangular static trimming;
- cubic Bézier linear extrusion;
- cubic Bézier revolution;
- bounded analytic plane;
- bounded analytic circular cylinder sector.

Therefore first- and second-order derivative semantics exist across multiple
independent representation families before this decision.

## 4. Literature and mature-kernel evidence

### 4.1 Differential geometry is defined from first/second surface derivatives

Patrikalakis, Maekawa and Cho, *Shape Interrogation for Computer Aided Design
and Manufacturing*, Differential Geometry of Surfaces:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node26.html

Relevant evidence:

- a regular parametric surface has tangent vectors `S_u,S_v`;
- the tangent plane and unit normal are derived from their cross product;
- the first fundamental form is determined by
  `E=S_u·S_u`, `F=S_u·S_v`, `G=S_v·S_v`;
- second-order curvature quantities are subsequent constructions.

Decision impact:

- AP Mesh already has the derivative data required for a generic first-order
  differential layer;
- no analytic sphere/cone/torus production class is required to define the
  first fundamental form or oriented normal.

### 4.2 Mature CAD kernels separate local surface properties from representations

Open CASCADE `GeomLProp_SLProps` / surface local properties:

https://dev.opencascade.org/doc/refman/html/_geom_l_prop___surface_utils_8hxx.html

Relevant evidence:

- local-property computation consumes surface derivatives;
- tangent/normal existence is checked separately from the underlying concrete
  surface family;
- curvature computation is a later property built from D1/D2 and a valid
  normal.

Decision impact:

- AP Mesh should introduce a generic surface-differential layer instead of
  duplicating normals/metrics in every concrete surface class;
- representation and differential regularity remain separate scientific
  concerns.

### 4.3 Gaussian/mean/principal curvature depend on the first-order foundation

Patrikalakis, Maekawa and Cho, Gaussian and mean curvature:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node31.html

Relevant evidence:

- Gaussian and mean curvature are defined after the first and second
  fundamental forms;
- principal curvatures are subsequent eigenvalues/directions of the shape
  operator.

Decision impact:

- metric + oriented normal is the smallest defensible first work unit;
- second fundamental form and curvature must remain later decisions.

External sources are scientific/design evidence only. No external CAD kernel is
admitted as a runtime dependency or numerical oracle.

## 5. Candidate comparison

| Candidate | New semantics | Existing prerequisite status | Direct downstream value | Decision |
| --- | --- | --- | --- | --- |
| Surface Differential Geometry entry | regularity, metric, area density, oriented normal | first/second derivatives already integrated across many surface families | **Very high**; prerequisite for sizing/error models | **SELECTED** |
| Bounded analytic sphere | new analytic representation + polar singularity/domain policy | not required for generic differential definitions | high as later analytic oracle/family | DEFER |
| Bounded analytic cone | new analytic representation + apex singularity policy | not required for first-order generic metric | high later | DEFER |
| Bounded analytic torus | new analytic representation + periodic/two-angle policy | not required for first-order generic metric | high later | DEFER |
| General trimming / p-curves / topological faces | curve-on-surface/topology identity/orientation | rectangular trim exists, but full CAD face semantics are separate | very high later for boundary certification | DEFER |

## 6. Decision

Open the scientific stage:

**Surface Differential Geometry — Metric, Normals, and Curvatures.**

Authorize exactly one future implementation work unit:

**Pointwise Surface Regularity, First Fundamental Form, Area Density, and
Oriented Unit Normal in 3D.**

No second fundamental form or curvature is authorized by this decision.

## 7. Conceptual API

Preferred repository mapping:

- public differential API:
  `include/apmesh/geometry/surface_differential.hpp`;
- production:
  `src/geometry/surface_differential.cpp`;
- focused contract:
  `tests/surface_metric_normal.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

Preferred value types:

`SurfaceFirstFundamentalForm`

with fields:

- `e`;
- `f`;
- `g`.

`SurfaceMetricNormal3`

with fields:

- first fundamental form;
- area density `|S_u x S_v|`;
- oriented unit normal.

Preferred error vocabulary:

`SurfaceDifferentialError`

with explicit categories for:

- propagated surface-parameter/domain/continuity failure;
- singular parameterization;
- non-representable/non-finite derived result.

The exact C++ spelling may be refined mechanically without changing these
scientific categories.

## 8. Pointwise regularity semantics

At a valid parameter pair, let

`a = S_u` and `b = S_v`.

The parameterization is regular exactly when

`a x b != 0`

in the declared finite floating representation.

No universal geometric epsilon defines singularity.

The work unit must distinguish:

- exactly zero cross product -> `singular_parameterization`;
- nonzero but very small area density -> regular if representable;
- non-representable arithmetic -> explicit numeric/differential failure.

Near-degenerate regular fixtures must prove that no hidden tolerance rejects a
nonzero tangent parallelogram.

## 9. First fundamental form

Required coefficients:

`E = S_u · S_u`

`F = S_u · S_v`

`G = S_v · S_v`

For a successful regular point:

- `E > 0`;
- `G > 0`;
- `EG-F^2 > 0` mathematically.

Production must not use `EG-F^2` with an arbitrary epsilon as the regularity
predicate because cancellation can be severe near degeneracy.

The metric coefficients themselves must be returned only when representable as
finite `double`.

## 10. Area density and oriented normal

Area density is

`J = |S_u x S_v|`.

The oriented unit normal is

`n = (S_u x S_v) / J`.

Production must use a scale-aware computation that avoids avoidable overflow
or underflow when the final `J` and `n` are representable.

The normal orientation is defined solely by the parameter order
`S_u x S_v`.

No topology-face orientation is introduced here.

## 11. Parameter reversal covariance

For U reversal:

- `S_u -> -S_u`;
- `S_v -> S_v`;
- `E,G,J` unchanged;
- `F -> -F`;
- normal flips sign.

For V reversal:

- analogous sign behavior.

For simultaneous U+V reversal:

- `E,F,G,J` return to the original values;
- oriented normal preserves sign.

Focused tests must exercise these relations on existing reversible surface
families.

## 12. Scale and frame covariance

For uniform coordinate scaling by exact power of two `lambda`:

- `E,F,G -> lambda^2 (E,F,G)`;
- `J -> lambda^2 J`;
- unit normal is unchanged for positive scaling.

For admitted orthogonal/signed-permutation frame transformations:

- metric and area density are invariant;
- the normal follows the orientation law of the orthogonal transformation.

Translation has no effect.

No arbitrary tolerance is introduced for these checks.

## 13. Analytic validation fixtures

Mandatory first-work-unit fixtures:

### Bounded analytic plane

For an orthonormal placement with direct parameter axes:

- `E=1`;
- `F=0`;
- `G=1`;
- `J=1`;
- normal equals the placement orientation.

### Bounded analytic cylinder

For radius `R` and the existing parameter convention:

- one tangent magnitude is `R`;
- the axial tangent magnitude is 1;
- tangents are orthogonal;
- metric coefficients and `J` must match the exact analytic formulas;
- unit normal must agree with the analytic radial direction under the
  repository's orientation convention.

### Polynomial saddle/paraboloid-style patch

Use an existing polynomial surface representation with an independent analytic
formula to validate non-orthogonal and non-constant metrics.

Sphere may be used as a test-only analytic oracle later, but no production
sphere class is required by this work unit.

## 14. Integrated-family conformance evidence

The generic differential function must be exercised at at least one regular
point on each currently integrated surface family:

- bicubic Bézier;
- rational bicubic Bézier;
- bicubic NURBS;
- Coons patch;
- rectangular trimmed surface;
- linear extrusion;
- revolution;
- analytic plane;
- analytic cylinder.

This is conformance evidence, not requalification of those families.

## 15. Singular and near-singular evidence

Focused validation must contain:

- an exactly singular test surface or patch where `S_u x S_v == 0`;
- deterministic `singular_parameterization` failure;
- a near-degenerate but nonzero fixture whose area density is representable and
  must succeed;
- repeated identical success/failure evidence.

No branch may classify near-degeneracy through a global tolerance.

## 16. Numeric strategy

The implementation should avoid direct raw cross products when they can
overflow despite a representable unit normal.

Preferred strategy:

1. obtain finite `S_u,S_v` from the surface;
2. compute scale-aware tangent magnitudes;
3. normalize tangents or otherwise scale both vectors before the cross product;
4. determine exact zero/nonzero tangent-plane area without a universal epsilon;
5. compute oriented unit normal from the scaled cross product;
6. reconstruct area density with scale-aware products;
7. compute metric coefficients with explicit representability checks.

A numerically equivalent approach is allowed if documented and independently
tested.

## 17. Explicit exclusions

This decision does not authorize:

- second fundamental form;
- normal derivatives;
- Gaussian curvature;
- mean curvature;
- principal curvatures;
- principal directions;
- umbilic classification;
- conditioning/condition-number policy;
- adaptive precision;
- analytic sphere/cone/torus production types;
- full-periodic cylinder;
- general trimming/p-curves/topological faces;
- Surface Representation qualification;
- boundary discretization;
- sizing;
- meshing;
- Quad-Dominant work;
- parallel execution.

## 18. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Generic API | Works from the common bounded-surface derivative contract. |
| Plane oracle | Exact/analytic E,F,G,J and normal. |
| Cylinder oracle | Analytic metric, area density and radial normal. |
| Polynomial oracle | Nontrivial metric agrees with independent formula. |
| All integrated families | At least one regular point succeeds for each family. |
| Singular fixture | Exact typed singular-parameterization failure. |
| Near-degenerate regular | Nonzero representable area succeeds without epsilon rejection. |
| U reversal | F and normal signs change as required. |
| V reversal | F and normal signs change as required. |
| U+V reversal | Metric/J/normal covariance preserved. |
| Translation | No differential change. |
| Power-of-two scale | Metric/J scale quadratically; normal preserved. |
| Signed Cartesian frame | Metric/J invariant; normal orientation law respected. |
| Extreme finite | Avoidable overflow/underflow rejected as implementation defect. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/meshing/I/O/threading/external CAD dependency. |
| Prerequisite preservation | Existing 36 ordinary tests remain PASS. |

If exactly one new focused contract is added, ordinary FAST/INTEGRATION
inventory becomes **37 tests**.

## 19. Why Surface Differential Geometry is selected now

The generic first-order differential quantities depend on first derivatives,
not on completion of every possible surface family.

The repository already has:

- a stable bounded 2D parameter contract;
- analytic first/second derivatives;
- multiple independent free-form and constructed surface families;
- two analytic production families (plane and cylinder);
- trimming and orientation/reversal semantics.

This is sufficient to establish and regress a generic first-order
surface-differential seam.

Delaying differential geometry until sphere/cone/torus and full CAD trimming
are all implemented would not resolve a prerequisite of the first-order metric
or normal calculation.

## 20. Why sphere, cone and torus are deferred

These remain important Surface Representation breadth obligations.

They introduce representation-specific issues:

- sphere: polar singularities and angular-domain policy;
- cone: apex singularity and radius/sign/domain policy;
- torus: two angular directions and periodic seam policy.

Those semantics should be decided independently and later used as additional
analytic differential-geometry oracles.

They are not prerequisites for the selected first-order generic work unit.

## 21. Why general trimming / p-curves / faces are deferred

General CAD trimming requires at least:

- parameter-space trim curves;
- physical-space realization evidence;
- loop orientation;
- curve-on-surface relation;
- explicit face/topology identity;
- potentially heterogeneous boundary-curve composition.

Those are crucial before Shared Boundary Certification and CAD-like face
coverage, but a local metric/normal computation on a bounded supporting surface
does not depend on them.

## 22. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 36 ordinary semantic tests must remain passing;
- the new first-order differential contract must pass.

Passing yields only:

**SURFACE DIFFERENTIAL GEOMETRY STAGE OPEN /
FIRST-ORDER METRIC+NORMAL IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

## 23. Stop conditions

Stop and require a new decision if implementation needs:

- any second-order curvature output;
- topology/face orientation;
- a global geometric tolerance;
- adaptive-precision predicates;
- changes to concrete surface evaluation semantics;
- a new surface representation family;
- third-party runtime dependencies;
- boundary discretization or meshing code.

## 24. Planned sequence after the first work unit

Planning only, not authorization.

After the first-order metric/normal work unit closes, a fresh decision should
compare:

1. second fundamental form + Gaussian/mean curvature;
2. principal curvatures/directions;
3. explicit conditioning/near-singular diagnostics;
4. returning to Surface Representation for sphere/cone/torus;
5. returning to general trimming/p-curves/topological faces.

No option is pre-authorized.

## 25. Effect if integrated and closed

After this decision integrates, post-merge validation passes, and a separate
decision checkpoint closes, the sole next implementation work item becomes:

**Pointwise Surface Regularity, First Fundamental Form, Area Density, and
Oriented Unit Normal in 3D.**

Surface Differential Geometry becomes the active scientific stage.

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED with retained
sphere, cone, torus, full-periodic cylinder and general-trimming obligations.

Boundary Curve Discretization and downstream meshing remain blocked.


## 26. Decision integration checkpoint

Decision PR #194 used final head:

`e48a6040bb4ff6463b170acce0651324881967b3`.

PR validation:

- FAST `36114864362`: PASS;
- INTEGRATION `36114864400`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #194 merged as:

`2a4b1df3732eaaa1a768c1d8d0b41bdd3310ffac`.

Post-merge validation:

- FAST `36114962084`: PASS;
- INTEGRATION `36114962093`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision result:

**SURFACE DIFFERENTIAL GEOMETRY ENTRY DECISION INTEGRATED /
CLOSURE PENDING / IMPLEMENTATION NOT STARTED / NOT QUALIFIED.**

After closure integration and post-merge validation, the sole next production
work item is the first-order metric/normal capability defined by Sections
6–23.

No second-order curvature or new surface family is authorized.
