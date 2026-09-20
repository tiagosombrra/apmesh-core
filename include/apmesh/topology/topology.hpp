#pragma once

#include <compare>
#include <cstdint>
#include <expected>
#include <span>
#include <vector>

namespace apmesh::topology {

enum class TopologyError {
    invalid_vertex_handle,
    invalid_edge_id,
    invalid_face_id,
    invalid_orientation,
    empty_face_boundary,
    empty_boundary_loop,
    open_boundary_loop,
    identity_exhausted,
    invalid_model,
};

class TopologyBuilder;

class VertexId {
public:
    constexpr VertexId() noexcept = default;

    [[nodiscard]] constexpr std::uint64_t value() const noexcept {
        return value_;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return value_ != 0U;
    }

    constexpr auto operator<=>(const VertexId&) const noexcept = default;

private:
    constexpr explicit VertexId(const std::uint64_t value) noexcept : value_(value) {}

    std::uint64_t value_{};

    friend class TopologyBuilder;
};

class EdgeId {
public:
    constexpr EdgeId() noexcept = default;

    [[nodiscard]] constexpr std::uint64_t value() const noexcept {
        return value_;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return value_ != 0U;
    }

    constexpr auto operator<=>(const EdgeId&) const noexcept = default;

private:
    constexpr explicit EdgeId(const std::uint64_t value) noexcept : value_(value) {}

    std::uint64_t value_{};

    friend class TopologyBuilder;
};

class FaceId {
public:
    constexpr FaceId() noexcept = default;

    [[nodiscard]] constexpr std::uint64_t value() const noexcept {
        return value_;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return value_ != 0U;
    }

    constexpr auto operator<=>(const FaceId&) const noexcept = default;

private:
    constexpr explicit FaceId(const std::uint64_t value) noexcept : value_(value) {}

    std::uint64_t value_{};

    friend class TopologyBuilder;
};

enum class Orientation {
    forward,
    reverse,
};

struct EdgeUse {
    EdgeId edge{};
    Orientation orientation{Orientation::forward};

    constexpr bool operator==(const EdgeUse&) const noexcept = default;
};

class Vertex {
public:
    [[nodiscard]] constexpr VertexId id() const noexcept {
        return id_;
    }

private:
    constexpr explicit Vertex(const VertexId id) noexcept : id_(id) {}

    VertexId id_{};

    friend class TopologyBuilder;
};

class Edge {
public:
    [[nodiscard]] constexpr EdgeId id() const noexcept {
        return id_;
    }

    [[nodiscard]] constexpr VertexId first() const noexcept {
        return first_;
    }

    [[nodiscard]] constexpr VertexId second() const noexcept {
        return second_;
    }

private:
    constexpr Edge(
        const EdgeId id,
        const VertexId first,
        const VertexId second) noexcept
        : id_(id), first_(first), second_(second) {}

    EdgeId id_{};
    VertexId first_{};
    VertexId second_{};

    friend class TopologyBuilder;
};

struct OrientedEndpoints {
    VertexId start{};
    VertexId end{};

    constexpr bool operator==(const OrientedEndpoints&) const noexcept = default;
};

class BoundaryLoop {
public:
    [[nodiscard]] std::span<const EdgeUse> uses() const noexcept;

private:
    explicit BoundaryLoop(std::vector<EdgeUse> uses) noexcept;

    std::vector<EdgeUse> uses_;

    friend class Face;
    friend class TopologyBuilder;
    friend class TopologyModel;
};

class Face {
public:
    [[nodiscard]] constexpr FaceId id() const noexcept {
        return id_;
    }

    [[nodiscard]] std::span<const BoundaryLoop> boundary_loops() const noexcept;

private:
    Face(FaceId id, std::vector<BoundaryLoop> boundary_loops) noexcept;

    FaceId id_{};
    std::vector<BoundaryLoop> boundary_loops_;

    friend class TopologyBuilder;
    friend class TopologyModel;
};

class TopologyModel {
public:
    [[nodiscard]] std::span<const Vertex> vertices() const noexcept;
    [[nodiscard]] std::span<const Edge> edges() const noexcept;
    [[nodiscard]] std::span<const Face> faces() const noexcept;

    [[nodiscard]] std::expected<Edge, TopologyError> edge(EdgeId id) const noexcept;
    [[nodiscard]] std::expected<Face, TopologyError> face(FaceId id) const noexcept;
    [[nodiscard]] std::expected<OrientedEndpoints, TopologyError> resolve(
        const EdgeUse& use) const noexcept;

private:
    TopologyModel(
        std::vector<Vertex> vertices,
        std::vector<Edge> edges,
        std::vector<Face> faces) noexcept;

    std::vector<Vertex> vertices_;
    std::vector<Edge> edges_;
    std::vector<Face> faces_;

    friend class TopologyBuilder;
};

class TopologyBuilder {
public:
    class VertexHandle {
    public:
        constexpr VertexHandle() noexcept = default;

        [[nodiscard]] constexpr VertexId id() const noexcept {
            return id_;
        }

    private:
        constexpr VertexHandle(const TopologyBuilder* owner, const VertexId id) noexcept
            : owner_(owner), id_(id) {}

        const TopologyBuilder* owner_{};
        VertexId id_{};

        friend class TopologyBuilder;
    };

    TopologyBuilder() = default;
    TopologyBuilder(const TopologyBuilder&) = delete;
    TopologyBuilder& operator=(const TopologyBuilder&) = delete;
    TopologyBuilder(TopologyBuilder&&) = delete;
    TopologyBuilder& operator=(TopologyBuilder&&) = delete;

    [[nodiscard]] std::expected<VertexHandle, TopologyError> add_vertex();
    [[nodiscard]] std::expected<EdgeId, TopologyError> add_edge(
        const VertexHandle& first,
        const VertexHandle& second);
    [[nodiscard]] std::expected<FaceId, TopologyError> add_face(
        std::span<const std::span<const EdgeUse>> boundary_loops);
    [[nodiscard]] std::expected<TopologyModel, TopologyError> finalize() const;

private:
    [[nodiscard]] bool owns(const VertexHandle& handle) const noexcept;

    std::vector<Vertex> vertices_;
    std::vector<Edge> edges_;
    std::vector<Face> faces_;
    std::uint64_t next_vertex_id_{1U};
    std::uint64_t next_edge_id_{1U};
    std::uint64_t next_face_id_{1U};
};

[[nodiscard]] std::expected<EdgeUse, TopologyError> reverse(const EdgeUse& use) noexcept;

} // namespace apmesh::topology
