cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

project(AtlasBM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -pthread -Wall -g -O3)

find_package(ROOT CONFIG REQUIRED)

cmake_path(SET BIN_DIR NORMALIZE "${CMAKE_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

add_subdirectory(bm-size)
add_subdirectory(bm-readspeed)
