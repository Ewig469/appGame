cmake_minimum_required(VERSION 3.25)

project(bruecken_spiel
    VERSION 0.1.0
    DESCRIPTION "Bruecken - strategisches Brettspiel (APP SoSe 2026)"
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

# -------------------------------------------------------------------
# Bundled libraries
# -------------------------------------------------------------------
add_subdirectory(lib/preset)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/raylib/raylib-6.0)

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
