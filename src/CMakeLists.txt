cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

project(AtlasBM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -pthread -Wall -g -O2)

find_package(ROOT 6.24 CONFIG REQUIRED)

message(STATUS ${CMAKE_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

set( _parentProjectNames Athena AthenaP1 AnalysisBase AthAnalysis
   AthSimulation AthDerivation AnalysisTop )
set( _defaultParentProject AnalysisBase )
foreach( _pp ${_parentProjectNames} )
   if( NOT "$ENV{${_pp}_DIR}" STREQUAL "" )
      set( _defaultParentProject ${_pp} )
      break()
   endif()
endforeach()

# Set the parent project name based on the previous findings:
set( ATLAS_PROJECT ${_defaultParentProject}
   CACHE STRING "The name of the parent project to build against" )


# Clean up:
unset( _parentProjectNames )
unset( _defaultParentProject )

# Find the AnalysisBase project. This is what, amongst other things, pulls
# in the definition of all of the "atlas_" prefixed functions/macros.
find_package( ${ATLAS_PROJECT} REQUIRED )

add_subdirectory(compression)

