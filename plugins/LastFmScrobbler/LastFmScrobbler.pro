# -------------------------------------------------
# Project created by QtCreator 2010-03-08T12:50:43
# -------------------------------------------------
QT += phonon \
    network \
    gui
TARGET = tepsonic_lastfmscrobbler
TEMPLATE = lib
CONFIG += plugin \
    debug
INCLUDEPATH += . \
    ../../src \
    ../../build/moc \
    /usr/include/Phonon \
    /usr/include/KDE
DEFINES += LASTFMSCROBBLER_LIBRARY
SOURCES += src/lastfmscrobbler.cpp
HEADERS += src/lastfmscrobbler.h
FORMS += ui/lastfmscrobblerconfig.ui
DESTDIR = ../../build/target/plugins
VERSION = 0.1.0
