cmake_minimum_required(VERSION 3.25)

project(apmesh_core LANGUAGES CXX)

option(APMESH_USE_LIBCXX "Build apmesh::core with libc++" OFF)

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
    src/math/linear_algebra.cpp
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

add_executable(apmesh_core_minimal_small_linear_algebra_export
    experiments/minimal_small_linear_algebra_export.cpp
)
target_link_libraries(apmesh_core_minimal_small_linear_algebra_export PRIVATE apmesh::core)
apmesh_apply_target_defaults(apmesh_core_minimal_small_linear_algebra_export)

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 3.10 COMPONENTS Interpreter REQUIRED)

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
        apmesh_core.bootstrap_smoke
        apmesh_core.bootstrap_export
        apmesh_core.bootstrap_tool
        apmesh_core.architecture_bootstrap_runner
        PROPERTIES LABELS "bootstrap;contract;foundation-preservation"
    )
    set_tests_properties(
        apmesh_core.numeric_contract
        apmesh_core.numeric_contract_evidence
        apmesh_core.numeric_contract_runner
        PROPERTIES LABELS "numeric;contract;foundation-preservation"
    )
    set_tests_properties(
        apmesh_core.geometry_primitives
        apmesh_core.geometry_primitives_cumulative_evidence
        apmesh_core.cartesian_frames
        apmesh_core.cartesian_frames_evidence
        apmesh_core.cartesian_frames_runner
        apmesh_core.cartesian_frames_retention
        apmesh_core.geometry_point_vector_evidence
        apmesh_core.geometry_point_vector_runner
        apmesh_core.geometry_point_vector_retention
        apmesh_core.geometry_point_vector_foundation_preservation
        PROPERTIES LABELS "geometry;contract"
    )
    set_tests_properties(
        apmesh_core.minimal_small_linear_algebra
        apmesh_core.math_header_isolation
        apmesh_core.minimal_small_linear_algebra_evidence
        apmesh_core.minimal_small_linear_algebra_runner
        apmesh_core.minimal_small_linear_algebra_retention
        PROPERTIES LABELS "math;geometry;contract"
    )
    set_tests_properties(
        apmesh_core.reproducible_experiment_evidence
        apmesh_core.reproducible_experiment_runner
        apmesh_core.reproducible_experiment_retention
        apmesh_core.reproducible_experiment_negatives
        PROPERTIES LABELS "reproducibility;contract;foundation-preservation"
    )
    set_tests_properties(
        apmesh_core.foundation_end_to_end_evidence
        PROPERTIES LABELS "foundation;reproducibility;contract"
    )
endif()
