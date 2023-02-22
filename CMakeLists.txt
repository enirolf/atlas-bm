cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

project(AtlasBM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -pthread -Wall -g -O2)

find_package(ROOT CONFIG REQUIRED)

cmake_path(SET BIN_DIR NORMALIZE "${CMAKE_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

add_library(size_util util/size_util.cxx util/size_util.hxx)
target_link_libraries(size_util PUBLIC ROOT::Graf)

add_executable(bm_ttree_vs_rntuple_size benchmarks/bm_ttree_vs_rntuple_size.cxx)
target_link_libraries(bm_ttree_vs_rntuple_size PUBLIC size_util ROOT::Tree ROOT::ROOTNTuple ROOT::ROOTNTupleUtil)
target_include_directories(bm_ttree_vs_rntuple_size PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                        )
