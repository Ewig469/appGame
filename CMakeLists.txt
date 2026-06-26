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
# Auf CIP-Pool (Ubuntu): systemweit installiert → find_package greift.
# Auf Windows/Mac: lokale Kopie unter lib/raylib/ wird verwendet.
find_package(raylib 6.0 QUIET)

if(NOT raylib_FOUND)
    message(STATUS "raylib nicht via find_package gefunden, verwende lokale Kopie")

    set(RAYLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib")
    set(RAYLIB_INCLUDE "${RAYLIB_ROOT}/include")
    if(WIN32)
        set(RAYLIB_LIB "${RAYLIB_ROOT}/lib/libraylib.a")
    else()
        set(RAYLIB_LIB "${RAYLIB_ROOT}/lib/libraylib.a")
    endif()

    add_library(raylib STATIC IMPORTED GLOBAL)
    target_include_directories(raylib INTERFACE "${RAYLIB_INCLUDE}")
    set_target_properties(raylib PROPERTIES IMPORTED_LOCATION "${RAYLIB_LIB}")

    if(WIN32)
        target_link_libraries(raylib INTERFACE winmm gdi32 opengl32)
    endif()
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
