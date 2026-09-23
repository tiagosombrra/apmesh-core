cmake_minimum_required(VERSION 3.25)

project(apmesh_core LANGUAGES CXX)

option(APMESH_USE_LIBCXX "Build apmesh::core with libc++" OFF)
option(
    APMESH_ENABLE_QUALIFICATION_TESTS
    "Register scientific qualification and evidence tests"
    OFF
)

include(CTest)

if(APMESH_ENABLE_QUALIFICATION_TESTS AND NOT BUILD_TESTING)
    message(FATAL_ERROR "APMESH_ENABLE_QUALIFICATION_TESTS requires BUILD_TESTING=ON")
endif()

function(apmesh_apply_target_defaults target)
    target_compile_features(${target} PRIVATE cxx_std_23)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

add_library(apmesh_core STATIC
    src/core/bootstrap.cpp
    src/core/geometry.cpp
    src/core/numeric.cpp
    src/geometry/curve.cpp
    src/geometry/line_segment.cpp
    src/geometry/rational_bezier.cpp
    src/geometry/bspline.cpp
    src/geometry/nurbs.cpp
    src/geometry/multi_span_nurbs.cpp
    src/geometry/surface.cpp
    src/geometry/rational_surface.cpp
    src/geometry/nurbs_surface.cpp
    src/geometry/coons_surface.cpp
    src/geometry/extrusion_surface.cpp
    src/geometry/revolution_surface.cpp
    src/math/linear_algebra.cpp
    src/topology/topology.cpp
)
add_library(apmesh::core ALIAS apmesh_core)

target_compile_features(apmesh_core PUBLIC cxx_std_23)
target_include_directories(apmesh_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
apmesh_apply_target_defaults(apmesh_core)

if(APMESH_USE_LIBCXX)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "APMESH_USE_LIBCXX requires a Clang compiler")
    endif()

    target_compile_options(apmesh_core PUBLIC -stdlib=libc++)
    target_link_options(apmesh_core INTERFACE -stdlib=libc++)
endif()

if(BUILD_TESTING AND APMESH_ENABLE_QUALIFICATION_TESTS)
    add_executable(apmesh_core_bootstrap_export
        experiments/bootstrap_export.cpp
    )
    target_link_libraries(apmesh_core_bootstrap_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_bootstrap_export)

    add_executable(apmesh_core_numeric_contract_export
        experiments/numeric_contract_export.cpp
    )
    target_link_libraries(apmesh_core_numeric_contract_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_numeric_contract_export)

    add_executable(apmesh_core_geometry_point_vector_export
        experiments/geometry_point_vector_export.cpp
    )
    target_link_libraries(apmesh_core_geometry_point_vector_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_geometry_point_vector_export)

    add_executable(apmesh_core_cartesian_frames_export
        experiments/cartesian_frames_export.cpp
    )
    target_link_libraries(apmesh_core_cartesian_frames_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_cartesian_frames_export)

    add_executable(apmesh_core_geometry_primitives_cumulative_export
        experiments/geometry_primitives_cumulative_export.cpp
    )
    target_link_libraries(apmesh_core_geometry_primitives_cumulative_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_geometry_primitives_cumulative_export)

    add_executable(apmesh_core_topological_model_cumulative_export
        experiments/topological_model_cumulative_export.cpp
    )
    target_link_libraries(apmesh_core_topological_model_cumulative_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_topological_model_cumulative_export)

    add_executable(apmesh_core_continuous_curve_geometry_regression_export
        experiments/continuous_curve_geometry_regression_export.cpp
    )
    target_link_libraries(
        apmesh_core_continuous_curve_geometry_regression_export
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(
        apmesh_core_continuous_curve_geometry_regression_export
    )

    add_executable(apmesh_core_minimal_small_linear_algebra_export
        experiments/minimal_small_linear_algebra_export.cpp
    )
    target_link_libraries(apmesh_core_minimal_small_linear_algebra_export PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_minimal_small_linear_algebra_export)
endif()

if(BUILD_TESTING)
    add_executable(apmesh_core_bootstrap_smoke
        tests/bootstrap_smoke.cpp
    )
    target_link_libraries(apmesh_core_bootstrap_smoke PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_bootstrap_smoke)

    add_test(NAME apmesh_core.bootstrap_smoke COMMAND apmesh_core_bootstrap_smoke)
    add_executable(apmesh_core_numeric_contract
        tests/numeric_contract.cpp
    )
    target_link_libraries(apmesh_core_numeric_contract PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_numeric_contract)

    add_test(NAME apmesh_core.numeric_contract COMMAND apmesh_core_numeric_contract)
    add_executable(apmesh_core.geometry_primitives
        tests/geometry_primitives.cpp
    )
    target_link_libraries(apmesh_core.geometry_primitives PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.geometry_primitives)

    add_test(NAME apmesh_core.geometry_primitives COMMAND apmesh_core.geometry_primitives)
    add_executable(apmesh_core.cartesian_frames
        tests/cartesian_frames.cpp
    )
    target_link_libraries(apmesh_core.cartesian_frames PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.cartesian_frames)

    add_test(NAME apmesh_core.cartesian_frames COMMAND apmesh_core.cartesian_frames)

    add_executable(apmesh_core.arbitrary_axis_placement
        tests/arbitrary_axis_placement.cpp
    )
    target_link_libraries(
        apmesh_core.arbitrary_axis_placement
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.arbitrary_axis_placement)

    add_test(
        NAME apmesh_core.arbitrary_axis_placement
        COMMAND apmesh_core.arbitrary_axis_placement
    )
    add_executable(apmesh_core.topological_model
        tests/topological_model.cpp
    )
    target_link_libraries(apmesh_core.topological_model PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.topological_model)

    add_test(NAME apmesh_core.topological_model COMMAND apmesh_core.topological_model)

    add_executable(apmesh_core.curve_representation
        tests/curve_representation.cpp
    )
    target_link_libraries(apmesh_core.curve_representation PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.curve_representation)

    add_test(
        NAME apmesh_core.curve_representation
        COMMAND apmesh_core.curve_representation
    )

    add_executable(apmesh_core.parametric_curve_contract
        tests/parametric_curve_contract.cpp
    )
    target_link_libraries(apmesh_core.parametric_curve_contract PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.parametric_curve_contract)

    add_test(
        NAME apmesh_core.parametric_curve_contract
        COMMAND apmesh_core.parametric_curve_contract
    )

    add_executable(apmesh_core.line_segment
        tests/line_segment.cpp
    )
    target_link_libraries(apmesh_core.line_segment PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.line_segment)

    add_test(
        NAME apmesh_core.line_segment
        COMMAND apmesh_core.line_segment
    )

    add_executable(apmesh_core.rational_quadratic_bezier
        tests/rational_quadratic_bezier.cpp
    )
    target_link_libraries(
        apmesh_core.rational_quadratic_bezier
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.rational_quadratic_bezier)

    add_test(
        NAME apmesh_core.rational_quadratic_bezier
        COMMAND apmesh_core.rational_quadratic_bezier
    )

    add_executable(apmesh_core.trimmed_curve
        tests/trimmed_curve.cpp
    )
    target_link_libraries(apmesh_core.trimmed_curve PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.trimmed_curve)

    add_test(
        NAME apmesh_core.trimmed_curve
        COMMAND apmesh_core.trimmed_curve
    )

    add_executable(apmesh_core.two_span_cubic_bspline
        tests/two_span_cubic_bspline.cpp
    )
    target_link_libraries(
        apmesh_core.two_span_cubic_bspline
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.two_span_cubic_bspline)

    add_test(
        NAME apmesh_core.two_span_cubic_bspline
        COMMAND apmesh_core.two_span_cubic_bspline
    )

    add_executable(apmesh_core.two_span_cubic_nurbs
        tests/two_span_cubic_nurbs.cpp
    )
    target_link_libraries(
        apmesh_core.two_span_cubic_nurbs
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.two_span_cubic_nurbs)

    add_test(
        NAME apmesh_core.two_span_cubic_nurbs
        COMMAND apmesh_core.two_span_cubic_nurbs
    )

    add_executable(apmesh_core.multi_span_cubic_nurbs
        tests/multi_span_cubic_nurbs.cpp
    )
    target_link_libraries(
        apmesh_core.multi_span_cubic_nurbs
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.multi_span_cubic_nurbs)

    add_test(
        NAME apmesh_core.multi_span_cubic_nurbs
        COMMAND apmesh_core.multi_span_cubic_nurbs
    )

    add_executable(apmesh_core.cubic_nurbs_double_knot_continuity
        tests/cubic_nurbs_double_knot_continuity.cpp
    )
    target_link_libraries(
        apmesh_core.cubic_nurbs_double_knot_continuity
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(
        apmesh_core.cubic_nurbs_double_knot_continuity
    )

    add_test(
        NAME apmesh_core.cubic_nurbs_double_knot_continuity
        COMMAND apmesh_core.cubic_nurbs_double_knot_continuity
    )

    add_executable(apmesh_core.surface_bicubic_bezier
        tests/surface_bicubic_bezier.cpp
    )
    target_link_libraries(
        apmesh_core.surface_bicubic_bezier
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.surface_bicubic_bezier)

    add_test(
        NAME apmesh_core.surface_bicubic_bezier
        COMMAND apmesh_core.surface_bicubic_bezier
    )

    add_executable(apmesh_core.surface_rational_bicubic_bezier
        tests/surface_rational_bicubic_bezier.cpp
    )
    target_link_libraries(
        apmesh_core.surface_rational_bicubic_bezier
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(
        apmesh_core.surface_rational_bicubic_bezier
    )

    add_test(
        NAME apmesh_core.surface_rational_bicubic_bezier
        COMMAND apmesh_core.surface_rational_bicubic_bezier
    )

    add_executable(apmesh_core.surface_bicubic_nurbs
        tests/surface_bicubic_nurbs.cpp
    )
    target_link_libraries(
        apmesh_core.surface_bicubic_nurbs
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.surface_bicubic_nurbs)

    add_test(
        NAME apmesh_core.surface_bicubic_nurbs
        COMMAND apmesh_core.surface_bicubic_nurbs
    )

    add_executable(apmesh_core.surface_bicubic_nurbs_double_knot_continuity
        tests/surface_bicubic_nurbs_double_knot_continuity.cpp
    )
    target_link_libraries(
        apmesh_core.surface_bicubic_nurbs_double_knot_continuity
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(
        apmesh_core.surface_bicubic_nurbs_double_knot_continuity
    )

    add_test(
        NAME apmesh_core.surface_bicubic_nurbs_double_knot_continuity
        COMMAND apmesh_core.surface_bicubic_nurbs_double_knot_continuity
    )

    add_executable(apmesh_core.surface_coons_patch
        tests/surface_coons_patch.cpp
    )
    target_link_libraries(
        apmesh_core.surface_coons_patch
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.surface_coons_patch)

    add_test(
        NAME apmesh_core.surface_coons_patch
        COMMAND apmesh_core.surface_coons_patch
    )

    add_executable(apmesh_core.surface_rectangular_trim
        tests/surface_rectangular_trim.cpp
    )
    target_link_libraries(
        apmesh_core.surface_rectangular_trim
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.surface_rectangular_trim)

    add_test(
        NAME apmesh_core.surface_rectangular_trim
        COMMAND apmesh_core.surface_rectangular_trim
    )

    add_executable(apmesh_core.surface_linear_extrusion
        tests/surface_linear_extrusion.cpp
    )
    target_link_libraries(
        apmesh_core.surface_linear_extrusion
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.surface_linear_extrusion)

    add_test(
        NAME apmesh_core.surface_linear_extrusion
        COMMAND apmesh_core.surface_linear_extrusion
    )

    add_executable(apmesh_core.surface_revolution
        tests/surface_revolution.cpp
    )
    target_link_libraries(
        apmesh_core.surface_revolution
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.surface_revolution)

    add_test(
        NAME apmesh_core.surface_revolution
        COMMAND apmesh_core.surface_revolution
    )

    add_executable(apmesh_core.curve_differential
        tests/curve_differential.cpp
    )
    target_link_libraries(apmesh_core.curve_differential PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.curve_differential)

    add_test(
        NAME apmesh_core.curve_differential
        COMMAND apmesh_core.curve_differential
    )

    add_executable(apmesh_core.curve_curvature
        tests/curve_curvature.cpp
    )
    target_link_libraries(apmesh_core.curve_curvature PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.curve_curvature)

    add_test(
        NAME apmesh_core.curve_curvature
        COMMAND apmesh_core.curve_curvature
    )

    add_executable(apmesh_core.curve_signed_curvature
        tests/curve_signed_curvature.cpp
    )
    target_link_libraries(apmesh_core.curve_signed_curvature PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.curve_signed_curvature)

    add_test(
        NAME apmesh_core.curve_signed_curvature
        COMMAND apmesh_core.curve_signed_curvature
    )

    add_executable(apmesh_core.curve_inflection_isolation
        tests/curve_inflection_isolation.cpp
    )
    target_link_libraries(
        apmesh_core.curve_inflection_isolation
        PRIVATE apmesh::core
    )
    target_include_directories(
        apmesh_core.curve_inflection_isolation
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    apmesh_apply_target_defaults(apmesh_core.curve_inflection_isolation)

    add_test(
        NAME apmesh_core.curve_inflection_isolation
        COMMAND apmesh_core.curve_inflection_isolation
    )

    add_executable(apmesh_core.curve_arc_length
        tests/curve_arc_length.cpp
    )
    target_link_libraries(apmesh_core.curve_arc_length PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.curve_arc_length)

    add_test(
        NAME apmesh_core.curve_arc_length
        COMMAND apmesh_core.curve_arc_length
    )

    add_executable(apmesh_core.curve_cumulative_arc_length
        tests/curve_cumulative_arc_length.cpp
    )
    target_link_libraries(
        apmesh_core.curve_cumulative_arc_length
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.curve_cumulative_arc_length)

    add_test(
        NAME apmesh_core.curve_cumulative_arc_length
        COMMAND apmesh_core.curve_cumulative_arc_length
    )

    add_executable(apmesh_core.curve_inverse_arc_length
        tests/curve_inverse_arc_length.cpp
    )
    target_link_libraries(
        apmesh_core.curve_inverse_arc_length
        PRIVATE apmesh::core
    )
    apmesh_apply_target_defaults(apmesh_core.curve_inverse_arc_length)

    add_test(
        NAME apmesh_core.curve_inverse_arc_length
        COMMAND apmesh_core.curve_inverse_arc_length
    )

    add_executable(apmesh_core.curve_regularity
        tests/curve_regularity.cpp
    )
    target_link_libraries(apmesh_core.curve_regularity PRIVATE apmesh::core)
    target_include_directories(
        apmesh_core.curve_regularity
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    apmesh_apply_target_defaults(apmesh_core.curve_regularity)

    add_test(
        NAME apmesh_core.curve_regularity
        COMMAND apmesh_core.curve_regularity
    )

    add_executable(apmesh_core.curve_header_isolation
        tests/curve_header_isolation.cpp
    )
    target_link_libraries(apmesh_core.curve_header_isolation PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.curve_header_isolation)

    add_test(
        NAME apmesh_core.curve_header_isolation
        COMMAND apmesh_core.curve_header_isolation
    )

    add_executable(apmesh_core.minimal_small_linear_algebra
        tests/minimal_small_linear_algebra.cpp
    )
    target_link_libraries(apmesh_core.minimal_small_linear_algebra PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.minimal_small_linear_algebra)

    add_test(
        NAME apmesh_core.minimal_small_linear_algebra
        COMMAND apmesh_core.minimal_small_linear_algebra
    )
    add_executable(apmesh_core.math_header_isolation
        tests/math_header_isolation.cpp
    )
    target_link_libraries(apmesh_core.math_header_isolation PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core.math_header_isolation)

    add_test(
        NAME apmesh_core.math_header_isolation
        COMMAND apmesh_core.math_header_isolation
    )

    set_tests_properties(
        apmesh_core.bootstrap_smoke
        PROPERTIES LABELS "bootstrap;contract;foundation-preservation;direct-semantic;fast;integration;qualification"
    )
    set_tests_properties(
        apmesh_core.numeric_contract
        PROPERTIES LABELS "numeric;contract;foundation-preservation;direct-semantic;fast;integration;qualification"
    )
    set_tests_properties(
        apmesh_core.geometry_primitives
        apmesh_core.cartesian_frames
        PROPERTIES LABELS "geometry;contract;direct-semantic;fast;integration;qualification"
    )
    set_tests_properties(
        apmesh_core.arbitrary_axis_placement
        PROPERTIES LABELS "geometry;surface-prerequisite;contract;focused-contract;direct-semantic;fast;integration"
    )
    set_tests_properties(
        apmesh_core.minimal_small_linear_algebra
        PROPERTIES LABELS "math;geometry;contract;direct-semantic;fast;integration;qualification"
    )
    set_tests_properties(
        apmesh_core.math_header_isolation
        PROPERTIES LABELS "math;geometry;contract;focused-contract;fast;integration;qualification"
    )
    set_tests_properties(
        apmesh_core.topological_model
        PROPERTIES LABELS "topology;contract;focused-contract;fast;integration;qualification"
    )
    set_tests_properties(
        apmesh_core.curve_representation
        apmesh_core.parametric_curve_contract
        apmesh_core.line_segment
        apmesh_core.rational_quadratic_bezier
        apmesh_core.trimmed_curve
        apmesh_core.two_span_cubic_bspline
        apmesh_core.two_span_cubic_nurbs
        apmesh_core.multi_span_cubic_nurbs
        apmesh_core.cubic_nurbs_double_knot_continuity
        apmesh_core.curve_differential
        apmesh_core.curve_curvature
        apmesh_core.curve_signed_curvature
        apmesh_core.curve_inflection_isolation
        apmesh_core.curve_regularity
        apmesh_core.curve_arc_length
        apmesh_core.curve_cumulative_arc_length
        apmesh_core.curve_inverse_arc_length
        PROPERTIES LABELS "curve;geometry;contract;focused-contract;direct-semantic;fast;integration"
    )
    set_tests_properties(
        apmesh_core.curve_header_isolation
        PROPERTIES LABELS "curve;geometry;contract;focused-contract;fast;integration"
    )

    set_tests_properties(
        apmesh_core.surface_bicubic_bezier
        apmesh_core.surface_rational_bicubic_bezier
        apmesh_core.surface_bicubic_nurbs
        apmesh_core.surface_bicubic_nurbs_double_knot_continuity
        apmesh_core.surface_coons_patch
        apmesh_core.surface_rectangular_trim
        apmesh_core.surface_linear_extrusion
        apmesh_core.surface_revolution
        PROPERTIES LABELS "surface;geometry;contract;focused-contract;direct-semantic;fast;integration"
    )

    if(APMESH_ENABLE_QUALIFICATION_TESTS)
        find_package(Python3 3.10 COMPONENTS Interpreter REQUIRED)

    add_test(
        NAME apmesh_core.geometry_primitives_cumulative_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/geometry_primitives_cumulative_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_geometry_primitives_cumulative_export>
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/geometry_primitives_cumulative_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/geometry_primitives_cumulative.json
    )
    add_test(
        NAME apmesh_core.geometry_primitives_cumulative_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/geometry_primitives_cumulative_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_geometry_primitives_cumulative.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/geometry_primitives_cumulative.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/geometry_primitives_cumulative_export.cpp
            --certificate-exporter $<TARGET_FILE:apmesh_core_geometry_primitives_cumulative_export>
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/geometry_primitives_cumulative_evidence.py
    )
    add_test(
        NAME apmesh_core.topological_model_cumulative_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/topological_model_cumulative_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_topological_model_cumulative_export>
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/topological_model_cumulative_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/topological_model_cumulative.json
    )
    add_test(
        NAME apmesh_core.topological_model_cumulative_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/topological_model_cumulative_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_topological_model_cumulative.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/topological_model_cumulative.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/topological_model_cumulative_export.cpp
            --certificate-exporter $<TARGET_FILE:apmesh_core_topological_model_cumulative_export>
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/topological_model_cumulative_evidence.py
    )
    add_test(
        NAME apmesh_core.topological_model_cumulative_prepare_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/topological_model_cumulative_prepare_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/topological-model-tmr-prepare.yml
    )
    add_test(
        NAME apmesh_core.topological_model_cumulative_execute_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/topological_model_cumulative_execute_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/topological-model-tmr-execute.yml
    )
    add_test(
        NAME apmesh_core.topological_model_tmr_authorization
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/topological_model_tmr_authorization_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/topological_model_tmr_authorization.py
    )
    add_test(
        NAME apmesh_core.topological_model_tmr_authorization_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/topological_model_tmr_authorization_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/topological-model-tmr-authorize.yml
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_regression_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_regression_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_continuous_curve_geometry_regression_export>
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_evidence.py
            --negative ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_negative.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/continuous_curve_geometry_regression.json
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_regression_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_regression_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_continuous_curve_geometry_regression.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/continuous_curve_geometry_regression.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/continuous_curve_geometry_regression_export.cpp
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_evidence.py
            --negative ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_negative.py
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_regression_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_regression_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/continuous-curve-geometry-regression-tooling.yml
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_campaign_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_campaign_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_continuous_curve_geometry_campaign.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/continuous_curve_geometry_regression.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/continuous_curve_geometry_regression_export.cpp
            --certificate-exporter $<TARGET_FILE:apmesh_core_continuous_curve_geometry_regression_export>
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_evidence.py
            --negative ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_negative.py
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_campaign_prepare_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_campaign_prepare_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/continuous-curve-geometry-regression-prepare.yml
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_campaign_execute_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_campaign_execute_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/continuous-curve-geometry-regression-execute.yml
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_regression_authorization
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_regression_authorization_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/continuous_curve_geometry_regression_authorization.py
    )
    add_test(
        NAME apmesh_core.continuous_curve_geometry_regression_authorization_workflow
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/continuous_curve_geometry_regression_authorization_workflow_test.py
            --workflow ${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/continuous-curve-geometry-regression-authorize.yml
    )
    add_test(
        NAME apmesh_core.cartesian_frames_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/cartesian_frames_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_cartesian_frames_export>
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/cartesian_frames_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/cartesian_frames.json
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME apmesh_core.cartesian_frames_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/cartesian_frames_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_cartesian_frames_qualification.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/cartesian_frames.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/cartesian_frames_export.cpp
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/cartesian_frames_evidence.py
    )
    add_test(
        NAME apmesh_core.cartesian_frames_retention
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/cartesian_frames_retention_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_cartesian_frames_qualification.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/cartesian_frames.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/cartesian_frames_export.cpp
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/cartesian_frames_evidence.py
    )
    add_test(
        NAME apmesh_core.minimal_small_linear_algebra_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/minimal_small_linear_algebra_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_minimal_small_linear_algebra_export>
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/minimal_small_linear_algebra.json
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/minimal_small_linear_algebra_evidence.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(
        NAME apmesh_core.minimal_small_linear_algebra_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/minimal_small_linear_algebra_runner_test.py
            --build-root ${CMAKE_CURRENT_BINARY_DIR}
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_minimal_small_linear_algebra_qualification.py
            --source-root ${CMAKE_CURRENT_SOURCE_DIR}
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/minimal_small_linear_algebra.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md
    )
    add_test(
        NAME apmesh_core.minimal_small_linear_algebra_retention
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/minimal_small_linear_algebra_retention_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_minimal_small_linear_algebra_qualification.py
    )
    add_test(
        NAME apmesh_core.geometry_point_vector_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/geometry_point_vector_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_geometry_point_vector_export>
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/geometry_point_vector_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/geometry_point_vector.json
            --compile-commands ${CMAKE_BINARY_DIR}/compile_commands.json
    )
    add_test(
        NAME apmesh_core.geometry_point_vector_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/geometry_point_vector_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_geometry_point_vector_qualification.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/geometry_point_vector.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md
            --exporter ${CMAKE_CURRENT_SOURCE_DIR}/experiments/geometry_point_vector_export.cpp
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/geometry_point_vector_evidence.py
    )
    add_test(
        NAME apmesh_core.geometry_point_vector_retention
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/geometry_point_vector_retention_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_geometry_point_vector_qualification.py
            --runtime ${CMAKE_CURRENT_SOURCE_DIR}/tools/experiment_runtime.py
    )
    add_test(
        NAME apmesh_core.geometry_point_vector_foundation_preservation
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/geometry_point_vector_foundation_preservation_test.py
            --ctest ${CMAKE_CTEST_COMMAND}
            --build-dir ${CMAKE_BINARY_DIR}
    )
    add_test(
        NAME apmesh_core.numeric_contract_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/numeric_contract_evidence_test.py
            --exporter $<TARGET_FILE:apmesh_core_numeric_contract_export>
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/numeric_contract_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/numeric_contract.json
            --compile-commands ${CMAKE_BINARY_DIR}/compile_commands.json
    )
    add_test(
        NAME apmesh_core.numeric_contract_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/numeric_contract_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_numeric_contract_regression.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/numeric_contract.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md
            --validator ${CMAKE_CURRENT_SOURCE_DIR}/tools/numeric_contract_evidence.py
            --architecture-evidence ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md
            --architecture-runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_architecture_bootstrap_regression.py
            --architecture-profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/architecture_bootstrap.json
            --architecture-expected ${CMAKE_CURRENT_SOURCE_DIR}/experiments/expected/bootstrap_certificate.json
            --architecture-comparer ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootstrap_regression.py
    )
    add_test(
        NAME apmesh_core.reproducible_experiment_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/reproducible_experiment_evidence_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/reproducible_experiment_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/reproducible_experiment_contract.json
    )
    add_test(
        NAME apmesh_core.reproducible_experiment_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/reproducible_experiment_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_reproducible_experiment_contract.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/reproducible_experiment_contract.json
            --contract ${CMAKE_CURRENT_SOURCE_DIR}/docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/FOUNDATION_REPRODUCIBLE_EXPERIMENT_QUALIFICATION.md
            --collector ${CMAKE_CURRENT_SOURCE_DIR}/tools/reproducible_experiment_evidence.py
    )
    add_test(
        NAME apmesh_core.reproducible_experiment_retention
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/reproducible_experiment_retention_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/retain_experiment_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/reproducible_experiment_contract.json
    )
    add_test(
        NAME apmesh_core.reproducible_experiment_negatives
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/reproducible_experiment_negatives_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/reproducible_experiment_negative.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/reproducible_experiment_contract.json
    )
    add_test(
        NAME apmesh_core.foundation_end_to_end_evidence
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/foundation_end_to_end_evidence_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/foundation_end_to_end_evidence.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/foundation_end_to_end.json
            --rec-profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/reproducible_experiment_contract.json
    )
    add_test(
        NAME apmesh_core.bootstrap_export
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap_export_test.py
            --exporter $<TARGET_FILE:apmesh_core_bootstrap_export>
            --expected ${CMAKE_CURRENT_SOURCE_DIR}/experiments/expected/bootstrap_certificate.json
    )
    add_test(
        NAME apmesh_core.bootstrap_tool
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/bootstrap_regression_tool_test.py
            --tool ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootstrap_regression.py
            --expected ${CMAKE_CURRENT_SOURCE_DIR}/experiments/expected/bootstrap_certificate.json
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/architecture_bootstrap.json
            --data-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/bootstrap_regression
    )
    add_test(
        NAME apmesh_core.architecture_bootstrap_runner
        COMMAND
            ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/architecture_bootstrap_runner_test.py
            --runner ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_architecture_bootstrap_regression.py
            --profile ${CMAKE_CURRENT_SOURCE_DIR}/experiments/profiles/architecture_bootstrap.json
            --protocol ${CMAKE_CURRENT_SOURCE_DIR}/docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md
            --expected ${CMAKE_CURRENT_SOURCE_DIR}/experiments/expected/bootstrap_certificate.json
            --comparer ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootstrap_regression.py
    )
    set_tests_properties(
        apmesh_core.bootstrap_export
        apmesh_core.bootstrap_tool
        apmesh_core.architecture_bootstrap_runner
        PROPERTIES LABELS "bootstrap;contract;foundation-preservation;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.numeric_contract_evidence
        apmesh_core.numeric_contract_runner
        PROPERTIES LABELS "numeric;contract;foundation-preservation;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.geometry_primitives_cumulative_evidence
        apmesh_core.geometry_primitives_cumulative_runner
        apmesh_core.cartesian_frames_evidence
        apmesh_core.cartesian_frames_runner
        apmesh_core.cartesian_frames_retention
        apmesh_core.geometry_point_vector_evidence
        apmesh_core.geometry_point_vector_runner
        apmesh_core.geometry_point_vector_retention
        PROPERTIES LABELS "geometry;contract;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.topological_model_cumulative_evidence
        apmesh_core.topological_model_cumulative_runner
        apmesh_core.topological_model_cumulative_prepare_workflow
        apmesh_core.topological_model_cumulative_execute_workflow
        apmesh_core.topological_model_tmr_authorization
        apmesh_core.topological_model_tmr_authorization_workflow
        PROPERTIES LABELS "topology;contract;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.continuous_curve_geometry_regression_evidence
        apmesh_core.continuous_curve_geometry_regression_runner
        apmesh_core.continuous_curve_geometry_regression_workflow
        PROPERTIES LABELS "curve;geometry;contract;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.geometry_point_vector_foundation_preservation
        PROPERTIES LABELS "geometry;contract;qualification;stage-regression"
    )
    set_tests_properties(
        apmesh_core.minimal_small_linear_algebra_evidence
        apmesh_core.minimal_small_linear_algebra_runner
        apmesh_core.minimal_small_linear_algebra_retention
        PROPERTIES LABELS "math;geometry;contract;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.reproducible_experiment_evidence
        apmesh_core.reproducible_experiment_runner
        apmesh_core.reproducible_experiment_retention
        apmesh_core.reproducible_experiment_negatives
        PROPERTIES LABELS "reproducibility;contract;foundation-preservation;qualification;tooling"
    )
    set_tests_properties(
        apmesh_core.foundation_end_to_end_evidence
        PROPERTIES LABELS "foundation;reproducibility;contract;qualification;stage-regression"
    )
    endif()
endif()
