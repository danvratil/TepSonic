# -------------------------------------------------
# Project created by QtCreator 2010-03-08T12:50:43
# -------------------------------------------------
QT += phonon \
    network \
    gui
TARGET = tepsonic_lastfmscrobbler
TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += . \
    ../../ \
    ../../build \
    /usr/include/Phonon \
    /usr/include/KDE
DEFINES += LASTFMSCROBBLER_LIBRARY
SOURCES += lastfmscrobbler.cpp
HEADERS += lastfmscrobbler.h
FORMS += lastfmscrobblerconfig.ui
DESTDIR = ../../build/plugins
VERSION = 0.1.0
