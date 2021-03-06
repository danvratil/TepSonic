project(tepsonic)

cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)

set(TEPSONIC_VERSION_MAJOR 1)
set(TEPSONIC_VERSION_MINOR 61)
set(TEPSONIC_VERSION_PATCH 0)
set(TEPSONIC_VERSION ${TEPSONIC_VERSION_MAJOR}.${TEPSONIC_VERSION_MINOR}.${TEPSONIC_VERSION_PATCH})

add_definitions(-DTEPSONIC_VERSION="\\\"${TEPSONIC_VERSION}\\\"")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)

include(InstallSettings)
include(GenerateExportHeader)
include(MacroWriteBasicCMakeVersionFile)
include(TepSonicMacros)

find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Taglib REQUIRED)
find_package(Phonon4Qt5 REQUIRED)

add_definitions(${QT_DEFINITIONS}
                ${TAGLIB_CFLAGS}
                -DQT_NO_CAST_FROM_ASCII
                -DQT_NO_CAST_TO_ASCII
                -DQT_NO_KEYWORDS
                -DQT_USE_FAST_CONCATENATION
                -DQT_USE_FAST_OPERATOR_PLUS
)

if (UNIX)
    find_package(Qt5X11Extras REQUIRED)
    include_directories(${Qt5X11Extras_INCLUDE_DIRS})
endif ()

if (NOT DEFINED LIB_SUFFIX)
  # Detect 32 vs. 64 bit arch
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(LIB_SUFFIX 64)
  else()
    set(LIB_SUFFIX "")
  endif()
else (NOT DEFINED LIB_SUFFIX)
   set(LIB_SUFFIX ${LIB_SUFFIX})
endif ()

set(LIB_INSTALL_DIR lib${LIB_SUFFIX})

add_definitions(-DPKGDATADIR="${DATA_INSTALL_DIR}"
                -DLIBDIR="${LIB_INSTALL_DIR}"
                -Wall)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER MATCHES "icc" OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  # more aggressive warnings and C++11 support
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wno-long-long -std=iso9899:1990 -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -Wextra -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wnon-virtual-dtor -Wundef -Wcast-align -Wchar-subscripts -Wall -Wextra -Wpointer-arith -Wformat-security -fno-common")

  # debugfull target
  set(CMAKE_CXX_FLAGS_DEBUGFULL "-g0 -fno-inline" CACHE STRING "Flags used by the C++ compiler during debugfull builds." FORCE)
  set(CMAKE_C_FLAGS_DEBUGFULL   "-g0 -fno-inline" CACHE STRING "Flags used by the C compiler during debugfull builds." FORCE)
  mark_as_advanced(CMAKE_CXX_FLAGS_DEBUGFULL CMAKE_C_FLAGS_DEBUGFULL)

  # Update the documentation string of CMAKE_BUILD_TYPE for ccache & cmake-gui
  set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
      "Choose the type of build, options are: None debugfull Debug Release RelWithDebInfo MinSizeRel."
      FORCE)

  # coverage support
  if(CMAKE_COMPILE_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofile_rt" CACHE STRING "Flags used by the linker" FORCE)
      set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -lprofile_rt" CACHE STRING "Flags used by the linker" FORCE)
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofile_rt" CACHE STRING "Flags used by the linker" FORCE)
    endif()
  endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${PHONON_INCLUDE_DIR}
)

add_subdirectory(3rdparty)
add_subdirectory(core)
add_subdirectory(player)
add_subdirectory(plugins)
