find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

add_definitions(-DQT_PLUGIN -DQT_SHARED)

include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_BINARY_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
)

set(lyricsplugin_HDRS
    src/lyrics.h
    src/lyricsscrollarea.h
)

set(lyricsplugin_SRCS
    src/lyrics.cpp
    src/lyricsscrollarea.cpp
)

#set(lyricsplugin_TSS)
#tepsonic_add_translations(tepsonic_lyricsplugin ${lyricsplugin_TSS})

tepsonic_add_plugin(tepsonic_lyricsplugin
                    ${lyricsplugin_SRCS}
                    ${lyricsplugin_QMOC_SRCS}
)

target_link_libraries(tepsonic_lyricsplugin
                      tepsonic_core
                      Qt5::Core
                      Qt5::Network
                      Qt5::Xml
)

install(TARGETS tepsonic_lyricsplugin
        DESTINATION ${LIB_INSTALL_DIR}/qt5/plugins/tepsonic/
)
