cmake_minimum_required(VERSION 3.16)

project(NebulaTahoe VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ACTIVAR EL PROCESADOR DE RECURSOS
set(CMAKE_AUTORCC ON)

# AGREGAMOS DBus AQUÍ
find_package(Qt6 REQUIRED COMPONENTS Quick Multimedia Core Gui Svg Core5Compat DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)

qt_standard_project_setup()

# AQUÍ CAMBIAMOS Main.qml POR resources.qrc
qt_add_executable(appNebulaTahoe
    main.cpp
    player.h
    resources.qrc
)

target_include_directories(appNebulaTahoe PRIVATE ${TAGLIB_INCLUDE_DIRS})

# ENLAZAMOS Qt6::DBus AQUÍ
target_link_libraries(appNebulaTahoe PRIVATE
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Core5Compat
    Qt6::DBus
    ${TAGLIB_LIBRARIES}
)

install(TARGETS appNebulaTahoe DESTINATION bin)
