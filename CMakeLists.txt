cmake_minimum_required(VERSION 2.6)

OPTION(APPLEBUNDLE "Enable Mac OS X bundle build" OFF)
OPTION(APPLEBUNDLE_STANDALONE "Do not copy required libs and tools into bundle (APPLEBUNDLE)" OFF)

if (NOT APPLE)
    # those vars are valid for macosx only
    unset (APPLEBUNDLE)
    unset (APPLEBUNDLE_STANDALONE)
else (NOT APPLE)
    if (APPLEBUNDLE_STANDALONE)
        set (APPLEBUNDLE 1)
    endif (APPLEBUNDLE_STANDALONE)
endif (NOT APPLE)


# Add custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if (NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif (NOT DEFINED CMAKE_BUILD_TYPE)

if (NOT DEFINED CMAKE_INSTALL_PREFIX) 
   set(CMAKE_INSTALL_PREFIX /usr/local)
endif (NOT DEFINED CMAKE_INSTALL_PREFIX)

if (NOT DEFINED LIB_SUFFIX)
   set(LIB_SUFFIX)
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

if (APPLE)
   FIND_LIBRARY(CARBON_LIBRARY Carbon)
   MESSAGE(STATUS "CARBON_LIBRARY: ${CARBON_LIBRARY}")
endif (APPLE)
if (APPLEBUNDLE)
    add_definitions(-DAPPLEBUNDLE)
    install(CODE "message(STATUS \"Cleaning previously installed bundle (rm -r)\")")
    install(CODE "execute_process(COMMAND rm -r ${CMAKE_INSTALL_PREFIX}/tepsonic.app)")
endif (APPLEBUNDLE)

add_definitions(${QT_DEFINITIONS}
		-DPKGDATADIR="${INSTALL_DATA_DIR}" 
		-DLIBDIR="${INSTALL_LIBRARY_DIR}"
		-Wall)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if (PROFILING)
    SET(CMAKE_CXX_FLAGS_PROFILE "-pg -O0")
    SET(CMAKE_CXX_FLAGS_COVERAGE "-ftest-coverage -fprofile-arcs -O0")
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -pg")
    SET(LINK_FLAGS "${LINK_FLAGS} -pg")
    MESSAGE("Profiling enabled")
endif (PROFILING)

# First player, then plugins, this is strict order!
add_subdirectory(3rdparty)
add_subdirectory(player)
add_subdirectory(plugins)
if (APPLEBUNDLE_STANDALONE)
    add_subdirectory(macosx)
endif (APPLEBUNDLE_STANDALONE)


# make dist custom target
SET(CPACK_PACKAGE_NAME "tepsonic")
# TODO/FIXME: versioning from player subdir... I don't know why it's separated...
SET(CPACK_PACKAGE_VERSION "0.98.0")
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\\\.qm$;/build/;")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

