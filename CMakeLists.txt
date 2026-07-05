cmake_minimum_required(VERSION 3.24)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(PROJECT_NAME "bruecken_spiel")

project(${PROJECT_NAME}
    VERSION 0.1.0
    DESCRIPTION "Brücken - strategisches Brettspiel (APP SoSe 2026)"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------------------------------------------
# Preset library bundled
# -------------------------------------------------------------------
add_subdirectory(lib/preset)

# -------------------------------------------------------------------
# Raylib 6.0 bundled library
# -------------------------------------------------------------------
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/raylib/raylib-6.0)

# -------------------------------------------------------------------
# Project executable
# -------------------------------------------------------------------

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    include
    lib/preset/include
)

file(GLOB_RECURSE PROJECT_SOURCES
    "src/*.cpp"
)

target_sources(${PROJECT_NAME}
  PRIVATE
    ${PROJECT_SOURCES}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    preset
    raylib
)











# ==========================================================
# Player Tests
# ==========================================================

add_executable(test_player
    tests/test_player.cpp
    src/board.cpp
    src/human_player.cpp
    src/random_ai_player.cpp
)

target_include_directories(test_player
    PRIVATE
        include
)

target_link_libraries(test_player
    PRIVATE
        preset
)




add_executable(test_invalid_move
    tests/test_invalid_move.cpp
    src/board.cpp
    src/random_ai_player.cpp
)

target_include_directories(test_invalid_move
    PRIVATE
        include
)

target_link_libraries(test_invalid_move
    PRIVATE
        preset
)

add_executable(test_gui_logic
    tests/test_gui_logic.cpp
    src/board.cpp
    src/gui_logic.cpp
)

target_include_directories(test_gui_logic
    PRIVATE
        include
)

target_link_libraries(test_gui_logic
    PRIVATE
        preset
)

# Keep MinGW executables independent of whichever GCC runtime DLLs are found
# first on Windows. This does not affect Linux/CIP builds.
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static)
    target_link_options(test_player PRIVATE -static)
    target_link_options(test_invalid_move PRIVATE -static)
    target_link_options(test_gui_logic PRIVATE -static)
endif()

enable_testing()
add_test(NAME player_contract COMMAND test_player)
add_test(NAME invalid_move_contract COMMAND test_invalid_move)
add_test(NAME gui_logic_contract COMMAND test_gui_logic)
