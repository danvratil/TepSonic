# -------------------------------------------------
# Project created by QtCreator 2010-03-08T12:50:43
# -------------------------------------------------
QT += phonon \
    network \
    gui
TARGET = tepsonic_lastfmscrobbler
PLUGINNAME = lastfmscrobbler
VERSION = 0.1.0
TEMPLATE = lib
CONFIG += plugin \
    debug
INCLUDEPATH += . \
    ../../src \
    ../../src/build/moc \
    /usr/include/Phonon \
    /usr/include/KDE
DEFINES += LASTFMSCROBBLER_LIBRARY
SOURCES += src/lastfmscrobbler.cpp \
           ../../src/constants.cpp
HEADERS += src/lastfmscrobbler.h \
           ../../src/constants.h
FORMS += ui/lastfmscrobblerconfig.ui
DESTDIR = build/target
MOC_DIR = build/moc
OBJECTS_DIR = build/obj

include(ts/ts.pri)
include(../../dirs.pri)

unix {
    INSTALLS += target
    target.path = $$LIBDIR
    INSTALLS += translations
    translations.path = $$PKGDATADIR/locale/$$PLUGINNAME
    translations.files += ts/*.qm
}
