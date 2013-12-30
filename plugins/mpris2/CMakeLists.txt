find_package(Qt5DBus REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_BINARY_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5DBus_INCLUDE_DIRS}
)

add_definitions(-DQT_PLUGIN -DQT_SHARED)

set(mpris2plugin_HDRS
    src/dbusabstractadaptor.h
    src/mpris2plugin.h
    src/mprismediaplayer2.h
    src/mprismediaplayer2player.h
)

set(mpris2plugin_SRCS
    src/dbusabstractadaptor.cpp
    src/mpris2plugin.cpp
    src/mprismediaplayer2.cpp
    src/mprismediaplayer2player.cpp
)

add_library(tepsonic_mpris2plugin SHARED
               ${mpris2plugin_SRCS}
               ${mpris2plugin_QMOC_SRCS}
)

target_link_libraries(tepsonic_mpris2plugin
               Qt5::Core
               Qt5::DBus
)

# standard UNIX and windows
set_target_properties(tepsonic_mpris2plugin
                PROPERTIES
                VERSION 0.1.0
                SOVERSION 1
)
install(TARGETS tepsonic_mpris2plugin
        DESTINATION ${INSTALL_PLUGINS_DIR}/tepsonic/
)
