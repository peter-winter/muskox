cmake_minimum_required(VERSION 3.28)

project(parse_table_generator CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define warning options once
if(MSVC)
    set(WARNING_OPTIONS /W4 /WX)
else()
    set(WARNING_OPTIONS -Wall -Wextra -pedantic -Werror -Wsign-conversion)
endif()

add_library(${PROJECT_NAME}_lib
  src/parse_table_generator.cpp
  src/base_index_subset.cpp
  src/index_subset.cpp
  src/symbol_collection.cpp
  src/ruleset.cpp
  src/nullable.cpp
)
target_include_directories(${PROJECT_NAME}_lib PUBLIC include)
target_compile_options(${PROJECT_NAME}_lib PRIVATE ${WARNING_OPTIONS})

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_OPTIONS})

enable_testing()
add_subdirectory(tests)
