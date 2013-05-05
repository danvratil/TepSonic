project(LyricsPlugin)

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
set(QT_USE_QTNETWORK 1)
set(QT_USE_QTXML 1)

include(${QT_USE_FILE})
# this is mandatory for Apple builds to load plugin correctly
add_definitions(-DQT_NO_DEBUG)

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

set(lyricsplugin_HDRS
    src/lyrics.h
    ../../player/abstractplugin.h
)

set(lyricsplugin_SRCS
    src/lyrics.cpp
    ../../player/constants.cpp
)

option(LANGS "Using translations" "")
if (NOT DEFINED LANGS)
  set (LANGS cs CACHE STRING "Using translations" FORCE)
  set (LANGUAGES cs)
else (NOT DEFINED LANGS)
  if (NOT LANGS)
    set(LANGUAGES "")
  else (NOT LANGS)
    string(REGEX MATCHALL [a-zA-Z]+
           langs1 ${LANGS})
    set (LANGUAGES ${langs1})
  endif (NOT LANGS)
endif (NOT DEFINED LANGS)


qt4_wrap_cpp(lyricsplugin_QMOC_SRCS ${lyricsplugin_HDRS})
#Create translations
set(TRANSLATIONS)
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/ts/${LANGUAGE}.ts)
        set (QM ${CMAKE_CURRENT_SOURCE_DIR}/ts/lyricsplugin_${LANGUAGE}.qm)
	set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
        add_custom_command (OUTPUT ${QM}
			    PRE_BUILD
	                    COMMAND ${QT_LRELEASE_EXECUTABLE}
		            ARGS ${TS} -qm ${QM})
	add_custom_target(lf_transl_${LANGUAGE} DEPENDS ${QM})
	set (TRANSLATIONS ${TRANSLATIONS} lf_transl_${LANGUAGE})
endforeach()


add_library(tepsonic_lyricsplugin SHARED
	       ${lyricsplugin_SRCS}
	       ${lyricsplugin_QMOC_SRCS}
	       ${TRANSLATIONS_BINARY}
)

if (TRANSLATIONS)
	add_dependencies(tepsonic_lyricsplugin ${TRANSLATIONS})
endif (TRANSLATIONS)

target_link_libraries(tepsonic_lyricsplugin
	       ${QT_LIBRARIES}
	       ${QT_QTNETWORK_LIBRARIES}
               ${QT_QTXML_LIBRARIES}
)

# standard UNIX and windows
set_target_properties(tepsonic_lyricsplugin
                PROPERTIES
                VERSION 0.1.0
                SOVERSION 1
)
install(TARGETS tepsonic_lyricsplugin
        DESTINATION ${INSTALL_LIBRARY_DIR}/
)
install(FILES ${TRANSLATIONS_BINARY}
        DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/
)
