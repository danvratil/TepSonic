# -------------------------------------------------
# Project created by QtCreator 2009-05-12T23:34:03
# -------------------------------------------------
QT += xml \
    phonon \
    dbus
TARGET = tepsonic
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    preferencesdialog.cpp \
    playlistmodel.cpp \
    playlistbrowser.cpp \
    collectionbrowser.cpp \
    playlistitem.cpp
HEADERS += mainwindow.h \
    preferencesdialog.h \
    playlistmodel.h \
    playlistbrowser.h \
    collectionbrowser.h \
    playlistitem.h
FORMS += mainwindow.ui \
    preferencesdialog.ui
RESOURCES += icons.qrc
