macro (tepsonic_add_translations _target _languages)
  set (_install_target_dir "${CMAKE_INSTALL_PREFIX}/share/tepsonic/locale/")
  set (_translations)
  foreach (language ${_languages})
        set (TS ${CMAKE_CURRENT_SOURCE_DIR}/ts/${language}.ts)
        set (QM ${CMAKE_CURRENT_BINARY_DIR}/qm/${_target}_${language}.qm)
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/qm)
        add_custom_command(OUTPUT ${QM}
                           PRE_BUILD
                           COMMAND Qt5::lrelease
                           ARGS ${TS} -qm ${QM})
        add_custom_target(${_target}_i18n_${language} DEPENDS ${QM})
        set (_translations ${_translations} ${_target}_i18n_${language})
        install (FILES ${QM} DESTINATION ${_install_target_dir})
    endforeach  ()
    add_dependencies(${_target} ${_translations})
endmacro ()
