# Default function for generating QM files from TS

CODECFORTR = UTF-8 
CODECFORSRC = UTF-8

!isEmpty(TRANSLATIONS) {
   isEmpty(QMAKE_LRELEASE) {
      win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
      else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
   }
   isEmpty(TRANSLOUT): TRANSLOUT = locale/tepsonic

   TSQM.name = lrelease ${QMAKE_FILE_IN}
   TSQM.input = TRANSLATIONS
   TSQM.output = build/target/locale/$$TRANSLOUT/${QMAKE_FILE_BASE}.qm
   TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm build/target/locale/$$TRANSLOUT/${QMAKE_FILE_BASE}.qm
   TSQM.CONFIG = no_link target_predeps
   QMAKE_EXTRA_COMPILERS += TSQM
   PRE_TARGETDEPS += compiler_TSQM_make_all
}
