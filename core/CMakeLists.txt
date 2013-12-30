configure_file(${CMAKE_CURRENT_SOURCE_DIR}/constants.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/constants.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
)

set(core_SRCS
    abstractplugin.cpp
    actionmanager.cpp
    collectionmodel.cpp
    constants.cpp
    databasemanager.cpp
    m3u.cpp
    metadata.cpp
    player.cpp
    playlistmodel.cpp
    pluginsmanager.cpp
    settings.cpp
    supportedformats.cpp
    utils.cpp
)

set(core_HDRS
    abstractplugin.h
    actionmanager.h
    collectionmodel.h
    ${CMAKE_CURRENT_BINARY_DIR}/contants.h
    databasemanager.h
    m3u.h
    metadata.h
    player.h
    playlistmodel.h
    pluginsmanager.h
    settings.h
    supportedformats.h
    ${CMAKE_CURRENT_BINARY_DIR}/tepsonic-core-export.h
    utils.h
)

add_library(tepsonic_core SHARED
            ${core_SRCS}
)

set_target_properties(tepsonic_core
    PROPERTIES VERSION ${TEPSONIC_VERSION}
    SOVERSION ${TEPSONIC_VERSION_MAJOR}
)

target_link_libraries(tepsonic_core
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Sql
                      Qt5::Widgets
                      Qt5::Concurrent
                      ${PHONON_LIBRARIES}
                      ${TAGLIB_LIBRARIES}
)

generate_export_header(tepsonic_core EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/tepsonic-core-export.h)

#set(core_TSS cs)
#tepsonic_add_translations(tepsonic_core ${core_TSS})

install(TARGETS tepsonic_core
        EXPORT TepSonicLibraryTargets
        ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES ${core_HDRS}
        DESTINATION ${INCLUDE_INSTALL_DIR}/tepsonic
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/TepSonicConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/TepSonicConfig.cmake @ONLY
)

# this file is used by to check if the installed version can be used.
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/TepSonicConfigVersion.cmake
                                     ${TEPSONIC_VERSION_MAJOR} ${TEPSONIC_VERSION_MINOR} ${TEPSONIC_VERSION_PATCH}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/TepSonicConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/TepSonicConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/TepSonic
)

# Install the file with the exported targets
install(EXPORT TepSonicLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/TepSonic
        FILE TepSonicCoreTargetsWithPrefix.cmake
)
