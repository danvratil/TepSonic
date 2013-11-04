cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)


include_directories(
        ${QT_INCLUDE_DIR}
)
include(${QT_USE_FILE})
# this is mandatory for Apple builds to load plugin correctly
add_definitions(-DQT_NO_DEBUG)
remove_definitions(-DQT_NO_KEYWORDS)

set(QXT-SOURCES
  qxtglobal.cpp
  qxtglobalshortcut.cpp
)

set(QXT-MOC-HEADERS
  qxtglobalshortcut.h
)

set(QXT-SOURCES ${QXT-SOURCES} qxtglobalshortcut_x11.cpp)

QT4_WRAP_CPP(QXT-SOURCES-MOC ${QXT-MOC-HEADERS})

ADD_LIBRARY(qxt STATIC
  ${QXT-SOURCES}
  ${QXT-SOURCES-MOC}
)
