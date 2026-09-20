#include "apmesh/core/geometry.hpp"
#include "apmesh/topology/topology.hpp"

#include <array>
#include <concepts>
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

using apmesh::topology::EdgeId;
using apmesh::topology::EdgeIncidenceClass;
using apmesh::topology::EdgeIncidenceSignature;
using apmesh::topology::EdgeUse;
using apmesh::topology::EdgeUseIncidence;
using apmesh::topology::FaceId;
using apmesh::topology::Orientation;
using apmesh::topology::TopologyBuilder;
using apmesh::topology::TopologyConsistencySummary;
using apmesh::topology::TopologyError;
using apmesh::topology::TopologyModel;
using apmesh::topology::VertexId;

template <typename Builder, typename Handle>
concept AddsEdge = requires(Builder& builder, const Handle& first, const Handle& second) {
    { builder.add_edge(first, second) } -> std::same_as<std::expected<EdgeId, TopologyError>>;
};

template <typename Model>
concept MutableTopologyModel = requires(Model& model) {
    model.add_vertex();
};

template <typename Builder>
concept AddsFace = requires(
    Builder& builder,
    const std::span<const std::span<const EdgeUse>> boundary_loops) {
    { builder.add_face(boundary_loops) } -> std::same_as<std::expected<FaceId, TopologyError>>;
};

template <typename Model>
concept EnumeratesEdgeUseIncidences = requires(const Model& model, const EdgeId id) {
    { model.edge_use_incidences(id) }
        -> std::same_as<std::expected<std::span<const EdgeUseIncidence>, TopologyError>>;
};

template <typename Model>
concept SummarizesEdgeIncidences = requires(const Model& model, const EdgeId id) {
    { model.edge_incidence_signature(id) }
        -> std::same_as<std::expected<EdgeIncidenceSignature, TopologyError>>;
};

template <typename Model>
concept SummarizesTopologyConsistency = requires(const Model& model) {
    { model.consistency_summary() }
        -> std::same_as<std::expected<TopologyConsistencySummary, TopologyError>>;
};

template <typename Model>
concept EmitsCanonicalTopologySnapshot = requires(const Model& model) {
    { model.canonical_snapshot() } -> std::same_as<std::expected<std::string, TopologyError>>;
};

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

template <typename Value>
bool require_value(
    const std::expected<Value, TopologyError>& value,
    const std::string_view message) {
    return require(value.has_value(), message);
}

template <typename Value>
bool require_error(
    const std::expected<Value, TopologyError>& value,
    const TopologyError error,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == error, message);
}

bool same_incidences(
    const std::span<const EdgeUseIncidence> first,
    const std::span<const EdgeUseIncidence> second) {
    if (first.size() != second.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < first.size(); ++index) {
        if (first[index] != second[index]) {
            return false;
        }
    }
    return true;
}

} // namespace

int main() {
    using apmesh::core::Point2;
    using apmesh::topology::OrientedEndpoints;
    using apmesh::topology::reverse;

    static_assert(!std::same_as<VertexId, EdgeId>);
    static_assert(!std::same_as<FaceId, VertexId>);
    static_assert(!std::same_as<FaceId, EdgeId>);
    static_assert(!std::is_constructible_v<VertexId, std::uint64_t>);
    static_assert(!std::is_constructible_v<EdgeId, std::uint64_t>);
    static_assert(AddsEdge<TopologyBuilder, TopologyBuilder::VertexHandle>);
    static_assert(AddsFace<TopologyBuilder>);
    static_assert(EnumeratesEdgeUseIncidences<TopologyModel>);
    static_assert(SummarizesEdgeIncidences<TopologyModel>);
    static_assert(SummarizesTopologyConsistency<TopologyModel>);
    static_assert(EmitsCanonicalTopologySnapshot<TopologyModel>);
    static_assert(!AddsEdge<TopologyBuilder, VertexId>);
    static_assert(!MutableTopologyModel<TopologyModel>);

    bool passed = true;

    TopologyBuilder empty_builder;
    const auto empty_model = empty_builder.finalize();
    const auto empty_summary = empty_model
                                   ? empty_model->consistency_summary()
                                   : std::expected<TopologyConsistencySummary, TopologyError>{
                                         std::unexpected{TopologyError::invalid_model}};
    const auto empty_snapshot = empty_model
                                    ? empty_model->canonical_snapshot()
                                    : std::expected<std::string, TopologyError>{
                                          std::unexpected{TopologyError::invalid_model}};
    passed = require_value(empty_model, "empty topology finalization failed") && passed;
    passed = require(
                 empty_summary && *empty_summary == TopologyConsistencySummary{},
                 "empty topology consistency counts differ") &&
             passed;
    passed = require(
                 empty_snapshot && *empty_snapshot == "apmesh-topology-v1\nvertices 0\nedges 0\nfaces 0\n",
                 "empty topology snapshot differs from the canonical bytes") &&
             passed;

    TopologyBuilder builder;
    const auto first = builder.add_vertex();
    const auto second = builder.add_vertex();
    passed = require_value(first, "first vertex construction failed") && passed;
    passed = require_value(second, "second vertex construction failed") && passed;
    if (!first || !second) {
        return 1;
    }

    const auto coincident_first = Point2::make(1.0, -2.0);
    const auto coincident_second = Point2::make(1.0, -2.0);
    passed = require(
                 coincident_first && coincident_second && *coincident_first == *coincident_second,
                 "coincident external test coordinates differ") &&
             passed;
    passed = require(
                 first->id().value() == 1U && second->id().value() == 2U &&
                     first->id() != second->id(),
                 "distinct coincident vertices did not receive deterministic identities") &&
             passed;

    const auto invalid_edge = builder.add_edge(TopologyBuilder::VertexHandle{}, *first);
    passed = require_error(
                 invalid_edge,
                 TopologyError::invalid_vertex_handle,
                 "invalid vertex handle was accepted") &&
             passed;

    const auto first_edge = builder.add_edge(*first, *second);
    const auto parallel_edge = builder.add_edge(*first, *second);
    passed = require_value(first_edge, "first edge construction failed") && passed;
    passed = require_value(parallel_edge, "parallel edge construction failed") && passed;
    if (!first_edge || !parallel_edge) {
        return 1;
    }
    passed = require(
                 first_edge->value() == 1U && parallel_edge->value() == 2U &&
                     *first_edge != *parallel_edge,
                 "failed edge insertion consumed identity or parallel edges collapsed") &&
             passed;

    TopologyBuilder foreign_builder;
    const auto foreign_vertex = foreign_builder.add_vertex();
    passed = require_value(foreign_vertex, "foreign vertex construction failed") && passed;
    if (!foreign_vertex) {
        return 1;
    }
    const auto foreign_edge = builder.add_edge(*foreign_vertex, *first);
    passed = require_error(
                 foreign_edge,
                 TopologyError::invalid_vertex_handle,
                 "foreign builder vertex handle was accepted") &&
             passed;

    const auto reverse_edge = builder.add_edge(*second, *first);
    passed = require_value(reverse_edge, "reverse edge construction failed") && passed;
    passed = require(
                 reverse_edge && reverse_edge->value() == 3U,
                 "foreign builder handle consumed an edge identity") &&
             passed;

    const auto model = builder.finalize();
    passed = require_value(model, "topology finalization failed") && passed;
    if (!model) {
        return 1;
    }
    passed = require(
                 model->vertices().size() == 2U && model->edges().size() == 3U,
                 "finalized topology cardinalities differ") &&
             passed;

    const auto stored_edge = model->edge(*first_edge);
    passed = require_value(stored_edge, "stored edge lookup failed") && passed;
    passed = require(
                 stored_edge && stored_edge->id() == *first_edge &&
                     stored_edge->first() == first->id() && stored_edge->second() == second->id(),
                 "stored edge endpoints differ") &&
             passed;
    passed = require_error(
                 model->edge(EdgeId{}),
                 TopologyError::invalid_edge_id,
                 "zero edge identifier resolved") &&
             passed;

    const EdgeUse forward{.edge = *first_edge, .orientation = Orientation::forward};
    const auto reversed = reverse(forward);
    const auto restored = reversed ? reverse(*reversed)
                                   : std::expected<EdgeUse, TopologyError>{
                                         std::unexpected{TopologyError::invalid_orientation}};
    const auto forward_endpoints = model->resolve(forward);
    const auto reverse_endpoints = reversed
                                       ? model->resolve(*reversed)
                                       : std::expected<OrientedEndpoints, TopologyError>{
                                             std::unexpected{TopologyError::invalid_orientation}};
    passed = require_value(reversed, "forward edge use did not reverse") && passed;
    passed = require_value(restored, "reversed edge use did not reverse") && passed;
    passed = require_value(forward_endpoints, "forward edge use did not resolve") && passed;
    passed = require_value(reverse_endpoints, "reverse edge use did not resolve") && passed;
    passed = require(
                 restored && *restored == forward,
                 "edge-use reversal is not an involution") &&
             passed;
    passed = require(
                 forward_endpoints && reverse_endpoints &&
                     *forward_endpoints == OrientedEndpoints{.start = first->id(), .end = second->id()} &&
                     *reverse_endpoints == OrientedEndpoints{.start = second->id(), .end = first->id()},
                 "edge-use orientation did not control endpoint order") &&
             passed;
    passed = require_error(
                 model->resolve(EdgeUse{}),
                 TopologyError::invalid_edge_id,
                 "default edge use resolved") &&
             passed;

    const EdgeUse malformed{
        .edge = *first_edge,
        .orientation = static_cast<Orientation>(42)};
    passed = require_error(
                 reverse(malformed),
                 TopologyError::invalid_orientation,
                 "invalid orientation reversed") &&
             passed;
    passed = require_error(
                 model->resolve(malformed),
                 TopologyError::invalid_orientation,
                 "invalid orientation resolved") &&
             passed;

    const std::array<std::span<const EdgeUse>, 0U> no_boundary_loops{};
    passed = require_error(
                 builder.add_face(no_boundary_loops),
                 TopologyError::empty_face_boundary,
                 "boundaryless face was accepted") &&
             passed;

    const std::array<EdgeUse, 0U> empty_loop{};
    const std::array<std::span<const EdgeUse>, 1U> one_empty_loop{
        std::span<const EdgeUse>{empty_loop},
    };
    passed = require_error(
                 builder.add_face(one_empty_loop),
                 TopologyError::empty_boundary_loop,
                 "empty boundary loop was accepted") &&
             passed;

    const std::array<EdgeUse, 1U> open_loop{forward};
    const std::array<std::span<const EdgeUse>, 1U> one_open_loop{
        std::span<const EdgeUse>{open_loop},
    };
    passed = require_error(
                 builder.add_face(one_open_loop),
                 TopologyError::open_boundary_loop,
                 "open boundary loop was accepted") &&
             passed;

    const std::array<EdgeUse, 1U> missing_edge_loop{
        EdgeUse{.edge = EdgeId{}, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> one_missing_edge_loop{
        std::span<const EdgeUse>{missing_edge_loop},
    };
    passed = require_error(
                 builder.add_face(one_missing_edge_loop),
                 TopologyError::invalid_edge_id,
                 "missing boundary edge was accepted") &&
             passed;

    const std::array<EdgeUse, 1U> malformed_loop{malformed};
    const std::array<std::span<const EdgeUse>, 1U> one_malformed_loop{
        std::span<const EdgeUse>{malformed_loop},
    };
    passed = require_error(
                 builder.add_face(one_malformed_loop),
                 TopologyError::invalid_orientation,
                 "invalid boundary orientation was accepted") &&
             passed;

    const std::array<EdgeUse, 2U> incorrectly_oriented_cycle{
        forward,
        EdgeUse{.edge = *reverse_edge, .orientation = Orientation::reverse},
    };
    const std::array<std::span<const EdgeUse>, 1U> incorrectly_oriented_face{
        std::span<const EdgeUse>{incorrectly_oriented_cycle},
    };
    passed = require_error(
                 builder.add_face(incorrectly_oriented_face),
                 TopologyError::open_boundary_loop,
                 "connected boundary with incorrect orientation was accepted") &&
             passed;

    const std::array<EdgeUse, 2U> two_edge_cycle{
        forward,
        EdgeUse{.edge = *reverse_edge, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> two_edge_face{
        std::span<const EdgeUse>{two_edge_cycle},
    };
    const auto first_face = builder.add_face(two_edge_face);
    const auto second_face = builder.add_face(two_edge_face);
    passed = require_value(first_face, "first face construction failed") && passed;
    passed = require_value(second_face, "identical face construction failed") && passed;
    passed = require(
                 first_face && second_face && first_face->value() == 1U &&
                     second_face->value() == 2U && *first_face != *second_face,
                 "failed face insertion consumed identity or identical faces merged") &&
             passed;

    const auto face_model = builder.finalize();
    passed = require_value(face_model, "face topology finalization failed") && passed;
    passed = require(
                 model->faces().empty() && face_model && face_model->faces().size() == 2U,
                 "finalized topology mutability or face cardinality differs") &&
             passed;
    const auto stored_face = first_face && face_model
                                 ? face_model->face(*first_face)
                                 : std::expected<apmesh::topology::Face, TopologyError>{
                                       std::unexpected{TopologyError::invalid_face_id}};
    passed = require_value(stored_face, "stored face lookup failed") && passed;
    passed = require(
                 stored_face && stored_face->boundary_loops().size() == 1U &&
                     stored_face->boundary_loops().front().uses().size() == 2U &&
                     stored_face->boundary_loops().front().uses().front() == two_edge_cycle.front() &&
                     stored_face->boundary_loops().front().uses().back() == two_edge_cycle.back(),
                 "stored face boundary order differs") &&
             passed;
    passed = require_error(
                 face_model ? face_model->face(FaceId{})
                            : std::expected<apmesh::topology::Face, TopologyError>{
                                  std::unexpected{TopologyError::invalid_face_id}},
                 TopologyError::invalid_face_id,
                 "zero face identifier resolved") &&
             passed;
    const auto face_summary = face_model
                                  ? face_model->consistency_summary()
                                  : std::expected<TopologyConsistencySummary, TopologyError>{
                                        std::unexpected{TopologyError::invalid_model}};
    const auto face_snapshot = face_model
                                   ? face_model->canonical_snapshot()
                                   : std::expected<std::string, TopologyError>{
                                         std::unexpected{TopologyError::invalid_model}};
    passed = require(
                 face_summary && *face_summary == TopologyConsistencySummary{
                     .vertex_count = 2U,
                     .edge_count = 3U,
                     .face_count = 2U,
                     .boundary_loop_count = 2U,
                     .edge_use_count = 4U,
                     .unused_edge_count = 1U,
                     .two_use_cooriented_edge_count = 2U,
                 },
                 "topology consistency summary lost authoritative cardinalities") &&
             passed;
    const std::string expected_face_snapshot{
        "apmesh-topology-v1\n"
        "vertices 2\n"
        "vertex 1\n"
        "vertex 2\n"
        "edges 3\n"
        "edge 1 1 2\n"
        "edge 2 1 2\n"
        "edge 3 2 1\n"
        "faces 2\n"
        "face 1 1\n"
        "loop 1 0 2\n"
        "use 1 0 0 1 forward\n"
        "use 1 0 1 3 forward\n"
        "face 2 1\n"
        "loop 2 0 2\n"
        "use 2 0 0 1 forward\n"
        "use 2 0 1 3 forward\n"};
    passed = require(
                 face_snapshot && *face_snapshot == expected_face_snapshot &&
                     face_snapshot->find('\r') == std::string::npos &&
                     face_snapshot->back() == '\n',
                 "canonical snapshot record order or line endings differ") &&
             passed;
    const auto unused_parallel_incidences = parallel_edge && face_model
                                                ? face_model->edge_use_incidences(*parallel_edge)
                                                : std::expected<
                                                      std::span<const EdgeUseIncidence>,
                                                      TopologyError>{std::unexpected{
                                                      TopologyError::invalid_edge_id}};
    passed = require(
                 unused_parallel_incidences && unused_parallel_incidences->empty(),
                 "unused valid edge did not produce an empty incidence sequence") &&
             passed;
    const auto unused_parallel_signature = parallel_edge && face_model
                                               ? face_model->edge_incidence_signature(*parallel_edge)
                                               : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                     std::unexpected{TopologyError::invalid_edge_id}};
    passed = require(
                 unused_parallel_signature &&
                     *unused_parallel_signature == EdgeIncidenceSignature{
                         .classification = EdgeIncidenceClass::unused,
                     },
                 "unused edge signature differs from exact zero-incidence facts") &&
             passed;
    const auto zero_incidences = face_model
                                     ? face_model->edge_use_incidences(EdgeId{})
                                     : std::expected<std::span<const EdgeUseIncidence>, TopologyError>{
                                           std::unexpected{TopologyError::invalid_edge_id}};
    passed = require_error(
                 zero_incidences,
                 TopologyError::invalid_edge_id,
                 "zero edge identifier resolved in incidence lookup") &&
             passed;
    const auto zero_signature = face_model
                                    ? face_model->edge_incidence_signature(EdgeId{})
                                    : std::expected<EdgeIncidenceSignature, TopologyError>{
                                          std::unexpected{TopologyError::invalid_edge_id}};
    passed = require_error(
                 zero_signature,
                 TopologyError::invalid_edge_id,
                 "zero edge identifier resolved in signature lookup") &&
             passed;
    const auto first_edge_incidences = first_edge && face_model
                                           ? face_model->edge_use_incidences(*first_edge)
                                           : std::expected<
                                                 std::span<const EdgeUseIncidence>,
                                                 TopologyError>{std::unexpected{
                                                 TopologyError::invalid_edge_id}};
    passed = require(
                 first_edge_incidences && first_edge_incidences->size() == 2U && first_face &&
                     second_face && (*first_edge_incidences)[0U] == EdgeUseIncidence{
                         .face = *first_face,
                         .boundary_loop_ordinal = 0U,
                         .edge_use_ordinal = 0U,
                         .orientation = Orientation::forward,
                     } && (*first_edge_incidences)[1U] == EdgeUseIncidence{
                         .face = *second_face,
                         .boundary_loop_ordinal = 0U,
                         .edge_use_ordinal = 0U,
                         .orientation = Orientation::forward,
                     },
                 "face edge-use incidence order or positional resolution differs") &&
             passed;
    const auto first_edge_signature = first_edge && face_model
                                          ? face_model->edge_incidence_signature(*first_edge)
                                          : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                std::unexpected{TopologyError::invalid_edge_id}};
    passed = require(
                 first_edge_signature &&
                     *first_edge_signature == EdgeIncidenceSignature{
                         .occurrence_count = 2U,
                         .distinct_face_count = 2U,
                         .distinct_boundary_count = 2U,
                         .forward_count = 2U,
                         .reverse_count = 0U,
                         .has_repeated_face = false,
                         .has_repeated_boundary = false,
                         .classification = EdgeIncidenceClass::two_use_cooriented,
                     } && first_edge_incidences && first_edge_incidences->size() == 2U &&
                     (*first_edge_incidences)[0U].face == *first_face &&
                     (*first_edge_incidences)[1U].face == *second_face,
                 "cooriented two-use signature or underlying incidence order differs") &&
             passed;

    TopologyBuilder loop_builder;
    const auto loop_vertex = loop_builder.add_vertex();
    const auto loop_edge = loop_vertex ? loop_builder.add_edge(*loop_vertex, *loop_vertex)
                                       : std::expected<EdgeId, TopologyError>{
                                             std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto loop_model = loop_builder.finalize();
    passed = require_value(loop_vertex, "self-loop vertex construction failed") && passed;
    passed = require_value(loop_edge, "self-loop edge construction failed") && passed;
    passed = require_value(loop_model, "self-loop topology finalization failed") && passed;
    if (loop_vertex && loop_edge && loop_model) {
        const EdgeUse loop_forward{.edge = *loop_edge, .orientation = Orientation::forward};
        const auto loop_reversed = reverse(loop_forward);
        const auto loop_forward_endpoints = loop_model->resolve(loop_forward);
        const auto loop_reverse_endpoints = loop_reversed
                                                ? loop_model->resolve(*loop_reversed)
                                                : std::expected<OrientedEndpoints, TopologyError>{
                                                      std::unexpected{TopologyError::invalid_orientation}};
        passed = require(
                     loop_forward_endpoints && loop_reverse_endpoints &&
                         *loop_forward_endpoints == *loop_reverse_endpoints &&
                         loop_forward_endpoints->start == loop_vertex->id() &&
                         loop_forward_endpoints->end == loop_vertex->id(),
                     "self-loop was not retained as explicit topology") &&
                 passed;

        const std::array<EdgeUse, 1U> self_loop_cycle{loop_forward};
        const std::array<std::span<const EdgeUse>, 1U> self_loop_face{
            std::span<const EdgeUse>{self_loop_cycle},
        };
        const auto self_loop_face_id = loop_builder.add_face(self_loop_face);
        const auto self_loop_face_model = loop_builder.finalize();
        passed = require_value(self_loop_face_id, "self-loop face construction failed") && passed;
        passed = require_value(
                     self_loop_face_model,
                     "self-loop face topology finalization failed") &&
                 passed;
        passed = require(
                     self_loop_face_id && self_loop_face_model &&
                         self_loop_face_model->faces().size() == 1U &&
                         self_loop_face_model->faces().front().boundary_loops().front().uses().size() ==
                             1U,
                     "one-use self-loop boundary was not retained") &&
                 passed;
        const auto self_loop_incidences = self_loop_face_model
                                              ? self_loop_face_model->edge_use_incidences(*loop_edge)
                                              : std::expected<
                                                    std::span<const EdgeUseIncidence>,
                                                    TopologyError>{std::unexpected{
                                                    TopologyError::invalid_edge_id}};
        passed = require(
                     self_loop_incidences && self_loop_face_id && self_loop_incidences->size() == 1U &&
                         (*self_loop_incidences)[0U] == EdgeUseIncidence{
                             .face = *self_loop_face_id,
                             .boundary_loop_ordinal = 0U,
                             .edge_use_ordinal = 0U,
                             .orientation = Orientation::forward,
                         },
                     "single edge-use incidence was not retained exactly") &&
                 passed;
        const auto self_loop_signature = self_loop_face_model
                                             ? self_loop_face_model->edge_incidence_signature(*loop_edge)
                                             : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                   std::unexpected{TopologyError::invalid_edge_id}};
        passed = require(
                     self_loop_signature &&
                         *self_loop_signature == EdgeIncidenceSignature{
                             .occurrence_count = 1U,
                             .distinct_face_count = 1U,
                             .distinct_boundary_count = 1U,
                             .forward_count = 1U,
                             .reverse_count = 0U,
                             .has_repeated_face = false,
                             .has_repeated_boundary = false,
                             .classification = EdgeIncidenceClass::single_use,
                         },
                     "single-use signature differs from exact incidence facts") &&
                 passed;
        const auto out_of_range_face = second_face
                                           ? self_loop_face_model->face(*second_face)
                                           : std::expected<apmesh::topology::Face, TopologyError>{
                                                 std::unexpected{TopologyError::invalid_face_id}};
        passed = require_error(
                     out_of_range_face,
                     TopologyError::invalid_face_id,
                     "out-of-range face identifier resolved") &&
                 passed;
    }

    TopologyBuilder repeat_builder;
    const auto repeat_first = repeat_builder.add_vertex();
    const auto repeat_second = repeat_builder.add_vertex();
    const auto repeat_edge = repeat_first && repeat_second
                                 ? repeat_builder.add_edge(*repeat_first, *repeat_second)
                                 : std::expected<EdgeId, TopologyError>{
                                       std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto repeat_parallel = repeat_first && repeat_second
                                     ? repeat_builder.add_edge(*repeat_first, *repeat_second)
                                     : std::expected<EdgeId, TopologyError>{
                                           std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto repeat_reverse = repeat_first && repeat_second
                                    ? repeat_builder.add_edge(*repeat_second, *repeat_first)
                                    : std::expected<EdgeId, TopologyError>{
                                          std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto repeat_model = repeat_builder.finalize();
    passed = require(
                 repeat_first && repeat_second && repeat_edge && repeat_parallel && repeat_reverse &&
                     repeat_model && repeat_first->id() == first->id() &&
                     repeat_second->id() == second->id() && *repeat_edge == *first_edge &&
                     *repeat_parallel == *parallel_edge && *repeat_reverse == *reverse_edge &&
                     repeat_model->edges().size() == model->edges().size(),
                 "repeated insertion sequence changed topology claim fields") &&
             passed;

    if (repeat_edge && repeat_reverse && repeat_model) {
        const std::array<EdgeUse, 2U> repeated_construction_cycle{
            EdgeUse{.edge = *repeat_edge, .orientation = Orientation::forward},
            EdgeUse{.edge = *repeat_reverse, .orientation = Orientation::forward},
        };
        const std::array<std::span<const EdgeUse>, 1U> repeated_construction_face{
            std::span<const EdgeUse>{repeated_construction_cycle},
        };
        const auto repeated_construction_first = repeat_builder.add_face(repeated_construction_face);
        const auto repeated_construction_second = repeat_builder.add_face(repeated_construction_face);
        const auto repeated_construction_model = repeat_builder.finalize();
        passed = require_value(
                     repeated_construction_first,
                     "repeated-construction first face failed") &&
                 passed;
        passed = require_value(
                     repeated_construction_second,
                     "repeated-construction second face failed") &&
                 passed;
        passed = require_value(
                     repeated_construction_model,
                     "repeated-construction finalization failed") &&
                 passed;
        passed = require(
                     face_model && repeated_construction_first && repeated_construction_second &&
                         repeated_construction_model &&
                         repeated_construction_model->faces().size() == face_model->faces().size() &&
                         repeated_construction_model->faces()[0U].id() == face_model->faces()[0U].id() &&
                         repeated_construction_model->faces()[1U].id() == face_model->faces()[1U].id() &&
                         repeated_construction_model->faces()[0U].boundary_loops().front().uses().front() ==
                             face_model->faces()[0U].boundary_loops().front().uses().front() &&
                         repeated_construction_model->faces()[0U].boundary_loops().front().uses().back() ==
                             face_model->faces()[0U].boundary_loops().front().uses().back() &&
                         repeated_construction_model->faces()[1U].boundary_loops().front().uses().front() ==
                             face_model->faces()[1U].boundary_loops().front().uses().front() &&
                         repeated_construction_model->faces()[1U].boundary_loops().front().uses().back() ==
                             face_model->faces()[1U].boundary_loops().front().uses().back(),
                     "independent construction did not reproduce face claim fields") &&
                 passed;
        const auto original_incidences = first_edge && face_model
                                             ? face_model->edge_use_incidences(*first_edge)
                                             : std::expected<
                                                   std::span<const EdgeUseIncidence>,
                                                   TopologyError>{std::unexpected{
                                                   TopologyError::invalid_edge_id}};
        const auto reconstructed_incidences = repeat_edge && repeated_construction_model
                                                  ? repeated_construction_model->edge_use_incidences(
                                                        *repeat_edge)
                                                  : std::expected<
                                                        std::span<const EdgeUseIncidence>,
                                                        TopologyError>{std::unexpected{
                                                        TopologyError::invalid_edge_id}};
        passed = require(
                     original_incidences && reconstructed_incidences &&
                         same_incidences(*original_incidences, *reconstructed_incidences),
                     "independent construction did not reproduce edge-use incidence records") &&
                 passed;
        const auto original_signature = first_edge && face_model
                                            ? face_model->edge_incidence_signature(*first_edge)
                                            : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                  std::unexpected{TopologyError::invalid_edge_id}};
        const auto reconstructed_signature = repeat_edge && repeated_construction_model
                                                 ? repeated_construction_model->edge_incidence_signature(
                                                       *repeat_edge)
                                                 : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                       std::unexpected{TopologyError::invalid_edge_id}};
        passed = require(
                     original_signature && reconstructed_signature &&
                         *original_signature == *reconstructed_signature,
                     "independent construction did not reproduce incidence signature") &&
                 passed;
        const auto original_summary = face_model
                                          ? face_model->consistency_summary()
                                          : std::expected<TopologyConsistencySummary, TopologyError>{
                                                std::unexpected{TopologyError::invalid_model}};
        const auto reconstructed_summary = repeated_construction_model
                                               ? repeated_construction_model->consistency_summary()
                                               : std::expected<TopologyConsistencySummary, TopologyError>{
                                                     std::unexpected{TopologyError::invalid_model}};
        const auto original_snapshot = face_model
                                           ? face_model->canonical_snapshot()
                                           : std::expected<std::string, TopologyError>{
                                                 std::unexpected{TopologyError::invalid_model}};
        const auto reconstructed_snapshot = repeated_construction_model
                                                ? repeated_construction_model->canonical_snapshot()
                                                : std::expected<std::string, TopologyError>{
                                                      std::unexpected{TopologyError::invalid_model}};
        passed = require(
                     original_summary && reconstructed_summary && original_snapshot &&
                         reconstructed_snapshot && *original_summary == *reconstructed_summary &&
                         *original_snapshot == *reconstructed_snapshot,
                     "independent equal construction did not preserve canonical summary or bytes") &&
                 passed;

        const std::array<EdgeUse, 2U> repeated_edge_cycle{
            EdgeUse{.edge = *repeat_edge, .orientation = Orientation::forward},
            EdgeUse{.edge = *repeat_edge, .orientation = Orientation::reverse},
        };
        const std::array<std::span<const EdgeUse>, 1U> repeated_edge_face{
            std::span<const EdgeUse>{repeated_edge_cycle},
        };
        const auto repeated_first_face = repeat_builder.add_face(repeated_edge_face);
        const auto repeated_second_face = repeat_builder.add_face(repeated_edge_face);
        const auto repeated_third_face = repeat_builder.add_face(repeated_edge_face);
        const auto repeated_face_model = repeat_builder.finalize();
        passed = require_value(
                     repeated_first_face,
                     "repeated-edge first face construction failed") &&
                 passed;
        passed = require_value(
                     repeated_second_face,
                     "repeated-edge second face construction failed") &&
                 passed;
        passed = require_value(
                     repeated_third_face,
                     "repeated-edge third face construction failed") &&
                 passed;
        passed = require_value(
                     repeated_face_model,
                     "repeated-edge face topology finalization failed") &&
                 passed;
        passed = require(
                     repeated_first_face && repeated_second_face && repeated_third_face &&
                         repeated_face_model && repeated_face_model->faces().size() == 5U &&
                         repeated_face_model->faces()[2U].boundary_loops().front().uses().front().edge ==
                             *repeat_edge,
                     "repeated edge or arbitrary face incidence was rejected") &&
                 passed;
        const auto repeated_incidences = repeated_face_model && repeat_edge
                                             ? repeated_face_model->edge_use_incidences(*repeat_edge)
                                             : std::expected<
                                                   std::span<const EdgeUseIncidence>,
                                                   TopologyError>{std::unexpected{
                                                   TopologyError::invalid_edge_id}};
        passed = require(
                     repeated_incidences && repeated_first_face && repeated_second_face &&
                         repeated_third_face && repeated_incidences->size() == 8U &&
                         (*repeated_incidences)[2U] == EdgeUseIncidence{
                             .face = *repeated_first_face,
                             .boundary_loop_ordinal = 0U,
                             .edge_use_ordinal = 0U,
                             .orientation = Orientation::forward,
                         } && (*repeated_incidences)[3U] == EdgeUseIncidence{
                             .face = *repeated_first_face,
                             .boundary_loop_ordinal = 0U,
                             .edge_use_ordinal = 1U,
                             .orientation = Orientation::reverse,
                         } && (*repeated_incidences)[6U] == EdgeUseIncidence{
                             .face = *repeated_third_face,
                             .boundary_loop_ordinal = 0U,
                             .edge_use_ordinal = 0U,
                             .orientation = Orientation::forward,
                         } && (*repeated_incidences)[7U] == EdgeUseIncidence{
                             .face = *repeated_third_face,
                             .boundary_loop_ordinal = 0U,
                             .edge_use_ordinal = 1U,
                             .orientation = Orientation::reverse,
                         },
                     "repeated, opposite-orientation, or three-face incidences were coalesced") &&
                 passed;
        const auto repeated_signature = repeated_face_model
                                            ? repeated_face_model->edge_incidence_signature(*repeat_edge)
                                            : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                  std::unexpected{TopologyError::invalid_edge_id}};
        passed = require(
                     repeated_signature &&
                         *repeated_signature == EdgeIncidenceSignature{
                             .occurrence_count = 8U,
                             .distinct_face_count = 5U,
                             .distinct_boundary_count = 5U,
                             .forward_count = 5U,
                             .reverse_count = 3U,
                             .has_repeated_face = true,
                             .has_repeated_boundary = true,
                             .classification = EdgeIncidenceClass::multi_use,
                         },
                     "multi-use signature lost repeated-owner or orientation facts") &&
                 passed;
    }

    TopologyBuilder multi_loop_builder;
    const auto first_loop_vertex = multi_loop_builder.add_vertex();
    const auto second_loop_vertex = multi_loop_builder.add_vertex();
    const auto first_loop_edge = first_loop_vertex
                                     ? multi_loop_builder.add_edge(*first_loop_vertex, *first_loop_vertex)
                                     : std::expected<EdgeId, TopologyError>{
                                           std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto second_loop_edge = second_loop_vertex
                                      ? multi_loop_builder.add_edge(*second_loop_vertex, *second_loop_vertex)
                                      : std::expected<EdgeId, TopologyError>{
                                            std::unexpected{TopologyError::invalid_vertex_handle}};
    passed = require_value(first_loop_vertex, "first multiple-loop vertex construction failed") && passed;
    passed = require_value(second_loop_vertex, "second multiple-loop vertex construction failed") && passed;
    passed = require_value(first_loop_edge, "first multiple-loop edge construction failed") && passed;
    passed = require_value(second_loop_edge, "second multiple-loop edge construction failed") && passed;
    if (first_loop_edge && second_loop_edge) {
        const std::array<EdgeUse, 1U> first_boundary{
            EdgeUse{.edge = *first_loop_edge, .orientation = Orientation::forward},
        };
        const std::array<EdgeUse, 1U> second_boundary{
            EdgeUse{.edge = *first_loop_edge, .orientation = Orientation::forward},
        };
        const std::array<std::span<const EdgeUse>, 2U> multiple_boundaries{
            std::span<const EdgeUse>{first_boundary},
            std::span<const EdgeUse>{second_boundary},
        };
        const auto multi_loop_face = multi_loop_builder.add_face(multiple_boundaries);
        const auto multi_loop_model = multi_loop_builder.finalize();
        passed = require_value(multi_loop_face, "multiple-loop face construction failed") && passed;
        passed = require_value(multi_loop_model, "multiple-loop topology finalization failed") && passed;
        passed = require(
                     multi_loop_model && multi_loop_model->faces().size() == 1U &&
                         multi_loop_model->faces().front().boundary_loops().size() == 2U &&
                         multi_loop_model->faces().front().boundary_loops()[0U].uses().front().edge ==
                             *first_loop_edge &&
                         multi_loop_model->faces().front().boundary_loops()[1U].uses().front().edge ==
                             *first_loop_edge,
                     "multiple boundary loop order was not retained") &&
                 passed;
        const auto first_loop_incidences = multi_loop_model
                                               ? multi_loop_model->edge_use_incidences(*first_loop_edge)
                                               : std::expected<
                                                     std::span<const EdgeUseIncidence>,
                                                     TopologyError>{std::unexpected{
                                                     TopologyError::invalid_edge_id}};
        passed = require(
                     first_loop_incidences && multi_loop_face && first_loop_incidences->size() == 2U &&
                         (*first_loop_incidences)[0U].face == *multi_loop_face &&
                         (*first_loop_incidences)[0U].boundary_loop_ordinal == 0U &&
                         (*first_loop_incidences)[1U].face == *multi_loop_face &&
                         (*first_loop_incidences)[1U].boundary_loop_ordinal == 1U,
                     "multiple boundary-loop incidences did not preserve loop ordinals") &&
                 passed;
        const auto multi_loop_signature = multi_loop_model
                                              ? multi_loop_model->edge_incidence_signature(*first_loop_edge)
                                              : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                    std::unexpected{TopologyError::invalid_edge_id}};
        passed = require(
                     multi_loop_signature &&
                         *multi_loop_signature == EdgeIncidenceSignature{
                             .occurrence_count = 2U,
                             .distinct_face_count = 1U,
                             .distinct_boundary_count = 2U,
                             .forward_count = 2U,
                             .reverse_count = 0U,
                             .has_repeated_face = true,
                             .has_repeated_boundary = false,
                             .classification = EdgeIncidenceClass::two_use_cooriented,
                         },
                     "multiple-loop signature lost exact owner cardinalities") &&
                 passed;
    }

    TopologyBuilder opposed_builder;
    const auto opposed_vertex = opposed_builder.add_vertex();
    const auto opposed_edge = opposed_vertex
                                  ? opposed_builder.add_edge(*opposed_vertex, *opposed_vertex)
                                  : std::expected<EdgeId, TopologyError>{
                                        std::unexpected{TopologyError::invalid_vertex_handle}};
    if (opposed_edge) {
        const std::array<EdgeUse, 2U> opposed_cycle{
            EdgeUse{.edge = *opposed_edge, .orientation = Orientation::forward},
            EdgeUse{.edge = *opposed_edge, .orientation = Orientation::reverse},
        };
        const std::array<std::span<const EdgeUse>, 1U> opposed_boundary{
            std::span<const EdgeUse>{opposed_cycle},
        };
        const auto opposed_face = opposed_builder.add_face(opposed_boundary);
        const auto opposed_model = opposed_builder.finalize();
        const auto opposed_signature = opposed_model
                                           ? opposed_model->edge_incidence_signature(*opposed_edge)
                                           : std::expected<EdgeIncidenceSignature, TopologyError>{
                                                 std::unexpected{TopologyError::invalid_edge_id}};
        passed = require_value(opposed_face, "opposed two-use face construction failed") && passed;
        passed = require_value(opposed_model, "opposed two-use finalization failed") && passed;
        passed = require(
                     opposed_signature &&
                         *opposed_signature == EdgeIncidenceSignature{
                             .occurrence_count = 2U,
                             .distinct_face_count = 1U,
                             .distinct_boundary_count = 1U,
                             .forward_count = 1U,
                             .reverse_count = 1U,
                             .has_repeated_face = true,
                             .has_repeated_boundary = true,
                             .classification = EdgeIncidenceClass::two_use_opposed,
                         },
                     "opposed two-use signature differs from exact same-face facts") &&
                 passed;
    } else {
        passed = false;
    }

    TopologyBuilder valence_builder;
    const auto valence_zero = valence_builder.add_vertex();
    const auto valence_one = valence_builder.add_vertex();
    const auto valence_two = valence_builder.add_vertex();
    const auto valence_three = valence_builder.add_vertex();
    const auto valence_four = valence_builder.add_vertex();
    passed = require_value(valence_zero, "valence vertex zero construction failed") && passed;
    passed = require_value(valence_one, "valence vertex one construction failed") && passed;
    passed = require_value(valence_two, "valence vertex two construction failed") && passed;
    passed = require_value(valence_three, "valence vertex three construction failed") && passed;
    passed = require_value(valence_four, "valence vertex four construction failed") && passed;
    if (valence_zero && valence_one && valence_two && valence_three && valence_four) {
        const auto triangle_zero_one = valence_builder.add_edge(*valence_zero, *valence_one);
        const auto triangle_one_two = valence_builder.add_edge(*valence_one, *valence_two);
        const auto triangle_two_zero = valence_builder.add_edge(*valence_two, *valence_zero);
        const auto pentagon_zero_one = valence_builder.add_edge(*valence_zero, *valence_one);
        const auto pentagon_one_two = valence_builder.add_edge(*valence_one, *valence_two);
        const auto pentagon_two_three = valence_builder.add_edge(*valence_two, *valence_three);
        const auto pentagon_three_four = valence_builder.add_edge(*valence_three, *valence_four);
        const auto pentagon_four_zero = valence_builder.add_edge(*valence_four, *valence_zero);
        passed = require(
                     triangle_zero_one && triangle_one_two && triangle_two_zero && pentagon_zero_one &&
                         pentagon_one_two && pentagon_two_three && pentagon_three_four &&
                         pentagon_four_zero,
                     "valence edge construction failed") &&
                 passed;
        if (triangle_zero_one && triangle_one_two && triangle_two_zero && pentagon_zero_one &&
            pentagon_one_two && pentagon_two_three && pentagon_three_four && pentagon_four_zero) {
            const std::array<EdgeUse, 3U> triangle_cycle{
                EdgeUse{.edge = *triangle_zero_one, .orientation = Orientation::forward},
                EdgeUse{.edge = *triangle_one_two, .orientation = Orientation::forward},
                EdgeUse{.edge = *triangle_two_zero, .orientation = Orientation::forward},
            };
            const std::array<EdgeUse, 5U> pentagon_cycle{
                EdgeUse{.edge = *pentagon_zero_one, .orientation = Orientation::forward},
                EdgeUse{.edge = *pentagon_one_two, .orientation = Orientation::forward},
                EdgeUse{.edge = *pentagon_two_three, .orientation = Orientation::forward},
                EdgeUse{.edge = *pentagon_three_four, .orientation = Orientation::forward},
                EdgeUse{.edge = *pentagon_four_zero, .orientation = Orientation::forward},
            };
            const std::array<std::span<const EdgeUse>, 1U> triangle_face{
                std::span<const EdgeUse>{triangle_cycle},
            };
            const std::array<std::span<const EdgeUse>, 1U> pentagon_face{
                std::span<const EdgeUse>{pentagon_cycle},
            };
            const auto triangle_id = valence_builder.add_face(triangle_face);
            const auto pentagon_id = valence_builder.add_face(pentagon_face);
            const auto valence_model = valence_builder.finalize();
            passed = require_value(triangle_id, "triangular face construction failed") && passed;
            passed = require_value(pentagon_id, "five-edge face construction failed") && passed;
            passed = require_value(valence_model, "valence topology finalization failed") && passed;
            passed = require(
                         triangle_id && pentagon_id && valence_model &&
                             valence_model->faces().size() == 2U &&
                             valence_model->faces()[0U].boundary_loops().front().uses().size() == 3U &&
                             valence_model->faces()[1U].boundary_loops().front().uses().size() == 5U,
                         "arbitrary positive boundary valence was not retained") &&
                     passed;
        }
    }

    TopologyBuilder classification_builder;
    const auto classification_vertex = classification_builder.add_vertex();
    const auto unused_edge = classification_vertex
                                 ? classification_builder.add_edge(*classification_vertex, *classification_vertex)
                                 : std::expected<EdgeId, TopologyError>{
                                       std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto single_edge = classification_vertex
                                 ? classification_builder.add_edge(*classification_vertex, *classification_vertex)
                                 : std::expected<EdgeId, TopologyError>{
                                       std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto classification_opposed_edge = classification_vertex
                                                 ? classification_builder.add_edge(
                                                       *classification_vertex,
                                                       *classification_vertex)
                                                 : std::expected<EdgeId, TopologyError>{
                                                       std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto cooriented_edge = classification_vertex
                                     ? classification_builder.add_edge(*classification_vertex, *classification_vertex)
                                     : std::expected<EdgeId, TopologyError>{
                                           std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto multi_edge = classification_vertex
                                ? classification_builder.add_edge(*classification_vertex, *classification_vertex)
                                : std::expected<EdgeId, TopologyError>{
                                      std::unexpected{TopologyError::invalid_vertex_handle}};
    passed = require(
                 classification_vertex && unused_edge && single_edge && classification_opposed_edge &&
                     cooriented_edge && multi_edge,
                 "classification-summary topology construction failed") &&
             passed;
    if (single_edge && classification_opposed_edge && cooriented_edge && multi_edge) {
        const std::array<EdgeUse, 1U> single_cycle{
            EdgeUse{.edge = *single_edge, .orientation = Orientation::forward},
        };
        const std::array<EdgeUse, 2U> opposed_cycle{
            EdgeUse{.edge = *classification_opposed_edge, .orientation = Orientation::forward},
            EdgeUse{.edge = *classification_opposed_edge, .orientation = Orientation::reverse},
        };
        const std::array<EdgeUse, 1U> cooriented_cycle{
            EdgeUse{.edge = *cooriented_edge, .orientation = Orientation::forward},
        };
        const std::array<EdgeUse, 1U> multi_cycle{
            EdgeUse{.edge = *multi_edge, .orientation = Orientation::forward},
        };
        const std::array<std::span<const EdgeUse>, 1U> single_boundary{
            std::span<const EdgeUse>{single_cycle},
        };
        const std::array<std::span<const EdgeUse>, 1U> opposed_boundary{
            std::span<const EdgeUse>{opposed_cycle},
        };
        const std::array<std::span<const EdgeUse>, 1U> cooriented_boundary{
            std::span<const EdgeUse>{cooriented_cycle},
        };
        const std::array<std::span<const EdgeUse>, 1U> multi_boundary{
            std::span<const EdgeUse>{multi_cycle},
        };
        const auto single_face = classification_builder.add_face(single_boundary);
        const auto opposed_face = classification_builder.add_face(opposed_boundary);
        const auto cooriented_first_face = classification_builder.add_face(cooriented_boundary);
        const auto cooriented_second_face = classification_builder.add_face(cooriented_boundary);
        const auto multi_first_face = classification_builder.add_face(multi_boundary);
        const auto multi_second_face = classification_builder.add_face(multi_boundary);
        const auto multi_third_face = classification_builder.add_face(multi_boundary);
        const auto classification_model = classification_builder.finalize();
        const auto classification_summary = classification_model
                                                ? classification_model->consistency_summary()
                                                : std::expected<TopologyConsistencySummary, TopologyError>{
                                                      std::unexpected{TopologyError::invalid_model}};
        passed = require(
                     single_face && opposed_face && cooriented_first_face && cooriented_second_face &&
                         multi_first_face && multi_second_face && multi_third_face && classification_summary &&
                         *classification_summary == TopologyConsistencySummary{
                             .vertex_count = 1U,
                             .edge_count = 5U,
                             .face_count = 7U,
                             .boundary_loop_count = 7U,
                             .edge_use_count = 8U,
                             .unused_edge_count = 1U,
                             .single_use_edge_count = 1U,
                             .two_use_opposed_edge_count = 1U,
                             .two_use_cooriented_edge_count = 1U,
                             .multi_use_edge_count = 1U,
                         },
                     "consistency summary did not count all structural signature classes") &&
                 passed;
    }

    TopologyBuilder forward_snapshot_builder;
    TopologyBuilder reverse_snapshot_builder;
    const auto forward_vertex = forward_snapshot_builder.add_vertex();
    const auto reverse_vertex = reverse_snapshot_builder.add_vertex();
    const auto forward_snapshot_edge = forward_vertex
                                           ? forward_snapshot_builder.add_edge(*forward_vertex, *forward_vertex)
                                           : std::expected<EdgeId, TopologyError>{
                                                 std::unexpected{TopologyError::invalid_vertex_handle}};
    const auto reverse_snapshot_edge = reverse_vertex
                                           ? reverse_snapshot_builder.add_edge(*reverse_vertex, *reverse_vertex)
                                           : std::expected<EdgeId, TopologyError>{
                                                 std::unexpected{TopologyError::invalid_vertex_handle}};
    if (forward_snapshot_edge && reverse_snapshot_edge) {
        const std::array<EdgeUse, 1U> forward_cycle{
            EdgeUse{.edge = *forward_snapshot_edge, .orientation = Orientation::forward},
        };
        const std::array<EdgeUse, 1U> reverse_cycle{
            EdgeUse{.edge = *reverse_snapshot_edge, .orientation = Orientation::reverse},
        };
        const std::array<std::span<const EdgeUse>, 1U> forward_boundary{
            std::span<const EdgeUse>{forward_cycle},
        };
        const std::array<std::span<const EdgeUse>, 1U> reverse_boundary{
            std::span<const EdgeUse>{reverse_cycle},
        };
        const auto forward_face = forward_snapshot_builder.add_face(forward_boundary);
        const auto reverse_face = reverse_snapshot_builder.add_face(reverse_boundary);
        const auto forward_snapshot_model = forward_snapshot_builder.finalize();
        const auto reverse_snapshot_model = reverse_snapshot_builder.finalize();
        const auto forward_snapshot = forward_snapshot_model
                                          ? forward_snapshot_model->canonical_snapshot()
                                          : std::expected<std::string, TopologyError>{
                                                std::unexpected{TopologyError::invalid_model}};
        const auto reverse_snapshot = reverse_snapshot_model
                                          ? reverse_snapshot_model->canonical_snapshot()
                                          : std::expected<std::string, TopologyError>{
                                                std::unexpected{TopologyError::invalid_model}};
        passed = require(
                     forward_face && reverse_face && forward_snapshot && reverse_snapshot &&
                         *forward_snapshot != *reverse_snapshot,
                     "declared orientation did not change canonical snapshot bytes") &&
                 passed;
    } else {
        passed = false;
    }

    return passed ? 0 : 1;
}
