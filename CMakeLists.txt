cmake_minimum_required(VERSION 3.16.9)
project(meta)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.4          # 指定历史版本
        GIT_SHALLOW    TRUE             # 仅克隆最新提交（节省时间）
        GIT_PROGRESS   TRUE             # 显示下载进度
)

# 设置构建选项（需在 FetchContent_MakeAvailable 前定义）
set(BENCHMARK_ENABLE_TESTING OFF          # 禁用测试套件
        CACHE BOOL "Disable benchmark tests")
set(BENCHMARK_ENABLE_INSTALL OFF          # 禁用安装目标
        CACHE BOOL "Disable benchmark install")
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON    # 自动下载 GoogleTest
        CACHE BOOL "Auto-download dependencies")

# 拉取并构建 benchmark
FetchContent_MakeAvailable(benchmark)

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
        GIT_TAG        1.3.0
)

set(BUILD_TESTING OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(oatpp)

add_executable(meta main.cpp
        oatpp_helper.hpp
        traits.hpp
        template_helper.hpp)

target_include_directories(meta PRIVATE
        ${benchmark_SOURCE_DIR}/include)

# 链接依赖库
target_link_libraries(meta PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
        absl::base
        absl::strings
        oatpp)
