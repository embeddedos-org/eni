# SPDX-License-Identifier: MIT
# Copyright (c) 2026 EoS Project
# Header-only C++17 interface library for eNI

cmake_minimum_required(VERSION 3.16)

add_library(eni_cpp INTERFACE)

target_include_directories(eni_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(eni_cpp INTERFACE cxx_std_17)

target_link_libraries(eni_cpp INTERFACE eni_common)

# Example target (optional)
if(ENI_BUILD_EXAMPLES)
    add_executable(eni_cpp_basic_pipeline examples/basic_pipeline.cpp)
    target_link_libraries(eni_cpp_basic_pipeline PRIVATE eni_cpp eni_providers)
    if(UNIX AND NOT APPLE)
        target_link_libraries(eni_cpp_basic_pipeline PRIVATE m)
    endif()
endif()
