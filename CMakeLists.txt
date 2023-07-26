cmake_minimum_required(VERSION 3.15)

project(test_vtk VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (WIN32 AND (CMAKE_BUILD_TYPE MATCHES "Release"))
    add_executable(${PROJECT_NAME} WIN32)
else ()
    add_executable(${PROJECT_NAME})
endif ()

if (WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

    list(APPEND CMAKE_PREFIX_PATH C:/Qt/VTK-8.2.0)
    list(APPEND CMAKE_PREFIX_PATH C:/Qt/Qt5.12.9/5.12.9/msvc2017_64)
else ()
    list(APPEND CMAKE_PREFIX_PATH /opt/VTK-8.2.0)
endif ()

####################################################################################
if (UNIX)
    set(VCPKG_LOCAL_PATH $ENV{HOME}/.local/vcpkg)
elseif (WIN32)
    set(VCPKG_LOCAL_PATH C:/Qt/vcpkg)
endif ()
if (EXISTS ${VCPKG_LOCAL_PATH})
    set(VCPKG_LIBRARY_LINKAGE "static")
    if (UNIX)
        set(VCPKG_TARGET_TRIPLET x64-linux CACHE STRING "x64-linux")
    elseif (WIN32)
        set(VCPKG_TARGET_TRIPLET x64-windows CACHE STRING "x64-windows")
    endif ()

    include(${VCPKG_LOCAL_PATH}/scripts/buildsystems/vcpkg.cmake)
else ()
    message(WARNING " Please setup vcpkg path at [${VCPKG_LOCAL_PATH}] ")
endif ()
####################################################################################
find_package(Qt5 COMPONENTS Widgets LinguistTools Concurrent REQUIRED)
find_package(VTK 8 CONFIG REQUIRED)

set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(PCL CONFIG REQUIRED)
####################################################################################
file(GLOB_RECURSE SRC_LISTS
        CONFIGURE_DEPENDS
        src/*.cpp
        src/*.h
        src/*.ui)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SRC_LISTS})
#==================================================================================#
file(GLOB_RECURSE TS_FILES
        trans/*.ts)
source_group("ts Files" FILES ${TS_FILES})
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_LIST_DIR}/src ${TS_FILES})
#==================================================================================#
target_sources(${PROJECT_NAME} PRIVATE ${SRC_LISTS} ${QM_FILES})
####################################################################################
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Concurrent ${VTK_LIBRARIES} ${PCL_LIBRARIES})

if (UNIX)
    # 设置目标的编译参数，讲警告等级设置为最高
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra            # 尽可能多的错误
            -Wno-unknown-pragmas     # 无效的pragmas
            -Werror -Wfatal-errors   # 将所有的警告视为错误并立即停止编译
            -Wfloat-equal            # 浮点数对比
            -ffunction-sections      # 用于section优化
            -Wsign-compare           # 有符号与无符号比较
            -Wdisabled-optimization  # 不能被优化
            -Wshadow                 # 当一个局部变量遮盖住了另一个局部变量
            -Wnarrowing              # 窄转换
            -Woverloaded-virtual     # 如果函数的声明隐藏住了基类的虚函数
            -Wdouble-promotion       # 禁止隐形float提升为double
            -Wno-return-local-addr   # 返回局部地址
            -Wswitch                 # switch覆盖
            )

    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)

    #在发布的时候去掉rpath属性并设置自己规则
    function(SetUpRPATH target_name)
        get_target_property(PROJECT_TYPE ${target_name} TYPE)

        set_target_properties(
                ${target_name}
                PROPERTIES
                SKIP_RPATH ON         # 关闭cmake自带的rpath，无用且冗余
                SKIP_BUILD_RPATH ON   # 关闭cmake自带的build_rpath
                SKIP_INSTALL_RPATH ON # 关闭cmake自带的install_rpath
        )

        get_target_property(OLD_LINK_FLAGS ${target_name} LINK_FLAGS)
        if ("${PROJECT_TYPE}" STREQUAL "EXECUTABLE")
            set_target_properties(${target_name} PROPERTIES LINK_FLAGS "${OLD_LINK_FLAGS} -Wl,-rpath,\$ORIGIN:\$ORIGIN/../lib ")
        else ()
            set_target_properties(${target_name} PROPERTIES LINK_FLAGS "${OLD_LINK_FLAGS} -Wl,-rpath,\$ORIGIN ")
        endif ()
    endfunction()

    if (CMAKE_BUILD_TYPE MATCHES "(MinSizeRel|Release)")
        SetUpRPATH(${PROJECT_NAME})
    endif ()
endif ()
########################################################################################################################
# 通过git记录编译时所在的提交ID，方便以后排查问题
if (NOT GIT_FOUND)
    find_package(Git QUIET)
endif ()

execute_process(COMMAND
        "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=8
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        OUTPUT_VARIABLE GIT_SHA1
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND
        "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        OUTPUT_VARIABLE GIT_BRANCH
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

string(TIMESTAMP COMPILE_TIME %Y%m%d)

# 生成版本信息
configure_file(version_internal.h.in src/version_internal.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR}/src)
########################################################################################################################