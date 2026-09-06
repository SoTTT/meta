cmake_minimum_required(VERSION 3.16.9)

# 纯头文件库，必须使用 INTERFACE：
# 之前声明为 STATIC 但没有 .cpp 源文件，ar 无法从空目标创建归档，导致构建失败
add_library(oatpp_meta INTERFACE)

target_sources(oatpp_meta INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/meta.hpp)

include(FetchContent)

FetchContent_Declare(
        oatpp
        GIT_REPOSITORY https://github.com/oatpp/oatpp
        GIT_TAG 1.3.0
)

set(BUILD_TESTING OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(oatpp)

target_include_directories(oatpp_meta INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(oatpp_meta INTERFACE oatpp)
