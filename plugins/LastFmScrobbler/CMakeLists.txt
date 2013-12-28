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


qt5_wrap_ui(lastfmscrobbler_QUIS_HDRS ${lastfmscrobbler_QUIS})

#Create translations
set(TRANSLATIONS)
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/ts/${LANGUAGE}.ts)
        set (QM ${CMAKE_CURRENT_SOURCE_DIR}/ts/lastfmscrobbler_${LANGUAGE}.qm)
	set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
        add_custom_command (OUTPUT ${QM}
			    PRE_BUILD
	                    COMMAND ${QT_LRELEASE_EXECUTABLE}
		            ARGS ${TS} -qm ${QM})
	add_custom_target(lf_transl_${LANGUAGE} DEPENDS ${QM})
	set (TRANSLATIONS ${TRANSLATIONS} lf_transl_${LANGUAGE})
endforeach()


add_library(tepsonic_lastfmscrobbler SHARED
	       ${lastfmscrobbler_SRCS}
	       ${lastfmscrobbler_QUIS_HDRS}
	       ${lastfmscrobbler_QMOC_SRCS}
	       ${TRANSLATIONS_BINARY}
)

if (TRANSLATIONS)
	add_dependencies(tepsonic_lastfmscrobbler ${TRANSLATIONS})
endif (TRANSLATIONS)

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

install(TARGETS tepsonic_lastfmscrobbler
        DESTINATION ${INSTALL_LIBRARY_DIR}/
)
install(FILES ${TRANSLATIONS_BINARY}
        DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/
)
