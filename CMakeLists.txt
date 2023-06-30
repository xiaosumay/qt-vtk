cmake_minimum_required(VERSION 3.5)

project(test-vtk VERSION 1.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME})

list(APPEND CMAKE_PREFIX_PATH $ENV{HOME}/Heli/VTK-9.2.6)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(VTK 9 CONFIG REQUIRED)

file(GLOB_RECURSE SRC_LISTS
        CONFIGURE_DEPENDS
        src/*.cpp
        src/*.h
        src/*.ui)

file(GLOB_RECURSE TS_FILES
        CONFIGURE_DEPENDS
        trans/*.ts)
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})

target_sources(${PROJECT_NAME} PRIVATE ${SRC_LISTS} ${TS_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets ${VTK_LIBRARIES})

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

    if ("${PROJECT_TYPE}" STREQUAL "EXECUTABLE")
        set_target_properties(${target_name} PROPERTIES LINK_FLAGS " -Wl,-rpath,\$ORIGIN:\$ORIGIN/../lib ")
    else ()
        set_target_properties(${target_name} PROPERTIES LINK_FLAGS " -Wl,-rpath,\$ORIGIN ")
    endif ()
endfunction()

if (CMAKE_BUILD_TYPE MATCHES "(MinSizeRel|Release)")
    SetUpRPATH(${PROJECT_NAME})
endif ()
