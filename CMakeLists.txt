project(tepsonic)

cmake_minimum_required(VERSION 2.6)

set(TEPSONIC_VERSION_MAJOR 0)
set(TEPSONIC_VERSION_MINOR 99)
set(TEPSONIC_VERSION_PATCH 0)
set(TEPSONIC_VERSION ${TEPSONIC_VERSION_MAJOR}.${TEPSONIC_VERSION_MINOR}.${TEPSONIC_VERSION_PATCH})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
find_package(Qt4 4.5 REQUIRED)
find_package(Taglib REQUIRED)
find_package(Phonon REQUIRED)

include(${QT_USE_FILE})

add_definitions(${QT_DEFINITIONS} ${TAGLIB_CFLAGS})


# Use native libqxt on Linux and patched sources in /3rdparty/qxt on Mac an Win.
if (NOT FORCE_INBUILD_QXT)
        set(QXT_USE_QXTCORE TRUE)
        set(QXT_USE_QXTGUI TRUE)
        find_package(Qxt REQUIRED)
        set(QXT_HEADERS "")
        set(QXT_SOURCES "")
else ()
        add_definitions(-DQXT_STATIC -DBUILD_QXT_GUI -DBUILD_QXT_CORE)
        set(QXT_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt")
        set(QXT_LIBRARIES qxt)
        set(QXT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt/qxtglobalshortcut.h")
        set(QXT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt/qxtglobalshortcut.cpp")
endif ()


include_directories(
        ${QT_INCLUDE_DIR}
        ${QT_QTSQL_INCLUDE_DIR}
        ${PHONON_INCLUDES}
        ${TAGLIB_INCLUDES}
        ${QXT_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/player
        ${CMAKE_CURRENT_BINARY_DIR}/player
)

# Add custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if (NOT DEFINED CMAKE_INSTALL_PREFIX)
   set(CMAKE_INSTALL_PREFIX /usr/local)
endif (NOT DEFINED CMAKE_INSTALL_PREFIX)

if (NOT DEFINED LIB_SUFFIX)
  # Detect 32 vs. 64 bit arch
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(LIB_SUFFIX 64)
  else()
    set(LIB_SUFFIX "")
  endif()
else (NOT DEFINED LIB_SUFFIX)
   set(LIB_SUFFIX ${LIB_SUFFIX})
endif (NOT DEFINED LIB_SUFFIX)

if (NOT DEFINED CMAKE_INSTALL_BINARY_DIR)
   set(INSTALL_BINARY_DIR ${CMAKE_INSTALL_PREFIX}/bin)
else (NOT DEFINED CMAKE_INSTALL_BINARY_DIR)
   set(INSTALL_BINARY_DIR ${CMAKE_INSTALL_BINARY_DIR})
endif (NOT DEFINED CMAKE_INSTALL_BINARY_DIR)

if (NOT DEFINED CMAKE_INSTALL_LIBRARY_DIR)
   set(INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
else (NOT DEFINED CMAKE_INSTALL_LIBRARY_DIR)
   set(INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_LIBRARY_DIR})
endif (NOT DEFINED CMAKE_INSTALL_LIBRARY_DIR)

if (NOT DEFINED CMAKE_INSTALL_DATA_DIR)
   set(INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share)
else (NOT DEFINED CMAKE_INSTALL_DATA_DIR)
   set(INSTALL_DATA_DIR ${CMAKE_INSTALL_DATA_DIR})
endif (NOT DEFINED CMAKE_INSTALL_DATA_DIR)

add_definitions(${QT_DEFINITIONS}
		-DPKGDATADIR="${INSTALL_DATA_DIR}"
		-DLIBDIR="${INSTALL_LIBRARY_DIR}"
		-Wall)

# First player, then plugins, this is strict order!
add_subdirectory(3rdparty)
add_subdirectory(player)
add_subdirectory(plugins)
