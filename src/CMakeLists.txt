################################# CMAKE SETUP ##################################

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

project(AtlasBM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -pthread -Wall -g -O2)

find_package(ROOT CONFIG REQUIRED)

cmake_path(SET BIN_DIR NORMALIZE "${CMAKE_SOURCE_DIR}/../bin")
file(MAKE_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

add_library(util util.cxx util.hxx)
target_link_libraries(util PUBLIC ROOT::ROOTDataFrame)


################################## PLOTTING ####################################


add_executable(plot_compression_factor compression/plot_compression_factor.cxx)
target_link_libraries(plot_compression_factor PUBLIC ROOT::ROOTDataFrame util)

target_include_directories(plot_compression_factor PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}"
                          )

############################## ATLAS-ONLY STUFF ################################

if(DEFINED ENV{AtlasProject})
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

  add_executable(bm_compression_factor_ttree compression/bm_compression_factor_ttree.cxx)
  target_link_libraries(bm_compression_factor_ttree PUBLIC ROOT::Tree)
endif()
