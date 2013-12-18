include_directories(bookmarks
                    collections
                    playlist)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/constants.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/constants.h)

set(player_SRCS
    main.cpp
    constants.cpp
    mainwindow.cpp
    player.cpp
    statusbar.cpp
    abstractplugin.cpp
    pluginsmanager.cpp
    taskmanager.cpp
    databasemanager.cpp
    tools.cpp
    trayicon.cpp
    supportedformats.cpp
    filesystembrowser.cpp
    metadataeditor.cpp
    settings/settingspage.cpp
    settings/settingsdialog.cpp
    settings/playerpage.cpp
    settings/collectionspage.cpp
    settings/pluginspage.cpp
    settings/shortcutspage.cpp
    settings/changeshortcutdialog.cpp
    settings/playereffectdialog.cpp
    collections/collectionbrowser.cpp
    collections/collectionmodel.cpp
    collections/collectionbuilder.cpp
    collections/collectionproxymodel.cpp
    collections/collectionitemdelegate.cpp
    playlist/playlistbrowser.cpp
    playlist/playlistitemdelegate.cpp
    playlist/playlistmodel.cpp
    playlist/playlistproxymodel.cpp
    playlist/playlistpopulator.cpp
    playlist/playlistwriter.cpp
    bookmarks/bookmarksbrowser.cpp
    bookmarks/bookmarksitem.cpp
    bookmarks/bookmarksmanager.cpp
    bookmarks/addbookmarkdlg.cpp
)

set(player_QUIS
    ui/mainwindow.ui
    ui/metadataeditor.ui
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

qt5_wrap_ui(player_QUIS_HDRS ${player_QUIS})
qt5_add_resources(player_RCCS_SRCS ${player_RCCS})
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

add_executable(tepsonic
               ${GUI_TYPE}
               ${player_RCCS_SRCS}
               ${player_QUIS_HDRS}
               ${player_QMOC_SRCS}
               ${player_SRCS}
               ${TRANSLATIONS_BINARY}
)

if (TRANSLATIONS)
    add_dependencies(tepsonic ${TRANSLATIONS})
endif (TRANSLATIONS)

set(QXT_LIBRARIES qxt)
if (UNIX)
    set(QXT_LIBRARIES ${QXT_LIBRARIES} Qt5::X11Extras ${X11_LIBRARIES})
endif ()

target_link_libraries(tepsonic
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql
                      Qt5::Widgets
                      ${PHONON_LIBRARIES}
                      ${TAGLIB_LIBRARIES}
                      ${QXT_LIBRARIES}
)

# standard instalation of the binary
install(TARGETS tepsonic
        RUNTIME DESTINATION ${INSTALL_BINARY_DIR}
)

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
