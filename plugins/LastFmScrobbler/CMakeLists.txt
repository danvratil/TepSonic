project(LastFmScrobbler)

cmake_minimum_required(VERSION 2.6)

# Add custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules ${CMAKE_MODULE_PATH})

if (NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif (NOT DEFINED CMAKE_BUILD_TYPE)

find_package(PkgConfig REQUIRED)
find_package(Qt4 REQUIRED)
find_package(LastFmLib REQUIRED)

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTNETWORK 1)

include(${QT_USE_FILE})
# this is mandatory for Apple builds to load plugin correctly
add_definitions(-DQT_NO_DEBUG)

add_definitions(${QT_DEFINITIONS}
		-DQT_PLUGIN
		-DQT_SHARED
)

# LASTFMLIB_INCLUDES points to BASE/lastfmlib/ and source uses "lastfmlib/foo.h"
# the LASTFMLIB_INCLUDES/../ solves it
include_directories(
	${QT_INCLUDE_DIR}
	${QT_QTNETWORK_INCLUDE_DIR}
        ${LASTFMLIB_INCLUDES}
        ${LASTFMLIB_INCLUDES}/../
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../player
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../player/constants.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/constants.h)

set(lastfmscrobbler_HDRS
    src/lastfmscrobblerplugin.h
    ../../player/abstractplugin.h
)

set(lastfmscrobbler_SRCS
    src/lastfmscrobblerplugin.cpp
    ../../player/constants.cpp
)

set(lastfmscrobbler_QUIS
    ui/lastfmscrobblerconfig.ui
)

option(LANGUAGES "Using translations")
if (NOT DEFINED LANGS)
  set (LANGUAGES cs CACHE STRING "Using translations" FORCE)
else (NOT DEFINED LANGS)
  if (NOT LANGS)
    set(LANGUAGES "")
  else (NOT LANGS)
    string(REGEX MATCHALL [a-zA-Z]+
           langs1 ${LANGS})
    set (LANGUAGES ${langs1} CACHE STRING "Using translations" FORCE)
  endif (NOT LANGS)
endif (NOT DEFINED LANGS)


qt4_wrap_ui(lastfmscrobbler_QUIS_HDRS ${lastfmscrobbler_QUIS})
qt4_wrap_cpp(lastfmscrobbler_QMOC_SRCS ${lastfmscrobbler_HDRS})
#Create translations
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/ts/${LANGUAGE}.ts)
        set (QM ${CMAKE_CURRENT_SOURCE_DIR}/ts/${LANGUAGE}.qm)
        set (TRANSLATIONS ${TRANSLATIONS} ${TS})
        set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
        add_custom_command (
                OUTPUT ${QM}
                COMMAND ${QT_LRELEASE_EXECUTABLE} ${TS}
                MAIN_DEPENDENCY ${TS})
endforeach()


add_library(tepsonic_lastfmscrobbler SHARED
	       ${lastfmscrobbler_SRCS}
	       ${lastfmscrobbler_QUIS_HDRS}
	       ${lastfmscrobbler_QMOC_SRCS}
	       ${TRANSLATIONS_BINARY}
)

target_link_libraries(tepsonic_lastfmscrobbler
	       ${QT_LIBRARIES}
	       ${QT_QTNETWORK_LIBRARIES}
               ${LASTFMLIB_LIBS}
)

if (NOT APPLEBUNDLE)
    # standard UNIX and windows
    set_target_properties(tepsonic_lastfmscrobbler
                  PROPERTIES
                  VERSION 0.2.0
                  SOVERSION 1
    )
    install(TARGETS tepsonic_lastfmscrobbler
	    DESTINATION ${INSTALL_LIBRARY_DIR}/
    )
    install(FILES ${TRANSLATIONS_BINARY}
	    DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/lastfmscrobbler
    )
else (NOT APPLEBUNDLE)
    # no symlinks for apple bundle
    #set_target_properties(tepsonic_lastfmscrobbler PROPERTIES PREFIX "" SUFFIX ".dylib")    
    install(TARGETS tepsonic_lastfmscrobbler
            DESTINATION ${CMAKE_INSTALL_PREFIX}/tepsonic.app/Contents/plugins
    )
    install(FILES ${TRANSLATIONS_BINARY}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/tepsonic.app/Contents/Resources/lastfmscrobbler
    )
endif (NOT APPLEBUNDLE)

