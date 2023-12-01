add_executable(bm_size bm_size.cxx)
target_link_libraries(bm_size PUBLIC ROOT::Tree ROOT::ROOTNTuple ROOT::ROOTNTupleUtil)
target_include_directories(bm_size PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                        )
