#include "apmesh/topology/topology.hpp"

#include <array>
#include <concepts>
#include <cstdint>
#include <expected>
#include <fstream>
#include <locale>
#include <sstream>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

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

template <typename Model>
concept MutableTopologyModel = requires(Model& model) {
    model.add_vertex();
};

static_assert(!std::same_as<VertexId, EdgeId>);
static_assert(!std::same_as<VertexId, FaceId>);
static_assert(!std::same_as<EdgeId, FaceId>);
static_assert(!std::is_constructible_v<VertexId, std::uint64_t>);
static_assert(!std::is_constructible_v<EdgeId, std::uint64_t>);
static_assert(!std::is_constructible_v<FaceId, std::uint64_t>);
static_assert(!MutableTopologyModel<TopologyModel>);

std::string error_name(const TopologyError value) {
    switch (value) {
    case TopologyError::invalid_vertex_handle: return "invalid_vertex_handle";
    case TopologyError::invalid_edge_id: return "invalid_edge_id";
    case TopologyError::invalid_face_id: return "invalid_face_id";
    case TopologyError::invalid_orientation: return "invalid_orientation";
    case TopologyError::empty_face_boundary: return "empty_face_boundary";
    case TopologyError::empty_boundary_loop: return "empty_boundary_loop";
    case TopologyError::open_boundary_loop: return "open_boundary_loop";
    case TopologyError::identity_exhausted: return "identity_exhausted";
    case TopologyError::invalid_model: return "invalid_model";
    }
    return "unknown";
}

std::string class_name(const EdgeIncidenceClass value) {
    switch (value) {
    case EdgeIncidenceClass::unused: return "unused";
    case EdgeIncidenceClass::single_use: return "single_use";
    case EdgeIncidenceClass::two_use_opposed: return "two_use_opposed";
    case EdgeIncidenceClass::two_use_cooriented: return "two_use_cooriented";
    case EdgeIncidenceClass::multi_use: return "multi_use";
    }
    return "unknown";
}

std::string orientation_name(const Orientation value) {
    return value == Orientation::forward ? "forward" : "reverse";
}

std::string json_string(const std::string_view text) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << '"';
    for (const unsigned char character : text) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                constexpr char digits[] = "0123456789abcdef";
                output << "\\u00" << digits[(character >> 4U) & 0x0FU] << digits[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    output << '"';
    return output.str();
}

template <typename Value>
std::string expected_error(const std::expected<Value, TopologyError>& value) {
    return value ? "none" : error_name(value.error());
}

std::string summary_json(const TopologyConsistencySummary& value) {
    std::ostringstream output;
    output << "{"
           << "\"vertex_count\":" << value.vertex_count << ","
           << "\"edge_count\":" << value.edge_count << ","
           << "\"face_count\":" << value.face_count << ","
           << "\"boundary_loop_count\":" << value.boundary_loop_count << ","
           << "\"edge_use_count\":" << value.edge_use_count << ","
           << "\"unused_edge_count\":" << value.unused_edge_count << ","
           << "\"single_use_edge_count\":" << value.single_use_edge_count << ","
           << "\"two_use_opposed_edge_count\":" << value.two_use_opposed_edge_count << ","
           << "\"two_use_cooriented_edge_count\":" << value.two_use_cooriented_edge_count << ","
           << "\"multi_use_edge_count\":" << value.multi_use_edge_count
           << "}";
    return output.str();
}

std::string signature_json(const EdgeIncidenceSignature& value) {
    std::ostringstream output;
    output << "{"
           << "\"occurrence_count\":" << value.occurrence_count << ","
           << "\"distinct_face_count\":" << value.distinct_face_count << ","
           << "\"distinct_boundary_count\":" << value.distinct_boundary_count << ","
           << "\"forward_count\":" << value.forward_count << ","
           << "\"reverse_count\":" << value.reverse_count << ","
           << "\"has_repeated_face\":" << (value.has_repeated_face ? "true" : "false") << ","
           << "\"has_repeated_boundary\":" << (value.has_repeated_boundary ? "true" : "false") << ","
           << "\"classification\":" << json_string(class_name(value.classification))
           << "}";
    return output.str();
}

std::string incidences_json(const std::span<const EdgeUseIncidence> values) {
    std::ostringstream output;
    output << '[';
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            output << ',';
        }
        const EdgeUseIncidence& value = values[index];
        output << '['
               << value.face.value() << ','
               << value.boundary_loop_ordinal << ','
               << value.edge_use_ordinal << ','
               << json_string(orientation_name(value.orientation))
               << ']';
    }
    output << ']';
    return output.str();
}

std::expected<TopologyModel, TopologyError> one_loop_model(const Orientation orientation) {
    TopologyBuilder builder;
    const auto vertex = builder.add_vertex();
    if (!vertex) {
        return std::unexpected{vertex.error()};
    }
    const auto edge = builder.add_edge(*vertex, *vertex);
    if (!edge) {
        return std::unexpected{edge.error()};
    }
    const std::array<EdgeUse, 1U> uses{
        EdgeUse{.edge = *edge, .orientation = orientation},
    };
    const std::array<std::span<const EdgeUse>, 1U> loops{
        std::span<const EdgeUse>{uses},
    };
    const auto face = builder.add_face(loops);
    if (!face) {
        return std::unexpected{face.error()};
    }
    return builder.finalize();
}

std::expected<std::size_t, TopologyError> retained_valence(const std::size_t valence) {
    TopologyBuilder builder;
    std::vector<TopologyBuilder::VertexHandle> vertices;
    vertices.reserve(valence);
    for (std::size_t index = 0U; index < valence; ++index) {
        const auto vertex = builder.add_vertex();
        if (!vertex) {
            return std::unexpected{vertex.error()};
        }
        vertices.push_back(*vertex);
    }

    std::vector<EdgeUse> uses;
    uses.reserve(valence);
    for (std::size_t index = 0U; index < valence; ++index) {
        const auto edge = builder.add_edge(vertices[index], vertices[(index + 1U) % valence]);
        if (!edge) {
            return std::unexpected{edge.error()};
        }
        uses.push_back(EdgeUse{.edge = *edge, .orientation = Orientation::forward});
    }

    const std::array<std::span<const EdgeUse>, 1U> loops{
        std::span<const EdgeUse>{uses},
    };
    const auto face = builder.add_face(loops);
    if (!face) {
        return std::unexpected{face.error()};
    }
    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }
    return model->faces().front().boundary_loops().front().uses().size();
}

std::expected<std::string, TopologyError> identity_and_edge_order_case() {
    TopologyBuilder builder;
    const auto first = builder.add_vertex();
    const auto second = builder.add_vertex();
    if (!first || !second) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto edge_a = builder.add_edge(*first, *second);
    const auto edge_b = builder.add_edge(*first, *second);
    const auto edge_c = builder.add_edge(*second, *first);
    const auto edge_d = builder.add_edge(*first, *first);
    if (!edge_a || !edge_b || !edge_c || !edge_d) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }

    std::ostringstream output;
    output << "{\"vertex_ids\":["
           << first->id().value() << ',' << second->id().value()
           << "],\"edge_ids\":["
           << edge_a->value() << ',' << edge_b->value() << ','
           << edge_c->value() << ',' << edge_d->value()
           << "],\"endpoints\":[";
    for (std::size_t index = 0U; index < model->edges().size(); ++index) {
        if (index != 0U) {
            output << ',';
        }
        output << '[' << model->edges()[index].first().value() << ','
               << model->edges()[index].second().value() << ']';
    }
    output << "]}";
    return output.str();
}

std::expected<std::string, TopologyError> transactional_rejection_case() {
    TopologyBuilder builder;
    const auto first = builder.add_vertex();
    const auto second = builder.add_vertex();
    if (!first || !second) {
        return std::unexpected{TopologyError::invalid_model};
    }

    const auto invalid_handle = builder.add_edge(TopologyBuilder::VertexHandle{}, *first);
    const auto first_edge = builder.add_edge(*first, *second);
    if (!first_edge) {
        return std::unexpected{first_edge.error()};
    }

    const std::array<std::span<const EdgeUse>, 0U> no_loops{};
    const auto empty_face = builder.add_face(no_loops);

    const std::array<EdgeUse, 0U> no_uses{};
    const std::array<std::span<const EdgeUse>, 1U> empty_loop_span{
        std::span<const EdgeUse>{no_uses},
    };
    const auto empty_loop = builder.add_face(empty_loop_span);

    const std::array<EdgeUse, 1U> open_uses{
        EdgeUse{.edge = *first_edge, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> open_loop_span{
        std::span<const EdgeUse>{open_uses},
    };
    const auto open_loop = builder.add_face(open_loop_span);

    const auto second_edge = builder.add_edge(*second, *first);
    if (!second_edge) {
        return std::unexpected{second_edge.error()};
    }
    const std::array<EdgeUse, 2U> closed_uses{
        EdgeUse{.edge = *first_edge, .orientation = Orientation::forward},
        EdgeUse{.edge = *second_edge, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> closed_loop_span{
        std::span<const EdgeUse>{closed_uses},
    };
    const auto face = builder.add_face(closed_loop_span);
    if (!face) {
        return std::unexpected{face.error()};
    }

    std::ostringstream output;
    output << "{"
           << "\"invalid_handle\":" << json_string(expected_error(invalid_handle)) << ","
           << "\"empty_face\":" << json_string(expected_error(empty_face)) << ","
           << "\"empty_loop\":" << json_string(expected_error(empty_loop)) << ","
           << "\"open_loop\":" << json_string(expected_error(open_loop)) << ","
           << "\"first_valid_edge_id\":" << first_edge->value() << ","
           << "\"second_valid_edge_id\":" << second_edge->value() << ","
           << "\"first_valid_face_id\":" << face->value()
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> invalid_lookup_case() {
    TopologyBuilder builder;
    const auto vertex = builder.add_vertex();
    if (!vertex) {
        return std::unexpected{vertex.error()};
    }
    const auto edge = builder.add_edge(*vertex, *vertex);
    if (!edge) {
        return std::unexpected{edge.error()};
    }
    const std::array<EdgeUse, 1U> uses{
        EdgeUse{.edge = *edge, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> loops{
        std::span<const EdgeUse>{uses},
    };
    const auto face = builder.add_face(loops);
    if (!face) {
        return std::unexpected{face.error()};
    }
    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }

    const EdgeUse malformed{
        .edge = *edge,
        .orientation = static_cast<Orientation>(42),
    };
    const auto reversed = apmesh::topology::reverse(malformed);
    const auto resolved = model->resolve(malformed);

    std::ostringstream output;
    output << "{"
           << "\"invalid_edge_lookup\":" << json_string(expected_error(model->edge(EdgeId{}))) << ","
           << "\"invalid_face_lookup\":" << json_string(expected_error(model->face(FaceId{}))) << ","
           << "\"invalid_incidence_lookup\":"
           << json_string(expected_error(model->edge_use_incidences(EdgeId{}))) << ","
           << "\"invalid_reverse\":" << json_string(expected_error(reversed)) << ","
           << "\"invalid_resolve\":" << json_string(expected_error(resolved))
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> boundary_valence_case() {
    constexpr std::array<std::size_t, 4U> requested{1U, 2U, 3U, 5U};
    std::array<std::size_t, 4U> retained{};
    for (std::size_t index = 0U; index < requested.size(); ++index) {
        const auto value = retained_valence(requested[index]);
        if (!value) {
            return std::unexpected{value.error()};
        }
        retained[index] = *value;
    }

    std::ostringstream output;
    output << "{\"requested\":[1,2,3,5],\"retained\":[";
    for (std::size_t index = 0U; index < retained.size(); ++index) {
        if (index != 0U) {
            output << ',';
        }
        output << retained[index];
    }
    output << "],\"all_finalized\":true}";
    return output.str();
}

std::expected<std::string, TopologyError> multiple_loops_case() {
    TopologyBuilder builder;
    const auto vertex = builder.add_vertex();
    if (!vertex) {
        return std::unexpected{vertex.error()};
    }
    const auto edge = builder.add_edge(*vertex, *vertex);
    if (!edge) {
        return std::unexpected{edge.error()};
    }
    const std::array<EdgeUse, 1U> first_use{
        EdgeUse{.edge = *edge, .orientation = Orientation::forward},
    };
    const std::array<EdgeUse, 1U> second_use{
        EdgeUse{.edge = *edge, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 2U> loops{
        std::span<const EdgeUse>{first_use},
        std::span<const EdgeUse>{second_use},
    };
    const auto face = builder.add_face(loops);
    if (!face) {
        return std::unexpected{face.error()};
    }
    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }
    const auto incidences = model->edge_use_incidences(*edge);
    const auto signature = model->edge_incidence_signature(*edge);
    if (!incidences || !signature) {
        return std::unexpected{TopologyError::invalid_model};
    }

    std::ostringstream output;
    output << "{\"loop_count\":" << model->faces().front().boundary_loops().size()
           << ",\"incidences\":" << incidences_json(*incidences)
           << ",\"signature\":" << signature_json(*signature)
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> structural_classes_case() {
    TopologyBuilder builder;
    const auto vertex = builder.add_vertex();
    if (!vertex) {
        return std::unexpected{vertex.error()};
    }

    std::array<EdgeId, 5U> edges{};
    for (EdgeId& edge : edges) {
        const auto created = builder.add_edge(*vertex, *vertex);
        if (!created) {
            return std::unexpected{created.error()};
        }
        edge = *created;
    }

    const std::array<EdgeUse, 1U> single{
        EdgeUse{.edge = edges[1], .orientation = Orientation::forward},
    };
    const std::array<EdgeUse, 2U> opposed{
        EdgeUse{.edge = edges[2], .orientation = Orientation::forward},
        EdgeUse{.edge = edges[2], .orientation = Orientation::reverse},
    };
    const std::array<EdgeUse, 1U> cooriented{
        EdgeUse{.edge = edges[3], .orientation = Orientation::forward},
    };
    const std::array<EdgeUse, 1U> multi{
        EdgeUse{.edge = edges[4], .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> single_loop{std::span<const EdgeUse>{single}};
    const std::array<std::span<const EdgeUse>, 1U> opposed_loop{std::span<const EdgeUse>{opposed}};
    const std::array<std::span<const EdgeUse>, 1U> cooriented_loop{std::span<const EdgeUse>{cooriented}};
    const std::array<std::span<const EdgeUse>, 1U> multi_loop{std::span<const EdgeUse>{multi}};

    if (!builder.add_face(single_loop) ||
        !builder.add_face(opposed_loop) ||
        !builder.add_face(cooriented_loop) ||
        !builder.add_face(cooriented_loop) ||
        !builder.add_face(multi_loop) ||
        !builder.add_face(multi_loop) ||
        !builder.add_face(multi_loop)) {
        return std::unexpected{TopologyError::invalid_model};
    }

    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }
    const auto summary = model->consistency_summary();
    if (!summary) {
        return std::unexpected{summary.error()};
    }

    std::ostringstream output;
    output << "{\"classes\":[";
    for (std::size_t index = 0U; index < edges.size(); ++index) {
        const auto signature = model->edge_incidence_signature(edges[index]);
        if (!signature) {
            return std::unexpected{signature.error()};
        }
        if (index != 0U) {
            output << ',';
        }
        output << json_string(class_name(signature->classification));
    }
    output << "],\"summary\":" << summary_json(*summary) << "}";
    return output.str();
}

std::expected<std::string, TopologyError> three_face_multi_use_case() {
    TopologyBuilder builder;
    const auto vertex = builder.add_vertex();
    if (!vertex) {
        return std::unexpected{vertex.error()};
    }
    const auto edge = builder.add_edge(*vertex, *vertex);
    if (!edge) {
        return std::unexpected{edge.error()};
    }
    const std::array<EdgeUse, 1U> use{
        EdgeUse{.edge = *edge, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> loop{std::span<const EdgeUse>{use}};
    for (std::size_t index = 0U; index < 3U; ++index) {
        if (!builder.add_face(loop)) {
            return std::unexpected{TopologyError::invalid_model};
        }
    }
    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }
    const auto incidences = model->edge_use_incidences(*edge);
    const auto signature = model->edge_incidence_signature(*edge);
    if (!incidences || !signature) {
        return std::unexpected{TopologyError::invalid_model};
    }

    std::ostringstream output;
    output << "{\"incidences\":" << incidences_json(*incidences)
           << ",\"signature\":" << signature_json(*signature)
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> incidence_bijection_case() {
    TopologyBuilder builder;
    const auto first = builder.add_vertex();
    const auto second = builder.add_vertex();
    const auto third = builder.add_vertex();
    if (!first || !second || !third) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto edge_a = builder.add_edge(*first, *second);
    const auto edge_b = builder.add_edge(*second, *third);
    const auto edge_c = builder.add_edge(*third, *first);
    if (!edge_a || !edge_b || !edge_c) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const std::array<EdgeUse, 3U> uses{
        EdgeUse{.edge = *edge_a, .orientation = Orientation::forward},
        EdgeUse{.edge = *edge_b, .orientation = Orientation::forward},
        EdgeUse{.edge = *edge_c, .orientation = Orientation::forward},
    };
    const std::array<std::span<const EdgeUse>, 1U> loops{std::span<const EdgeUse>{uses}};
    const auto face = builder.add_face(loops);
    if (!face) {
        return std::unexpected{face.error()};
    }
    const auto model = builder.finalize();
    if (!model) {
        return std::unexpected{model.error()};
    }

    const std::array<EdgeId, 3U> edges{*edge_a, *edge_b, *edge_c};
    std::vector<EdgeUseIncidence> observed;
    for (const EdgeId edge : edges) {
        const auto incidences = model->edge_use_incidences(edge);
        if (!incidences) {
            return std::unexpected{incidences.error()};
        }
        observed.insert(observed.end(), incidences->begin(), incidences->end());
    }
    bool exact = observed.size() == uses.size();
    for (std::size_t index = 0U; exact && index < observed.size(); ++index) {
        exact = observed[index].face == *face &&
                observed[index].boundary_loop_ordinal == 0U &&
                observed[index].edge_use_ordinal == index &&
                observed[index].orientation == Orientation::forward;
    }

    std::ostringstream output;
    output << "{\"forward_occurrences\":3,\"reverse_records\":" << observed.size()
           << ",\"exact_bijection\":" << (exact ? "true" : "false")
           << ",\"incidences\":" << incidences_json(observed)
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> canonical_snapshot_case() {
    const auto model = one_loop_model(Orientation::forward);
    if (!model) {
        return std::unexpected{model.error()};
    }
    const auto snapshot = model->canonical_snapshot();
    if (!snapshot) {
        return std::unexpected{snapshot.error()};
    }
    const bool lf_only = snapshot->find('\r') == std::string::npos;
    const bool final_lf = !snapshot->empty() && snapshot->back() == '\n';

    std::ostringstream output;
    output << "{\"snapshot\":" << json_string(*snapshot)
           << ",\"lf_only\":" << (lf_only ? "true" : "false")
           << ",\"final_lf\":" << (final_lf ? "true" : "false")
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> deterministic_equal_case() {
    const auto first = one_loop_model(Orientation::forward);
    const auto second = one_loop_model(Orientation::forward);
    if (!first || !second) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto first_snapshot = first->canonical_snapshot();
    const auto second_snapshot = second->canonical_snapshot();
    const auto first_summary = first->consistency_summary();
    const auto second_summary = second->consistency_summary();
    if (!first_snapshot || !second_snapshot || !first_summary || !second_summary) {
        return std::unexpected{TopologyError::invalid_model};
    }

    std::ostringstream output;
    output << "{\"snapshot_equal\":" << (*first_snapshot == *second_snapshot ? "true" : "false")
           << ",\"summary_equal\":" << (*first_summary == *second_summary ? "true" : "false")
           << ",\"snapshot\":" << json_string(*first_snapshot)
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> orientation_difference_case() {
    const auto forward = one_loop_model(Orientation::forward);
    const auto reverse = one_loop_model(Orientation::reverse);
    if (!forward || !reverse) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto forward_snapshot = forward->canonical_snapshot();
    const auto reverse_snapshot = reverse->canonical_snapshot();
    if (!forward_snapshot || !reverse_snapshot) {
        return std::unexpected{TopologyError::invalid_model};
    }

    std::ostringstream output;
    output << "{\"different\":" << (*forward_snapshot != *reverse_snapshot ? "true" : "false")
           << ",\"forward\":" << json_string(*forward_snapshot)
           << ",\"reverse\":" << json_string(*reverse_snapshot)
           << "}";
    return output.str();
}

std::expected<std::string, TopologyError> insertion_difference_case() {
    TopologyBuilder first_builder;
    const auto first_a = first_builder.add_vertex();
    const auto first_b = first_builder.add_vertex();
    if (!first_a || !first_b) {
        return std::unexpected{TopologyError::invalid_model};
    }
    if (!first_builder.add_edge(*first_a, *first_b) || !first_builder.add_edge(*first_b, *first_a)) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto first_model = first_builder.finalize();
    if (!first_model) {
        return std::unexpected{first_model.error()};
    }

    TopologyBuilder second_builder;
    const auto second_a = second_builder.add_vertex();
    const auto second_b = second_builder.add_vertex();
    if (!second_a || !second_b) {
        return std::unexpected{TopologyError::invalid_model};
    }
    if (!second_builder.add_edge(*second_b, *second_a) || !second_builder.add_edge(*second_a, *second_b)) {
        return std::unexpected{TopologyError::invalid_model};
    }
    const auto second_model = second_builder.finalize();
    if (!second_model) {
        return std::unexpected{second_model.error()};
    }

    const auto first_snapshot = first_model->canonical_snapshot();
    const auto second_snapshot = second_model->canonical_snapshot();
    if (!first_snapshot || !second_snapshot) {
        return std::unexpected{TopologyError::invalid_model};
    }

    std::ostringstream output;
    output << "{\"different\":" << (*first_snapshot != *second_snapshot ? "true" : "false")
           << ",\"first\":" << json_string(*first_snapshot)
           << ",\"second\":" << json_string(*second_snapshot)
           << "}";
    return output.str();
}

void write_case(
    std::ostream& output,
    bool& first,
    const std::string_view identifier,
    const std::expected<std::string, TopologyError>& observed) {
    if (!observed) {
        throw observed.error();
    }
    if (!first) {
        output << ',';
    }
    first = false;
    output << "{\"id\":" << json_string(identifier) << ",\"observed\":" << *observed << "}";
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 2;
    }

    try {
        output
            << "{\"schema_version\":1,"
            << "\"kind\":\"topological-model-cumulative-certificate\","
            << "\"type_contract\":{"
            << "\"vertex_edge_face_distinct\":true,"
            << "\"raw_id_construction_rejected\":true,"
            << "\"model_has_no_public_add_vertex\":true},"
            << "\"non_claims\":["
            << "\"coordinate_welding\","
            << "\"adjacency\","
            << "\"pairing\","
            << "\"boundary_classification\","
            << "\"manifold_classification\","
            << "\"geometry\","
            << "\"curves\","
            << "\"surfaces\","
            << "\"meshing\"],"
            << "\"cases\":[";

        bool first = true;

        TopologyBuilder empty_builder;
        const auto empty_model = empty_builder.finalize();
        if (!empty_model) {
            return 3;
        }
        const auto empty_summary = empty_model->consistency_summary();
        const auto empty_snapshot = empty_model->canonical_snapshot();
        if (!empty_summary || !empty_snapshot) {
            return 3;
        }
        std::ostringstream empty_observed;
        empty_observed << "{\"summary\":" << summary_json(*empty_summary)
                       << ",\"snapshot\":" << json_string(*empty_snapshot) << "}";
        write_case(output, first, "empty_model", empty_observed.str());

        write_case(output, first, "identity_and_edge_order", identity_and_edge_order_case());
        write_case(output, first, "transactional_rejection", transactional_rejection_case());
        write_case(output, first, "invalid_lookup_and_orientation", invalid_lookup_case());
        write_case(output, first, "boundary_valence", boundary_valence_case());
        write_case(output, first, "multiple_loops_repeated_owner", multiple_loops_case());
        write_case(output, first, "structural_classes", structural_classes_case());
        write_case(output, first, "three_face_multi_use", three_face_multi_use_case());
        write_case(output, first, "incidence_bijection", incidence_bijection_case());
        write_case(output, first, "canonical_snapshot", canonical_snapshot_case());
        write_case(output, first, "deterministic_equal_construction", deterministic_equal_case());
        write_case(output, first, "controlled_orientation_difference", orientation_difference_case());
        write_case(output, first, "controlled_insertion_difference", insertion_difference_case());

        output << "]}";
    } catch (const TopologyError) {
        return 3;
    }

    return output.good() ? 0 : 4;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string_view{argv[1]} != "certificate") {
        return 64;
    }
    return write_certificate(argv[2]);
}
