# 纯头文件库，必须使用 INTERFACE：
# 之前声明为 STATIC 但没有 .cpp 源文件，ar 无法从空目标创建归档，导致构建失败
add_library(oatpp_meta INTERFACE)

target_sources(oatpp_meta INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/meta.hpp)

# 接入根 CMakeLists.txt 选定的 oatpp 来源（OATPP_MODULES_LOCATION）：
#   - INSTALLED / 复用调用方安装：链接 imported target oatpp::oatpp（头文件与链接信息由其传递）
#   - EXTERNAL / CUSTOM：OATPP_DIR_SRC 为 oatpp 头文件目录，OATPP_DIR_LIB 为含 liboatpp 的目录
if(TARGET oatpp::oatpp)
    target_link_libraries(oatpp_meta INTERFACE oatpp::oatpp)
else()
    if(OATPP_DIR_SRC)
        target_include_directories(oatpp_meta INTERFACE ${OATPP_DIR_SRC})
    endif()
    if(OATPP_DIR_LIB)
        target_link_directories(oatpp_meta INTERFACE ${OATPP_DIR_LIB})
    endif()
    target_link_libraries(oatpp_meta INTERFACE oatpp)
endif()

target_include_directories(oatpp_meta INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
