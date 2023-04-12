add_executable(bm_readspeed bm_readspeed.cxx)
target_link_libraries(bm_readspeed PUBLIC ROOT::Tree ROOT::ROOTNTuple ROOT::ROOTDataFrame)
target_include_directories(bm_readspeed PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                        )
