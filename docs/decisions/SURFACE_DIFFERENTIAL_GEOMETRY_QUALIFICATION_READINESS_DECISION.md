# Surface Differential Geometry Qualification Readiness — Scientific Decision

Status: DECISION CLOSED / QUALIFICATION PROTOCOL PRE-REGISTRATION AUTHORIZED /
NOT QUALIFIED  
Date: 2026-09-27  
Stage: Surface Differential Geometry — Metric, Normals, and Curvatures

## 1. Question

After terminal closure of Pointwise Surface Metric Conditioning Diagnostics,
which single bounded scientific work item should AP Mesh admit next?

Required comparison:

1. principal directions / curvature-line semantics;
2. bounded analytic cone;
3. bounded analytic torus;
4. general trimming / p-curves / topological faces;
5. Surface Representation qualification readiness;
6. Surface Differential Geometry qualification readiness;
7. Boundary Curve Discretization readiness.

The decision must select at most one work item, preserve the 42-test ordinary
semantic baseline, and must not silently authorize a formal qualification
campaign.

## 2. Fresh repository authority

Decision-entry protected `main`:

`49f465e942b032f2a19592b6f2c5737b08d698cd`.

Decision-entry validation:

- FAST #617: PASS;
- INTEGRATION #608 / GCC 13 Debug: PASS;
- INTEGRATION #608 / Clang 18/libc++ Debug: PASS;
- ordinary semantic registration inventory: **42 tests**;
- open PRs: none;
- active production/scientific work item: none.

Active decision branch:

`surface/differential-geometry-qualification-readiness-decision`.

The repository state, not conversation history, is the authority for this
decision.

## 3. Current integrated Surface Differential Geometry capability

The public differential layer now contains:

- first fundamental form `E,F,G`;
- area density;
- oriented unit normal;
- pointwise parameterization condition number;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state;
- typed propagation of parameter/domain/continuity/singularity/
  non-representable failures.

The generic bounded-surface overloads reuse the existing
`BoundedParametricSurface3` derivative contract.

Current ordinary focused evidence includes:

- plane;
- circular cylinder sector;
- bounded sphere sector and exact pole singularity;
- elliptic synthetic second-order geometry;
- hyperbolic-paraboloid/saddle synthetic geometry;
- exact and near-umbilic principal-value cases;
- exactly singular and near-singular regular parameterizations;
- extreme finite scale separation;
- translation, frame, reversal and scale laws;
- conformance over every currently admitted bounded surface family.

No Surface Differential Geometry qualification tooling or retained formal
qualification package exists yet.

## 4. Scientific and mature-kernel evidence

External references are design/scientific evidence only. They introduce no
runtime dependency and are not accepted as numerical oracles.

### 4.1 The scalar/local differential stack is already mathematically coherent

Patrikalakis, Maekawa and Cho describe surface local differential properties
from first and second fundamental forms, including Gaussian/mean curvature and
principal curvature quantities:

- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node31.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node26.html

Open CASCADE exposes normal, Gaussian/mean curvature, min/max principal
curvature, umbilic state and curvature directions as distinct local properties:

- https://dev.opencascade.org/doc/refman/html/class_geom_l_prop___s_l_props.html

Decision impact:

- AP Mesh already implements the scalar/local quantities required for the
  declared isotropic geometric baseline;
- a cumulative protocol can now test those quantities together rather than add
  another production formula first;
- qualifying this differential layer must not imply qualification of every
  surface representation supplying derivatives.

### 4.2 Principal directions are not required for the isotropic baseline

Patrikalakis, Maekawa and Cho show that curvature-line integration needs
direction-sign/orientation handling and special treatment around umbilic or
indeterminate cases:

- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node186.html

Laug, *Some aspects of parametric surface meshing*,
doi:10.1016/j.finel.2009.06.015, distinguishes:

- isotropic sizing from the strongest principal curvature; and
- anisotropic sizing, which additionally needs principal directions.

Reference:

- https://www.sciencedirect.com/science/article/pii/S0168874X09000936

Decision impact:

- principal directions remain scientifically important;
- they are not a missing prerequisite for the serial isotropic baseline;
- adding them now would open eigenvector sign/non-uniqueness and line-field
  semantics that belong naturally with future anisotropy.

### 4.3 Cone and torus are independent representation breadth

Open CASCADE conical surfaces use an axis placement, reference radius,
semi-angle, an angular U coordinate, a naturally unbounded V coordinate and an
apex-specific seam:

- https://dev.opencascade.org/doc/refman/html/class_geom___conical_surface.html
- https://dev.opencascade.org/doc/refman/html/class_g_c___make_conical_surface.html

Open CASCADE toroidal surfaces use major/minor radii and two periodic angular
directions:

- https://dev.opencascade.org/doc/refman/html/class_geom___toroidal_surface.html

Decision impact:

- cone and torus remain important Surface Representation obligations;
- neither is required to define or cumulatively validate the already-integrated
  generic differential operators;
- adding either now would widen representation semantics instead of closing the
  current differential stage.

### 4.4 General trimming is a cross-layer geometry/topology seam

Mature B-rep models bind a 2D p-curve to a supporting surface and use ordered
wire/edge orientation in a face parameter space:

- https://dev.opencascade.org/doc/refman/html/class_shape_persistent___b_rep_1_1_curve_on_surface.html
- https://dev.opencascade.org/doc/occt-7.6.0/refman/html/class_b_rep_tools___wire_explorer.html

Decision impact:

- AP Mesh currently has rectangular parametric subdomain trimming and a
  separate qualified topological identity/incidence layer;
- it does not yet have the p-curve/physical-edge/face binding needed for
  general trimmed faces;
- that seam must remain an explicit later cross-layer decision.

## 5. Repository-specific readiness audit

### 5.1 Surface Differential Geometry qualification blockers that are now closed

The earlier metric-conditioning decision explicitly identified missing
pointwise conditioning as the direct blocker for Surface Differential Geometry
qualification readiness.

That capability is now:

**IMPLEMENTED / FOCUSED CONTRACTS PASS / INTEGRATED / CLOSED /
NOT QUALIFIED**.

Its terminal chain is recorded through:

- implementation PR #225;
- implementation closure PR #226;
- terminal synchronization PR #227;
- terminal receipt PR #228;
- protected-main FAST #617 / INTEGRATION #608.

### 5.2 Existing analytic/synthetic fixture coverage

Current ordinary contracts already include the mathematical classes named by
the roadmap for cumulative differential regression:

- plane: exact zero-curvature regular baseline;
- cylinder: one zero and one non-zero principal curvature;
- sphere: non-zero exact umbilic curvature plus pole singularity;
- elliptic synthetic second-order geometry;
- hyperbolic-paraboloid/saddle synthetic second-order geometry;
- near-singular regular parameterization;
- exact singular parameterization;
- near-umbilic and exact-umbilic represented-data semantics;
- extreme finite conditioning.

Qualification must strengthen these into independent, retained cumulative
evidence; it must not merely count the ordinary tests.

### 5.3 Representation-stage separation

Surface Representation remains:

**IN INVESTIGATION / NOT QUALIFIED**.

This does not prevent a bounded qualification protocol for the generic
differential layer because the Surface Differential Geometry entry decision
explicitly opened this stage while representation breadth remained under
investigation.

The qualification claim must therefore be phrased narrowly:

- qualify the declared local differential operators over the admitted finite
  derivative/regularity contract and enumerated fixtures;
- treat concrete surface-family executions as conformance/prerequisite
  preservation evidence;
- do **not** claim that Surface Representation as a whole is qualified.

If a future protocol cannot maintain that separation, qualification is blocked
and Surface Representation must be addressed first.

## 6. Candidate comparison

| Candidate | Immediate new burden | Current prerequisite state | Direct doctoral-pipeline value | Decision |
| --- | --- | --- | --- | --- |
| Principal directions / curvature lines | eigenvectors, sign/orientation continuity, umbilic non-uniqueness, line-field semantics | scalar principal values/umbilic state exist | high for future anisotropy/quad orientation, not required for isotropic baseline | DEFER |
| Bounded analytic cone | semi-angle/reference-radius/apex/domain policy | placement exists; representation policy still open | useful analytic breadth/oracle | DEFER |
| Bounded analytic torus | two radii, two periodic directions, seam policy | placement exists; periodic policy still open | useful analytic breadth/oracle | DEFER |
| General trimming / p-curves / topological faces | p-curves, loops, physical-edge/surface binding, orientation | rectangular trim + topology exist separately, binding absent | critical for CAD-face/shared-boundary work | DEFER |
| Surface Representation qualification readiness | cumulative breadth/admissible-input claim | cone/torus/general-trim policy remains open | high, but current representation envelope is still intentionally incomplete | DEFER |
| **Surface Differential Geometry qualification readiness** | cumulative protocol, independent evidence, figures, cross-cell/repetition policy | **local differential stack and conditioning are now complete for the isotropic baseline** | **closes the current mathematical stage before sizing/discretization** | **SELECTED** |
| Boundary Curve Discretization readiness | physical error, shared trace, sizing, orientation/face ownership | curve geometry qualified but surface/face/sizing seams remain open | critical downstream | BLOCKED / DEFER |

## 7. Decision

Select exactly one future bounded work item:

**Surface Differential Geometry — Cumulative Qualification Protocol
Pre-registration.**

This is a **documentation/governance scientific work unit**.

It must define the qualification claim, fixture matrix, gates, cross-cell
equivalence, retained evidence, figures, failure policy and prerequisite
preservation before any qualification tooling or formal execution is
authorized.

This decision does **not** qualify the stage.

## 8. Proposed qualification claim boundary

A future protocol may attempt to qualify only:

**Deterministic pointwise local differential geometry for regular bounded
parametric surfaces in 3D within an explicitly admitted compiler/environment
envelope.**

The protocol must cover the already-integrated public quantities:

- first fundamental form;
- area density;
- oriented unit normal;
- metric condition number;
- second fundamental form;
- Gaussian curvature;
- mean curvature;
- ordered principal curvature values;
- exact represented-data umbilic state;
- typed exact-singular and finite-representability behavior.

It must not claim:

- Surface Representation qualification;
- principal directions or curvature-line continuity;
- near-umbilic classification thresholds;
- acceptable conditioning thresholds for meshing;
- p-curves/topological faces;
- boundary discretization;
- physical sizing;
- mesh generation;
- anisotropic/tensor metrics;
- Quad-Dominant behavior;
- parallel equivalence;
- native-Windows qualification.

## 9. Required independent fixture families for the protocol

At minimum, the future protocol must pre-register independent expectations for:

1. exact plane baseline;
2. circular cylinder with radius variants;
3. sphere at equator/interior latitude and exact pole singularity;
4. elliptic paraboloid local derivative fixture;
5. hyperbolic paraboloid/saddle local derivative fixture;
6. exact singular parameterization;
7. regular near-singular parameterization;
8. exact umbilic and near-umbilic represented-data cases;
9. non-orthogonal metric;
10. extreme finite scale separation;
11. all admitted translation/frame/reversal/scale laws;
12. integrated-family conformance across every current bounded surface family.

Where a production representation is unavailable, a synthetic derivative-level
fixture is acceptable only when its expected values are derived independently
from the analytic surface formula and its limitation is explicit.

## 10. Required invariance and covariance laws

The future protocol must pre-register at least:

- translation invariance;
- rigid/signed-frame covariance;
- U reversal;
- V reversal;
- both-direction reversal;
- parameter-axis interchange where applicable;
- uniform positive spatial scaling;
- dimensionless invariance of metric conditioning;
- inverse-length scaling of mean/principal curvature;
- inverse-square scaling of Gaussian curvature;
- orientation-sign behavior for unit normal, second form, mean and principal
  curvatures;
- orientation invariance of first form, area density, Gaussian curvature and
  metric condition number.

No expectation may be adjusted after formal evidence is observed.

## 11. Qualification-gate shape to be pre-registered

The protocol should use descriptive gates equivalent to:

- **SDG0 — Scope and candidate identity**
- **SDG1 — Parameter/domain/error and exact singularity semantics**
- **SDG2 — First-order metric, area, normal and conditioning**
- **SDG3 — Second fundamental form, Gaussian and mean curvature**
- **SDG4 — Principal values, identities and umbilic semantics**
- **SDG5 — Invariance, orientation, scale and near-degenerate behavior**
- **SDG6 — Surface-family conformance and prerequisite preservation**
- **SDG7 — Repeatability, cross-cell equivalence, figures and evidence integrity**

The protocol may refine names/content before integration, but it must not omit
a distinct risk class merely to reduce gate count.

## 12. Qualification execution envelope to be decided by the protocol

Consistent with existing qualified scientific work, the protocol should
evaluate whether the formal matrix requires:

- GCC 13 / libstdc++ Debug;
- GCC 13 / libstdc++ Release;
- Clang 18 / libc++ Debug;
- Clang 18 / libc++ Release;
- repeated independent executions;
- clean-build source/command/dependency inventories;
- detached-candidate verification.

This decision does not itself freeze that matrix. The protocol work item must
justify and pre-register the final execution envelope.

## 13. Required scientific figures

The repository workflow requires figures when spatial/numerical structure is
part of the claim.

The protocol must therefore define reproducible figures that answer declared
questions, for example:

- Gaussian/mean/principal-curvature fields on analytic fixtures;
- condition-number field on a deliberately distorted parameterization;
- singular/near-singular parameter-domain map;
- scale/orientation comparison where a field visualization is scientifically
  informative.

Figures must be generated from retained machine-readable evidence; manual
scientific-content editing is forbidden.

## 14. Formal-execution boundary

This decision explicitly does **not** authorize:

- a prepared qualification manifest;
- a one-time execution token;
- dispatch of a formal qualification workflow;
- adaptation of expectations after observing a formal run;
- a `QUALIFIED` status.

The intended sequence is:

1. integrate and close this readiness decision;
2. pre-register the cumulative qualification protocol;
3. separately validate/integrate/close that protocol;
4. decide and implement report-only qualification tooling;
5. audit the tooling;
6. separately design preparation/fail-closed execution mechanics;
7. prepare one immutable candidate only after explicit authorization semantics
   are in place;
8. authorize formal execution separately;
9. execute once;
10. audit retained evidence before any qualification status change.

Each transition remains separately reviewable and reversible until formal
execution.

## 15. Stop conditions

Stop and require a new decision if protocol design reveals that qualification
requires any of:

- principal-direction or line-field semantics;
- cone/torus production support;
- general p-curves/topological faces;
- a new surface representation;
- a universal geometric epsilon;
- a meshing-quality or conditioning-acceptance threshold;
- third derivatives;
- arbitrary precision or third-party runtime numerical dependency;
- boundary discretization/sizing/meshing;
- weakening any current typed error/singularity semantics;
- claiming Surface Representation qualification implicitly.

A contradiction in a prerequisite reopens that prerequisite instead of being
absorbed into Surface Differential Geometry qualification.

## 16. Work-class plan

Current decision branch:

- Implementation: 0%;
- Tests/validation: 10%;
- Evidence/experiments: 20%;
- Documentation/governance: 70%.

Selected future protocol work item:

- Implementation: 0%;
- Tests/validation: 15%;
- Evidence/experiments: 35%;
- Documentation/governance: 50%.

These percentages are process accounting only.

## 17. Effect if this decision is integrated and separately closed

After this decision receives required FAST/INTEGRATION checks, merges,
receives protected-main validation, and its decision closure is separately
integrated and post-merge validated, the sole next work item becomes:

**Surface Differential Geometry — Cumulative Qualification Protocol
Pre-registration.**

Until that closure:

- no qualification protocol work begins;
- no qualification tooling is authorized;
- no formal qualification execution is authorized;
- no production capability is active;
- Surface Representation remains IN INVESTIGATION / NOT QUALIFIED;
- Surface Differential Geometry remains IN INVESTIGATION / NOT QUALIFIED;
- Boundary Curve Discretization remains NOT STARTED.

No later qualification tooling, preparation, formal execution, principal
direction, representation breadth or discretization work is pre-authorized.


## 18. Decision integration checkpoint

Decision PR #229 used final head:

`d45674b13bd91ffd4ffa790e1b91eac1cb77a8ef`.

Required PR validation:

- FAST #618: PASS;
- INTEGRATION #609 / GCC 13 Debug: PASS;
- INTEGRATION #609 / Clang 18/libc++ Debug: PASS;
- no reviews or unresolved review threads;
- branch relation at merge gate: ahead=5, behind=0;
- diff restricted to decision/continuity/reference documentation.

PR #229 squash-merged as:

`46f67121a81b8d4f448411d7ebb2ef5bcd1aa02f`.

Protected-main validation on that exact revision:

- FAST #619: PASS;
- INTEGRATION #610 / GCC 13 Debug: PASS;
- INTEGRATION #610 / Clang 18/libc++ Debug: PASS.

Ordinary semantic registration inventory remains **42 tests**.

Decision integration result:

**SURFACE DIFFERENTIAL GEOMETRY QUALIFICATION READINESS SELECTED /
DECISION INTEGRATED / CLOSURE PENDING /
QUALIFICATION PROTOCOL NOT YET AUTHORIZED / NOT QUALIFIED.**

The active closure branch is:

`docs/surface-differential-geometry-qualification-readiness-decision-closure`.

The selected protocol work item becomes authorized only after this separate
decision closure itself receives required checks, merges and passes
protected-main validation.

No qualification tooling, preparation, formal execution, production capability
or `QUALIFIED` claim is authorized by this integration checkpoint.


## 19. Decision closure checkpoint

Decision closure PR #230 used final head:

`6e2e57ffad3f7a208b5398b39c084dad3f92c9de`.

Required closure validation:

- FAST #620: PASS;
- INTEGRATION #611 / GCC 13 Debug: PASS;
- INTEGRATION #611 / Clang 18/libc++ Debug: PASS;
- no reviews or unresolved review threads;
- branch relation at merge gate: ahead=4, behind=0;
- diff restricted to documentation.

PR #230 squash-merged as:

`f8d12c2e0e39611758af1b10d01311913603622f`.

Protected-main validation on that exact closure revision:

- FAST #621: PASS;
- INTEGRATION #612 / GCC 13 Debug: PASS;
- INTEGRATION #612 / Clang 18/libc++ Debug: PASS.

Ordinary semantic registration inventory remains **42 tests**.

Decision checkpoint result:

**DECISION CLOSED / SURFACE DIFFERENTIAL GEOMETRY CUMULATIVE
QUALIFICATION PROTOCOL PRE-REGISTRATION AUTHORIZED / NOT QUALIFIED.**

The sole next scientific work item is:

**Surface Differential Geometry — Cumulative Qualification Protocol
Pre-registration.**

This authorization is limited to protocol documentation/governance. It does not
authorize:

- qualification tooling implementation;
- prepared manifests;
- one-time formal execution;
- workflow dispatch;
- a `QUALIFIED` status;
- principal directions or curvature-line fields;
- new surface representations;
- p-curves/topological faces;
- boundary discretization, sizing or meshing.

The current terminal-sync branch publishes this closed decision state before
protocol work begins.


## 20. Terminal synchronization checkpoint

Terminal synchronization PR #231 used final head:

`9d2876876cb4ea5bffe3e3d88507c7be814e4139`.

Required terminal-sync validation:

- FAST #622: PASS;
- INTEGRATION #613 / GCC 13 Debug: PASS;
- INTEGRATION #613 / Clang 18/libc++ Debug: PASS;
- no reviews or unresolved review threads;
- branch relation at merge gate: ahead=4, behind=0;
- diff restricted to continuity/decision documentation.

PR #231 squash-merged as:

`ce73cf28c7359f1bac8986c4b7ea57180dc7d109`.

Protected-main validation on that exact revision:

- FAST #623: PASS;
- INTEGRATION #614 / GCC 13 Debug: PASS;
- INTEGRATION #614 / Clang 18/libc++ Debug: PASS.

Ordinary semantic registration inventory remains **42 tests**.

Terminal decision result:

**DECISION CLOSED / SURFACE DIFFERENTIAL GEOMETRY CUMULATIVE
QUALIFICATION PROTOCOL PRE-REGISTRATION AUTHORIZED / NOT QUALIFIED /
REMOTE CONTINUITY SYNCHRONIZED.**

No production, scientific or repository-transition work item remains active.

The sole next scientific work item is:

**Surface Differential Geometry — Cumulative Qualification Protocol
Pre-registration.**

That authorization remains limited to protocol documentation/governance. It
does not authorize qualification tooling, manifest preparation, formal
execution, workflow dispatch or a `QUALIFIED` status.
