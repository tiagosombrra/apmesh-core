#include "apmesh/topology/topology.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace apmesh::topology {
namespace {

[[nodiscard]] bool contains_vertex(
    const std::vector<Vertex>& vertices,
    const VertexId id) noexcept {
    return id.valid() && id.value() <= vertices.size();
}

[[nodiscard]] bool contains_edge(
    const std::vector<Edge>& edges,
    const EdgeId id) noexcept {
    return id.valid() && id.value() <= edges.size();
}

[[nodiscard]] bool contains_face(
    const std::vector<Face>& faces,
    const FaceId id) noexcept {
    return id.valid() && id.value() <= faces.size();
}

void advance_identity(std::uint64_t& next) noexcept {
    if (next == std::numeric_limits<std::uint64_t>::max()) {
        next = 0U;
        return;
    }
    ++next;
}

[[nodiscard]] std::expected<OrientedEndpoints, TopologyError> resolve_edge_use(
    const std::vector<Edge>& edges,
    const EdgeUse& use) noexcept {
    if (!contains_edge(edges, use.edge)) {
        return std::unexpected{TopologyError::invalid_edge_id};
    }

    const Edge& edge = edges[static_cast<std::size_t>(use.edge.value() - 1U)];
    switch (use.orientation) {
    case Orientation::forward:
        return OrientedEndpoints{
            .start = edge.first(),
            .end = edge.second(),
        };
    case Orientation::reverse:
        return OrientedEndpoints{
            .start = edge.second(),
            .end = edge.first(),
        };
    }

    return std::unexpected{TopologyError::invalid_orientation};
}

[[nodiscard]] std::expected<void, TopologyError> validate_boundary_loops(
    const std::vector<Edge>& edges,
    const std::span<const std::span<const EdgeUse>> boundary_loops) noexcept {
    if (boundary_loops.empty()) {
        return std::unexpected{TopologyError::empty_face_boundary};
    }

    for (const std::span<const EdgeUse> loop : boundary_loops) {
        if (loop.empty()) {
            return std::unexpected{TopologyError::empty_boundary_loop};
        }

        const auto first = resolve_edge_use(edges, loop.front());
        if (!first.has_value()) {
            return std::unexpected{first.error()};
        }
        VertexId previous_end = first->end;

        for (std::size_t index = 1U; index < loop.size(); ++index) {
            const auto current = resolve_edge_use(edges, loop[index]);
            if (!current.has_value()) {
                return std::unexpected{current.error()};
            }
            if (previous_end != current->start) {
                return std::unexpected{TopologyError::open_boundary_loop};
            }
            previous_end = current->end;
        }

        if (previous_end != first->start) {
            return std::unexpected{TopologyError::open_boundary_loop};
        }
    }

    return {};
}

[[nodiscard]] std::expected<void, TopologyError> validate_boundary_loops(
    const std::vector<Edge>& edges,
    const std::span<const BoundaryLoop> boundary_loops) noexcept {
    if (boundary_loops.empty()) {
        return std::unexpected{TopologyError::empty_face_boundary};
    }

    for (const BoundaryLoop& loop : boundary_loops) {
        const std::span<const EdgeUse> uses = loop.uses();
        if (uses.empty()) {
            return std::unexpected{TopologyError::empty_boundary_loop};
        }

        const auto first = resolve_edge_use(edges, uses.front());
        if (!first.has_value()) {
            return std::unexpected{first.error()};
        }
        VertexId previous_end = first->end;

        for (std::size_t index = 1U; index < uses.size(); ++index) {
            const auto current = resolve_edge_use(edges, uses[index]);
            if (!current.has_value()) {
                return std::unexpected{current.error()};
            }
            if (previous_end != current->start) {
                return std::unexpected{TopologyError::open_boundary_loop};
            }
            previous_end = current->end;
        }

        if (previous_end != first->start) {
            return std::unexpected{TopologyError::open_boundary_loop};
        }
    }

    return {};
}

[[nodiscard]] std::expected<void, TopologyError> validate(
    const std::vector<Vertex>& vertices,
    const std::vector<Edge>& edges,
    const std::vector<Face>& faces) noexcept {
    for (std::size_t index = 0U; index < vertices.size(); ++index) {
        const std::uint64_t expected_id = static_cast<std::uint64_t>(index) + 1U;
        if (vertices[index].id().value() != expected_id) {
            return std::unexpected{TopologyError::invalid_model};
        }
    }

    for (std::size_t index = 0U; index < edges.size(); ++index) {
        const std::uint64_t expected_id = static_cast<std::uint64_t>(index) + 1U;
        const Edge& edge = edges[index];
        if (edge.id().value() != expected_id ||
            !contains_vertex(vertices, edge.first()) ||
            !contains_vertex(vertices, edge.second())) {
            return std::unexpected{TopologyError::invalid_model};
        }
    }

    for (std::size_t index = 0U; index < faces.size(); ++index) {
        const std::uint64_t expected_id = static_cast<std::uint64_t>(index) + 1U;
        const Face& face = faces[index];
        if (face.id().value() != expected_id ||
            !validate_boundary_loops(edges, face.boundary_loops()).has_value()) {
            return std::unexpected{TopologyError::invalid_model};
        }
    }

    return {};
}

[[nodiscard]] std::vector<std::vector<EdgeUseIncidence>> build_edge_use_incidences(
    const std::vector<Edge>& edges,
    const std::vector<Face>& faces) {
    std::vector<std::vector<EdgeUseIncidence>> incidences(edges.size());
    for (const Face& face : faces) {
        const std::span<const BoundaryLoop> loops = face.boundary_loops();
        for (std::size_t loop_index = 0U; loop_index < loops.size(); ++loop_index) {
            const std::span<const EdgeUse> uses = loops[loop_index].uses();
            for (std::size_t use_index = 0U; use_index < uses.size(); ++use_index) {
                const EdgeUse& use = uses[use_index];
                const std::size_t edge_index = static_cast<std::size_t>(use.edge.value() - 1U);
                incidences[edge_index].push_back(EdgeUseIncidence{
                    .face = face.id(),
                    .boundary_loop_ordinal = loop_index,
                    .edge_use_ordinal = use_index,
                    .orientation = use.orientation,
                });
            }
        }
    }
    return incidences;
}

[[nodiscard]] EdgeIncidenceSignature summarize_edge_incidences(
    const std::span<const EdgeUseIncidence> incidences) noexcept {
    EdgeIncidenceSignature signature{
        .occurrence_count = incidences.size(),
    };

    for (std::size_t index = 0U; index < incidences.size(); ++index) {
        const EdgeUseIncidence& incidence = incidences[index];
        if (incidence.orientation == Orientation::forward) {
            ++signature.forward_count;
        } else {
            ++signature.reverse_count;
        }

        bool seen_face = false;
        bool seen_boundary = false;
        for (std::size_t previous = 0U; previous < index; ++previous) {
            const EdgeUseIncidence& prior = incidences[previous];
            if (prior.face == incidence.face) {
                seen_face = true;
                signature.has_repeated_face = true;
            }
            if (prior.face == incidence.face &&
                prior.boundary_loop_ordinal == incidence.boundary_loop_ordinal) {
                seen_boundary = true;
                signature.has_repeated_boundary = true;
            }
        }
        if (!seen_face) {
            ++signature.distinct_face_count;
        }
        if (!seen_boundary) {
            ++signature.distinct_boundary_count;
        }
    }

    if (signature.occurrence_count == 0U) {
        signature.classification = EdgeIncidenceClass::unused;
    } else if (signature.occurrence_count == 1U) {
        signature.classification = EdgeIncidenceClass::single_use;
    } else if (signature.occurrence_count == 2U) {
        signature.classification = signature.forward_count == 1U && signature.reverse_count == 1U
                                     ? EdgeIncidenceClass::two_use_opposed
                                     : EdgeIncidenceClass::two_use_cooriented;
    } else {
        signature.classification = EdgeIncidenceClass::multi_use;
    }
    return signature;
}

} // namespace

TopologyModel::TopologyModel(
    std::vector<Vertex> vertices,
    std::vector<Edge> edges,
    std::vector<Face> faces,
    std::vector<std::vector<EdgeUseIncidence>> edge_use_incidences) noexcept
    : vertices_(std::move(vertices)),
      edges_(std::move(edges)),
      faces_(std::move(faces)),
      edge_use_incidences_(std::move(edge_use_incidences)) {}

std::span<const Vertex> TopologyModel::vertices() const noexcept {
    return vertices_;
}

std::span<const Edge> TopologyModel::edges() const noexcept {
    return edges_;
}

std::span<const Face> TopologyModel::faces() const noexcept {
    return faces_;
}

std::expected<Edge, TopologyError> TopologyModel::edge(const EdgeId id) const noexcept {
    if (!contains_edge(edges_, id)) {
        return std::unexpected{TopologyError::invalid_edge_id};
    }
    return edges_[static_cast<std::size_t>(id.value() - 1U)];
}

std::expected<Face, TopologyError> TopologyModel::face(const FaceId id) const noexcept {
    if (!contains_face(faces_, id)) {
        return std::unexpected{TopologyError::invalid_face_id};
    }
    return faces_[static_cast<std::size_t>(id.value() - 1U)];
}

std::expected<std::span<const EdgeUseIncidence>, TopologyError>
TopologyModel::edge_use_incidences(const EdgeId id) const noexcept {
    if (!contains_edge(edges_, id)) {
        return std::unexpected{TopologyError::invalid_edge_id};
    }
    return edge_use_incidences_[static_cast<std::size_t>(id.value() - 1U)];
}

std::expected<EdgeIncidenceSignature, TopologyError>
TopologyModel::edge_incidence_signature(const EdgeId id) const noexcept {
    const auto incidences = edge_use_incidences(id);
    if (!incidences.has_value()) {
        return std::unexpected{incidences.error()};
    }
    return summarize_edge_incidences(*incidences);
}

std::expected<OrientedEndpoints, TopologyError> TopologyModel::resolve(
    const EdgeUse& use) const noexcept {
    return resolve_edge_use(edges_, use);
}

BoundaryLoop::BoundaryLoop(std::vector<EdgeUse> uses) noexcept : uses_(std::move(uses)) {}

std::span<const EdgeUse> BoundaryLoop::uses() const noexcept {
    return uses_;
}

Face::Face(FaceId id, std::vector<BoundaryLoop> boundary_loops) noexcept
    : id_(id), boundary_loops_(std::move(boundary_loops)) {}

std::span<const BoundaryLoop> Face::boundary_loops() const noexcept {
    return boundary_loops_;
}

bool TopologyBuilder::owns(const VertexHandle& handle) const noexcept {
    return handle.owner_ == this && contains_vertex(vertices_, handle.id_);
}

std::expected<TopologyBuilder::VertexHandle, TopologyError> TopologyBuilder::add_vertex() {
    if (next_vertex_id_ == 0U) {
        return std::unexpected{TopologyError::identity_exhausted};
    }

    const VertexId id{next_vertex_id_};
    vertices_.push_back(Vertex{id});
    advance_identity(next_vertex_id_);
    return VertexHandle{this, id};
}

std::expected<EdgeId, TopologyError> TopologyBuilder::add_edge(
    const VertexHandle& first,
    const VertexHandle& second) {
    if (!owns(first) || !owns(second)) {
        return std::unexpected{TopologyError::invalid_vertex_handle};
    }
    if (next_edge_id_ == 0U) {
        return std::unexpected{TopologyError::identity_exhausted};
    }

    const EdgeId id{next_edge_id_};
    edges_.push_back(Edge{id, first.id_, second.id_});
    advance_identity(next_edge_id_);
    return id;
}

std::expected<FaceId, TopologyError> TopologyBuilder::add_face(
    const std::span<const std::span<const EdgeUse>> boundary_loops) {
    if (next_face_id_ == 0U) {
        return std::unexpected{TopologyError::identity_exhausted};
    }
    if (const auto validation = validate_boundary_loops(edges_, boundary_loops);
        !validation.has_value()) {
        return std::unexpected{validation.error()};
    }

    std::vector<BoundaryLoop> stored_loops;
    stored_loops.reserve(boundary_loops.size());
    for (const std::span<const EdgeUse> loop : boundary_loops) {
        stored_loops.push_back(BoundaryLoop{std::vector<EdgeUse>{loop.begin(), loop.end()}});
    }

    const FaceId id{next_face_id_};
    faces_.push_back(Face{id, std::move(stored_loops)});
    advance_identity(next_face_id_);
    return id;
}

std::expected<TopologyModel, TopologyError> TopologyBuilder::finalize() const {
    if (const auto validation = validate(vertices_, edges_, faces_); !validation.has_value()) {
        return std::unexpected{validation.error()};
    }
    return TopologyModel{
        vertices_,
        edges_,
        faces_,
        build_edge_use_incidences(edges_, faces_),
    };
}

std::expected<EdgeUse, TopologyError> reverse(const EdgeUse& use) noexcept {
    switch (use.orientation) {
    case Orientation::forward:
        return EdgeUse{
            .edge = use.edge,
            .orientation = Orientation::reverse,
        };
    case Orientation::reverse:
        return EdgeUse{
            .edge = use.edge,
            .orientation = Orientation::forward,
        };
    }

    return std::unexpected{TopologyError::invalid_orientation};
}

} // namespace apmesh::topology
