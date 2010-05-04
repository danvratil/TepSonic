
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    "collections"
		    "playlist"
)

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
    collections/collectionmodel.h
    collections/collectionbuilder.h
    collections/collectionproxymodel.h
    collections/collectionpopulator.h
    playlist/playlistbrowser.h
    playlist/playlistmodel.h
    playlist/playlistpopulator.h
    playlist/playlistwriter.h
    playlist/playlistproxymodel.h
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
    collections/collectionpopulator.cpp
    collections/collectionbrowser.cpp
    collections/collectionmodel.cpp
    collections/collectionitem.cpp
    collections/collectionbuilder.cpp
    collections/collectionproxymodel.cpp
    playlist/playlistbrowser.cpp
    playlist/playlistitem.cpp
    playlist/playlistmodel.cpp
    playlist/playlistproxymodel.cpp
    playlist/playlistpopulator.cpp
    playlist/playlistwriter.cpp
)

set(player_QUIS
    ui/mainwindow.ui
    ui/player.ui
    ui/preferencesdialog.ui
    ui/collections.ui
    ui/plugins.ui
)

set(player_RCCS  
    icons.qrc
)

set(player_TRANS
    ts/cs_CZ.ts
)

qt4_wrap_ui(player_QUIS_HDRS ${player_QUIS})
qt4_wrap_cpp(player_QMOC_SRCS ${player_HDRS})
qt4_add_resources(player_RCCS_SRCS ${player_RCCS})
qt4_add_translation(player_TRANS_QM ${player_TRANS})

add_executable(tepsonic
	       ${player_RCCS_SRCS}
	       ${player_QUIS_HDRS}
	       ${player_QMOC_SRCS}
	       ${player_TRANS_QM}
	       ${player_SRCS}
)

target_link_libraries(tepsonic
		      ${QT_LIBRARIES}
		      ${QT_PHONON_LIBRARIES}
		      ${QT_SQL_LIBRARIES}
		      ${Taglib_LIBRARIES}
)

install(FILES ${EXECUTABLE_OUTPUT_PATH}/tepsonic
	DESTINATION ${INSTALL_BINARY_DIR}
)
install(FILES ${player_TRANS_QM}
	DESTINATION ${INSTALL_DATA_DIR}/locale/tepsonic
)
