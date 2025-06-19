cmake_minimum_required(VERSION 3.16.9)

add_executable(meta_operation_test test.cpp)

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(meta_operation_test PRIVATE Catch2::Catch2 meta_operation)