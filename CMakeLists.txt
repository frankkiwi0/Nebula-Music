cmake_minimum_required(VERSION 3.16)
project(NebulaMusic VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Multimedia DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswscale)

set(SOURCES
    main.cpp
    NebulaCenter.cpp
    NebulaCenter.h
    resources.qrc
    core/NebulaPlayer.cpp
    core/NebulaPlayer.h
    data/DataManager.cpp
    data/DataManager.h
    mpris/MprisHandler.cpp
    mpris/MprisHandler.h
    visualizer/AudioVisualizer.qml 
)

add_executable(NebulaMusic ${SOURCES})

target_include_directories(NebulaMusic PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core        
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/mpris
    ${TAGLIB_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(NebulaMusic PRIVATE 
    Qt6::Core Qt6::Gui Qt6::Quick Qt6::Multimedia Qt6::DBus
    ${TAGLIB_LIBRARIES}
    ${FFMPEG_LIBRARIES}
)