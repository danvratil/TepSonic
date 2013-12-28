find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

add_definitions(-DQT_PLUGIN -DQT_SHARED)

include_directories(
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set(lyricsplugin_HDRS
    src/lyrics.h
    src/lyricsscrollarea.h
)

set(lyricsplugin_SRCS
    src/lyrics.cpp
    src/lyricsscrollarea.cpp
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
               Qt5::Core
               Qt5::Network
               Qt5::Xml
)

# standard UNIX and windows
set_target_properties(tepsonic_lyricsplugin
                PROPERTIES
                VERSION 0.1.0
                SOVERSION 1
)
install(TARGETS tepsonic_lyricsplugin
        DESTINATION ${INSTALL_PLUGINS_DIR}/tepsonic
)
install(FILES ${TRANSLATIONS_BINARY}
        DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/
)
