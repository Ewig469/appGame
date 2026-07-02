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
# Preset library (bundled)
# -------------------------------------------------------------------
add_subdirectory(lib/preset)

# -------------------------------------------------------------------
# Raylib 6.0
# -------------------------------------------------------------------
include(FetchContent)

set(RAYLIB_VERSION 6.0)

find_package(raylib ${RAYLIB_VERSION} QUIET)

if(NOT raylib_FOUND)
    message(STATUS "raylib ${RAYLIB_VERSION} nicht gefunden, lade raylib automatisch herunter")

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
    )

    FetchContent_MakeAvailable(raylib)
endif()

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
