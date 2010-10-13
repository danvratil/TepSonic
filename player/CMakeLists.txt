project(tepsonic)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules ${CMAKE_MODULE_PATH})

cmake_minimum_required(VERSION 2.6)

set(TEPSONIC_VERSION_MAJOR 0)
set(TEPSONIC_VERSION_MINOR 98)
set(TEPSONIC_VERSION_PATCH 0)
set(TEPSONIC_VERSION ${TEPSONIC_VERSION_MAJOR}.${TEPSONIC_VERSION_MINOR}.${TEPSONIC_VERSION_PATCH})

find_package(PkgConfig REQUIRED)
find_package(Qt4 4.5 REQUIRED)
find_package(Taglib REQUIRED)
find_package(Phonon REQUIRED)

add_definitions(${TAGLIB_CFLAGS})

add_definitions(-DQXT_STATIC -DBUILD_QXT_GUI -DBUILD_QXT_CORE)
set(QXT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qxt)
set(QXT_LIBRARY qxt)

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTSQL 1)

include(${QT_USE_FILE})

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_INCLUDE_DIR}
        ${PHONON_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
	${TAGLIB_INCLUDES}
        ${QXT_INCLUDE_DIR}
	"collections"
	"playlist"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/constants.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/constants.h)

set(player_HDRS
    mainwindow.h
    player.h
    statusbar.h
    abstractplugin.h
    pluginsmanager.h
    taskmanager.h
    databasemanager.h
    preferencesdialog.h
    preferencespages.h
    trayicon.h
    changeshortcutdialog.h
    filesystembrowser.h
    collections/collectionpopulator.h
#    collections/collectionbrowser.h
    collections/collectionmodel.h
#    collections/collectionitem.h
    collections/collectionbuilder.h
    collections/collectionproxymodel.h
    collections/collectionitemdelegate.h
    playlist/playlistbrowser.h
#    playlist/playlistitem.h
    playlist/playlistitemdelegate.h
    playlist/playlistmodel.h
    playlist/playlistproxymodel.h
    playlist/playlistpopulator.h
    playlist/playlistwriter.h

)

set(player_SRCS
    main.cpp
    constants.cpp
    mainwindow.cpp
    player.cpp
    statusbar.cpp
    pluginsmanager.cpp
    taskmanager.cpp
    databasemanager.cpp
    preferencesdialog.cpp
    preferencespages.cpp
    tools.cpp
    trayicon.cpp
    supportedformats.cpp
    changeshortcutdialog.cpp
    filesystembrowser.cpp
    collections/collectionpopulator.cpp
    collections/collectionbrowser.cpp
    collections/collectionmodel.cpp
    collections/collectionitem.cpp
    collections/collectionbuilder.cpp
    collections/collectionproxymodel.cpp
    collections/collectionitemdelegate.cpp
    playlist/playlistbrowser.cpp
    playlist/playlistitem.cpp
    playlist/playlistitemdelegate.cpp
    playlist/playlistmodel.cpp
    playlist/playlistproxymodel.cpp
    playlist/playlistpopulator.cpp
    playlist/playlistwriter.cpp
    ../3rdparty/qxt/qxtglobalshortcut.cpp
)

set(player_QUIS
    ui/mainwindow.ui
    ui/player.ui
    ui/preferencesdialog.ui
    ui/collections.ui
    ui/plugins.ui
    ui/shortcuts.ui
    ui/changeshortcutdialog.ui
)

set(player_RCCS  
    icons.qrc
)

option(LANGUAGES "Using translations")
if (NOT DEFINED LANGS)
  set (LANGUAGES cs CACHE STRING "Using translations" FORCE)
else (NOT DEFINED LANGS)
  if (NOT LANGS) 
    set (LANGUAGES "")
  else (NOT LANGS)
    string(REGEX MATCHALL [a-zA-Z]+
           langs1 ${LANGS})
    set (LANGUAGES ${langs1} CACHE STRING "Using translations" FORCE)
  endif (NOT LANGS)
endif (NOT DEFINED LANGS)

message("Translations to build: ${LANGUAGES}")

qt4_wrap_ui(player_QUIS_HDRS ${player_QUIS})
qt4_wrap_cpp(player_QMOC_SRCS ${player_HDRS})
qt4_add_resources(player_RCCS_SRCS ${player_RCCS})
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

add_custom_target (
        translations_qt
        COMMAND ${QT_LUPDATE_EXECUTABLE} ${HEADERS} ${SOURCES} -ts ${TRANSLATIONS})
add_custom_command (
        TARGET translations_qt
        COMMAND ${QT_LRELEASE_EXECUTABLE} ${TRANSLATIONS})



if (WIN32)
  set(GUI_TYPE WIN32)
endif (WIN32)

if (APPLEBUNDLE)
    set(GUI_TYPE MACOSX_BUNDLE)
    # create Info.plist file
    SET(MACOSX_BUNDLE_ICON_FILE tepsonic.icns)
    SET(MACOSX_BUNDLE_INFO_STRING "tepsonic ${VERSION}")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.tepsonic")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${TEPSONIC_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "${tepsonic}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${TEPSONIC_VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "${TEPSONIC_VERSION}")
    SET(MACOSX_BUNDLE_COPYRIGHT "(c) tepsonic.org")
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/macosx/tepsonic.icns
                                PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    # use icon for app bundle to be visible in finder
    SET(APPLE_BUNDLE_SOURCES ${CMAKE_SOURCE_DIR}/macosx/tepsonic.icns)
else (APPLEBUNDLE)
    # failback variable settings for non-bundle environments
    SET(APPLE_BUNDLE_SOURCES "")
endif (APPLEBUNDLE)

add_executable(tepsonic
  	       ${GUI_TYPE}
	       ${player_RCCS_SRCS}
	       ${player_QUIS_HDRS}
	       ${player_QMOC_SRCS}
	       ${player_SRCS}
	       ${TRANSLATIONS_BINARY}
           ${APPLE_BUNDLE_SOURCES}
)

target_link_libraries(tepsonic
		      ${QT_LIBRARIES}
		      ${QT_QTSQL_LIBRARIES}
		      ${PHONON_LIBS}
              ${TAGLIB_LIBRARIES}
              ${QXT_LIBRARY}
)
if (APPLE)
    target_link_libraries(tepsonic ${CARBON_LIBRARY})
endif (APPLE)



if (APPLEBUNDLE)
    
    # Used only for mac's bundle builds
    # Warning: This is dangerous to touch. Except you know what are you doing.
    #          I ususaly don't know anything about it. But it's working ;)
    INSTALL(TARGETS tepsonic BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

else (APPLEBUNDLE)        

    # standard instalation of the binary
    install(TARGETS tepsonic
	    RUNTIME DESTINATION ${INSTALL_BINARY_DIR}
    	PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

endif (APPLEBUNDLE)



if (UNIX AND NOT APPLEBUNDLE)
	install(FILES ${TRANSLATIONS_BINARY}
		DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/tepsonic
	)

	install(FILES ../tepsonic.desktop
		DESTINATION ${INSTALL_DATA_DIR}/applications
	)
	install(FILES ../icons/app_icons/16x16/tepsonic.png
		DESTINATION ${INSTALL_DATA_DIR}/icons/hicolor/16x16/apps
	)
	install(FILES ../icons/app_icons/32x32/tepsonic.png
        	DESTINATION ${INSTALL_DATA_DIR}/icons/hicolor/32x32/apps
	)
	install(FILES ../icons/app_icons/48x48/tepsonic.png
        	DESTINATION ${INSTALL_DATA_DIR}/icons/hicolor/48x48/apps
	)
	install(FILES ../icons/app_icons/64x64/tepsonic.png
        	DESTINATION ${INSTALL_DATA_DIR}/icons/hicolor/64x64/apps
	)
	install(FILES ../icons/app_icons/128x128/tepsonic.png
        	DESTINATION ${INSTALL_DATA_DIR}/icons/hicolor/128x128/apps
	)
	install(FILES ../icons/app_icons/256x256/tepsonic.png
        	DESTINATION ${INSTALL_DATA_DIR}/icons/hicolor/256x256/apps
	)
endif(UNIX AND NOT APPLEBUNDLE)

if (APPLEBUNDLE)
    install(FILES ${TRANSLATIONS_BINARY}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/tepsonic.app/Contents/Resources)
endif (APPLEBUNDLE)

