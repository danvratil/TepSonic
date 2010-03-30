TEMPLATE = subdirs

CONFIG += debug

SUBDIRS = src \
  plugins

DISTFILES += LICENSE

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

