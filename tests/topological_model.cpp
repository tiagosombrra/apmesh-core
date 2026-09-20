#include "apmesh/core/geometry.hpp"
#include "apmesh/topology/topology.hpp"

#include <concepts>
#include <cstdint>
#include <expected>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

using apmesh::topology::EdgeId;
using apmesh::topology::EdgeUse;
using apmesh::topology::Orientation;
using apmesh::topology::TopologyBuilder;
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

} // namespace

int main() {
    using apmesh::core::Point2;
    using apmesh::topology::OrientedEndpoints;
    using apmesh::topology::reverse;

    static_assert(!std::same_as<VertexId, EdgeId>);
    static_assert(!std::is_constructible_v<VertexId, std::uint64_t>);
    static_assert(!std::is_constructible_v<EdgeId, std::uint64_t>);
    static_assert(AddsEdge<TopologyBuilder, TopologyBuilder::VertexHandle>);
    static_assert(!AddsEdge<TopologyBuilder, VertexId>);
    static_assert(!MutableTopologyModel<TopologyModel>);

    bool passed = true;

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

    return passed ? 0 : 1;
}
