cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

project(AtlasBM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -pthread -Wall -g -O2)

find_package(ROOT CONFIG REQUIRED)

cmake_path(SET BIN_DIR NORMALIZE "${CMAKE_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

add_library(util benchmarks/util.cxx benchmarks/util.hxx)
target_link_libraries(util PUBLIC ROOT::ROOTDataFrame)

add_executable(bm_compression benchmarks/bm_compression.cxx)
target_link_libraries(bm_compression PUBLIC ROOT::Tree ROOT::Gui util)
target_include_directories(bm_compression PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                        )
