# Surface Differential Geometry Post-Tooling Continuation — Scientific Decision

Status: **DECISION ACTIVE / DOCUMENTATION ONLY / PREPARATION INFRASTRUCTURE
SELECTED / IMPLEMENTATION NOT AUTHORIZED / FORMAL PREPARATION NOT AUTHORIZED /
NOT QUALIFIED**
Date: 2026-09-28
Stage: **Surface Differential Geometry — Metric, Normals, and Curvatures**

## 1. Question

After integration and closure of the report-only Surface Differential Geometry
qualification tooling and the metric-conditioning portability maintenance,
which single bounded work item should AP Mesh admit next?

The decision must reconcile the planning candidates retained in Section 24 of
`SURFACE_DIFFERENTIAL_GEOMETRY_ENTRY_DECISION.md` with the capabilities now
integrated, preserve the frozen qualification boundary, and advance the
doctoral serial isotropic baseline without silently opening anisotropy,
representation breadth, topology, meshing, or a formal campaign.

This decision is documentation-only. It creates no production code,
qualification infrastructure, PREPARED package, execution authorization,
workflow dispatch, or qualification result.

## 2. Fresh repository authority

Decision-entry protected `main`:

`8dfacb97fd8bbd8f8fa67e5131efee78af17e27e`.

At decision entry:

- local `main` and `origin/main` matched;
- the worktree was clean;
- no pull request was open;
- protected-main FAST and INTEGRATION had passed on the entry revision;
- the ordinary semantic inventory was **42 tests**;
- no production, documentation, preparation, or formal-execution work item
  was active.

Active decision branch:

`decision/surface-differential-post-tooling-next-step`.

The repository and its integrated decision records are authoritative. Earlier
planning lists are historical inputs and do not override later completed work.

## 3. Reconciled integrated capability

The Section 24 planning list predates the following integrated work:

1. **Second fundamental form plus Gaussian/mean curvature** — implemented,
   focused-contract PASS, integrated and closed;
2. **Ordered principal curvature values plus exact represented-data umbilic
   state** — implemented, focused-contract PASS, integrated and closed;
3. **Pointwise metric conditioning** — implemented, focused-contract PASS,
   integrated and closed, including the later normalization-portability
   maintenance;
4. **Bounded analytic sphere** — implemented, focused-contract PASS,
   integrated and closed;
5. **Cumulative qualification protocol** — preregistered, integrated and
   closed;
6. **Report-only qualification tooling** — implemented, focused validation
   PASS, integrated and closed, with all 32 frozen semantic blobs unchanged.

Therefore second-order scalar curvature, principal values, metric
conditioning and the sphere are no longer candidate work items.

The unresolved candidates are:

- principal directions / curvature-line fields;
- bounded analytic cone;
- bounded analytic torus;
- general trimming / p-curves / topological faces.

The qualification lifecycle also has one explicit unfinished prerequisite:

- fail-closed preparation infrastructure, required by Section 18 of
  `SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md` before any formal
  PREPARED package may be created.

## 4. Scientific and engineering evidence

External sources are scientific or mature-kernel design evidence only. They
introduce no runtime dependency, numerical oracle, tolerance, or acceptance
threshold.

### 4.1 Principal directions belong with later directional behavior

Patrikalakis, Maekawa and Cho derive principal directions from the first and
second fundamental forms. They also show that:

- the directions are undefined at an umbilic;
- pointwise directions are sign-ambiguous;
- curvature-line integration needs orientation continuity and equation
  switching even away from umbilics;
- specialized treatment is needed near umbilics.

References:

- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node30.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node183.html
- https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node186.html

Laug distinguishes isotropic surface sizing, which can use the strongest
principal-curvature magnitude, from anisotropic sizing, which additionally
uses directional information:

- Patrick Laug, *Some aspects of parametric surface meshing*,
  doi:10.1016/j.finel.2009.06.015.

The AP Mesh roadmap deliberately places Tensor/Anisotropic Extension after the
certified serial isotropic baseline. Principal directions are scientifically
valuable, but they are not a missing prerequisite for the current scalar
isotropic sizing path. Adding them now would amend the frozen differential API,
production implementation, focused tests and qualification profile.

### 4.2 Cone and torus remain Surface Representation breadth

Open CASCADE's conical model requires placement, reference radius and
semi-angle and exposes an apex-specific singular seam:

- https://dev.opencascade.org/doc/refman/html/class_geom___conical_surface.html
- https://dev.opencascade.org/doc/refman/html/classgce___make_cone.html

Its toroidal model requires major/minor radii and is periodic in both parameter
directions:

- https://dev.opencascade.org/doc/occt-7.9.0/refman/html/class_geom___toroidal_surface.html

Both families would provide useful later analytic and representation evidence,
but each introduces new production representation semantics. Neither is
required to prepare evidence for the already-integrated generic differential
operators. Either choice would require a prior amendment of the frozen
qualification boundary if admitted into the current campaign.

### 4.3 General trimming is a cross-layer geometry/topology seam

Open CASCADE models a p-curve as a 2D curve associated with a supporting
surface and carries face/wire/edge orientation through explicit B-rep objects:

- https://dev.opencascade.org/doc/refman/html/class_shape_persistent___b_rep_1_1_curve_on_surface.html
- https://dev.opencascade.org/doc/occt-7.6.0/refman/html/class_b_rep_tools___wire_explorer.html

General trimming therefore requires parameter-space curves, physical
realization, loop orientation, curve-on-surface binding and topological face
identity. It is important for Shared Boundary Certification and CAD input, but
it is a larger cross-stage architecture decision rather than a remaining local
Surface Differential Geometry operation.

### 4.4 The current qualification boundary is already complete and frozen

The integrated qualification protocol already preregisters the bounded claim,
42-test allowlist, 32 frozen semantic files, four compiler/build cells, two
repetitions per cell, independent analytic/adversarial fixtures, SDG0–SDG7
gates, deterministic figures and terminal evidence requirements.

The report-only tooling implements the exporter, independent validator,
comparers, negative checks, deterministic figures and non-terminal reports
without changing the frozen semantics.

Section 18 of the protocol deliberately orders the next lifecycle transition:

1. report-only tooling integrated and closed — **complete**;
2. independent tooling audit — **complete**;
3. decide and implement fail-closed preparation infrastructure — **pending**;
4. validate, integrate and close preparation infrastructure — **pending**;
5. only later, under separate authorization, prepare one immutable candidate.

This sequence closes a current prerequisite without expanding the scientific
claim.

## 5. Candidate comparison

| Candidate | Current gap | Immediate doctoral value | Boundary cost | Decision |
| --- | --- | --- | --- | --- |
| Principal directions / curvature lines | line-field sign, continuity and umbilic semantics | Low for scalar isotropic baseline; high for later anisotropy | Amends frozen differential semantics and protocol | **DEFER** |
| Bounded analytic cone | radius/semi-angle/apex/domain policy | Useful representation breadth and later oracle | New surface family; protocol amendment if admitted now | **DEFER** |
| Bounded analytic torus | two radii and double-periodic seam policy | Rich later representation/oracle coverage | New surface family and periodicity seam | **DEFER** |
| General trimming / p-curves / faces | curve-on-surface, loops, orientation and topology identity | Essential before CAD-wide shared boundaries | Large cross-layer architecture seam | **DEFER** |
| Fail-closed qualification preparation infrastructure | candidate/upstream binding, frozen-boundary enforcement and sealed preparation mechanics | Directly completes the next declared prerequisite for the already-integrated claim | No production semantic expansion | **SELECTED** |

## 6. Decision

Select exactly one future bounded work item:

**Fail-Closed Surface Differential Geometry Qualification Preparation
Infrastructure.**

This selection advances the already-preregistered Surface Differential
Geometry qualification lifecycle. It does not begin formal preparation or
formal qualification.

No production implementation begins on this decision branch. The selected
infrastructure is not authorized for implementation until this decision is
integrated, post-merge validated, separately closed, and that closure is also
post-merge validated.

## 7. Bounded future infrastructure contract

The selected future implementation may provide only reusable preparation
mechanisms that:

1. require an explicit candidate commit and exact upstream repository identity;
2. reject a dirty, unpublished, divergent or moving candidate;
3. verify the protocol, profile and report-tool identities;
4. enforce all 32 frozen semantic blob identities;
5. enforce the exact 42-test ordinary semantic registration allowlist;
6. bind the complete tracked-source inventory;
7. bind the admitted Ubuntu 24.04 x86_64 four-cell toolchain plan;
8. describe the exact planned command/repetition cardinality without executing
   the formal campaign;
9. create deterministic manifest and state-history structures suitable for a
   later PREPARED package;
10. fail closed on every identity, inventory, schema, state or cardinality
    mismatch;
11. prove the mechanisms with synthetic positive and negative fixtures;
12. remain incapable of issuing execution authorization or a qualification
    result.

The infrastructure implementation and its tests must create no real formal
PREPARED package. A real package remains a later, explicit, one-candidate
dispatch after the infrastructure is integrated and independently audited.

## 8. Required synthetic negative evidence

The future focused contract must reject at least:

- candidate/upstream mismatch;
- dirty or uncommitted candidate state;
- missing or altered frozen semantic blob;
- missing, extra or duplicate ordinary test registration;
- profile, protocol, exporter or validator identity mismatch;
- incomplete tracked-source inventory;
- unexpected compiler, standard library, build type, runner image or cell;
- wrong repetition or core-command cardinality;
- malformed or non-canonical manifest/state history;
- a previously consumed package represented as fresh;
- any attempt to embed execution authorization or a `QUALIFIED` result.

Synthetic success proves only the infrastructure contract. It is not formal
preparation evidence.

## 9. Explicit exclusions

This decision does not authorize:

- principal directions, direction signs, line fields or curvature lines;
- near-umbilic thresholds;
- cone or torus production support;
- general trimming, p-curves, wires or topological faces;
- changed surface or differential production semantics;
- changed public error vocabulary or numeric tolerances;
- a protocol amendment;
- a real candidate selection;
- creation or dispatch of a formal PREPARED package;
- execution authorization or workflow dispatch;
- formal qualification execution;
- retry, fallback, rescue or acceptance relaxation;
- Surface Representation or Surface Differential Geometry `QUALIFIED` status;
- Boundary Curve Discretization, sizing, meshing, anisotropy or parallelism.

## 10. Stop conditions

Stop and require a new decision or protocol amendment if infrastructure design
requires:

- changing any frozen semantic blob;
- changing the exact 42-test allowlist;
- changing the admitted four-cell environment or two-repetition contract;
- adding a production surface/differential capability;
- creating a real PREPARED package during implementation validation;
- combining preparation with authorization or execution;
- weakening any fail-closed identity or inventory gate;
- adding a third-party runtime dependency;
- changing an SDG0–SDG7 scientific acceptance condition.

## 11. Validation boundary for this decision

This decision changes documentation only. Before integration it requires:

- diff restricted to this decision, continuity documents and reference
  register;
- FAST PASS;
- GCC 13 Debug INTEGRATION PASS;
- Clang 18/libc++ Debug INTEGRATION PASS;
- no production, test, experiment, tool or workflow change;
- no scientific qualification claim.

No local scientific test or formal campaign is required for this
documentation-only decision.

## 12. Work-class plan

This decision work item:

- Implementation: 0%;
- Tests/validation: 10%;
- Evidence/literature analysis: 35%;
- Documentation/governance: 55%.

These percentages describe work distribution, not scientific completion.

The selected future infrastructure implementation should target:

- Implementation/tooling: 60%;
- Tests/validation: 30%;
- Evidence/audit: 5%;
- Documentation/governance: 5%.

## 13. Effect if integrated and separately closed

After this decision receives required checks, merges, receives protected-main
validation, and a separate documentation closure also merges and passes
protected-main validation, the sole next implementation work item becomes:

**Fail-Closed Surface Differential Geometry Qualification Preparation
Infrastructure.**

Until that separate closure:

- implementation is not authorized;
- formal preparation and execution remain unauthorized;
- no scientific candidate is active;
- Surface Representation remains **IN INVESTIGATION / NOT QUALIFIED**;
- Surface Differential Geometry remains **IN INVESTIGATION / NOT QUALIFIED**;
- Boundary Curve Discretization remains **NOT STARTED**.

After the infrastructure is later implemented, validated, integrated, closed
and independently audited, a new explicit action is still required before one
formal PREPARED package may be dispatched. Execution authorization remains a
later, separate decision bound to the exact audited package.

## 14. References

- `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_ENTRY_DECISION.md`;
- `docs/decisions/SURFACE_METRIC_CONDITIONING_DIAGNOSTICS_DECISION.md`;
- `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_READINESS_DECISION.md`;
- `docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md`;
- `docs/audits/2026-09-28-surface-differential-geometry-report-tooling-validation.md`;
- `docs/research/REFERENCE_REGISTER.md`.


## 15. Decision closure checkpoint

Decision closure PR #244 used final head:

`b25cb1d11b36855aea1fb5f5078fbc2f2e74cbe1`.

Required closure validation:

- FAST #651: PASS;
- INTEGRATION #642 / GCC 13 Debug: PASS;
- INTEGRATION #642 / Clang 18/libc++ Debug: PASS;
- no production/tool/test changes;
- no qualification execution or status claim.

PR #244 squash-merged as:

`77079f570ddfb9fce74db41fb38114a000a94c51`.

Protected-main validation on that exact revision:

- FAST #652: PASS;
- INTEGRATION #643 / GCC 13 Debug: PASS;
- INTEGRATION #643 / Clang 18/libc++ Debug: PASS.

Ordinary semantic inventory remains **42 tests**.

Terminal decision result:

**POST-TOOLING CONTINUATION DECISION CLOSED /
FAIL-CLOSED QUALIFICATION PREPARATION INFRASTRUCTURE AUTHORIZED /
FORMAL PREPARATION NOT STARTED / FORMAL EXECUTION NOT AUTHORIZED /
NOT QUALIFIED.**

No production, scientific or repository-transition work item remains active.

The sole next implementation work item is:

**Fail-Closed Surface Differential Geometry Qualification Preparation
Infrastructure.**

This authorization is limited to reusable preparation mechanisms and synthetic
validation. It does not authorize selection of a real formal candidate,
creation/dispatch of a real PREPARED package, execution authorization, formal
campaign execution, or a `QUALIFIED` status.
