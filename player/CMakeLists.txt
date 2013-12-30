include_directories(${CMAKE_SOURCE_DIR}/core
                    ${CMAKE_BINARY_DIR}/core
                    bookmarks
                    collections
                    playlist)


set(player_SRCS
    main.cpp
    mainwindow.cpp
    statusbar.cpp
    taskmanager.cpp
    trayicon.cpp
    filesystemview.cpp
    filesystemwidget.cpp
    metadataeditor.cpp
    settings/settingspage.cpp
    settings/settingsdialog.cpp
    settings/playerpage.cpp
    settings/collectionspage.cpp
    settings/pluginspage.cpp
    settings/shortcutspage.cpp
    settings/changeshortcutdialog.cpp
    settings/playereffectdialog.cpp
    collections/collectionbuilder.cpp
    collections/collectionproxymodel.cpp
    collections/collectionitemdelegate.cpp
    collections/collectionview.cpp
    playlist/playlistview.cpp
    playlist/playlistitemdelegate.cpp
    playlist/playlistproxymodel.cpp
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
)

set(player_RCCS
    icons.qrc
)

qt5_wrap_ui(player_QUIS_HDRS ${player_QUIS})
qt5_add_resources(player_RCCS_SRCS ${player_RCCS})

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

set(QXT_LIBRARIES qxt)
if (UNIX)
    set(QXT_LIBRARIES ${QXT_LIBRARIES} Qt5::X11Extras ${X11_LIBRARIES})
endif ()

target_link_libraries(tepsonic
                      tepsonic_core
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql
                      Qt5::Widgets
                      Qt5::Concurrent
                      ${PHONON_LIBRARIES}
                      ${TAGLIB_LIBRARIES}
                      ${QXT_LIBRARIES}
)

set(player_TSS cs)
tepsonic_add_translations(tepsonic ${player_TSS})

# standard instalation of the binary
install(TARGETS tepsonic
        DESTINATION ${BIN_INSTALL_DIR}
)

install(FILES ../tepsonic.desktop
        DESTINATION ${DATA_INSTALL_DIR}/applications
)
install(FILES ../icons/app_icons/16x16/tepsonic.png
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/16x16/apps
)
install(FILES ../icons/app_icons/32x32/tepsonic.png
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/32x32/apps
)
install(FILES ../icons/app_icons/48x48/tepsonic.png
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/48x48/apps
)
install(FILES ../icons/app_icons/64x64/tepsonic.png
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/64x64/apps
)
install(FILES ../icons/app_icons/128x128/tepsonic.png
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/128x128/apps
)
install(FILES ../icons/app_icons/256x256/tepsonic.png
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/256x256/apps
)
