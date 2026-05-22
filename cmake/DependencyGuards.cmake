# Forbid rfs_core from acquiring any link interface against SFML or miniaudio.
function(rfs_assert_no_forbidden_deps target forbidden_targets)
    get_target_property(_link_libs ${target} LINK_LIBRARIES)
    if(_link_libs)
        foreach(_lib IN LISTS _link_libs)
            foreach(_forbidden IN LISTS forbidden_targets)
                if(_lib MATCHES "${_forbidden}")
                    message(FATAL_ERROR
                        "Dependency guard: target '${target}' must not link '${_lib}' "
                        "(forbidden pattern: '${_forbidden}'). "
                        "This breaks invariant I-01.")
                endif()
            endforeach()
        endforeach()
    endif()
endfunction()