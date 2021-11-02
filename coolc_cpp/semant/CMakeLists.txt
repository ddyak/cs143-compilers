cmake_minimum_required(VERSION 3.14)
project(semant)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")

# lib
# add_library(
#     semant_lib
#     lib/syntax.cc
# )

# target_include_directories(
#     semant_lib
#     PUBLIC include
# )

# app
add_executable(
    ${PROJECT_NAME}
    src/inheritance.cc
    src/main.cc
)

target_include_directories(
    ${PROJECT_NAME}
    # PUBLIC include
    PRIVATE src
)

target_link_libraries(
    ${PROJECT_NAME}
    parser_lib
)

# tests
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    test_semant
    tests/test_semant.cc
)

target_link_libraries(
    test_semant
    gtest_main
)
