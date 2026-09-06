cmake_minimum_required(VERSION 3.16.9)

add_executable(oatpp_meta_test
        test.cpp)

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.10
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(oatpp_meta_test PRIVATE Catch2::Catch2 oatpp_meta)

if(DEFINED OATPP_EXTERNAL_TARGET)
    # EXTERNAL / AUTO（回退 EXTERNAL）：先构建远程 oatpp，再编译测试
    add_dependencies(oatpp_meta_test ${OATPP_EXTERNAL_TARGET})
endif()
