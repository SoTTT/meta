cmake_minimum_required(VERSION 3.16.9)

add_executable(meta_operation_test
        test.cpp)

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(meta_operation_test PRIVATE Catch2::Catch2 meta_operation)

# ---------------------------------------------------------------------------
# 预期编译失败的缺陷探针（test/compile_fail/，详见该目录 README.md）
# 默认 OFF；启用后每个探针是独立的 EXCLUDE_FROM_ALL target，
# 单独构建某个 target 即可观察对应的编译错误：
#   cmake -DMETA_BUILD_COMPILE_FAIL_PROBES=ON ..
#   cmake --build . --target cf04_dto
# ---------------------------------------------------------------------------
option(META_BUILD_COMPILE_FAIL_PROBES "Build expected-compile-fail probes (each is its own target)" OFF)
if (META_BUILD_COMPILE_FAIL_PROBES)
    file(GLOB COMPILE_FAIL_PROBES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/compile_fail/cf*.cpp)
    foreach (probe ${COMPILE_FAIL_PROBES})
        get_filename_component(probe_name ${probe} NAME_WE)
        add_executable(${probe_name} EXCLUDE_FROM_ALL ${probe})
        target_link_libraries(${probe_name} PRIVATE meta_operation)
    endforeach ()
endif ()
