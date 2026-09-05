cmake_minimum_required(VERSION 3.25)

project(apmesh_core_bootstrap_consumer LANGUAGES CXX)

if(NOT DEFINED APMESH_CORE_SOURCE_DIR)
    message(FATAL_ERROR "APMESH_CORE_SOURCE_DIR must name the AP Mesh Core source root")
endif()

set(BUILD_TESTING OFF CACHE BOOL "Build tests in the consumed project")
if(BUILD_TESTING)
    message(FATAL_ERROR "The consumer fixture requires BUILD_TESTING=OFF")
endif()

add_subdirectory("${APMESH_CORE_SOURCE_DIR}" apmesh-core-build EXCLUDE_FROM_ALL)

add_executable(apmesh_core_external_consumer main.cpp)
target_compile_features(apmesh_core_external_consumer PRIVATE cxx_std_23)
set_target_properties(apmesh_core_external_consumer PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(apmesh_core_external_consumer PRIVATE apmesh::core)

add_executable(apmesh_core_unrelated_target unrelated.cpp)
