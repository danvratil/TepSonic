# - Try to find the LastFmLib
# Once done this will define
#
#  LASTFMLIB_FOUND - system has the library
#  LASTFMLIB_INCLUDE_DIR - includes
#  LASTFMLIB_LIBS - libs

FIND_PATH(LASTFMLIB_INCLUDE_DIR NAMES lastfmscrobbler.h  PATHS /usr/include/lastfmlib)

FIND_LIBRARY(LASTFMLIB_LIBS NAMES lastfmlib PATHS /usr/lib /usr/local/lib)

MARK_AS_ADVANCED(LASTFMLIB_INCLUDE_DIR LASTFMLIB_LIBS)

INCLUDE(FindPackageMessage)
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LASTFMLIB DEFAULT_MSG LASTFMLIB_INCLUDE_DIR LASTFMLIB_LIBS)
