unix {
    isEmpty(PREFIX): PREFIX = /usr
    isEmpty(LIBDIR): LIBDIR = lib
    isEmpty(BINDIR): BINDIR = bin
    isEmpty(PKGDATADIR): PKGDATADIR = share/tepsonic

    BINDIR = $$PREFIX/$$BINDIR
    LIBDIR = $$PREFIX/$$LIBDIR
    PKGDATADIR = $$PREFIX/$$PKGDATADIR

    DEFINES += PKGDATADIR=\\\"$$PKGDATADIR\\\" \
	       PREFIX=\\\"$$PEFIX\\\" \
	       LIBDIR=\\\"$$LIBDIR\\\" \
	       BINDIR=\\\"$$BINDIR\\\"
}

