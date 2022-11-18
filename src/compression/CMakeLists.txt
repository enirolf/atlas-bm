add_executable(gen_ttree gen_ttree.cxx)

target_link_libraries(gen_ttree PUBLIC ROOT::Tree)
