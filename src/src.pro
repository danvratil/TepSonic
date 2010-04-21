QT += phonon \
    sql
TEMPLATE = app
CONFIG += debug
TARGET = tepsonic
LIBS += -ltag
DEPENDPATH += .
INCLUDEPATH += . \
    collections \
    playlist \
    /usr/include/KDE \
    /usr/include/Phonon
DESTDIR = ../build/target/
OBJECTS_DIR = ../build/obj/
MOC_DIR = ../build/moc/


# Input
HEADERS += constants.h \
    mainwindow.h \
    player.h \
    statusbar.h \
    abstractplugin.h \
    plugininterface.h \
    pluginsmanager.h \
    taskmanager.h \
    databasemanager.h \
    preferencesdialog.h \
    preferencespages.h \
    tools.h \
    collections/collectionbrowser.h \
    collections/collectionmodel.h \
    collections/collectionitem.h \
    collections/collectionbuilder.h \
    collections/collectionproxymodel.h \
    collections/collectionpopulator.h \
    playlist/playlistbrowser.h \
    playlist/playlistitem.h \
    playlist/playlistmodel.h \
    playlist/playlistpopulator.h \
    playlist/playlistwriter.h \
    playlist/playlistproxymodel.h
FORMS += ui/mainwindow.ui \
    ui/player.ui \
    ui/preferencesdialog.ui \
    ui/collections.ui \
    ui/plugins.ui
SOURCES += main.cpp \
    constants.cpp \
    mainwindow.cpp \
    player.cpp \
    statusbar.cpp \
    pluginsmanager.cpp \
    taskmanager.cpp \
    databasemanager.cpp \
    preferencesdialog.cpp \
    preferencespages.cpp \
    tools.cpp \
    collections/collectionpopulator.cpp \
    collections/collectionbrowser.cpp \
    collections/collectionmodel.cpp \
    collections/collectionitem.cpp \
    collections/collectionbuilder.cpp \
    collections/collectionproxymodel.cpp \
    playlist/playlistbrowser.cpp \
    playlist/playlistitem.cpp \
    playlist/playlistmodel.cpp \
    playlist/playlistproxymodel.cpp \
    playlist/playlistpopulator.cpp \
    playlist/playlistwriter.cpp
RESOURCES += ../icons.qrc

# Translations
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
include(../ts/ts.pri)

unix {
    isEmpty(PREFIX):PREFIX = /usr
    BINDIR = $$PREFIX/bin
    INSTALLS += target
    target.path = $$BINDIR
    DATADIR = $$PREFIX/share
    PKGDATADIR = $$DATADIR/tepsonic
    DEFINES += DATADIR=\\\"$$DATADIR\\\" \
        PKGDATADIR=\\\"$$PKGDATADIR\\\"
    INSTALLS += translations
    translations.path = $$PKGDATADIR
    translations.files += $$DESTDIR/ts
}


