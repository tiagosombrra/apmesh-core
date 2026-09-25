# Surface Second Fundamental Form and Gaussian/Mean Curvature — Bounded Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-25  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Question

After the terminal closure of pointwise surface regularity, first fundamental
form, area density and oriented unit normal, which bounded scientific capability
should be implemented next?

Required comparison:

1. second fundamental form plus Gaussian/mean curvature;
2. principal curvatures/directions;
3. explicit conditioning / near-singular diagnostics;
4. return to Surface Representation for sphere/cone/torus;
5. return to general trimming/p-curves/topological faces.

No candidate is pre-authorized at decision entry.

## 2. Fresh repository authority

Canonical decision-entry `main`:
`793fd46575e0c00d11ad2739b8e9c92227d576a1`.

Terminal first-order Surface Differential Geometry evidence:

- implementation PR #196:
  `e476eaaaa56f59a5d66574f180083ecd621f8b95`;
- implementation post-merge FAST `36118945024`: PASS, 37/37;
- implementation post-merge INTEGRATION `36118944987`: PASS, 37/37;
- implementation closure PR #197:
  `5fc561a2e0e927369c5dc5c4644ef18699602914`;
- closure post-merge FAST `36119643984`: PASS, 37/37;
- closure post-merge INTEGRATION `36119643997`: PASS, 37/37;
- terminal-state publication:
  `793fd46575e0c00d11ad2739b8e9c92227d576a1`;
- terminal-state FAST `36120283995`: PASS;
- terminal-state INTEGRATION `36120284007`: PASS;
- no open PR and no active production/documentation work item.

Protected-main ordinary semantic inventory at entry: **37 tests**.

## 3. Existing prerequisites

Already integrated:

- `SurfaceFirstDerivatives3`;
- `SurfaceSecondDerivatives3`;
- `SurfaceMetricNormal3`;
- first fundamental form coefficients `E,F,G`;
- area density `|S_u x S_v|`;
- oriented unit normal from `S_u x S_v`;
- exact singular-parameterization failure;
- explicit insufficient-continuity propagation;
- no universal regularity epsilon;
- representative polynomial, rational, NURBS, Coons, trimmed, swept and
  analytic bounded surfaces.

The next work unit must reuse these semantics rather than fork them.

## 4. Literature basis

### 4.1 Classical differential-geometry ordering

Patrikalakis, Maekawa and Cho, MIT Hyperbook:

- Differential Geometry of Surfaces:
  https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node26.html
- First fundamental form:
  https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node28.html
- Gaussian and mean curvatures:
  https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node31.html

Relevant structure:

- first-order metric/normal geometry precedes second-order curvature geometry;
- Gaussian and mean curvature depend on the first and second fundamental forms;
- principal curvatures are a subsequent spectral/extremal interpretation of
  the same second-order geometry.

### 4.2 CAD-kernel interrogation ordering

Open CASCADE `GeomLProp_SLProps`:

https://dev.opencascade.org/doc/occt-6.9.1/refman/html/class_geom_l_prop___s_l_props.html

Relevant structure:

- surface local properties expose first/second derivatives, normal,
  Gaussian/mean curvature, principal curvatures and directions as distinct
  queried properties;
- curvature requires stronger derivative information than a normal alone.

Open CASCADE is design/reference evidence only. It is not a runtime dependency,
oracle, tolerance authority or API template.

### 4.3 Downstream geometric value

MIT Hyperbook curve/surface interrogation:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node141.html

Relevant structure:

- curvature-based interrogation is explicitly second-order;
- Gaussian, mean and principal curvature are later inputs to shape analysis,
  offset behavior and curvature-driven algorithms.

This supports introducing scalar second-order geometry before attempting global
curvature extrema or downstream meshing adaptation.

## 5. Candidate comparison

| Candidate | New semantic burden | Dependency status | Immediate downstream value | Decision |
| --- | --- | --- | --- | --- |
| Second fundamental form + Gaussian/mean curvature | second-order projections and stable scalar ratios | all inputs already integrated | **Direct second-order surface geometry** | **SELECTED** |
| Principal curvatures/directions | eigenvalues/eigenvectors, umbilics, direction sign/order degeneracy | depends naturally on II and K/H | High, but one layer later | DEFER |
| Conditioning diagnostics | dimensionless conditioning metric and threshold/policy semantics | independent scientific policy unresolved | Important, but must not become hidden epsilon | DEFER |
| Sphere/cone/torus representation | new analytic placement/domain/singularity families | representation-stage gap, not differential prerequisite | High breadth value | DEFER |
| General trimming/p-curves/faces | curve-on-surface + topology identity + loop semantics | independent representation/topology seam | High CAD value | DEFER |

## 6. Decision

Authorize exactly one future implementation work unit:

**Pointwise Surface Second Fundamental Form plus Gaussian and Mean Curvature
for regular C2 bounded parametric surfaces in 3D.**

No principal curvature/direction API is authorized.

## 7. Public result model

Preferred public additions in
`include/apmesh/geometry/surface_differential.hpp`:

`SurfaceSecondFundamentalForm`

with coefficients:

- `l`;
- `m`;
- `n`.

`SurfaceSecondOrderGeometry3`

containing:

- existing `SurfaceMetricNormal3 metric_normal`;
- `SurfaceSecondFundamentalForm second_fundamental_form`;
- `double gaussian_curvature`;
- `double mean_curvature`.

Names may be mechanically adjusted for C++ clarity, but the semantic partition
must remain the same.

## 8. Core mathematical convention

Let:

- `S_u, S_v`: first partials;
- `S_uu, S_uv, S_vv`: second partials;
- `n`: the already defined oriented unit normal from `S_u x S_v`.

First fundamental form:

- `E = S_u · S_u`;
- `F = S_u · S_v`;
- `G = S_v · S_v`.

Second fundamental form convention:

- `L = S_uu · n`;
- `M = S_uv · n`;
- `N = S_vv · n`.

This convention is frozen for AP Mesh.

Consequences:

- reversing surface orientation changes the sign of the oriented second
  fundamental form as dictated by the parameter transformation;
- Gaussian curvature is orientation invariant;
- mean curvature is orientation sensitive and changes sign when the oriented
  normal reverses.

No alternative sign convention may be introduced silently later.

## 9. Gaussian curvature

For a regular parameterization:

`D = E G - F^2 = |S_u x S_v|^2 > 0`.

Gaussian curvature:

`K = (L N - M^2) / D`.

Production must not use geometric sampling or finite differences.

## 10. Mean curvature

Using the convention in Section 8:

`H = (E N - 2 F M + G L) / (2 D)`.

`H` is oriented.

Expected analytic sign example with current cylinder orientation:

- outward-oriented circular cylinder of radius `r`;
- `K = 0`;
- `H = -1/(2r)` under the frozen `L=S_uu·n` convention.

The test contract must freeze this sign.

## 11. Error model

Reuse `SurfaceDifferentialError` unchanged.

Required propagation/order for the templated surface query:

1. query first derivatives;
2. map any surface-domain/finiteness error;
3. compute first-order metric/normal;
4. if singular, return
   `SurfaceDifferentialError::singular_parameterization`;
5. query second derivatives;
6. propagate `insufficient_continuity` or other surface error;
7. compute II/K/H;
8. if final representable geometry cannot be produced, return
   `SurfaceDifferentialError::non_representable_result`.

No new common error enum is authorized.

This order intentionally makes first-order regularity a prerequisite before
second-order curvature availability.

## 12. Continuity semantics

The work unit requires ordinary unique second partials.

If the underlying surface returns
`SurfaceError::insufficient_continuity` for second derivatives at a parameter,
the curvature query returns
`SurfaceDifferentialError::insufficient_continuity`.

No one-sided curvature is introduced.

No averaging across C1 knot lines is permitted.

## 13. Singular and near-singular semantics

Exact singular parameterization remains the only automatic regularity failure.

No universal epsilon or hidden condition-number threshold is introduced.

A near-singular but exactly regular parameterization:

- must be accepted if the requested II/K/H results are representable;
- may return `non_representable_result` if the final requested values cannot
  be represented.

A later conditioning decision may add diagnostics without retroactively
changing this exact semantic boundary.

## 14. Numerical strategy

Production must avoid avoidable intermediate overflow/cancellation where the
final requested scalar is representable.

Requirements:

- reuse the already scale-aware first-order metric/normal result;
- project second derivatives onto the unit normal using a scale-aware finite
  dot/projection mechanism;
- do not recompute `D` naively as `E*G-F*F` when the already available
  area density gives `D = area_density^2`;
- compute curvature products/ratios with exponent-aware or equivalent
  scale-aware arithmetic;
- reject non-representable final values explicitly;
- no arbitrary precision or third-party dependency.

The implementation may use existing project patterns such as `frexp/ldexp`
or a verified equivalent.

## 15. Affine/scale covariance

For a uniform positive coordinate scale `s`:

- `E,F,G` scale as `s^2`;
- area density scales as `s^2`;
- `L,M,N` scale as `s`;
- `K` scales as `1/s^2`;
- `H` scales as `1/s`;
- oriented unit normal is unchanged.

Focused tests must include exact power-of-two scaling.

Translation leaves II/K/H unchanged.

## 16. U/V reversal covariance

Focused evidence must cover existing concrete surfaces whose U/V reversal is
available.

At minimum verify:

- Gaussian curvature unchanged under U reversal;
- Gaussian curvature unchanged under V reversal;
- mean curvature changes sign when exactly one parameter direction reverses and
  therefore the oriented normal reverses;
- mean curvature is preserved when both U and V reverse;
- second-form coefficient transformations are consistent with the explicit
  derivative/normal transformation;
- no recomputed normal convention is introduced.

## 17. Analytic fixtures

Mandatory analytic/reference fixtures:

### Plane

- `L=M=N=0`;
- `K=0`;
- `H=0`.

### Circular cylinder

For radius `r` and the integrated outward orientation:

- one principal bending direction is zero;
- `K=0`;
- `H=-1/(2r)` under the AP Mesh sign convention.

### Saddle / hyperbolic paraboloid

Use an integrated polynomial surface or test-only synthetic derivative fixture
with known negative Gaussian curvature.

This independently checks a nonzero mixed/second-order case.

### Synthetic derivative fixture

A direct `SurfaceFirstDerivatives3 + SurfaceSecondDerivatives3` fixture must
exercise non-orthogonal tangent vectors so the implementation is not validated
only on diagonal first fundamental forms.

## 18. Cross-family conformance

The templated API must be exercised on representative integrated regular
families with C2 availability, including as applicable:

- bicubic Bézier;
- rational bicubic Bézier;
- bicubic NURBS away from insufficient-continuity knot lines;
- Coons patch;
- rectangular trimmed surface;
- linear extrusion;
- revolution surface;
- bounded analytic plane;
- bounded analytic cylinder sector.

The work unit does not certify all points of all surfaces.

It proves common pointwise semantics on bounded representative fixtures.

## 19. Insufficient-continuity fixture

Use the already integrated multiplicity-two bicubic NURBS surface seam.

At an exact C1 knot line where unique ordinary second partials are unavailable:

- first-order metric/normal may succeed;
- second-order curvature query must fail with
  `insufficient_continuity`.

Immediately off the knot line, curvature may succeed if regular and
representable.

## 20. Extreme finite evidence

Focused testing must include:

- very large/small finite first/second derivative scales;
- a regular case where final K/H remain representable;
- a case where final curvature is not representable and explicit
  `non_representable_result` is required.

No silent infinity, NaN or zero-underflow substitution is permitted.

## 21. Determinism

Repeated identical inputs must produce exactly identical:

- successful second-form coefficients;
- K/H values;
- typed failures.

Serial deterministic order is the reference.

## 22. Principal-curvature deferral

Even though scalar principal curvatures can be algebraically related to K/H,
this work unit does not expose them.

A later decision must define:

- ordering convention for max/min curvature;
- numerically stable discriminant evaluation;
- umbilic semantics;
- principal-direction representation;
- direction sign ambiguity;
- repeated-eigenvalue/umbilic direction behavior;
- reversal/orientation covariance.

This is a distinct scientific seam.

## 23. Conditioning deferral

No condition number, near-singular classification or tolerance-based warning is
added now.

A later decision must define a dimensionless diagnostic and whether it is:

- pure evidence;
- a returned property;
- a failure criterion.

It may not retroactively turn the exact nonzero-area regularity rule into a
hidden epsilon.

## 24. Surface Representation gaps remain active

This decision does not close Surface Representation.

Retained gaps include:

- sphere;
- cone;
- torus;
- full-periodic cylinder/revolution seams where required;
- general trimming;
- p-curves / curve-on-surface;
- topological faces and oriented loops.

These remain future representation/topology decisions and independent
regression oracles for differential geometry when implemented.

## 25. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Direct derivative API | first + second derivative structs produce II/K/H. |
| Surface template API | domain/continuity errors propagate deterministically. |
| Plane | II=0, K=0, H=0. |
| Cylinder | K=0 and oriented H=-1/(2r). |
| Saddle | negative K and analytic/reference parity. |
| Non-orthogonal tangents | general E/F/G formula exercised. |
| C1 NURBS knot line | first-order success, curvature insufficient-continuity failure. |
| Off-knot NURBS | finite curvature where regular/representable. |
| U reversal | K invariant; H orientation behavior correct. |
| V reversal | K invariant; H orientation behavior correct. |
| Double reversal | K/H restored as expected. |
| Translation | II/K/H unchanged. |
| Power-of-two scale | II ×s, K /s², H /s. |
| Near singular regular | accepted without epsilon if representable. |
| Exact singular | singular-parameterization failure. |
| Unrepresentable | explicit non-representable-result failure. |
| Cross-family | representative integrated surfaces conform. |
| Determinism | repeated success/failure identical. |
| Header isolation | no topology/meshing/I/O/threading/external dependency. |
| Regression | existing 37 ordinary tests remain PASS. |

If one new focused ordinary contract is added, the inventory becomes
**38 tests**.

## 26. Repository mapping for future implementation

Authorized future mapping:

- public API extension:
  `include/apmesh/geometry/surface_differential.hpp`;
- production extension:
  `src/geometry/surface_differential.cpp`;
- focused contract:
  `tests/surface_second_order_curvature.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  this decision;
- focused implementation audit after integration/closure.

No change to `parametric_surface.hpp` is expected.

If the common bounded-surface concept must change, stop and require a new
decision.

## 27. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18/libc++ Debug / INTEGRATION must pass;
- all existing 37 ordinary semantic tests must remain PASS;
- the new second-order curvature contract must pass.

Passing yields only:

**SECOND FUNDAMENTAL FORM + GAUSSIAN/MEAN CURVATURE IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

It does not qualify Surface Differential Geometry.

## 28. Stop conditions

Stop and require a new decision if implementation needs:

- principal curvatures or directions;
- umbilic classification;
- one-sided curvature;
- tolerance-based regularity rejection;
- a new surface error enum value;
- a change to `BoundedParametricSurface3`;
- a new surface representation family;
- trimming/topology;
- discretization/sizing/meshing;
- third-party runtime dependency.

## 29. Planned next comparison

Planning only, not authorization.

After this work unit is integrated and closed, compare:

1. principal curvatures/directions;
2. explicit conditioning diagnostics;
3. sphere/cone/torus Surface Representation breadth;
4. general trimming/p-curves/topological faces;
5. whether pointwise differential geometry is sufficient to resume
   Boundary Curve Discretization preparation.

No option is pre-authorized.

## 30. Effect if integrated and closed

After decision integration, post-merge validation and separate closure, the
sole next production work item becomes:

**Pointwise Surface Second Fundamental Form plus Gaussian and Mean Curvature.**

No production implementation begins on this decision branch.


## 31. Decision integration checkpoint

PR #199 integrated this bounded decision using head
`b2608af43c6b41a3faf1332343fdc5a0bab2a593`.

Decision-head validation:

- FAST `36124245751`: PASS;
- INTEGRATION `36124245699`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #199 merged as
`f5288a3c1388ee1c0255276b770461fb9ee70fae`.

Post-merge validation:

- FAST `36124361467`: PASS;
- INTEGRATION `36124361479`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The decision checkpoint is ready for documentation closure.

After closure integration and its post-merge validation, the sole next
production work item is the II/K/H implementation bounded by Sections 6–28.

No principal curvature/direction, conditioning diagnostic, new surface
representation, trimming/topology or downstream meshing capability is
authorized.


## 32. Decision closure checkpoint

Decision closure PR #200 used head
`a3f15ab622b0e16cafb49e6148961d8d96bc6f36`.

Closure PR validation:

- FAST `36124601027`: PASS;
- INTEGRATION `36124601120`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #200 merged as
`204f0456ae12ecfd367f38cdca874fcf83f4a952`.

Closure post-merge validation:

- FAST `36124711127`: PASS;
- INTEGRATION `36124711118`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / II+K/H IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is
`surface/second-order-curvature`.

No principal curvature/direction, conditioning diagnostic, new surface
representation, trimming/topology or downstream meshing capability is
authorized.


## 33. Active implementation mapping

The sole authorized implementation is active on:

`surface/second-order-curvature`.

Candidate mapping:

- public API:
  `include/apmesh/geometry/surface_differential.hpp`;
- production:
  `src/geometry/surface_differential.cpp`;
- focused contract:
  `tests/surface_second_order_curvature.cpp`;
- build/test registration:
  `CMakeLists.txt`.

Candidate implementation:

- reuses `SurfaceMetricNormal3`;
- adds oriented II coefficients L/M/N;
- adds Gaussian curvature K;
- adds oriented mean curvature H;
- queries first derivatives/regularity before second derivatives in the
  generic surface API;
- uses `long double` only as an internal bounded-range intermediate and
  converts requested results back to representable `double` explicitly;
- uses area-density squared for the first-form determinant rather than a
  naïve `E*G-F*F` reconstruction;
- adds no error enum value, no tolerance threshold and no surface family.

Focused evidence includes:

- general non-orthogonal first form;
- positive and negative Gaussian-curvature synthetic references;
- plane and outward-cylinder analytic K/H;
- U/V/double reversal orientation covariance;
- power-of-two scale covariance;
- exact singular and near-singular regular semantics;
- unrepresentable-curvature failure;
- actual multiplicity-two NURBS insufficient-continuity propagation;
- representative cross-family conformance;
- deterministic success/failure.

Expected ordinary semantic inventory: **38 tests**.

Initial validation incident:

- candidate head
  `c3cd327ebbd4add551c7d331f0149513d15324c7`;
- FAST `36125805859`: PASS;
- INTEGRATION `36125805817`: GCC 13 Debug PASS 38/38; Clang 18/libc++
  Debug FAIL 37/38;
- the only failure was the extreme non-representable-curvature fixture;
- all production and prerequisite contracts compiled; every other ordinary
  semantic test passed;
- classification:
  compiler-sensitive fixture construction, not a production semantic defect;
- focused fixture correction commit:
  `89326bb1ad70449f4ed75c7b00d45699e998ce09`;
- no production/API file changed in the correction;
- retained audit:
  `docs/audits/2026-09-25-surface-second-order-curvature-candidate-validation.md`.

Corrected candidate validation:

- corrected/documented head:
  `32e05ddec7b81f099f0384c95f44f0df27ed24e2`;
- FAST `36126869683`: PASS, 38/38;
- INTEGRATION `36126869675`: PASS, 38/38 in GCC 13 Debug and
  Clang 18/libc++ Debug;
- focused second-order curvature contract PASS in all three jobs;
- every prior ordinary semantic contract remained PASS.

Current status:

**IMPLEMENTED CANDIDATE / FOCUSED CONTRACTS PASS /
FINAL DOCUMENTATION-HEAD REVALIDATION PENDING / NOT QUALIFIED.**
