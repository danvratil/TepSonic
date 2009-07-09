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
    collectionbrowser.cpp \
    playlistbrowser.cpp \
    preferencesdialog.cpp
HEADERS += mainwindow.h \
    collectionbrowser.h \
    playlistbrowser.h \
    preferencesdialog.h
FORMS += mainwindow.ui \
    preferencesdialog.ui
RESOURCES += icons.qrc
