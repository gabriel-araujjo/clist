cmake_minimum_required(VERSION 3.12)

# Choose a convenient CMAKE_BUILD_TYPE ----------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(default_build_type "Debug")
    else()
        set(default_build_type "RelWithDebInfo")
    endif()

    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING
        "Default BUILD_TYPE is ${default_build_type}" FORCE)
endif()

# Read clist.h file and get project version from it ---------------------------
set(CLIST_HEADER "clist.h")

file(STRINGS "${CLIST_HEADER}" CLIST_HEADER_CONTENT
    REGEX "^#define CLIST_VERSION_(MAJOR|MINOR|PATCH) +[0-9]+$"
)

set(CLIST_HEADER_VERSION_MAJOR 0)
set(CLIST_HEADER_VERSION_MINOR 0)
set(CLIST_HEADER_VERSION_PATCH 0)

foreach(VERSION_LINE IN LISTS CLIST_HEADER_CONTENT)
    foreach(VERSION_PART MAJOR MINOR PATCH)
        string(CONCAT REGEX_STRING "#define UNITY_VERSION_"
            "${VERSION_PART}"
            " +([0-9]+)"
        )

        if(VERSION_LINE MATCHES "${REGEX_STRING}")
            set(CLIST_HEADER_VERSION_${VERSION_PART} "${CMAKE_MATCH_1}")
        endif()
    endforeach()
endforeach()

# Define project --------------------------------------------------------------
project(clist
    VERSION "${CLIST_HEADER_VERSION_MAJOR}.${CLIST_HEADER_VERSION_MINOR}.${CLIST_HEADER_VERSION_PATCH}"
    LANGUAGES C
    DESCRIPTION "Sinple linked list implemented in C"
)

# Options ---------------------------------------------------------------------
include(CMakeDependentOption)
cmake_dependent_option(WITH_TESTS "build with tests" ON
    "CMAKE_BUILD_TYPE STREQUAL Debug" OFF)

# Main target -----------------------------------------------------------------
add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
    clist.c
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

set(${PROJECT_NAME}_PUBLIC_HEADERS
    clist.h
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    PUBLIC_HEADER "${${PROJECT_NAME}_PUBLIC_HEADERS}"
)

# Tests -----------------------------------------------------------------------
if(WITH_TESTS)
    include(FetchContent)
    FetchContent_Declare(unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG 0b899aec14d3a9abb2bf260ac355f0f28630a6a3 # v2.5.2
    )

    FetchContent_MakeAvailable(unity)

    add_executable(${PROJECT_NAME}_test)

    target_sources(${PROJECT_NAME}_test
        PRIVATE
        clist_test.c
    )

    target_link_libraries(${PROJECT_NAME}_test
        PRIVATE
            ${PROJECT_NAME}
            unity
    )
endif()
