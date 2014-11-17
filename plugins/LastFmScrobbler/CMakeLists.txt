# Make sure all Qt parts will be available
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

add_definitions(-DQT_PLUGIN -DQT_SHARED)

include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_BINARY_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
)

set(lastfmscrobbler_SRCS
    src/lastfm.cpp
    src/lastfmscrobblerplugin.cpp
    src/auth.cpp
    src/cache.cpp
    src/scrobbler.cpp
    src/track.cpp
)

set(lastfmscrobbler_QUIS
    ui/lastfmscrobblerconfig.ui
)

qt5_wrap_ui(lastfmscrobbler_QUIS_HDRS ${lastfmscrobbler_QUIS})


tepsonic_add_plugin(tepsonic_lastfmscrobbler
                    ${lastfmscrobbler_SRCS}
                    ${lastfmscrobbler_QUIS_HDRS}
                    ${lastfmscrobbler_QMOC_SRCS}
)

target_link_libraries(tepsonic_lastfmscrobbler
                      tepsonic_core
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Network
                      Qt5::Xml
)

set(lastfmscrobbler_TSS cs)
tepsonic_add_translations(tepsonic_lastfmscrobbler ${lastfmscrobbler_TSS})

install(TARGETS tepsonic_lastfmscrobbler
        DESTINATION ${LIB_INSTALL_DIR}/qt5/plugins/tepsonic/
)
