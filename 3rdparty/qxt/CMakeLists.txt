cmake_minimum_required(VERSION 2.6)

# this is mandatory for Apple builds to load plugin correctly
add_definitions(-DQT_NO_DEBUG -DQXT_STATIC -DBUILD_QXT_GUI -DBUILD_QXT_CORE)
remove_definitions(-DQT_NO_KEYWORDS)

set(QXT_SOURCES
  qxtglobal.cpp
  qxtglobalshortcut.cpp
)

set(QXT-MOC-HEADERS
  qxtglobalshortcut.h
)

if (UNIX)
    set(QXT_SOURCES ${QXT_SOURCES} qxtglobalshortcut_x11.cpp)
elseif (WIN32)
    set(QXT_SOURCES ${QXT_SOURCES} qxtglobalshortcut_win.cpp)
elseif (APPLE)
    set(QXT_SOURCES ${QXT_SOURCES} qxtglobalshortcut_mac.cpp)
endif ()

qt5_wrap_cpp(QXT_SOURCES_MOC ${QXT_MOC_HEADERS})

add_library(qxt STATIC
  ${QXT_SOURCES}
  ${QXT_SOURCES_MOC}
)
