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

tepsonic_add_plugin(tepsonic_mpris2plugin
                    ${mpris2plugin_SRCS}
                    ${mpris2plugin_QMOC_SRCS}
)

target_link_libraries(tepsonic_mpris2plugin
                      tepsonic_core
                      Qt5::Core
                      Qt5::DBus
)

install(TARGETS tepsonic_mpris2plugin
        DESTINATION ${LIB_INSTALL_DIR}/qt5/plugins/tepsonic/
)
