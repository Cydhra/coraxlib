# Taken from: https://crascit.com/2016/01/31/enhanced-source-file-handling-with-target_sources/
# Cmake earlier to 3.12, paths passed to target_sources were relative to the main CMakeLists.txt
# This is very unintuitive, we therefore emulate the new behaviour of taking absolute paths.

# NOTE: This helper function assumes no generator expressions are used for the source files.
function (get_sources_local SRC_LIST)
    unset(_srcList)

    foreach (src ${ARGN})
        if (NOT src STREQUAL "PRIVATE"
            AND NOT src STREQUAL "PUBLIC"
            AND NOT src STREQUAL "INTERFACE"
        )
            # Append absolute path of source
            if (NOT IS_ABSOLUTE "${src}")
                set(src "${CMAKE_CURRENT_LIST_DIR}/${src}")
            endif()
        endif()
        list(APPEND _srcList "${src}")
    endforeach ()
    set(${SRC_LIST} ${_srcList} PARENT_SCOPE)
endfunction ()

function (target_sources_local target)
    if (POLICY CMP0076)
        # New behavior is available, so just forward to it by ensuring that we have the policy set
        # to request the new behavior, but don't change the policy setting for the calling scope.
        cmake_policy(PUSH)
        cmake_policy(SET CMP0076 NEW)
        target_sources(${target} ${ARGN})
        cmake_policy(POP)
        return()
    endif ()

    # Must be using CMake 3.12 or earlier, so simulate the new behavior
    unset(_srcList)
    get_sources_local(_srcList ${ARGN})
    target_sources(${target} ${_srcList})
endfunction ()
