project(LastFmScrobbler)

cmake_minimum_required(VERSION 2.6)

# Add custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules ${CMAKE_MODULE_PATH})

set(CMAKE_BUILD_TYPE "Release")

find_package(PkgConfig REQUIRED)
find_package(Qt4 4.6 REQUIRED)

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTNETWORK 1)

include(${QT_USE_FILE})

#add_definitions(${QT_DEFINITIONS}
#		-DQT_PLUGIN
#		-DQT_NO_DEBUG
#		-DQT_SHARED
#)

include_directories(${QT_INCLUDE_DIR}
		    ${QT_QTNETWORK_INCLUDE_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
)

set(lastfmscrobbler_HDRS
    src/lastfmscrobbler.h
    src/abstractplugin.h
)

set(lastfmscrobbler_SRCS
    src/lastfmscrobbler.cpp
)

set(lastfmscrobbler_QUIS
    ui/lastfmscrobblerconfig.ui
)

set(lastfmscrobbler_TRNS
    ts/cs_CZ.ts
)

qt4_wrap_ui(lastfmscrobbler_QUIS_HDRS ${lastfmscrobbler_QUIS})
qt4_wrap_cpp(lastfmscrobbler_QMOC_SRCS ${lastfmscrobbler_HDRS})
qt4_add_translation(lastfmscrobbler_TRNS_QM ${lastfmscrobbler_TRNS})

add_library(tepsonic_lastfmscrobbler SHARED
	       ${lastfmscrobbler_QUIS_HDRS}
	       ${lastfmscrobbler_QMOC_SRCS}
	       ${lastfmscrobbler_TRNS_QM}
	       ${lastfmsrobbler_SRCS}
)

set_target_properties(tepsonic_lastfmscrobbler
                      PROPERTIES
                      VERSION 0.1.0
                      SOVERSION 1
)

target_link_libraries(tepsonic_lastfmscrobbler LINK_INTERFACE_LIBRARIES
	       ${QT_LIBRARIES}
	       ${QT_QTNETWORK_LIBRARIES}
)

install(TARGETS tepsonic_lastfmscrobbler
	DESTINATION ${INSTALL_LIBRARY_DIR}/
)
install(FILES ${lastfmscrobbler_TRNS_QM}
	DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/lastfmscrobbler
)
