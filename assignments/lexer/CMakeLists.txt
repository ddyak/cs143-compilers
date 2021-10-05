cmake_minimum_required(VERSION 3.14)
project(my_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")


include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    lexer
    lexer.cc
    main.cc
)

add_executable(
    test_lexer
    test_lexer.cc
)

target_link_libraries(
    test_lexer
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_lexer)
