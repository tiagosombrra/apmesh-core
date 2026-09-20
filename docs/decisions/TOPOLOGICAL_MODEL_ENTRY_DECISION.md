# Topological Model - Bounded Entry Decision

Status: EDGE KERNEL IMPLEMENTED / FACE-BOUNDARY IMPLEMENTED / FOCUSED CONTRACT PASS / EDGE-USE INCIDENCE IMPLEMENTED / FOCUSED CONTRACT PASS / STRUCTURAL INCIDENCE CONTRACT ACCEPTED / STAGE UNQUALIFIED
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

## Implementation result

The bounded candidate adds only
`include/apmesh/topology/topology.hpp`, `src/topology/topology.cpp`, and
`tests/topological_model.cpp`, with one `topology;contract` CTest registration.
It implements the authorized strong IDs, builder-local construction handles,
edge endpoint records, oriented uses, deterministic allocation, atomic
finalization, and read-only model queries. The focused contract passed in the
declared WSL Ubuntu 24.04 GCC 13 Debug and Clang 18/libc++ Debug builds.

This is focused implementation evidence only. It does not qualify the
Topological Model stage, native Windows, Release builds, face/patch incidence,
manifold behavior, canonical serialization, or any later geometry/meshing
capability.

## Second bounded contract: Face Identity and Boundary Cycles

### Decision

Authorize one further bounded implementation work unit named **Face Identity
and Ordered Boundary Cycles**.

The work unit may add a strong topological `FaceId`, immutable face records,
and one or more boundary loops represented as ordered non-empty sequences of
existing `EdgeUse` values. It may validate only reference validity,
orientation validity, and exact topological cycle closure. It must not attach
Geometry, classify loops geometrically, assume four sides, or introduce
`PatchId`, `SurfaceId`, curves, surfaces, shells, manifold classification, or
meshing.

This decision refines the Architecture Contract's explicitly unfrozen candidate
terminology. `FaceId` is the identity of topological incidence. A future
`PatchId` will identify a higher-level association involving topology and
geometric data; it is not an alias for `FaceId`. The cardinality and ownership
of that future association remain undecided. Foundation and Geometry
Primitives are not reopened because no qualified runtime behavior or
topology/geometry separation claim changes.

### Sources and decision impact

| Source | Supports | Does not establish | Decision impact |
| --- | --- | --- | --- |
| `docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md` | Model-local strong identities, explicit edge-use orientation, and topology/geometry separation. | A frozen face, patch, or surface schema. | Introduce `FaceId` in topology while keeping `PatchId` and `SurfaceId` outside this work unit. |
| [CGAL Halfedge Data Structures](https://doc.cgal.org/latest/HalfedgeDS/index.html) | Ordered oriented uses around a face can be expressed through successor incidence. | That AP Mesh needs paired halfedges, pointer-linked storage, a single loop per face, or two-manifold assumptions. | Preserve ordered boundary semantics without adopting CGAL storage. |
| [ISO 10303-42 face and face-bound schema](https://steptools.com/stds/smrl/data/resource_docs/geometric_and_topological_representation/sys/5_schema.htm) | A face is bounded by one or more loops; an outer-bound distinction is additional semantics and may be unavailable for some closed/partially closed surfaces. | STEP conformance, its complete validity rules, or that geometric outer/inner classification is available now. | Permit multiple loops but leave outer/inner meaning to a later surface-aware work unit. |
| [Open CASCADE topological shape kinds](https://dev.opencascade.org/doc/refman/html/_top_abs___shape_enum_8hxx.html) | Face, wire, edge, and vertex are distinct topological levels; a wire is an edge sequence and a face is bounded by closed wire data. | That Open CASCADE is a dependency or its hierarchy is the AP Mesh storage schema. | Keep face identity and loop structure explicit while retaining a small value-based model. |

No third-party dependency or external acceptance oracle is admitted.

### Bounded semantic model

- `FaceId` follows the accepted model-local, entity-kind-local allocation
  policy: zero invalid, sequential allocation from one, checked exhaustion,
  and no reuse.
- A boundary loop is an immutable value containing an ordered, non-empty
  sequence of `EdgeUse` values. It has no independent identity in this work
  unit.
- A face contains a `FaceId` and an ordered, non-empty collection of boundary
  loops. Loop collection order is preserved for deterministic inspection but
  conveys no outer/inner, nesting, winding, or geometric-containment meaning.
- For consecutive uses `u_i` and `u_(i+1)`, the oriented end vertex of `u_i`
  equals the oriented start vertex of `u_(i+1)`. The last use closes onto the
  first.
- Boundary valence is arbitrary and positive. No four-edge or quadrilateral
  assumption is permitted.
- Repeated use of an underlying edge, parallel edges, a one-edge self-loop,
  and reuse of an edge by multiple faces remain representable. This admission
  makes no manifold, regularity, embedding, or mesh-validity claim.
- Distinct successful face insertions receive distinct `FaceId` values even if
  their declared boundary loops are identical.
- Face construction is atomic. Invalid references, invalid orientation, an
  empty face boundary, an empty loop, an open loop, or identity exhaustion
  inserts no face and consumes no face identity.
- Finalized face identities and boundary incidence are immutable and are
  exposed in deterministic identity/insertion order.

Boundaryless faces are deferred because their validity depends on future
closed-surface and seam semantics. A later decision may admit them without
changing the meaning of the bounded loop representation.

### Required invariants

For one model with vertex set `V`, edge set `E`, face set `F`, and declared
boundary loops `B`:

1. `FaceId` is strongly distinct from `VertexId`, `EdgeId`, and every future
   geometry/domain identity kind.
2. Every face ID is valid only in its owning model and obeys the accepted
   deterministic allocation policy.
3. Every `EdgeUse` in every loop references an edge in `E` and has a valid
   orientation.
4. Every loop contains at least one use and closes exactly by resolved vertex
   identity, not by coordinates or tolerance.
5. Loop closure is cyclic: every use's end equals the next use's start and the
   last end equals the first start.
6. A face in this bounded work unit contains at least one valid loop.
7. Loop valence is not fixed and no `PatchSide` enumeration participates in
   topology.
8. Multiple loops carry no outer/inner or nesting semantics until a later
   surface-aware contract.
9. Repeated edge identities and arbitrary face incidence counts are not
   rejected merely by multiplicity.
10. Equal boundary declarations do not merge distinct face insertions.
11. A failed face insertion is transactional and consumes no `FaceId`.
12. Finalization remains all-or-nothing and the finalized model exposes no
    mutation of face identity or boundary order.
13. Repeating the same ordered construction reproduces face IDs, loop order,
    and edge-use order exactly.
14. No coordinate, distance, tolerance, pointer, hash iteration, curve,
    surface, or parameter value participates in face identity or cycle closure.

### Required focused risk cases

| Risk class | Required observation |
| --- | --- |
| Strong identity | `FaceId` cannot substitute for vertex/edge identity; zero and out-of-range faces do not resolve. |
| Non-four-sided valence | Valid triangular and five-edge cycles are accepted without a side enum. |
| Orientation-dependent closure | A connected edge set closes only when each `EdgeUse` orientation produces matching consecutive vertex identities. |
| Open chain | A non-closing sequence fails explicitly, inserts no face, and consumes no identity. |
| Invalid use | Missing edge identity and invalid orientation fail transactionally. |
| Empty input | Zero loops and an empty loop are rejected by the bounded admission policy. |
| Self-loop boundary | One self-loop edge may form a one-use closed boundary without geometric inference. |
| Seam-compatible repetition | A connected cycle may use the same underlying edge more than once, including opposite orientations, without duplicating edge identity. |
| Multiple boundaries | Two independently closed loops are retained in declared order without outer/inner classification. |
| Identical faces | Two equal boundary declarations receive distinct `FaceId` values. |
| Arbitrary incidence | Three faces may reference one edge without premature manifold rejection. |
| Immutability and repeatability | Finalized boundaries cannot mutate and equal construction sequences reproduce exact claim fields. |
| Topology/patch separation | Production topology contains no `PatchId`, `SurfaceId`, Geometry include, coordinate, or four-side assumption. |

Use table-driven focused tests where several valences share the same closure
property. No formal manifest, certificate matrix, repeated four-cell campaign,
or component-specific evidence tooling is required.

### Explicit exclusions

This bounded contract does not authorize:

- `PatchId`, `CurveId`, `SurfaceId`, `LoopId`, shell, region, solid, or
  topological face-use identities;
- outer/inner loop classification, nesting, winding, parameter-space
  orientation, trimming evaluation, or geometric intersection checks;
- curve-on-surface, pcurve, seam pairing, curve ownership, surface ownership,
  or face--patch--surface cardinality;
- manifold, boundary-manifold, non-manifold, fan, shell, or connected-component
  classification;
- deletion, identity reuse, Euler operators, topology repair, welding,
  snapping, or coordinate-based closure;
- canonical serialization/hash, STEP import/export, CAD adapters, or any
  claim of STEP/Open CASCADE/CGAL conformance;
- meshing, discretization, sizing, acceptance, parallel execution, or formal
  Topological Model qualification.

### Admission and stop conditions

The bounded implementation extends the existing topology kernel without
changing accepted vertex/edge behavior. Its focused FAST CTest passed in GCC
13 Debug and Clang 18/libc++ Debug for every declared risk case. Topological
Model remains unqualified. Stop for a new scientific decision before any work
requiring geometric outer/inner classification, boundaryless faces, explicit
loop identity, face orientation relative to a surface,
face--patch--surface ownership/cardinality, manifold restrictions, or a change
to the existing edge identity/orientation semantics.

### Effect on the roadmap

Topological Model remains `IN INVESTIGATION`. The first kernel remains
`IMPLEMENTED / FOCUSED CONTRACT PASS / UNQUALIFIED`; this second work unit is
also `IMPLEMENTED / FOCUSED CONTRACT PASS / UNQUALIFIED`. Qualified prerequisite
stages remain closed. Formal Topological Model qualification remains deferred
until the stage's cumulative regression.

## Third bounded contract: Deterministic Edge-Use Incidence Enumeration

### Decision

The Face Identity and Ordered Boundary Cycles contract is closed at
`IMPLEMENTED / FOCUSED CONTRACT PASS / UNQUALIFIED`. The published focused
contract now directly covers incorrect connected orientation, nonzero
out-of-range `FaceId`, and repeatability across independent constructions.

The bounded implementation **Deterministic Edge-Use Incidence Enumeration** adds an immutable factual record for
each occurrence of an existing edge in a face boundary and a model query that
enumerates those records deterministically. It must not classify the result as
manifold, boundary, non-manifold, adjacent, paired, outer, inner, or geometric.

This is the smallest next unit because the model already owns the forward
relation `Face -> BoundaryLoop -> EdgeUse`, while later incidence validation
requires the exact reverse relation `Edge -> EdgeUse occurrences`. The accepted
CGAL, ISO 10303-42, and Open CASCADE references support explicit oriented-use
incidence, but do not prescribe this storage layout or authorize a manifold
policy. No new dependency or external oracle is introduced.

### Bounded semantic model

- An edge-use incidence record contains the owning `FaceId`, zero-based
  boundary-loop ordinal, zero-based edge-use ordinal, and declared
  `Orientation`.
- Loop and use ordinals are positions in immutable face storage. They are not
  `LoopId`, halfedge identity, or persistent identities outside the model.
- Querying a valid edge returns every occurrence exactly once in deterministic
  face-ID, loop-order, and use-order traversal.
- A valid edge with no face use returns an empty result. An invalid or
  out-of-range `EdgeId` fails explicitly.
- Repeated occurrences of one edge in one loop, multiple loops of one face,
  and occurrences in multiple faces remain distinct records and are never
  deduplicated.
- The record preserves the orientation declared by the corresponding
  `EdgeUse`; it does not infer an opposite, mate, neighbor, side, or normal.
- Finalization builds or validates the reverse incidence atomically and exposes
  it read-only with the rest of `TopologyModel`.

### Required invariants and focused cases

1. The multiset of incidence records is in one-to-one correspondence with all
   stored `EdgeUse` occurrences.
2. Every record resolves to its exact owning face, loop position, use position,
   edge identity, and orientation.
3. Deterministic construction order produces deterministic incidence order.
4. An unused valid edge produces an empty sequence; zero and out-of-range
   edges produce `invalid_edge_id`.
5. Repeated-edge, opposite-orientation, multiple-loop, and three-face cases
   preserve all occurrences without classification or coalescing.
6. Independent equal constructions reproduce identical incidence records.
7. No coordinate, tolerance, geometry type, hash iteration, or pointer value
   participates in enumeration or ordering.
8. Existing vertex, edge, face, boundary-loop, and error behavior remains
   unchanged.

Focused FAST tests must exercise: unused edge, one occurrence, repeated edge in
one loop, opposite orientations, multiple loops, three faces sharing one edge,
invalid lookup, exact positional resolution, and independent repeatability.
No formal manifest, component-specific evidence tooling, Release matrix, or
qualification campaign is required.

### Explicit exclusions and stop conditions

This work unit does not authorize edge adjacency, face adjacency, mate/opposite
pairing, manifold or non-manifold classification, boundary classification,
fan ordering, connected components, shells, Euler operators, loop identity,
canonical serialization/hash, `PatchId`, curves, surfaces, trimming, meshing,
or coordinate-based inference.

Stop for a new scientific decision if implementation requires any such
classification, a persistent identity for an edge use or loop, geometric
orientation, face--patch--surface ownership, or a change to existing identity
and boundary-cycle semantics.

### Effect on the roadmap

Topological Model remains `IN INVESTIGATION / UNQUALIFIED`. This third bounded
contract completes only factual reverse incidence needed before later
consistency and classification decisions. It neither closes the Immutable
Validated Model problem nor authorizes Topological Model Regression.

### Closure handoff

The required new bounded topological decision is the fourth contract below. The
third contract itself does not authorize classification, geometry,
serialization, qualification infrastructure, or a formal campaign.

## Fourth bounded contract: Deterministic Edge-Incidence Structural Classification

### Decision

The Deterministic Edge-Use Incidence Enumeration contract is closed at
`IMPLEMENTED / FOCUSED CONTRACT PASS / UNQUALIFIED`. Its reverse records are
the sole input admitted by this fourth contract.

Authorize one bounded implementation work unit named **Deterministic
Edge-Incidence Structural Classification**. It may derive an immutable summary
for each valid edge from its complete ordered incidence sequence. The summary
describes only directly observable combinatorial facts; it does not decide
adjacency, mate pairing, boundary membership, manifoldness, fan order, shell
membership, embedding validity, or geometric orientation.

This separation is necessary because cardinality and declared orientation are
insufficient by themselves to establish every later topological claim. Two
opposed uses may belong to distinct faces or to one seam face, repeated uses may
occur within one loop, and a multi-use edge has no canonical cyclic fan order
without additional structure. The structural signature therefore preserves
those distinctions instead of collapsing them into a premature manifold label.

### Bounded semantic model

For each valid edge, the derived signature contains:

- total occurrence count;
- distinct owning-face count;
- distinct owning-boundary count, where an owner is the pair `(FaceId,
  boundary-loop ordinal)`;
- forward and reverse occurrence counts;
- whether any owning face or owning boundary contributes more than one
  occurrence; and
- exactly one structural class:
  - `unused`: zero occurrences;
  - `single_use`: one occurrence;
  - `two_use_opposed`: exactly two occurrences with one forward and one reverse;
  - `two_use_cooriented`: exactly two occurrences with equal orientation;
  - `multi_use`: more than two occurrences.

These names are exhaustive structural categories, not aliases for isolated,
boundary, manifold, orientation-valid, orientation-invalid, non-manifold, seam,
or adjacent. In particular, `two_use_opposed` does not distinguish two faces
from a same-face seam; the ownership counts and repetition flags retain that
fact separately.

The signature is derived deterministically from immutable
`edge_use_incidences(EdgeId)` results. It introduces no persistent identity,
stored mate, adjacency edge, unordered traversal, tolerance, coordinate, or
independent source of truth. Invalid and out-of-range `EdgeId` values retain
the existing explicit `invalid_edge_id` failure.

### Required invariants and focused FAST cases

1. The five structural classes are mutually exclusive and exhaustive for every
   valid edge.
2. Total, forward, and reverse counts are exact and satisfy
   `total = forward + reverse`.
3. Distinct face and boundary-owner counts are computed from exact IDs and loop
   ordinals, never from coordinates or pointer identity.
4. Repeated ownership is reported without deduplication or rejection.
5. Input incidence order does not change counts or class, while the underlying
   ordered incidence sequence remains unchanged.
6. Repeated equal model construction yields identical signatures.
7. Existing model construction, face cycles, reverse incidence, lookup errors,
   and acceptance behavior remain unchanged.

Focused FAST tests must cover: unused, single-use, opposed two-use,
co-oriented two-use, same-face seam, repeated use within one loop, multi-use by
three faces, distinct face/boundary counts, invalid lookup, and independent
repeatability. The test must also prove that classification does not mutate or
reorder the underlying incidence records.

No formal manifest, new evidence framework, Release matrix, or qualification
campaign is required.

### Explicit exclusions and stop conditions

This contract does not authorize:

- face adjacency or self-adjacency APIs;
- mate/opposite pairing or pair identities;
- boundary, manifold, non-manifold, seam, fan, shell, region, or connected-
  component classification;
- cyclic ordering of multi-use incidences;
- rejection or repair based on any structural class;
- loop/use persistent identity, canonical serialization/hash, geometry,
  `PatchId`, curves, surfaces, trimming, meshing, or coordinate inference.

Stop for a new scientific decision if implementation requires interpreting a
signature as one of those excluded concepts, changing accepted model inputs,
or introducing a second incidence store that can diverge from the existing
reverse relation.

### Alternatives considered

- **Declare one use boundary, two uses manifold, and three or more uses
  non-manifold:** rejected because it erases same-face seams, repeated owners,
  and the assumptions needed to interpret cardinality.
- **Create all pairwise face adjacencies:** rejected because a multi-use edge
  has no authorized pairing or fan order and all-pairs expansion would invent a
  relation not present in the model.
- **Introduce persistent halfedge or mate identities:** deferred because the
  current positional incidence records already preserve every fact required by
  this work unit.
- **Chosen structural signature:** provides deterministic, testable evidence
  for the later classification decision without changing model admissibility.

### Effect on the roadmap

Topological Model remains `IN INVESTIGATION / UNQUALIFIED`. This fourth
contract creates the factual bridge from exact reverse incidence to a later,
separately authorized consistency and manifold-classification policy. It does
not close Immutable Validated Model or authorize Topological Model Regression.

## Next bounded action

Implement only the deterministic edge-incidence structural signature and its
focused FAST cases. Do not add adjacency, pairing, acceptance changes,
manifold/boundary labels, geometry, serialization, qualification tooling, or a
formal campaign.
