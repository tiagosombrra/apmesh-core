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

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)

    add_executable(apmesh_core_bootstrap_smoke
        tests/bootstrap_smoke.cpp
    )
    target_link_libraries(apmesh_core_bootstrap_smoke PRIVATE apmesh::core)
    apmesh_apply_target_defaults(apmesh_core_bootstrap_smoke)

    add_test(NAME apmesh_core.bootstrap_smoke COMMAND apmesh_core_bootstrap_smoke)
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
        PROPERTIES LABELS "bootstrap;contract"
    )
endif()
