project(LastFmScrobbler)

cmake_minimum_required(VERSION 2.6)

# Add custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules ${CMAKE_MODULE_PATH})

find_package(PkgConfig REQUIRED)
find_package(Qt4 4.6 REQUIRED)

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTNETWORK 1)
include(${QT_USE_FILE})

add_definitions(${QT_DEFINITIONS}
		-DQT_PLUGIN
		-DQT_SHARED
)

include_directories(${QT_INCLUDE_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}/../../../player
)

set(lastfmscrobbler_HDRS
    src/lastfmscrobbler.h
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
qt4_add_resources(lastfmscrobbler_RCCS_SRCS ${lastfmscrobbler_RCCS})
qt4_add_translation(lastfmscrobbler_TRNS_QM ${lastfmscrobbler_TRNS})

add_library(tepsonic_lastfmscrobbler SHARED
	       ${lastfmscrobbler_RCCS_SRCS}
	       ${lastfmscrobbler_QUIS_HDRS}
	       ${lastfmscrobbler_QMOC_SRCS}
	       ${lastfmscrobbler_TRNS_QM}
	       ${lastfmsrobbler_SRCS}
)

target_link_libraries(tepsonic_lastfmscrobbler
	       ${QT_LIBRARIES}
	       ${QT_NETWORK_LIBRARIES}
)

install(FILES ${LIBRARY_OUTPUT_PATH}/libtepsonic_lastfmscrobbler.so
	DESTINATION ${INSTALL_LIBRARY_DIR}/
)
install(FILES ${lastfmscrobbler_TRNS_QM}
	DESTINATION ${INSTALL_DATA_DIR}/locale/lastfmscrobbler
)
