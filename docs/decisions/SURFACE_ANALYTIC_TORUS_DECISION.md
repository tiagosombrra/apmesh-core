# Bounded Analytic Ring-Torus Surface — Scientific Decision

Status: DECISION ACTIVE / DOCUMENTATION ONLY / IMPLEMENTATION NOT AUTHORIZED / NOT QUALIFIED  
Date: 2026-09-26  
Parent stage: Surface Representation — Continuous Patch Geometry

## 1. Question

After terminal closure of the bounded analytic spherical-surface work unit,
which single bounded scientific capability should be admitted next?

Required comparison:

1. bounded analytic cone;
2. bounded analytic torus;
3. principal directions / line-field semantics;
4. explicit conditioning diagnostics;
5. general trimming / p-curves / topological faces;
6. Surface Representation qualification readiness;
7. Surface Differential Geometry qualification readiness;
8. Boundary Curve Discretization readiness.

The next work unit must add a materially new scientific fixture or close a
real prerequisite gap without pre-authorizing a larger CAD/topology/meshing
seam.

## 2. Fresh repository authority

Decision-entry `main`:

`23c9a93d6b06f10a33b4ce41b8fc18f801b7752e`.

Terminal sphere lineage:

- implementation PR #214:
  `461c3f04454cc4f8aa789d9baba5085362aa6254`;
- implementation post-merge FAST `36263360375`: PASS, 40/40;
- implementation post-merge INTEGRATION `36263360805`: PASS, 40/40;
- closure PR #215:
  `87ff882606f92259da6a6e788af8a8e6a220a3bf`;
- closure post-merge FAST `36264093555`: PASS;
- closure post-merge INTEGRATION `36264093548`: PASS;
- terminal publication PR #216 head:
  `9e1c2a8279e1fa69eedbbff5c12eb91d65d3bd8f`;
- terminal publication FAST `36264389065`: PASS;
- terminal publication INTEGRATION `36264389066`: PASS;
- terminal publication merge:
  `23c9a93d6b06f10a33b4ce41b8fc18f801b7752e`;
- terminal publication post-merge FAST `36264482439`: PASS;
- terminal publication post-merge INTEGRATION `36264482453`: PASS.

Protected-main ordinary semantic inventory at decision entry: **40 tests**.

No open PR and no active production work item exists at decision entry.

## 3. Relevant integrated prerequisites

Surface Representation already includes focused implementations for:

- polynomial bicubic Bézier;
- positive-weight rational bicubic Bézier;
- bicubic NURBS with simple/double knot continuity;
- Coons/transfinite patch;
- rectangular trimming;
- linear extrusion;
- revolution;
- arbitrary right-handed `AxisPlacement3`;
- bounded analytic plane;
- bounded analytic circular-cylinder sector;
- bounded analytic spherical sector.

Surface Differential Geometry already includes:

- pointwise regularity;
- first fundamental form;
- area density;
- oriented unit normal;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state.

Still absent or intentionally deferred:

- bounded analytic cone;
- bounded analytic torus;
- full-periodic elementary-surface seams;
- general trimming / p-curves / topological faces;
- principal directions / line fields;
- explicit conditioning diagnostics;
- stage-level Surface Representation qualification;
- stage-level Surface Differential Geometry qualification;
- Boundary Curve Discretization.

## 4. External scientific and CAD evidence

### 4.1 Mature torus representation

Open CASCADE `Geom_ToroidalSurface`:

https://dev.opencascade.org/doc/occt-7.9.0/refman/html/class_geom___toroidal_surface.html

Relevant evidence:

- a torus is represented by local 3D placement, major radius and minor radius;
- both natural parameter directions are angular and periodic;
- standard evaluation and first/second derivatives are first-class surface
  operations;
- major/minor radius policy distinguishes ring, horn and spindle regimes.

Decision impact:

- AP Mesh already has the placement and bounded-surface prerequisites;
- the first torus work unit can remain bounded/non-periodic by admitting only
  strict sub-`2*pi` U/V sectors;
- selecting the strict ring regime `R > r > 0` avoids horn/spindle
  singular/self-intersecting semantics in this work unit.

### 4.2 Ring torus supplies a new curvature-sign fixture

A standard ring-torus parameterization is

`S(u,v) = O + (R + r cos(v))(cos(u)X + sin(u)Y) + r sin(v)Z`.

For `R > r > 0`, the first fundamental form is

- `E = (R + r cos(v))^2`;
- `F = 0`;
- `G = r^2`;

and the area density is

`J = r (R + r cos(v)) > 0`.

References:

- https://mathworld.wolfram.com/Torus.html
- https://www.maths.gla.ac.uk/~mpowell/M435-chapter-4-2nd-FF-curvature.pdf

With the outward normal induced by the AP Mesh `S_u x S_v` convention,
principal curvatures are analytically

- `k_u = -cos(v)/(R + r cos(v))`;
- `k_v = -1/r`.

Therefore

`K = cos(v) / [r (R + r cos(v))]`

and

`H = -(R + 2 r cos(v)) / [2 r (R + r cos(v))]`.

Decision impact:

- one regular production family supplies positive Gaussian curvature near the
  outer equator;
- exact/analytic zero Gaussian curvature is available at represented
  `v=+/-pi/2`;
- negative Gaussian curvature occurs on the inner side near `v=pi`;
- the two principal curvatures are distinct for every regular ring-torus point
  because equality would require `R=0`;
- the torus therefore provides a stronger future fixture for principal
  directions and curvature-field regression than sphere alone.

### 4.3 Cone adds a different singularity rather than the missing regular
sign-changing fixture

Open CASCADE `Geom_ConicalSurface`:

https://dev.opencascade.org/doc/refman/html/class_geom___conical_surface.html

Relevant evidence:

- cone representation introduces reference radius plus semi-angle;
- the natural V direction is unbounded;
- an apex introduces family-specific singular behavior;
- U remains angular/periodic.

Decision impact:

- the project already has explicit singular-parameterization evidence at
  canonical sphere poles;
- cone remains important Surface Representation breadth, but it does not supply
  the same regular positive/zero/negative-K fixture as a ring torus;
- cone remains a later explicit decision.

### 4.4 Principal directions remain a separate line-field problem

Patrikalakis, Maekawa and Cho, lines of curvature:

https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node186.html

Relevant evidence:

- principal directions correspond to extremal normal-curvature directions;
- at non-umbilic points they define orthogonal directions;
- at umbilics a unique principal-direction pair is not available;
- a direction is naturally sign-ambiguous, so field/line continuity is a
  separate semantic question.

Decision impact:

- scalar principal values are already integrated;
- sphere is intentionally an all-umbilic fixture and therefore cannot by
  itself exercise non-umbilic direction fields;
- a ring torus supplies regular non-umbilic points across changing Gaussian
  curvature before the project chooses a direction/line-field API.

### 4.5 General trimming and qualification remain broader stage decisions

General CAD face trimming requires p-curves, loop orientation and explicit
topological identity in addition to supporting-surface evaluation.

Reference family:

https://dev.opencascade.org/doc/refman/html/class_b_rep_builder_a_p_i___make_face.html

Decision impact:

- general trimming is a cross-layer geometry/topology seam, not a small
  elementary-surface value type;
- Surface Representation qualification remains premature while admitted
  elementary/general-trimming obligations are still unresolved;
- Boundary Curve Discretization remains blocked by the incomplete general
  face/boundary seam.

External sources are scientific/design evidence only. No third-party runtime
dependency or numerical oracle is admitted.

## 5. Candidate comparison

| Candidate | New semantics | Current prerequisite state | Incremental scientific value | Decision |
| --- | --- | --- | --- | --- |
| Bounded analytic ring torus | two radii + two bounded angular directions | placement, surface partials and differential stack already integrated | **Very high**: regular K>0/K=0/K<0 fixture and non-umbilic principal values | **SELECTED** |
| Bounded analytic cone | radius + semi-angle + apex behavior | placement/differential stack available | high, but mainly another singularity policy | DEFER |
| Principal directions | eigenvectors + sign/line-field + umbilic semantics | scalar principal values exist; fixture diversity still weak | high after torus | DEFER |
| Conditioning diagnostics | conditioning representation and threshold/policy semantics | metric/curvature stack exists | high, but policy is independent and not yet frozen | DEFER |
| General trim/p-curves/faces | p-curves + loops + topology binding | only rectangular static trim exists | very high but cross-layer and larger | DEFER |
| Surface Representation qualification | stage-level claim/campaign | cone/general trim/full periodic scope still unresolved | premature | DEFER |
| Surface Differential Geometry qualification | stage-level claim/campaign | principal directions/conditioning remain unresolved | premature | DEFER |
| Boundary Curve Discretization | canonical shared physical trace | general boundary/face seam incomplete | downstream-critical | BLOCKED |

## 6. Decision

Authorize exactly one future implementation work unit:

**Bounded Analytic Ring-Torus Surface Sector in 3D.**

Proposed public type:

`BoundedTorusSurface3`.

This work unit belongs to **Surface Representation**.

It also supplies independent analytic evidence for the already open but
unqualified Surface Differential Geometry stage.

It does not qualify either stage.

## 7. Bounded representation scope

Stored state:

- existing `AxisPlacement3 placement`;
- finite major radius `R`;
- finite minor radius `r`;
- finite strict U domain;
- finite strict V domain;
- U-reversal state;
- V-reversal state.

Construction requires exactly:

- `R > 0`;
- `r > 0`;
- `R > r`.

Therefore the admitted family is a **ring torus only**.

Explicitly excluded:

- horn torus `R == r`;
- spindle/self-intersecting torus `R < r`;
- zero/negative radii.

No tolerance defines the ring-torus inequality.

## 8. Bounded angular domains

Both U and V are angular parameters in radians.

Each direction uses an already-valid finite strict `CurveParameterDomain`.

Additional requirement in each direction:

- represented width is strictly less than represented `2*pi`;
- full or multiple revolution is rejected;
- no modulo normalization;
- no periodic wrapping;
- no seam equivalence is claimed.

The domain endpoints may be any finite values satisfying that width rule.

A future full-periodic seam decision remains separate.

## 9. Construction-error vocabulary

The future type may add a torus-specific construction-error enum with at least:

- `non_finite_major_radius`;
- `non_positive_major_radius`;
- `non_finite_minor_radius`;
- `non_positive_minor_radius`;
- `major_radius_not_greater_than_minor_radius`;
- `full_or_multiple_u_revolution_not_admitted`;
- `full_or_multiple_v_revolution_not_admitted`.

Existing `SurfaceError` query semantics remain unchanged.

No common error is added speculatively.

## 10. Mathematical representation

For a right-handed placement `(O,X,Y,Z)`:

`e_r(u) = cos(u) X + sin(u) Y`.

The represented torus is

`S(u,v) = O + (R + r cos(v)) e_r(u) + r sin(v) Z`.

Required analytic partials:

`S_u = (R + r cos(v))[-sin(u)X + cos(u)Y]`;

`S_v = -r sin(v)e_r(u) + r cos(v)Z`;

`S_uu = -(R + r cos(v))e_r(u)`;

`S_uv = r sin(v)[sin(u)X - cos(u)Y]`;

`S_vv = -r cos(v)e_r(u) - r sin(v)Z`.

Finite-difference derivatives are forbidden.

## 11. Regularity of the admitted ring family

For `R > r > 0`:

`R + r cos(v) >= R-r > 0`.

Therefore:

- `|S_u| = R + r cos(v) > 0`;
- `|S_v| = r > 0`;
- `S_u · S_v = 0`;
- `|S_u x S_v| = r(R+r cos(v)) > 0`.

The admitted torus sector is regular everywhere in its parameter domain.

The representation constructor does not need an epsilon-based regularity test.

## 12. Canonical represented angular states

For focused deterministic evidence, production may recognize literal represented
canonical angles without modulo equivalence:

- `0`;
- `+/-pi/2`;
- `+/-pi`.

At those exact represented values, the implementation may enforce exact
`sin/cos` states to avoid replacing a mathematically exact zero by a tiny
library-rounding residual.

This is a local elementary-surface numerical policy, not a periodic wrapping
rule.

No tolerance or nearest-angle snapping is permitted.

## 13. Query and finite-result semantics

The existing bounded-surface query-validation order is preserved.

For valid U/V parameters:

- finite representable value/partial -> success;
- unrepresentable final value/partial -> existing
  `SurfaceError::non_finite_result`.

No NaN/inf sanitization or silent coordinate rescaling changes the scientific
result.

No query failure is introduced merely because curvature is positive, zero or
negative.

## 14. U/V reversal

The existing elementary-surface reversal model should be preserved.

U reversal:

- reflects U through the existing bounded-domain reversal primitive;
- preserves represented locus;
- flips sign of `S_u`;
- preserves `S_v`;
- preserves `S_uu` and `S_vv`;
- flips sign of `S_uv`.

V reversal is analogous with U/V roles exchanged.

Double reversal in either direction must recover the exact stored
representation.

Combined U+V reversal must preserve the orientation induced by
`S_u x S_v`; a single reversal flips it.

No periodic wrap participates in reversal.

## 15. Independent torus oracle

Focused tests must include an independent long-double analytic oracle that does
not reuse production torus helpers.

At minimum compare:

- value;
- `S_u`;
- `S_v`;
- `S_uu`;
- `S_uv`;
- `S_vv`;

for:

- identity placement;
- genuinely arbitrary right-handed `AxisPlacement3`;
- multiple interior U/V pairs;
- both sides of the torus;
- represented canonical V values where admitted.

## 16. Cross-layer Surface Differential Geometry evidence

The generic differential-production code must remain unchanged.

For selected regular points, focused tests must verify the generic layer
against analytic torus values.

With the outward `S_u x S_v` orientation:

- `E=(R+r cos(v))^2`;
- `F=0`;
- `G=r^2`;
- `J=r(R+r cos(v))`;
- `k_u=-cos(v)/(R+r cos(v))`;
- `k_v=-1/r`;
- `K=cos(v)/[r(R+r cos(v))]`;
- `H=-(R+2r cos(v))/[2r(R+r cos(v))]`.

The generic ordered-principal-value API should match the sorted analytic pair.

The torus must be classified non-umbilic at all tested regular points.

No principal-direction API is introduced by this work unit.

## 17. Required curvature-sign fixtures

At minimum include:

### Outer equator

Represented `v=0`:

- `K > 0`;
- both principal curvatures negative under the outward-normal convention;
- not umbilic.

### Parabolic circle

Represented `v=+pi/2` or `-pi/2`:

- analytic `K=0`;
- one principal curvature zero;
- the other is `-1/r`;
- parameterization remains regular;
- not umbilic.

### Inner equator

Represented `v=pi` or `-pi`:

- `K < 0`;
- principal curvatures have opposite sign;
- parameterization remains regular;
- not umbilic.

This sign-transition evidence is a primary reason for selecting torus now.

## 18. Affine and scale evidence

Focused tests must include:

- translation covariance;
- arbitrary admitted placement;
- exact power-of-two common coordinate/radius scaling where representable.

For scale `lambda>0`:

- value and first/second geometric derivatives scale by `lambda`;
- metric coefficients and area density scale according to the generic
  differential contract;
- `K` scales by `lambda^-2`;
- `H` and principal curvatures scale by `lambda^-1`;
- Gaussian-curvature sign and non-umbilic state are preserved.

## 19. Extreme finite evidence

At least one fixture must use extreme but finite placement/radius scales.

Required outcome:

- succeed when final value/partials are representable and the established
  scale-aware arithmetic can preserve them;
- otherwise return explicit `SurfaceError::non_finite_result`.

No universal epsilon, hidden fallback or radius renormalization is introduced.

## 20. Determinism

Repeated identical calls must produce identical:

- stored representation;
- value/partial results;
- typed construction failures;
- typed query failures;
- cross-layer differential values where exact representation permits.

No mutable global trigonometric or cache state is permitted.

## 21. Focused evidence matrix

| Case | Required observation |
| --- | --- |
| Concept satisfaction | Torus satisfies existing `BoundedParametricSurface3`. |
| Construction | finite `R>r>0`; invalid radii and full/multiple angular widths rejected. |
| Stored state | placement/radii/domains/reversal flags preserved exactly. |
| Query order | existing U/V `SurfaceError` ordering unchanged. |
| Independent oracle | value and all first/second partials match long-double torus oracle. |
| Ring regularity | metric area density positive across tested sectors. |
| Outer equator | positive K, negative principal values, non-umbilic. |
| Parabolic circle | analytic K=0, one zero principal value, regular. |
| Inner equator | negative K, opposite-sign principal values, regular. |
| Generic differential layer | E/F/G/J/normal/K/H/principal/umbilic match analytic oracle. |
| U reversal | value/partial covariance and involution. |
| V reversal | value/partial covariance and involution. |
| Combined reversal | orientation relation preserved as declared. |
| Placement | arbitrary right-handed placement evidence. |
| Translation/scale | declared covariance and curvature scaling. |
| Extreme finite | explicit success/failure without silent fallback. |
| Determinism | repeated results/failures identical. |
| Header isolation | no topology/trimming/discretization/meshing/external dependency. |
| Regression | existing 40 ordinary tests remain PASS. |

If exactly one new focused contract is added, ordinary FAST/INTEGRATION
inventory becomes **41 tests**.

## 22. Why torus is selected before cone

Sphere already supplied an exact parameterization singularity at its canonical
poles.

A cone would add important apex semantics, but the next missing analytic
regression phenomenon is a **regular** surface whose Gaussian curvature changes
sign.

The ring torus supplies:

- no singular parameter inside the admitted family;
- positive/zero/negative Gaussian curvature;
- distinct principal curvature values;
- no umbilic ambiguity;
- two angular directions.

This is more immediately useful to the integrated differential stack and to a
later principal-direction decision.

Cone remains an explicit retained Surface Representation obligation.

## 23. Why principal directions are deferred

Principal curvature **values** already exist.

Principal directions additionally require:

- tangent-space eigenvector representation;
- sign ambiguity;
- line-field equivalence;
- reversal covariance;
- undefined/non-unique semantics at umbilics;
- potentially continuity/transport policy across samples.

Sphere is all-umbilic and therefore cannot validate the non-umbilic path.

The ring torus supplies a strong non-umbilic analytic fixture before that API is
designed.

## 24. Why conditioning diagnostics are deferred

Conditioning diagnostics require a deliberate representation and policy:

- what quantity is reported;
- scale behavior;
- whether thresholds are advisory or scientific acceptance criteria;
- how near-degenerate but nonzero parameterizations are classified.

The current work unit adds a regular exact analytic family without introducing
a hidden threshold policy.

## 25. Why general trimming remains deferred

General trimming requires more than continuous surface evaluation:

- parameter-space p-curves;
- physical edge realization;
- loop orientation;
- explicit topology identity;
- potentially shared-boundary compatibility.

This is a larger cross-layer decision and remains necessary before CAD-like
general face coverage or downstream shared-boundary certification.

## 26. Why qualification and Boundary Curve Discretization remain premature

Surface Representation still retains at least:

- cone;
- full-periodic seams where admitted;
- general trim/p-curve/topological-face policy.

Surface Differential Geometry still retains:

- principal directions/line fields;
- explicit conditioning diagnostics.

Boundary Curve Discretization still requires a declared general boundary/face
seam and broader retained curve obligations.

Therefore no stage-level qualification or discretization entry is authorized
by this decision.

## 27. Repository mapping for future implementation

Authorized future mapping is limited to:

- elementary-surface public API:
  `include/apmesh/geometry/elementary_surface.hpp`;
- existing elementary-surface production:
  `src/geometry/elementary_surface.cpp`;
- focused semantic/cross-layer contract:
  `tests/surface_torus.cpp`;
- build/test registration:
  `CMakeLists.txt`;
- synchronized authorities:
  `docs/APMESH_CORE_STATE.md`,
  `docs/APMESH_CORE_ROADMAP.md`,
  `docs/APMESH_CORE_WORKLOG.md`,
  this decision;
- candidate/implementation audit after validation evidence exists.

No common `SurfaceError`, `BoundedParametricSurface3`,
`AxisPlacement3` or Surface Differential Geometry production change is
expected.

If such a common-contract change becomes necessary, stop and require a new
scientific decision.

## 28. Validation boundary

Before implementation integration:

- GCC 13 Debug / FAST must pass;
- GCC 13 Debug / INTEGRATION must pass;
- Clang 18 libc++ Debug / INTEGRATION must pass;
- all existing 40 ordinary semantic tests must remain passing;
- the new torus focused contract must pass.

Passing yields only:

**BOUNDED ANALYTIC TORUS IMPLEMENTED / FOCUSED CONTRACTS PASS /
INTEGRATED / SURFACE REPRESENTATION NOT QUALIFIED /
SURFACE DIFFERENTIAL GEOMETRY NOT QUALIFIED.**

No stage qualification is implied.

## 29. Stop conditions

Stop and require a new decision if implementation needs:

- `R<=r` horn/spindle torus admission;
- full-periodic U or V seam semantics;
- modulo angle normalization;
- a common `SurfaceError` change;
- a `BoundedParametricSurface3` change;
- an `AxisPlacement3` change;
- principal directions;
- conditioning thresholds;
- analytic cone;
- general trimming/p-curves/topological faces;
- Boundary Curve Discretization;
- Physical Sizing;
- meshing;
- a third-party runtime dependency.

## 30. Planned sequence after torus

Planning only, not authorization.

After torus implementation and closure, a fresh decision should recompare:

1. analytic cone;
2. principal directions / line-field semantics;
3. conditioning diagnostics;
4. general trimming / p-curves / topological faces;
5. full-periodic elementary-surface seams;
6. Surface Representation qualification readiness;
7. Surface Differential Geometry qualification readiness;
8. Boundary Curve Discretization readiness.

No option is pre-authorized.

## 31. Effect if integrated and closed

After this decision is integrated, protected-main validation passes and a
separate decision checkpoint closes, the sole next production work item is:

**Bounded Analytic Ring-Torus Surface Sector in 3D.**

No implementation begins on this decision branch.

Surface Representation remains IN INVESTIGATION / NOT QUALIFIED.

Surface Differential Geometry remains IN INVESTIGATION / NOT QUALIFIED.

Boundary Curve Discretization and downstream meshing remain blocked.
