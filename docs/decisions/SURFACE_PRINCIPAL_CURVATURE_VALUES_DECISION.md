# Surface Principal Curvature Values — Bounded Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-25  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Question

After terminal closure of the pointwise second fundamental form plus Gaussian
and mean curvature work unit, which next capability should be selected from:

1. principal curvatures/directions;
2. explicit conditioning diagnostics;
3. remaining Surface Representation breadth (sphere/cone/torus);
4. general trimming/p-curves/topological faces;
5. Boundary Curve Discretization readiness?

The selected work unit must add one bounded scientific concept only.

## 2. Fresh repository authority

Decision-entry main:

`6a3814332ed28d88fb9949684af92a8cb3390d39`.

Terminal second-order Surface Differential Geometry evidence includes:

- implementation PR #201:
  `ed91d70d11446925148cc0ee5f0efab879022270`;
- implementation post-merge FAST `36127252797`: PASS, 38/38;
- implementation post-merge INTEGRATION `36127252837`: PASS, 38/38;
- closure PR #202:
  `2ddd991bb9246bb8e6330f0cdf9a87afde93283f`;
- closure post-merge FAST `36127811804`: PASS, 38/38;
- closure post-merge INTEGRATION `36127811852`: PASS, 38/38;
- terminal publication PR #203:
  `db382cf6218ad4fbf18cbafb3b246be812f6b3b7`;
- authority-reconciliation PR #204:
  `d7793cd50637e28b92ca1b8e1fbf21ba01121595`;
- current-stage reconciliation PR #205:
  `6a3814332ed28d88fb9949684af92a8cb3390d39`;
- PR #205 post-merge FAST `36146747809`: PASS;
- PR #205 post-merge INTEGRATION `36146747762`: PASS.

Ordinary semantic inventory: **38 tests**.

No production work item is active.

## 3. Existing differential data

Production already exposes:

- first fundamental form `E,F,G`;
- area density `J=|S_u x S_v|`;
- oriented unit normal;
- second fundamental form `L,M,N`;
- Gaussian curvature `K`;
- oriented mean curvature `H`.

Therefore the two principal curvature values are a bounded next scalar property.

## 4. Literature and mature-kernel evidence

Patrikalakis, Maekawa and Cho:

- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node26.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node31.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node186.html

Relevant evidence:

- principal curvatures are the extremal normal curvatures / eigenvalues of the
  shape operator;
- `K=k1*k2` and `H=(k1+k2)/2`;
- principal directions are a separate eigenvector problem;
- principal directions are not unique at umbilics and curvature-line tracing
  has additional numerical/orientation issues.

Open CASCADE surface-local-property utilities:

- https://dev.opencascade.org/doc/refman/html/_geom_l_prop___surface_utils_8hxx.html
- https://dev.opencascade.org/doc/occt-6.9.0/refman/html/class_geom_l_prop___s_l_props.html

Relevant evidence:

- mature kernels expose min/max principal curvature separately from curvature
  directions;
- curvature is computed from first/second fundamental-form information;
- umbilic state is an explicit local-property concern.

Surface meshing literature:

https://www.sciencedirect.com/science/article/pii/S0168874X09000936

Relevant evidence:

- isotropic geometric sizing can be based on the strongest principal
  curvature;
- principal directions become relevant when anisotropic metrics/oriented
  stretching are introduced.

External sources are design/scientific evidence only. No external runtime
dependency is admitted.

## 5. Candidate comparison

| Candidate | Immediate new semantics | Dependency/risk | Downstream value | Decision |
| --- | --- | --- | --- | --- |
| Principal curvature values + exact represented-data umbilic state | two generalized eigenvalues | bounded scalar extension of existing I/II | **Very high** for completing scalar differential geometry and later isotropic sizing | **SELECTED** |
| Principal directions | eigenvectors, sign/orientation, umbilic non-uniqueness | requires a separate directional contract | high mainly for future anisotropy | DEFER |
| Conditioning diagnostics | near-singular / near-umbilic policy | would require explicit scale/tolerance semantics | high later | DEFER |
| Sphere/cone/torus representations | new representation/domain/singularity semantics | independent of principal-value definition | useful new oracles/families | DEFER |
| General trimming/p-curves/faces | topology + curve-on-surface identity | large architectural seam | critical later for CAD faces/shared boundaries | DEFER |
| Boundary Curve Discretization | approximation/trace/sizing policy | representation breadth and sizing policy still open | critical pipeline stage | DEFER |

## 6. Decision

Authorize exactly one future implementation work unit:

**Pointwise Ordered Principal Curvature Values plus Exact Represented-Data
Umbilic State for Regular C2 Bounded Parametric Surfaces in 3D.**

Principal directions are explicitly **not** part of this work unit.

## 7. Conceptual API

Preferred extension in
`include/apmesh/geometry/surface_differential.hpp`:

`SurfacePrincipalCurvatures`

with:

- `maximum_curvature`;
- `minimum_curvature`;
- `is_umbilic`.

Required ordering:

`maximum_curvature >= minimum_curvature`

for the currently oriented surface normal.

Preferred functions:

- value-level function consuming `SurfaceSecondOrderGeometry3`;
- generic bounded-surface overload reusing
  `surface_second_order_geometry(surface,u,v)`.

No public shape-operator matrix or principal direction is authorized.

## 8. Mathematical contract

Principal curvatures solve the generalized symmetric eigenproblem

`II x = k I x`

with

`I=[[E,F],[F,G]]`

and

`II=[[L,M],[M,N]]`.

The two real eigenvalues are the principal curvature values.

They must satisfy, within the declared numeric contract:

- `k_max + k_min = 2H`;
- `k_max * k_min = K`.

These are validation identities, not the required production algorithm.

## 9. Numerically stable strategy

Production must **not** blindly evaluate

`H +/- sqrt(H*H-K)`

followed by an epsilon clamp of a negative discriminant.

Preferred strategy:

1. use the already validated positive-definite first fundamental form and
   positive area density;
2. transform the generalized eigenproblem to an equivalent symmetric 2x2
   eigenproblem in an orthonormalized tangent coordinate system;
3. use scale-aware arithmetic and a stable symmetric 2x2 eigenvalue formula
   based on `hypot`/equivalent;
4. convert final eigenvalues to finite `double` only when representable.

A Cholesky/metric-whitening construction is preferred because it avoids
introducing a tolerance for `H^2-K >= 0`.

No universal epsilon is authorized.

## 10. Umbilic semantics

The work unit admits only an **exact represented-data** umbilic state.

After the symmetric 2x2 curvature operator is formed, the point is umbilic
exactly when the represented eigenvalue gap is exactly zero.

No near-umbilic threshold is introduced.

Required cases:

- exact plane: umbilic, both principal curvatures zero;
- exact synthetic isotropic shape operator: umbilic with repeated nonzero
  principal value;
- arbitrarily close but non-equal representable eigenvalues: non-umbilic.

A later conditioning decision may introduce an explicit near-umbilic
diagnostic, but this work unit must not anticipate it.

## 11. Orientation covariance

Principal curvatures are signed with the repository's oriented normal.

For a single U or V reversal, the oriented normal and second fundamental form
flip sign. Therefore:

- new maximum = negative old minimum;
- new minimum = negative old maximum;
- umbilic state is unchanged.

For simultaneous U+V reversal, orientation and ordered values are preserved.

## 12. Scale and frame covariance

For positive uniform coordinate scaling by representable factor `lambda`:

- each principal curvature scales by `1/lambda`;
- umbilic state is unchanged.

Translation has no effect.

Admitted orthogonal frame transformations preserve the principal values subject
to the repository's normal-orientation law.

## 13. Mandatory analytic/reference fixtures

Focused validation must include:

### Plane

- `k_max = 0`;
- `k_min = 0`;
- exact umbilic.

### Cylinder

For radius `R`, under the repository orientation convention:

- one principal curvature is zero;
- the other has magnitude `1/R`;
- not umbilic.

### Synthetic diagonal shape operator

Use exact first form `I=identity` and selected second form
`II=diag(a,b)` to validate ordered signed values independently of surface
representation.

Required subcases:

- elliptic distinct values;
- hyperbolic opposite signs;
- parabolic one zero;
- exact nonzero umbilic `a=b`;
- near-umbilic but exactly distinct represented values.

### Generic non-orthogonal metric

Use a synthetic positive-definite non-diagonal first form and an independently
constructed symmetric generalized eigenproblem to prove the implementation is
not assuming `F=0`.

## 14. Cross-checks with existing H/K

For every successful focused fixture:

- `k_max + k_min` agrees with `2H`;
- `k_max*k_min` agrees with `K`;

using the repository's explicit proximity policy in tests only.

Production does not use a tolerance to repair these identities.

## 15. Extreme finite evidence

At least one fixture must exercise extreme but finite form scales.

Required behavior:

- succeed when principal values are representable and a scale-aware
  generalized eigenvalue evaluation can produce them;
- otherwise return
  `SurfaceDifferentialError::non_representable_result`.

No saturation, infinity, NaN sanitization or fallback to H/K subtraction is
permitted.

## 16. Error propagation

The generic surface overload preserves all existing failures:

- parameter/domain failure;
- insufficient continuity;
- singular parameterization;
- non-representable differential result.

No new error enumerator is expected for principal values.

Exact umbilicity is a successful state, not an error.

## 17. Explicit exclusions

This decision does not authorize:

- principal directions/eigenvectors;
- direction sign conventions;
- curvature-line integration;
- near-umbilic thresholds;
- condition numbers or conditioning classes;
- adaptive precision;
- analytic sphere/cone/torus production types;
- full-periodic cylinder;
- general trimming/p-curves/topological faces;
- Boundary Curve Discretization;
- physical sizing implementation;
- anisotropic/tensor metrics;
- meshing;
- Quad-Dominant;
- parallel execution.

## 18. Why directions are deferred

At non-umbilic points, principal directions are eigenvectors of the shape
operator.

At umbilics the eigenvalue is repeated and there is no unique distinguished
principal direction.

A production direction API would therefore require separate decisions for:

- sign ambiguity;
- orientation continuity;
- exact umbilic behavior;
- near-umbilic conditioning;
- optional line-field rather than vector semantics.

Those issues are orthogonal to the scalar values and are particularly relevant
to the future anisotropic extension, which is deliberately sequenced much
later.

## 19. Why conditioning diagnostics are deferred

The current repository deliberately avoids universal epsilon policies.

A near-singular or near-umbilic diagnostic must define:

- a dimensionless condition measure;
- scale laws;
- exact threshold ownership;
- whether it is advisory or failure-producing.

No such policy is needed to return exact represented-data eigenvalues.

## 20. Why Surface Representation breadth is deferred

Plane, cylinder, polynomial/rational/NURBS/Coons/constructed surfaces and
synthetic forms already provide sufficient independent fixtures for principal
values.

Sphere/cone/torus remain explicit representation obligations, but they are not
a mathematical prerequisite for the selected generic differential property.

## 21. Why trimming/topological faces are deferred

General p-curves and topological faces are essential for CAD boundary identity
and shared-boundary certification, but pointwise principal curvature on a
supporting surface does not depend on those ownership/identity semantics.

That architectural seam remains separate.

## 22. Why Boundary Curve Discretization is not resumed yet

Boundary Curve Discretization requires its own approximation-error,
parameterization-invariance, shared-trace and sizing decisions.

The roadmap's future isotropic sizing stage benefits directly from the strongest
principal curvature, while the declared boundary regression envelope still has
retained curve/topology breadth obligations.

Therefore the bounded scalar principal-curvature step is selected first.

## 23. Repository mapping for future implementation

Authorized future mapping is limited to:

- public API:
  `include/apmesh/geometry/surface_differential.hpp`;
- production:
  `src/geometry/surface_differential.cpp`;
- focused contract:
  `tests/surface_principal_curvatures.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision;
- one candidate-validation audit and one implementation audit when applicable.

No concrete surface representation source should require modification.

## 24. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| API | Ordered principal-value result and exact umbilic flag. |
| Plane | 0/0 and exact umbilic. |
| Cylinder | 0 and signed 1/R under current orientation. |
| Elliptic synthetic | Two same-sign distinct values. |
| Hyperbolic synthetic | Opposite-sign values. |
| Parabolic synthetic | Exactly one zero. |
| Exact nonzero umbilic | Repeated nonzero value succeeds. |
| Near-umbilic | Exactly distinct represented values remain distinct. |
| Non-orthogonal metric | Generalized eigenproblem matches independent oracle. |
| H/K identities | Sum/product match existing mean/Gaussian curvature. |
| U reversal | Ordered sign/swap law. |
| V reversal | Ordered sign/swap law. |
| U+V reversal | Values preserved. |
| Positive scaling | Curvatures scale reciprocally. |
| Translation/frame | Declared invariance/covariance. |
| Extreme finite | Scale-aware success or explicit non-representable failure. |
| Determinism | Repeated successes/failures identical. |
| Header isolation | No topology/meshing/I/O/threading/external CAD dependency. |
| Prerequisite preservation | Existing 38 ordinary tests remain PASS. |

If one new focused contract is added, ordinary inventory becomes **39 tests**.

## 25. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 38 ordinary tests remain passing;
- the principal-curvature focused contract passes.

Passing yields only:

**PRINCIPAL CURVATURE VALUES IMPLEMENTED /
FOCUSED CONTRACTS PASS / INTEGRATED / NOT QUALIFIED.**

## 26. Stop conditions

Stop and require a new decision if implementation needs:

- a principal direction API;
- a tolerance/threshold for umbilicity;
- adaptive precision;
- a new error category beyond existing differential failures;
- changes to concrete surface evaluation semantics;
- a new surface representation;
- topology/trimming changes;
- discretization/sizing/meshing code;
- third-party runtime dependency.

## 27. Planned sequence after closure

Planning only, not authorization.

After principal-value implementation closes, a fresh decision should recompare:

1. principal directions and their line-field/umbilic semantics;
2. explicit conditioning diagnostics;
3. sphere/cone/torus Surface Representation breadth;
4. general trimming/p-curves/topological faces;
5. Boundary Curve Discretization readiness;
6. whether Surface Differential Geometry has enough evidence to prepare a
   bounded qualification campaign.

No option is pre-authorized.

## 28. Effect if integrated and closed

After this decision is integrated, post-merge validated and separately closed,
the sole next production work item is:

**Pointwise Ordered Principal Curvature Values plus Exact Represented-Data
Umbilic State.**

No production implementation begins on this decision branch.


## 29. Decision integration checkpoint

Decision PR #206 used final head
`3a5af9d27aaf9b3e563e9fd6f89b868745c7edda`.

Decision PR validation:

- FAST `36148367215`: PASS;
- INTEGRATION `36148367239`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #206 merged as
`8d42bf38871d7a79f1c01aacbad77ed5e106505f`.

Post-merge validation:

- FAST `36190790100`: PASS;
- INTEGRATION `36190790157`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

The bounded decision is integrated and ready for checkpoint closure.

After closure integration and its post-merge validation, the sole next
production work item is the ordered principal-curvature-values capability
defined in Sections 6–26.

No principal direction, tolerance/conditioning, representation, trimming,
discretization, sizing, anisotropic or meshing capability is authorized.


## 30. Decision closure checkpoint

Decision closure PR #207 used head
`937f90b8fbac129bb4e92e30db5d42dfb2dcff6c`.

Closure PR validation:

- FAST `36191030571`: PASS;
- INTEGRATION `36191030276`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

PR #207 merged as
`bb33396050b4ccc85ca9cc4e1683d491e89bcac0`.

Closure post-merge validation:

- FAST `36191169978`: PASS;
- INTEGRATION `36191169982`: PASS in GCC 13 Debug and Clang 18/libc++
  Debug.

Decision checkpoint result:

**DECISION CLOSED / IMPLEMENTATION AUTHORIZED / NOT QUALIFIED.**

The sole active production work item is the scalar principal-curvature-values
capability defined in Sections 6–26.

No principal direction, conditioning threshold, representation, trimming,
discretization, sizing, anisotropic or meshing capability is authorized.


## 31. Active implementation mapping

The sole authorized implementation is active on:

`surface/principal-curvature-values`.

Candidate mapping:

- public API:
  `include/apmesh/geometry/surface_differential.hpp`;
- production:
  `src/geometry/surface_differential.cpp`;
- focused contract:
  `tests/surface_principal_curvatures.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized STATE / ROADMAP / WORKLOG / this decision.

The candidate introduces:

- `SurfacePrincipalCurvatures` with ordered maximum/minimum values and exact
  represented-data umbilic state;
- a value-level overload consuming `SurfaceSecondOrderGeometry3`;
- a generic bounded-surface overload preserving existing differential errors;
- metric whitening based on normalized I/II and the already validated area
  density;
- symmetric 2x2 eigenvalue evaluation using `hypot` and determinant-based
  recovery of the smaller-magnitude value;
- no `H^2-K` clamp or tolerance-based umbilic classification.

Focused evidence includes:

- plane and bounded cylinder production surfaces;
- elliptic, hyperbolic, parabolic, exact-umbilic and near-umbilic synthetic
  shape operators;
- a non-orthogonal generalized eigenproblem;
- H/K identities;
- single/double reversal laws;
- positive scaling, translation and signed Cartesian frame covariance;
- extreme finite success and explicit non-representable failure;
- generic error propagation and deterministic repetition.

Expected ordinary semantic inventory: **39 tests**.

Current status:

**IMPLEMENTED CANDIDATE / PRE-PR VALIDATION PENDING / NOT QUALIFIED.**

No principal direction, conditioning threshold, representation breadth,
trimming/topology, discretization/sizing, anisotropic or meshing capability is
implied.
