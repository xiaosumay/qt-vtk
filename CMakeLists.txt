cmake_minimum_required(VERSION 3.5)

project(test-vtk LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH $ENV{HOME}/Heli/VTK-9.2.6)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(VTK 9.2.6 CONFIG REQUIRED)

file(GLOB_RECURSE SRC_LISTS
        CONFIGURE_DEPENDS
        src/*.cpp
        src/*.h
        src/*.ui)

file(GLOB_RECURSE TS_FILES
        CONFIGURE_DEPENDS
        trans/*.ts)
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(${PROJECT_NAME} ${SRC_LISTS} ${TS_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets ${VTK_LIBRARIES})
