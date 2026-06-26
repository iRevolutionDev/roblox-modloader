if (NOT GIT_EXECUTABLE)
    find_package(Git QUIET)
endif ()

set(GIT_SHA1 "unknown")
set(GIT_SHORT_SHA1 "unknown")
set(GIT_BRANCH "unknown")
set(GIT_DATE "unknown")
set(GIT_COMMIT_SUBJECT "")
set(GIT_DIRTY_BOOL "false")

if (GIT_EXECUTABLE)
    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
            WORKING_DIRECTORY "${VERSION_REPO_DIR}"
            OUTPUT_VARIABLE _sha RESULT_VARIABLE _rc
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (_rc EQUAL 0 AND _sha)
        if (_sha MATCHES "-dirty$")
            set(GIT_DIRTY_BOOL "true")
            string(REGEX REPLACE "-dirty$" "" _sha "${_sha}")
        endif ()
        set(GIT_SHA1 "${_sha}")
        string(SUBSTRING "${_sha}" 0 7 GIT_SHORT_SHA1)
    endif ()

    execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --format=%cd --date=format:%Y-%m-%d
            WORKING_DIRECTORY "${VERSION_REPO_DIR}"
            OUTPUT_VARIABLE _date ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (_date)
        set(GIT_DATE "${_date}")
    endif ()

    execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --format=%s
            WORKING_DIRECTORY "${VERSION_REPO_DIR}"
            OUTPUT_VARIABLE _subject ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
            
    string(REPLACE "\\" "\\\\" _subject "${_subject}")
    string(REPLACE "\"" "\\\"" _subject "${_subject}")
    set(GIT_COMMIT_SUBJECT "${_subject}")

    execute_process(COMMAND "${GIT_EXECUTABLE}" branch --show-current
            WORKING_DIRECTORY "${VERSION_REPO_DIR}"
            OUTPUT_VARIABLE _branch ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (_branch)
        set(GIT_BRANCH "${_branch}")
    else ()
        set(GIT_BRANCH "detached")
    endif ()
endif ()

set(_dirty_suffix "")
if (GIT_DIRTY_BOOL STREQUAL "true")
    set(_dirty_suffix "-dirty")
endif ()

set(RML_VERSION_STRING "${GIT_SHORT_SHA1}${_dirty_suffix} (${GIT_BRANCH}) ${GIT_DATE}")

get_filename_component(_out_dir "${VERSION_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_out_dir}")
configure_file("${VERSION_INPUT}" "${VERSION_OUTPUT}.tmp" @ONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${VERSION_OUTPUT}.tmp" "${VERSION_OUTPUT}")
file(REMOVE "${VERSION_OUTPUT}.tmp")
