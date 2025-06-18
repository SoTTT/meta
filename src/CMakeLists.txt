cmake_minimum_required(VERSION 3.16.9)

add_library(meta_operation STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/md_operation.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/oatpp_helper.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/template_helper.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/traits.hpp)

include(FetchContent)

# 声明 Abseil
FetchContent_Declare(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 20240116.2  # 指定稳定版本
)

# 启用 C++ 标准传递
set(ABSL_PROPAGATE_CXX_STD ON)

# 下载并构建 Abseil
FetchContent_MakeAvailable(abseil)

FetchContent_Declare(
        oatpp
        GIT_REPOSITORY https://github.com/oatpp/oatpp
        GIT_TAG 1.3.0
)

set(BUILD_TESTING OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(oatpp)

target_include_directories(meta_operation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(meta_operation PUBLIC
        absl::base
        absl::strings
        oatpp)