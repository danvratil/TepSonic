project(mpris2Plugin)

cmake_minimum_required(VERSION 2.6)

# Add custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules ${CMAKE_MODULE_PATH})

if (NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif (NOT DEFINED CMAKE_BUILD_TYPE)

find_package(PkgConfig REQUIRED)
find_package(Qt4 REQUIRED)

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTDBUS 1)

include(${QT_USE_FILE})

add_definitions(${QT_DEFINITIONS}
                -DQT_PLUGIN
                -DQT_SHARED
)

include_directories(
        ${QT_INCLUDE_DIR}
        ${QT_QTNETWORK_INCLUDE_DIR}
        ${QT_QTXML_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../player
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../player/constants.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/constants.h)

set(mpris2plugin_HDRS
    src/dbusabstractadaptor.h
    src/mpris2plugin.h
    src/mprismediaplayer2.h
    src/mprismediaplayer2player.h
    ../../player/abstractplugin.h
)

set(mpris2plugin_SRCS
    src/dbusabstractadaptor.cpp
    src/mpris2plugin.cpp
    src/mprismediaplayer2.cpp
    src/mprismediaplayer2player.cpp
    ../../player/constants.cpp
)

qt4_wrap_cpp(mpris2plugin_QMOC_SRCS ${mpris2plugin_HDRS})

add_library(tepsonic_mpris2plugin SHARED
               ${mpris2plugin_SRCS}
               ${mpris2plugin_QMOC_SRCS}
)

target_link_libraries(tepsonic_mpris2plugin
               ${QT_LIBRARIES}
               ${QT_DBUS_LIBRARY}
)

# standard UNIX and windows
set_target_properties(tepsonic_mpris2plugin
                PROPERTIES
                VERSION 0.1.0
                SOVERSION 1
)
install(TARGETS tepsonic_mpris2plugin
        DESTINATION ${INSTALL_LIBRARY_DIR}/
)
