add_executable(bm_size bm_size.cxx)
target_link_libraries(bm_size PUBLIC ROOT::Tree ROOT::ROOTNTuple ROOT::ROOTNTupleUtil)
target_include_directories(bm_size PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                        )

add_executable(bm_index_field_size bm_index_field_size.cxx)
target_link_libraries(bm_index_field_size PUBLIC ROOT::Tree ROOT::ROOTNTuple ROOT::ROOTNTupleUtil)
target_include_directories(bm_index_field_size PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                        )

# add_executable(bm_float_vector_size bm_float_vector_size.cxx)
# target_link_libraries(bm_float_vector_size PUBLIC ROOT::Tree ROOT::ROOTNTuple ROOT::ROOTNTupleUtil)
# target_include_directories(bm_float_vector_size PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#                           "${PROJECT_SOURCE_DIR}"
#                         )
