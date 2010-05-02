unix {
    isEmpty(PREFIX): PREFIX = /usr
    isEmpty(LIBDIR): LIBDIR = $$PREFIX/lib
    isEmpty(BINDIR): BINDIR = $$PREFIX/bin
    isEmpty(PKGDATADIR): PKGDATADIR = $$PREFIX/share/tepsonic
    DEFINES += PKGDATADIR=\\\"$$PKGDATADIR\\\"
}

