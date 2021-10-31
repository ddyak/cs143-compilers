cmake_minimum_required(VERSION 3.14)
project(parser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")

# app
add_executable(
    ${PROJECT_NAME}
    src/parser.cc
    src/main.cc
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE src
)

target_link_libraries(
    ${PROJECT_NAME}
    lexer_lib # my lib!
)

# # lib
# add_library(
#     parser_lib
#     lib/token.cc
# )

# target_include_directories(
#     parser_lib
#     PUBLIC include
# )

# tests
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    test_parser
    tests/test_parser.cc
)

target_link_libraries(
    test_parser
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_parser)
