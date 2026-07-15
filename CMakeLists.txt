cmake_minimum_required(VERSION 3.25)

project(knight_bridge
    VERSION 0.1.0
    DESCRIPTION "Knight Bridge - strategic board game (APP SoSe 2026)"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(enable_bruecken_warnings target_name)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
        )
    endif()
endfunction()

function(configure_prebuilt_raylib)
    set(RAYLIB_VERSION "6.0")
    set(RAYLIB_PACKAGE_DIR
        "${CMAKE_BINARY_DIR}/_deps/raylib-${RAYLIB_VERSION}"
        CACHE PATH "Directory where the prebuilt raylib package is downloaded"
    )

    if(WIN32)
        if(MINGW)
            set(RAYLIB_PACKAGE_URL
                "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_win64_mingw-w64.zip"
            )
            set(RAYLIB_LIBRARY "${RAYLIB_PACKAGE_DIR}/lib/libraylib.a")
        else()
            set(RAYLIB_PACKAGE_URL
                "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_win64_msvc16.zip"
            )
            set(RAYLIB_LIBRARY "${RAYLIB_PACKAGE_DIR}/lib/raylib.lib")
        endif()
    elseif(APPLE)
        set(RAYLIB_PACKAGE_URL
            "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_macos.tar.gz"
        )
        set(RAYLIB_LIBRARY "${RAYLIB_PACKAGE_DIR}/lib/libraylib.a")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" BRUECKEN_SYSTEM_PROCESSOR)
        if(NOT BRUECKEN_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
            message(FATAL_ERROR
                "Prebuilt raylib ${RAYLIB_VERSION} is configured for Linux amd64. "
                "Use -DBRUECKEN_RAYLIB_PROVIDER=bundled or system on this machine."
            )
        endif()

        set(RAYLIB_PACKAGE_URL
            "https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_linux_amd64.tar.gz"
        )
        set(RAYLIB_LIBRARY "${RAYLIB_PACKAGE_DIR}/lib/libraylib.a")
    else()
        message(FATAL_ERROR
            "Unsupported platform for prebuilt raylib. "
            "Use -DBRUECKEN_RAYLIB_PROVIDER=bundled or system."
        )
    endif()

    set(RAYLIB_INCLUDE_DIR "${RAYLIB_PACKAGE_DIR}/include")

    if(EXISTS "${RAYLIB_INCLUDE_DIR}/raylib.h" AND EXISTS "${RAYLIB_LIBRARY}")
        message(STATUS "Using local prebuilt raylib ${RAYLIB_VERSION}: ${RAYLIB_PACKAGE_DIR}")
    elseif(EXISTS "${RAYLIB_PACKAGE_DIR}")
        message(FATAL_ERROR
            "RAYLIB_PACKAGE_DIR exists but does not look like a raylib prebuilt package: "
            "${RAYLIB_PACKAGE_DIR}\n"
            "Expected files:\n"
            "  ${RAYLIB_INCLUDE_DIR}/raylib.h\n"
            "  ${RAYLIB_LIBRARY}"
        )
    else()
        message(STATUS "Downloading prebuilt raylib ${RAYLIB_VERSION}: ${RAYLIB_PACKAGE_URL}")

        get_filename_component(RAYLIB_PACKAGE_ARCHIVE
            "${RAYLIB_PACKAGE_URL}"
            NAME
        )
        set(RAYLIB_PACKAGE_ARCHIVE
            "${CMAKE_BINARY_DIR}/_deps/${RAYLIB_PACKAGE_ARCHIVE}"
        )

        file(DOWNLOAD
            "${RAYLIB_PACKAGE_URL}"
            "${RAYLIB_PACKAGE_ARCHIVE}"
            STATUS RAYLIB_DOWNLOAD_STATUS
            SHOW_PROGRESS
        )
        list(GET RAYLIB_DOWNLOAD_STATUS 0 RAYLIB_DOWNLOAD_CODE)
        list(GET RAYLIB_DOWNLOAD_STATUS 1 RAYLIB_DOWNLOAD_MESSAGE)
        if(NOT RAYLIB_DOWNLOAD_CODE EQUAL 0)
            message(FATAL_ERROR
                "Failed to download raylib prebuilt package: "
                "${RAYLIB_DOWNLOAD_MESSAGE}"
            )
        endif()

        file(MAKE_DIRECTORY "${RAYLIB_PACKAGE_DIR}")
        file(ARCHIVE_EXTRACT
            INPUT "${RAYLIB_PACKAGE_ARCHIVE}"
            DESTINATION "${RAYLIB_PACKAGE_DIR}"
        )
    endif()

    if(NOT EXISTS "${RAYLIB_INCLUDE_DIR}/raylib.h" OR NOT EXISTS "${RAYLIB_LIBRARY}")
        message(FATAL_ERROR
            "raylib prebuilt package is incomplete after setup.\n"
            "Expected files:\n"
            "  ${RAYLIB_INCLUDE_DIR}/raylib.h\n"
            "  ${RAYLIB_LIBRARY}"
        )
    endif()

    add_library(raylib UNKNOWN IMPORTED GLOBAL)
    set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
    )

    if(WIN32)
        target_link_libraries(raylib INTERFACE winmm gdi32 opengl32)
    elseif(APPLE)
        find_library(COCOA_LIBRARY Cocoa REQUIRED)
        find_library(IOKIT_LIBRARY IOKit REQUIRED)
        find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
        find_library(OPENGL_LIBRARY OpenGL REQUIRED)
        target_link_libraries(raylib INTERFACE
            "${COCOA_LIBRARY}"
            "${IOKIT_LIBRARY}"
            "${COREVIDEO_LIBRARY}"
            "${OPENGL_LIBRARY}"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(OpenGL REQUIRED)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
        target_link_libraries(raylib INTERFACE
            OpenGL::GL
            X11::X11
            Threads::Threads
            ${CMAKE_DL_LIBS}
            m
            rt
        )
    endif()
endfunction()

# -------------------------------------------------------------------
# Libraries
# -------------------------------------------------------------------
add_subdirectory(lib/preset)

set(BRUECKEN_RAYLIB_PROVIDER "prebuilt" CACHE STRING
    "raylib provider: prebuilt, bundled, or system"
)
set_property(CACHE BRUECKEN_RAYLIB_PROVIDER PROPERTY STRINGS
    prebuilt
    bundled
    system
)

if(BRUECKEN_RAYLIB_PROVIDER STREQUAL "prebuilt")
    configure_prebuilt_raylib()
elseif(BRUECKEN_RAYLIB_PROVIDER STREQUAL "bundled")
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(lib/raylib/raylib-6.0)
elseif(BRUECKEN_RAYLIB_PROVIDER STREQUAL "system")
    find_package(raylib 6.0 REQUIRED)
    if(NOT TARGET raylib AND TARGET raylib::raylib)
        add_library(raylib ALIAS raylib::raylib)
    endif()
    if(NOT TARGET raylib)
        message(FATAL_ERROR "raylib package was found, but no raylib target is available")
    endif()
else()
    message(FATAL_ERROR
        "Unknown BRUECKEN_RAYLIB_PROVIDER='${BRUECKEN_RAYLIB_PROVIDER}'. "
        "Use prebuilt, bundled, or system."
    )
endif()

# -------------------------------------------------------------------
# Project library shared by executable and tests
# -------------------------------------------------------------------
add_library(bruecken_core
    src/board.cpp
    src/gui_logic.cpp
    src/human_player.cpp
    src/random_ai_player.cpp
)

target_include_directories(bruecken_core
    PUBLIC
        include
)

target_compile_features(bruecken_core PUBLIC cxx_std_20)

target_link_libraries(bruecken_core
    PUBLIC
        preset
)

enable_bruecken_warnings(bruecken_core)

# -------------------------------------------------------------------
# Game executable
# -------------------------------------------------------------------
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/gui_renderer.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        bruecken_core
        raylib
)

enable_bruecken_warnings(${PROJECT_NAME})

# -------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------
enable_testing()

add_executable(board_tests
    tests/board_tests.cpp
)
target_link_libraries(board_tests PRIVATE bruecken_core)
enable_bruecken_warnings(board_tests)

add_executable(test_player
    tests/test_player.cpp
)
target_link_libraries(test_player PRIVATE bruecken_core)
enable_bruecken_warnings(test_player)

add_executable(test_gui_logic
    tests/test_gui_logic.cpp
)
target_link_libraries(test_gui_logic PRIVATE bruecken_core)
enable_bruecken_warnings(test_gui_logic)

add_test(NAME board_rules COMMAND board_tests)
add_test(NAME player_contract COMMAND test_player)
add_test(NAME gui_logic_contract COMMAND test_gui_logic)

# Keep MinGW executables independent of whichever GCC runtime DLLs are found
# first on Windows. This does not affect Linux/CIP builds.
if(MINGW)
    foreach(target_name
            ${PROJECT_NAME}
            board_tests
            test_player
            test_gui_logic)
        target_link_options(${target_name} PRIVATE -static)
    endforeach()
endif()
