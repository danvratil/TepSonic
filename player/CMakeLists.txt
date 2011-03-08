project(tepsonic)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules ${CMAKE_MODULE_PATH})

cmake_minimum_required(VERSION 2.6)

set(TEPSONIC_VERSION_MAJOR 0)
set(TEPSONIC_VERSION_MINOR 98)
set(TEPSONIC_VERSION_PATCH 2)
set(TEPSONIC_VERSION ${TEPSONIC_VERSION_MAJOR}.${TEPSONIC_VERSION_MINOR}.${TEPSONIC_VERSION_PATCH})

find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
find_package(Qt4 4.5 REQUIRED)
find_package(Taglib REQUIRED)
find_package(Phonon REQUIRED)

# Use native libqxt on Linux and patched sources in /3rdparty/qxt on Mac an Win.
if (UNIX AND NOT APPLE)
	set(QXT_USE_QXTCORE TRUE)
	set(QXT_USE_QXTGUI TRUE)
	find_package(Qxt REQUIRED)
	set(QXT_HEADERS "")
	set(QXT_SOURCES "")
else (UNIX AND NOT APPLE)
	add_definitions(-DQXT_STATIC -DBUILD_QXT_GUI -DBUILD_QXT_CORE)
	set(QXT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qxt)
	set(QXT_LIBRARIES qxt)
        set(QXT_HEADERS "../3rdparty/qxt/qxtglobalshortcut.h")
	set(QXT_SOURCES "../3rdparty/qxt/qxtglobalshortcut.cpp")
endif (UNIX AND NOT APPLE)

  

if (NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif (NOT DEFINED CMAKE_BUILD_TYPE)

add_definitions(${TAGLIB_CFLAGS})

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTSQL 1)

include(${QT_USE_FILE})
# this is mandatory for Apple builds to load plugin correctly
add_definitions(-DQT_NO_DEBUG)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_INCLUDE_DIR}
        ${PHONON_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
	${TAGLIB_INCLUDES}
        ${QXT_INCLUDE_DIRS}
	"collections"
	"playlist"
	"settings"
        "bookmarks"
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
    trayicon.h
    filesystembrowser.h
    settings/settingsdialog.h
    settings/playerpage.h
    settings/collectionspage.h
    settings/pluginspage.h
    settings/shortcutspage.h
    settings/changeshortcutdialog.h
    collections/collectionpopulator.h
    collections/collectionmodel.h
    collections/collectionbuilder.h
    collections/collectionproxymodel.h
    collections/collectionitemdelegate.h
    playlist/playlistbrowser.h
    playlist/playlistitemdelegate.h
    playlist/playlistmodel.h
    playlist/playlistproxymodel.h
    playlist/playlistpopulator.h
    playlist/playlistwriter.h
    bookmarks/bookmarksbrowser.h
    bookmarks/bookmarksmanager.h
    bookmarks/addbookmarkdlg.h
    ${QXT_HEADERS}
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
    tools.cpp
    trayicon.cpp
    supportedformats.cpp
    filesystembrowser.cpp
    settings/settingsdialog.cpp
    settings/playerpage.cpp
    settings/collectionspage.cpp
    settings/pluginspage.cpp
    settings/shortcutspage.cpp
    settings/changeshortcutdialog.cpp
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
    bookmarks/bookmarksbrowser.cpp
    bookmarks/bookmarksitem.cpp
    bookmarks/bookmarksmanager.cpp
    bookmarks/addbookmarkdlg.cpp
    ${QXT_SOURCES}
)

set(player_QUIS
    ui/mainwindow.ui
    settings/playerpage.ui
    settings/settingsdialog.ui
    settings/collectionspage.ui
    settings/pluginspage.ui
    settings/shortcutspage.ui
    settings/changeshortcutdialog.ui
    bookmarks/addbookmarkdlg.ui
)

set(player_RCCS  
    icons.qrc
)

option(LANGS "Using translations" "")
if (NOT DEFINED LANGS)
  set (LANGS cs CACHE STRING "Using translations" FORCE)
  set (LANGUAGES cs)
else (NOT DEFINED LANGS)
  if (NOT LANGS) 
    set (LANGUAGES "")
  else (NOT LANGS)
    string(REGEX MATCHALL [a-zA-Z]+
           langs1 ${LANGS})
    set (LANGUAGES ${langs1})
  endif (NOT LANGS)
endif (NOT DEFINED LANGS)

message(STATUS "Translations to build: ${LANGUAGES}")

qt4_wrap_ui(player_QUIS_HDRS ${player_QUIS})
qt4_wrap_cpp(player_QMOC_SRCS ${player_HDRS})
qt4_add_resources(player_RCCS_SRCS ${player_RCCS})
#Create translations
set(TRANSLATIONS)
foreach (LANGUAGE ${LANGUAGES})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/ts/${LANGUAGE}.ts)
        set (QM ${CMAKE_CURRENT_SOURCE_DIR}/ts/tepsonic_${LANGUAGE}.qm)
	set (TRANSLATIONS_BINARY ${TRANSLATIONS_BINARY} ${QM})
	add_custom_command(OUTPUT ${QM}
			   PRE_BUILD
			   COMMAND ${QT_LRELEASE_EXECUTABLE}
			   ARGS ${TS} -qm ${QM})			   
	add_custom_target(transl_${LANGUAGE} DEPENDS ${QM})
	set (TRANSLATIONS ${TRANSLATIONS} transl_${LANGUAGE})
endforeach()

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

if (TRANSLATIONS)
	add_dependencies(tepsonic ${TRANSLATIONS})
endif (TRANSLATIONS)

target_link_libraries(tepsonic
		      ${QT_LIBRARIES}
		      ${QT_QTSQL_LIBRARIES}
		      ${PHONON_LIBS}
	              ${TAGLIB_LIBRARIES}
        	      ${QXT_LIBRARIES}
		      ${X11_LIBRARIES}
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
    )

endif (APPLEBUNDLE)



if (UNIX AND NOT APPLEBUNDLE)
	install(FILES ${TRANSLATIONS_BINARY}
		DESTINATION ${INSTALL_DATA_DIR}/tepsonic/locale/
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

