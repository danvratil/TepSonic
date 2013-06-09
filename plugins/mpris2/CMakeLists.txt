set(QT_USE_QTDBUS 1)

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

qt4_wrap_cpp(mpris2plugin_QMOC_SRCS ${mpris2plugin_HDRS})

add_library(tepsonic_mpris2plugin SHARED
               ${mpris2plugin_SRCS}
               ${mpris2plugin_QMOC_SRCS}
)

target_link_libraries(tepsonic_mpris2plugin
               ${QT_CORE_LIBRARY}
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
install(FILES mpris2.desktop
        DESTINATION ${INSTALL_DATA_DIR}/tepsonic/plugins/
)
