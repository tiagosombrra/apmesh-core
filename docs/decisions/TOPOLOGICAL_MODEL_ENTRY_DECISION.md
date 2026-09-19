# Topological Model - Bounded Entry Decision

Status: ACCEPTED FOR BOUNDED IMPLEMENTATION
Date: 2026-09-19
Stage: Topological Model - Explicit Identity and Incidence
Prerequisites: Foundation `QUALIFIED`; Geometry Primitives `QUALIFIED` on
candidate `2f22ffd` in the declared WSL Ubuntu 24.04 envelope

## Question

What is the smallest topological capability that can establish explicit entity
identity and oriented incidence without prematurely introducing curves,
surfaces, patches, manifold assumptions, coordinate welding, or a complete
half-edge data structure?

## Decision

Authorize one bounded implementation work unit named **Identity and Oriented
Edge Incidence Kernel**.

The work unit may introduce strong vertex and edge identities, topological
edges with explicit endpoint identities, oriented edge uses, deterministic
mutable construction, and atomic finalization into an immutable topology
model. It must contain no coordinate and must not infer, merge, or compare
identity through Geometry.

This decision fixes scientific semantics and evidence obligations. Exact C++
spelling and file partitioning remain implementation choices consistent with
the Architecture Contract. This decision does not implement code and does not
authorize a formal qualification campaign.

## Sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md` | Strong model-local IDs, explicit orientation, mutable-builder to immutable-model transition, and topology/geometry separation are already architectural authorities. | A completed topology implementation or a full patch-complex schema. | The bounded work unit must refine these accepted rules rather than create a competing model. |
| [CGAL Halfedge Data Structures](https://doc.cgal.org/latest/HalfedgeDS/index.html) | An oriented use can represent one direction of an edge, with an opposite orientation and explicit incidence. | That AP Mesh must adopt CGAL, a paired-halfedge storage layout, or two-manifold assumptions. | Preserve explicit orientation semantics while deferring face cycles and storage topology. |
| [Open CASCADE `TopoDS_Shape`](https://dev.opencascade.org/doc/refman/html/class_topo_d_s___shape.html) | Underlying topological identity is distinct from the orientation of a use of that identity. | That Open CASCADE is a dependency or that its B-rep hierarchy is the AP Mesh schema. | Keep `EdgeId` stable while orientation belongs to `EdgeUse`, not to the edge identity. |

No third-party dependency is admitted. External implementations are references
for vocabulary and separation of concerns, never acceptance oracles.

## Authorized scope

The first implementation work unit may add only:

- strong `VertexId` and `EdgeId` value types whose kinds cannot be mixed;
- model-local, entity-kind-local `uint64_t` identity values, with zero invalid,
  sequential allocation from one in declared builder insertion order, checked
  exhaustion, and no reuse;
- a topological vertex record containing identity only;
- a topological edge record containing its identity and an ordered pair of
  existing `VertexId` endpoints;
- `Orientation::{forward, reverse}` and an `EdgeUse` containing one `EdgeId`
  and one orientation;
- read-only resolution of an edge use to its oriented start/end vertices;
- a mutable builder that can add vertices and edges and report structured
  construction errors;
- atomic validation/finalization into an immutable topology model;
- deterministic read-only lookup and iteration in identity order; and
- focused compile-time, analytic, adversarial, and deterministic contracts.

The topology module must not depend on Geometry. A test may associate identical
external `Point2` or `Point3` values with distinct vertex IDs to demonstrate
that coordinates do not participate in identity, but that association is not a
production topology field in this work unit.

Self-loop edges and distinct parallel edges are admitted topological inputs.
They cannot be rejected merely because their endpoint identities coincide with
one another or with another edge's endpoint pair. No geometric regularity,
length, embedding, or manifold meaning is inferred from that admission.

## Hypotheses

### Explicit identity is sufficient for the first separation claim

If identity is allocated only by the builder and topology stores no coordinate,
then coincident coordinates cannot silently create shared topology. Distinct
insertions remain distinct even when an external fixture assigns identical
Geometry values to them.

### Orientation is a property of use, not identity

One edge identity can be consumed in forward or reverse orientation without
duplicating or mutating the edge. Reversing a use swaps its oriented endpoints
and preserves the referenced edge identity.

### Deterministic construction is reproducible

The same valid insertion sequence yields the same kind-local IDs, entity order,
endpoint relations, and focused claim fields. Pointer values, container hash
order, process state, and coordinates do not affect identity allocation.

### Atomic finalization prevents partial scientific input

Invalid references or exhausted identity space produce a classified failure
and leave the builder unchanged. No partially validated immutable topology
model escapes a failed finalization.

### The kernel can remain independent of later patch semantics

Vertex/edge identity and oriented endpoint resolution can be implemented and
tested without deciding face cycles, patch sides, curve ownership, surface
parameterization, manifoldness, or canonical experiment serialization.

## Required invariants

For one topology model with vertex set `V`, edge set `E`, and edge uses `U`:

1. Every valid ID is nonzero and belongs to exactly one entity kind.
2. IDs are meaningful only in the context of their owning model; this work unit
   makes no global or cross-model identity claim.
3. Every edge endpoint references a vertex in `V`.
4. Multiple edges may have the same ordered or unordered endpoint pair and
   remain distinct by `EdgeId`.
5. A self-loop remains one valid edge identity whose two endpoints are the same
   valid `VertexId`.
6. Every edge use references one edge in `E` and has exactly one declared
   orientation.
7. For edge `(a,b)`, a forward use resolves to `(a,b)` and a reverse use to
   `(b,a)`.
8. Reversal is an involution: `reverse(reverse(use)) == use`.
9. A failed builder mutation is transactional: it inserts no entity and
   consumes no identity.
10. Finalization is all-or-nothing; validation failure returns no model.
11. A finalized model exposes no mutation of identities or incidence.
12. Repeated construction from the same ordered input yields identical claim
    fields.
13. No distance, tolerance, coordinate equality, pointer address, hash, or
    unordered traversal participates in identity or incidence.

## Required analytic and adversarial cases

| Case | Required observation |
| --- | --- |
| Strong type separation | Compile-time checks reject substituting `EdgeId` for `VertexId` and vice versa. |
| Invalid zero | Zero cannot resolve as a valid vertex or edge identity. |
| Deterministic allocation | The first valid insertion receives one and equal insertion sequences reproduce the exact ID sequence. |
| Coincident but disconnected | Two distinct vertex insertions associated only in the test with the same point remain distinct IDs and unconnected unless an edge is explicitly added. |
| Explicit connection | Adding one edge changes only declared incidence; no other vertex pair becomes adjacent. |
| Parallel edges | Two edge insertions with the same endpoints receive distinct IDs and remain independently addressable. |
| Self-loop | One edge may reference the same valid vertex twice without being merged, dropped, or reclassified geometrically. |
| Forward use | A forward use resolves to the stored endpoint order. |
| Reverse use | A reverse use resolves to the swapped endpoint order while preserving `EdgeId`. |
| Reversal involution | Double reversal exactly recovers the original use. |
| Missing endpoint | Zero and out-of-range endpoint IDs produce explicit failure, insert no edge, consume no ID, and leave prior valid builder state unchanged. |
| Missing edge use | Zero and out-of-range edge IDs produce explicit failure during model-aware resolution. |
| Atomic finalization | Finalization returns one immutable result or a classified error; no partially valid model is observable. |
| Immutability | Public compile-time checks find no mutation path for finalized entities or incidence. |
| Repeatability | Repeated executions produce identical ordered identity/incidence claim fields. |

Expected results must be derived from the declared insertion sequence and
incidence definitions. Legacy AP Mesh output is not an oracle.

## Explicit exclusions

This decision does not authorize:

- `CurveId`, `PatchId`, `SurfaceId`, face/shell/region identities, or their
  ownership relations;
- face boundary cycles, `next`/`previous` links, patch-side enumeration,
  complete side coverage, or coedge ordering beyond one oriented edge use;
- manifold, boundary-manifold, or non-manifold classification and fan
  traversal;
- curve or surface geometry, vertex coordinates inside topology, geometric
  embedding, proximity queries, welding, snapping, or repair;
- canonical topology serialization/hash, external persistent-ID mapping, graph
  isomorphism, or input-permutation invariance;
- topological predicates, intersections, meshing, adaptation, or acceptance;
- a generic graph framework, Euler operators, deletion/reuse of identities, or
  post-finalization mutation;
- third-party dependencies, filesystem I/O, logging, global state, OpenMP/MPI,
  or native-Windows qualification; or
- a stage-level manifest, evidence runner, cumulative campaign, or claim that
  the complete Topological Model is qualified.

Manifold seams, reversed patch seams, non-manifold fans, and complete patch
incidence remain mandatory later Topological Model work. They are not silently
removed from the roadmap by this bounded entry.

## Admission and stop conditions

Implementation may proceed only if the candidate:

1. preserves the accepted model-local allocation policy and strong type
   separation;
2. keeps topology independent of Geometry and every future module;
3. represents orientation on `EdgeUse`, never by duplicating edge identity;
4. reports invalid references and exhaustion explicitly with no fallback;
5. implements the complete focused case table above; and
6. preserves all currently qualified Foundation and Geometry Primitives
   behavior.

Stop and request a new scientific decision if the work requires face or patch
semantics, coordinate-based identity, a global identity service, hidden pointer
ownership, deletion/reuse, an approximate comparison, or reinterpretation of
the Architecture Contract.

## Alternatives considered

- **Coordinate-keyed adjacency:** rejected because it violates the fundamental
  identity invariant and cannot represent coincident disconnected entities.
- **Full half-edge or B-rep structure immediately:** deferred because faces,
  boundary cycles, patches, curves, and non-manifold policy are not yet bounded.
- **Strong ID wrappers only:** rejected as too weak; it would not exercise an
  actual incidence or orientation invariant.
- **Generic third-party graph library:** rejected for this work unit because no
  required capability exceeds small deterministic standard-library storage and
  it would not define AP Mesh scientific semantics.
- **Chosen kernel:** sufficient to test identity, incidence, orientation,
  validation, and immutability while preserving later design freedom.

## Effect on the roadmap

Topological Model moves from `NOT STARTED` to `IN INVESTIGATION / ENTRY
DECISION APPROVED`. Foundation and Geometry Primitives remain qualified and are
not reopened by this documentation-only decision.

## Next bounded action

Implement only the Identity and Oriented Edge Incidence Kernel and its focused
contracts. Do not implement faces, patches, curves, manifold classification,
canonical serialization, qualification infrastructure, or a formal campaign.
