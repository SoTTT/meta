cmake_minimum_required(VERSION 3.16.9)

project(oatpp-meta VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###################################################################################################
## oatpp 依赖来源选择
## 机制设计参考自 oatpp 官方模块 oatpp-swagger（Apache License 2.0）顶层 CMakeLists.txt
## 的三态机制（INSTALLED / EXTERNAL / CUSTOM），本文件为独立实现，
## 另加 AUTO（默认）与子项目嵌入保护。
## 注：打印给用户看的 message()/CACHE STRING 一律用英文，避免部分终端出现乱码。
###################################################################################################

if(NOT TARGET oatpp AND NOT TARGET oatpp::oatpp)
    # 嵌入保护：仅在本项目顶层未定义时写入默认值，父项目已定义则沿用其选择
    if(NOT DEFINED OATPP_MODULES_LOCATION)
        set(OATPP_MODULES_LOCATION "AUTO" CACHE STRING "Where to find oatpp. [INSTALLED|EXTERNAL|CUSTOM|AUTO]")
    endif()

    # CUSTOM 模式的 oatpp 目录与 EXTERNAL 模式的可选 git tag
    set(OATPP_DIR_SRC "" CACHE STRING "CUSTOM: directory containing oatpp headers")
    set(OATPP_DIR_LIB "" CACHE STRING "CUSTOM: directory containing liboatpp")
    set(OATPP_GIT_TAG "origin/master" CACHE STRING "EXTERNAL: oatpp git tag/branch to fetch")

    set(OATPP_MODULES_LOCATION_VALID INSTALLED EXTERNAL CUSTOM AUTO)
    if(NOT OATPP_MODULES_LOCATION IN_LIST OATPP_MODULES_LOCATION_VALID)
        message(FATAL_ERROR "oatpp-meta: unknown OATPP_MODULES_LOCATION '${OATPP_MODULES_LOCATION}'.\n"
                "    Valid values: INSTALLED / EXTERNAL / CUSTOM / AUTO (default).\n"
                "    Example: cmake -DOATPP_MODULES_LOCATION=EXTERNAL ..")
    endif()

    if(OATPP_MODULES_LOCATION STREQUAL "AUTO")
        # 自动顺序：已安装 oatpp 优先，找不到再远程拉取并本地构建
        message(STATUS "oatpp-meta: OATPP_MODULES_LOCATION=AUTO, probing for installed oatpp 1.3.0+ ...")
        find_package(oatpp 1.3.0 QUIET)
        if(oatpp_FOUND)
            set(_OATPP_MODE INSTALLED)
        else()
            message(STATUS "oatpp-meta: no installed oatpp 1.3.0+ found, falling back to EXTERNAL (download and build)")
            set(_OATPP_MODE EXTERNAL)
        endif()
    else()
        set(_OATPP_MODE ${OATPP_MODULES_LOCATION})
    endif()

    if(_OATPP_MODE STREQUAL "INSTALLED")
        message(STATUS "oatpp-meta: using installed oatpp (find_package)")
        find_package(oatpp 1.3.0 QUIET)
        if(NOT oatpp_FOUND)
            message(FATAL_ERROR "oatpp-meta: INSTALLED mode: installed oatpp 1.3.0+ not found.\n"
                    "    Install oatpp first, or choose another source, e.g.:\n"
                    "      cmake -DOATPP_MODULES_LOCATION=EXTERNAL ..\n"
                    "      cmake -DOATPP_MODULES_LOCATION=CUSTOM \\\n"
                    "            -DOATPP_DIR_SRC=<oatpp headers dir> -DOATPP_DIR_LIB=<dir containing liboatpp> ..\n"
                    "    If oatpp is installed elsewhere, point to it with -Doatpp_DIR=<dir containing oatppConfig.cmake>.")
        endif()
    elseif(_OATPP_MODE STREQUAL "EXTERNAL")
        message(STATUS "oatpp-meta: using EXTERNAL oatpp (GIT_TAG='${OATPP_GIT_TAG}', local build)")
        include(ExternalProject)
        set(LIB_OATPP_EXTERNAL oatpp_external)
        ExternalProject_Add(${LIB_OATPP_EXTERNAL}
                GIT_REPOSITORY "https://github.com/oatpp/oatpp.git"
                GIT_TAG ${OATPP_GIT_TAG}
                CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DOATPP_INSTALL=OFF -DOATPP_BUILD_TESTS=OFF
                INSTALL_COMMAND cmake -E echo "SKIP INSTALL '${LIB_OATPP_EXTERNAL}'"
        )
        ExternalProject_Get_Property(${LIB_OATPP_EXTERNAL} BINARY_DIR)
        set(OATPP_DIR_LIB ${BINARY_DIR}/src)
        ExternalProject_Get_Property(${LIB_OATPP_EXTERNAL} SOURCE_DIR)
        set(OATPP_DIR_SRC ${SOURCE_DIR}/src)
        set(OATPP_EXTERNAL_TARGET ${LIB_OATPP_EXTERNAL})
        message(STATUS "oatpp-meta: OATPP_DIR_SRC='${OATPP_DIR_SRC}'")
        message(STATUS "oatpp-meta: OATPP_DIR_LIB='${OATPP_DIR_LIB}'")
    elseif(_OATPP_MODE STREQUAL "CUSTOM")
        if(NOT OATPP_DIR_SRC OR NOT OATPP_DIR_LIB)
            message(FATAL_ERROR "oatpp-meta: CUSTOM mode requires both an oatpp headers dir and a library dir:\n"
                    "    cmake -DOATPP_MODULES_LOCATION=CUSTOM \\\n"
                    "          -DOATPP_DIR_SRC=<oatpp headers dir (e.g. <oatpp sources>/src)> \\\n"
                    "          -DOATPP_DIR_LIB=<dir containing liboatpp> ..")
        endif()
        if(NOT EXISTS "${OATPP_DIR_SRC}/oatpp/core/Types.hpp")
            message(FATAL_ERROR "oatpp-meta: oatpp/core/Types.hpp not found under OATPP_DIR_SRC.\n"
                    "    Make sure the dir contains oatpp headers (e.g. <oatpp sources>/src). Current value: '${OATPP_DIR_SRC}'")
        endif()
        message(STATUS "oatpp-meta: using CUSTOM oatpp")
        message(STATUS "oatpp-meta: OATPP_DIR_SRC='${OATPP_DIR_SRC}'")
        message(STATUS "oatpp-meta: OATPP_DIR_LIB='${OATPP_DIR_LIB}'")
    endif()
else()
    message(STATUS "oatpp-meta: reusing an oatpp target already provided by the caller (skipping dependency resolution)")
endif()

###################################################################################################

add_subdirectory(src)
add_subdirectory(test)
