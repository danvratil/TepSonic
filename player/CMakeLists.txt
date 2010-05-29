project(tepsonic)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules ${CMAKE_MODULE_PATH})

cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE "Release")
set(TEPSONIC_VERSION_MAJOR 0)
set(TEPSONIC_VERSION_MINOR 96)
set(TEPSONIC_VERSION_PATCH 0)
set(TEPSONIC_VERSION ${TEPSONIC_VERSION_MAJOR}.${TEPSONIC_VERSION_MINOR}.${TEPSONIC_VERSION_PATCH})

find_package(PkgConfig REQUIRED)
find_package(Qt4 REQUIRED)
find_package(Taglib REQUIRED)
find_package(Phonon REQUIRED)

add_definitions(${TAGLIB_CFLAGS})

# Make sure all Qt parts will be available
set(QT_USE_QTMAIN 1)
set(QT_USE_QTSQL 1)

include(${QT_USE_FILE})

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_INCLUDE_DIR}
        ${PHONON_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
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

if (WIN32)
  set(GUI_TYPE WIN32)
endif (WIN32)

add_executable(tepsonic
  	       ${GUI_TYPE}
	       ${player_RCCS_SRCS}
	       ${player_QUIS_HDRS}
	       ${player_QMOC_SRCS}
	       ${player_TRANS_QM}
	       ${player_SRCS}
)

target_link_libraries(tepsonic
		      ${QT_LIBRARIES}
		      ${QT_QTSQL_LIBRARIES}
		      ${PHONON_LIBS}
                      ${TAGLIB_LIBRARIES}
)

install(TARGETS tepsonic
	RUNTIME DESTINATION ${INSTALL_BINARY_DIR}
	PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

if (UNIX)
	install(FILES ${player_TRANS_QM}
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
endif(UNIX)
