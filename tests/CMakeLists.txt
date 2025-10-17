include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0  # Use a stable version; update as needed
)

FetchContent_MakeAvailable(Catch2)

include(Catch)

add_executable(unit_tests
  test_base_index_subset.cpp
  test_parse_table_generator.cpp
  test_index_subset.cpp
  test_grammar_error.cpp
  test_associativity.cpp
  test_symbol_collection.cpp
  test_ruleset.cpp
  test_nullable.cpp
  test_lr1_state.cpp
  test_firsts.cpp
  test_vector_n.cpp
  test_flat_indexer.cpp
  test_closure.cpp
  test_list_printer.cpp
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain parse_table_generator_lib)
target_include_directories(unit_tests PRIVATE ../include)

if(MSVC)
    target_compile_options(unit_tests PRIVATE /W4 /WX)
else()
    target_compile_options(unit_tests PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

catch_discover_tests(unit_tests)

