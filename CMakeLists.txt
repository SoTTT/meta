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

add_executable(meta main.cpp
)

target_include_directories(meta PRIVATE
        ${benchmark_SOURCE_DIR}/include)

# 链接依赖库
target_link_libraries(meta PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main)
