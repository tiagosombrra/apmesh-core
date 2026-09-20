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

void advance_identity(std::uint64_t& next) noexcept {
    if (next == std::numeric_limits<std::uint64_t>::max()) {
        next = 0U;
        return;
    }
    ++next;
}

[[nodiscard]] std::expected<void, TopologyError> validate(
    const std::vector<Vertex>& vertices,
    const std::vector<Edge>& edges) noexcept {
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

    return {};
}

} // namespace

TopologyModel::TopologyModel(
    std::vector<Vertex> vertices,
    std::vector<Edge> edges) noexcept
    : vertices_(std::move(vertices)), edges_(std::move(edges)) {}

std::span<const Vertex> TopologyModel::vertices() const noexcept {
    return vertices_;
}

std::span<const Edge> TopologyModel::edges() const noexcept {
    return edges_;
}

std::expected<Edge, TopologyError> TopologyModel::edge(const EdgeId id) const noexcept {
    if (!contains_edge(edges_, id)) {
        return std::unexpected{TopologyError::invalid_edge_id};
    }
    return edges_[static_cast<std::size_t>(id.value() - 1U)];
}

std::expected<OrientedEndpoints, TopologyError> TopologyModel::resolve(
    const EdgeUse& use) const noexcept {
    const auto resolved_edge = edge(use.edge);
    if (!resolved_edge.has_value()) {
        return std::unexpected{resolved_edge.error()};
    }

    switch (use.orientation) {
    case Orientation::forward:
        return OrientedEndpoints{
            .start = resolved_edge->first(),
            .end = resolved_edge->second(),
        };
    case Orientation::reverse:
        return OrientedEndpoints{
            .start = resolved_edge->second(),
            .end = resolved_edge->first(),
        };
    }

    return std::unexpected{TopologyError::invalid_orientation};
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

std::expected<TopologyModel, TopologyError> TopologyBuilder::finalize() const {
    if (const auto validation = validate(vertices_, edges_); !validation.has_value()) {
        return std::unexpected{validation.error()};
    }
    return TopologyModel{vertices_, edges_};
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
