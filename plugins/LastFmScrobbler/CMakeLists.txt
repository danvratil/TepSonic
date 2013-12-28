# Make sure all Qt parts will be available
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

add_definitions(-DQT_PLUGIN -DQT_SHARED)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
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

add_library(tepsonic_lastfmscrobbler SHARED
	       ${lastfmscrobbler_SRCS}
	       ${lastfmscrobbler_QUIS_HDRS}
	       ${lastfmscrobbler_QMOC_SRCS}
)

target_link_libraries(tepsonic_lastfmscrobbler
	       Qt5::Core
	       Qt5::Gui
	       Qt5::Network
               Qt5::Xml
)

# standard UNIX and windows
set_target_properties(tepsonic_lastfmscrobbler
                PROPERTIES
                VERSION 0.2.0
                SOVERSION 1
)

set(lastfmscrobbler_TSS cs)
tepsonic_add_translations(tepsonic_lastfmscrobbler ${lastfmscrobbler_TSS})

install(TARGETS tepsonic_lastfmscrobbler
        DESTINATION ${INSTALL_PLUGINS_DIR}/tepsonic/
)
