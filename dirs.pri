unix {
    isEmpty(PREFIX): PREFIX = /usr
    PKGDATADIR = $$PREFIX/share/tepsonic
    BINDIR = $$PREFIX/bin
    LIBDIR = $$PREFIX/lib
    DEFINES += PKGDATADIR=\\\"$$PKGDATADIR\\\"
}

